/* argparser.c
 * Functions used for basic parsing of the command line. It uses argp.h
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <argp.h>

#include "../include/global.h"
#include "../include/argparser.h"

const char *argp_program_version = ENGINE_NAME;
const char *argp_program_bug_address = "https://github.com/usr25/NoC/issues";

/* Program documentation. */
static char doc[] = "UCI compatible chess engine. Implements both Gaviota tablebases and NNUE. For more information, see the github https://github.com/usr25/NoC.git";

static struct argp_option options[] = {
  {"train", 't', "PATH", 0, "Path which specifies the values to use as part of the training (not in use)"},
  {"nnue", 'n', "PATH", 0, "Path which locates the NNUE" },
  {"gaviota", 'g', "PATH", 0, "Path which locates the Gaviota tablebases" },
  { 0 }
};

static char args_doc[] = "[ARG1 ...]";

static error_t parse_opt (int key, char *arg, struct argp_state *state)
{
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  Arguments *arguments = state->input;
  arguments->numArgs = state->arg_num;

  switch (key)
    {
    case 't':
      	arguments->train = arg;
      	break;
    case 'n':
      	arguments->nnue = arg;
      	break;
    case 'g':
    	arguments->gaviota = arg;

    case ARGP_KEY_ARG:
      if (state->arg_num >= 2)
        /* Too many arguments. */
        argp_usage (state);

      arguments->args[state->arg_num] = arg;

      break;

    case ARGP_KEY_END:
      if (state->arg_num < 0)
        /* Not enough arguments. */
        argp_usage (state);
      break;

    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}


static struct argp argp = { options, parse_opt, args_doc, doc };

Arguments parseArguments(int argc, char** argv)
{
	Arguments arguments;
	arguments.train = NULL;
	arguments.nnue = NULL;
	arguments.gaviota = NULL;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    return arguments;
}