/*
 * builtin-inject.c
 *
 * Builtin inject command: Examine the live mode (stdin) event stream
 * and repipe it to stdout while optionally injecting additional
 * events into it.
 */
#include "builtin.h"

#include "perf.h"
#include "util/session.h"
#include "util/tool.h"
#include "util/debug.h"
#include "util/trace-event.h"

#include "util/parse-options.h"

static char		const *input_name = "-";
static const char	*output_name		= "-";
static int		pipe_output		= 0;
static int		output;
static u64		bytes_written		= 0;

static bool		inject_build_ids;
static bool		inject_sched_stat;
static struct perf_session *session;

static int perf_event__repipe_synth(struct perf_tool *tool __used,
				    union perf_event *event,
				    struct machine *machine __used)
{
	uint32_t size;
	void *buf = event;

	size = event->header.size;

	while (size) {
		int ret = write(output, buf, size);
		if (ret < 0)
			return -errno;

		size -= ret;
		buf += ret;

		bytes_written += ret;
	}

	return 0;
}

static int perf_event__repipe_op2_synth(struct perf_tool *tool,
					union perf_event *event,
					struct perf_session *s __used)
{
	return perf_event__repipe_synth(tool, event, NULL);
}

static int perf_event__repipe_event_type_synth(struct perf_tool *tool,
					       union perf_event *event)
{
	return perf_event__repipe_synth(tool, event, NULL);
}

static int perf_event__repipe_tracing_data_synth(union perf_event *event,
						 struct perf_session *s __used)
{
	return perf_event__repipe_synth(NULL, event, NULL);
}

static int perf_event__repipe_attr(union perf_event *event,
				   struct perf_evlist **pevlist __used)
{
	return perf_event__repipe_synth(NULL, event, NULL);
}

static int perf_event__repipe(struct perf_tool *tool,
			      union perf_event *event,
			      struct perf_sample *sample __used,
			      struct machine *machine)
{
	return perf_event__repipe_synth(tool, event, machine);
}

static int perf_event__repipe_sample(struct perf_tool *tool,
				     union perf_event *event,
			      struct perf_sample *sample __used,
			      struct perf_evsel *evsel __used,
			      struct machine *machine)
{
	return perf_event__repipe_synth(tool, event, machine);
}

static int perf_event__repipe_mmap(struct perf_tool *tool,
				   union perf_event *event,
				   struct perf_sample *sample,
				   struct machine *machine)
{
	int err;

	err = perf_event__process_mmap(tool, event, sample, machine);
	perf_event__repipe(tool, event, sample, machine);

	return err;
}

static int perf_event__repipe_task(struct perf_tool *tool,
				   union perf_event *event,
				   struct perf_sample *sample,
				   struct machine *machine)
{
	int err;

	err = perf_event__process_task(tool, event, sample, machine);
	perf_event__repipe(tool, event, sample, machine);

	return err;
}

static int perf_event__repipe_tracing_data(union perf_event *event,
					   struct perf_session *s)
{
	int err;

	perf_event__repipe_synth(NULL, event, NULL);
	err = perf_event__process_tracing_data(event, s);

	return err;
}

static int dso__read_build_id(struct dso *self)
{
	if (self->has_build_id)
		return 0;

	if (filename__read_build_id(self->long_name, self->build_id,
				    sizeof(self->build_id)) > 0) {
		self->has_build_id = true;
		return 0;
	}

	return -1;
}

static int dso__inject_build_id(struct dso *self, struct perf_tool *tool,
				struct machine *machine)
{
	u16 misc = PERF_RECORD_MISC_USER;
	int err;

	if (dso__read_build_id(self) < 0) {
		pr_debug("no build_id found for %s\n", self->long_name);
		return -1;
	}

	if (self->kernel)
		misc = PERF_RECORD_MISC_KERNEL;

	err = perf_event__synthesize_build_id(tool, self, misc, perf_event__repipe,
					      machine);
	if (err) {
		pr_err("Can't synthesize build_id event for %s\n", self->long_name);
		return -1;
	}

	return 0;
}

static int perf_event__inject_buildid(struct perf_tool *tool,
				      union perf_event *event,
				      struct perf_sample *sample,
				      struct perf_evsel *evsel __used,
				      struct machine *machine)
{
	struct addr_location al;
	struct thread *thread;
	u8 cpumode;

	cpumode = event->header.misc & PERF_RECORD_MISC_CPUMODE_MASK;

	thread = machine__findnew_thread(machine, event->ip.pid);
	if (thread == NULL) {
		pr_err("problem processing %d event, skipping it.\n",
		       event->header.type);
		goto repipe;
	}

	thread__find_addr_map(thread, machine, cpumode, MAP__FUNCTION,
			      event->ip.ip, &al);

	if (al.map != NULL) {
		if (!al.map->dso->hit) {
			al.map->dso->hit = 1;
			if (map__load(al.map, NULL) >= 0) {
				dso__inject_build_id(al.map->dso, tool, machine);
				/*
				 * If this fails, too bad, let the other side
				 * account this as unresolved.
				 */
			} else
				pr_warning("no symbols found in %s, maybe "
					   "install a debug package?\n",
					   al.map->dso->long_name);
		}
	}

repipe:
	perf_event__repipe(tool, event, sample, machine);
	return 0;
}

