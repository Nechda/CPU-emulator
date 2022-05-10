#include "CPU.h"

#include <argp.h>
#include <cstdlib>

const char *argp_program_version = "cpu-emulator";
const char *argp_program_bug_address = "<...>";
static char doc[] = "Simple cpu emulator";
static char args_doc[] = "input-file";
enum { RAM_SIZE_KEY = 256, ENTRYPOINT, STEP_BY_STEP };
static struct argp_option options[] = {{"output", 'o', "FILE", 0, "Output to FILE instead of standard output"},
                                       {"ram-size", RAM_SIZE_KEY, "N_BYTES", 0, "Set size of CPU's RAM"},
                                       {"entrypoint", ENTRYPOINT, "pc", 0, "Set program counter"},
                                       {"step-by-step", STEP_BY_STEP, 0, 0, "Set step by step mode"},
                                       {0}};

static error_t parse_opt(int key, char *arg, argp_state *state) {
    Arguments *arguments = (Arguments *)state->input;
    switch (key) {
    case 'o':
        arguments->output_file = arg;
        break;
    case RAM_SIZE_KEY:
        arguments->ram_size = std::atoi(arg);
        break;
    case STEP_BY_STEP:
        arguments->step_by_step = true;
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num >= 1)
            argp_usage(state);
        arguments->input_file = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}
static struct argp argp = {options, parse_opt, args_doc, doc};

Arguments parse(int argc, char **argv) {
    struct Arguments arguments;
    arguments.output_file = "a.out";
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    return arguments;
}