struct event_entry
{
	struct list_head list;
	u32 tid;
	union perf_event event[0];
};

static LIST_HEAD(samples);

static int perf_event__sched_stat(struct perf_tool *tool,
				      union perf_event *event,
				      struct perf_sample *sample,
				      struct perf_evsel *evsel __used,
				      struct machine *machine)
{
	int type;
	struct event *e;
	const char *evname = NULL;
	uint32_t size;
	struct event_entry *ent;
	union perf_event *event_sw = NULL;
	struct perf_sample sample_sw;
	int sched_process_exit;

	size = event->header.size;

	type = trace_parse_common_type(sample->raw_data);
	e = trace_find_event(type);
	if (e)
		evname = e->name;

	sched_process_exit = !strcmp(evname, "sched_process_exit");

	if (!strcmp(evname, "sched_switch") ||  sched_process_exit) {
		list_for_each_entry(ent, &samples, list)
			if (sample->pid == ent->tid)
				break;

		if (&ent->list != &samples) {
			list_del(&ent->list);
			free(ent);
		}

		if (sched_process_exit)
			return 0;

		ent = malloc(size + sizeof(struct event_entry));
		ent->tid = sample->tid;
		memcpy(&ent->event, event, size);
		list_add(&ent->list, &samples);
		return 0;

	} else if (!strncmp(evname, "sched_stat_", 11)) {
		u32 pid;

		pid = raw_field_value(e, "pid", sample->raw_data);

		list_for_each_entry(ent, &samples, list) {
			if (pid == ent->tid)
				break;
		}

		if (&ent->list == &samples) {
			pr_debug("Could not find sched_switch for pid %u\n", pid);
			return 0;
		}

		event_sw = &ent->event[0];
		perf_session__parse_sample(session, event_sw, &sample_sw);
		sample_sw.period = sample->period;
		sample_sw.time = sample->time;
		perf_session__synthesize_sample(session, event_sw, &sample_sw);
		perf_event__repipe(tool, event_sw, &sample_sw, machine);
		return 0;
	}

	perf_event__repipe(tool, event, sample, machine);

	return 0;
}

struct perf_tool perf_inject = {
	.sample		= perf_event__repipe_sample,
	.mmap		= perf_event__repipe,
	.comm		= perf_event__repipe,
	.fork		= perf_event__repipe,
	.exit		= perf_event__repipe,
	.lost		= perf_event__repipe,
	.read		= perf_event__repipe_sample,
	.throttle	= perf_event__repipe,
	.unthrottle	= perf_event__repipe,
	.attr		= perf_event__repipe_attr,
	.event_type	= perf_event__repipe_event_type_synth,
	.tracing_data	= perf_event__repipe_tracing_data_synth,
	.build_id	= perf_event__repipe_op2_synth,
};

extern volatile int session_done;

static void sig_handler(int sig __attribute__((__unused__)))
{
	session_done = 1;
}

static int __cmd_inject(void)
{
	int ret = -EINVAL;

	signal(SIGINT, sig_handler);

	if (inject_build_ids) {
		perf_inject.sample	 = perf_event__inject_buildid;
		perf_inject.mmap	 = perf_event__repipe_mmap;
		perf_inject.fork	 = perf_event__repipe_task;
		perf_inject.tracing_data = perf_event__repipe_tracing_data;
	} else if (inject_sched_stat) {
		perf_inject.sample	= perf_event__sched_stat;
		perf_inject.ordered_samples = true;
	}

	session = perf_session__new(input_name, O_RDONLY, false, true, &perf_inject);
	if (session == NULL)
		return -ENOMEM;

	if (!pipe_output)
		lseek(output, session->header.data_offset, SEEK_SET);

	ret = perf_session__process_events(session, &perf_inject);

	if (!pipe_output) {
		session->header.data_size = bytes_written;
		perf_session__write_header(session, session->evlist, output, true);
	}

	perf_session__delete(session);

	return ret;
}

static const char * const report_usage[] = {
	"perf inject [<options>]",
	NULL
};

static const struct option options[] = {
	OPT_BOOLEAN('b', "build-ids", &inject_build_ids,
		    "Inject build-ids into the output stream"),
	OPT_BOOLEAN('s', "sched-stat", &inject_sched_stat,
		    "correct call-chains for shed-stat-*"),
	OPT_STRING('i', "input", &input_name, "file",
		    "input file name"),
	OPT_STRING('o', "output", &output_name, "file",
		    "output file name"),
	OPT_INCR('v', "verbose", &verbose,
		 "be more verbose (show build ids, etc)"),
	OPT_END()
};

int cmd_inject(int argc, const char **argv, const char *prefix __used)
{
	argc = parse_options(argc, argv, options, report_usage, 0);

	/*
	 * Any (unrecognized) arguments left?
	 */
	if (argc)
		usage_with_options(report_usage, options);

	if (!strcmp(output_name, "-")) {
		pipe_output = 1;
		output = STDOUT_FILENO;
	} else {
		output = open(output_name, O_CREAT| O_WRONLY | O_TRUNC,
							S_IRUSR | S_IWUSR);
		if (output < 0) {
			perror("failed to create output file");
			exit(-1);
		}
	}

	if (symbol__init() < 0)
		return -1;

	return __cmd_inject();
}
