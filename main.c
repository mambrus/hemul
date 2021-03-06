/***************************************************************************
 *   Copyright (C) 2013 by Michael Ambrus                                  *
 *   ambrmi09@gmail.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <getopt.h>
#include <mtime.h>
#include <hemul.h>

#include "assert_np.h"
/*Get rid of this if HEMUL is made to module or lib*/
#include "local.h"

#ifndef DEF_PERIOD
#define DEF_PERIOD -1
#endif
#ifndef DEF_SUBEXPR
#define DEF_SUBEXPR 1
#endif
#ifndef DEF_BUFFER_TIMEOUT
//#define DEF_BUFFER_TIMEOUT 1000000ul
#define DEF_BUFFER_TIMEOUT -1l
#else
#define BUFFER_TIMEOUT_CC_DEFINED
#endif

#define xstr(S) str(S)
#define str(S) #S

const char *program_version =
"hemul 0.10";

#define HELP_USAGE     1
#define HELP_LONG      2
#define HELP_VERSION   4
#define HELP_TRY       8
#define HELP_EXIT     16
#define HELP_EXIT_ERR 32

void help(FILE* file, int flags) {
	if (file && flags & HELP_USAGE) {
		fprintf(file, "%s",
			"Usage: hemul [-DlvhuVE] [-d level] [-p ptime]\n"
			"            [--debuglevel=level] [--documentation]\n"
			"            [--period=ptime] [--outfile=file-name]\n"
			"            [--pipe] [--echo]\n"
			"            [--verbose] [--help] [--usage] [--version]\n");
		fflush(file);
	}
	if (file && flags & HELP_LONG) {
		fprintf(file, "%s",
			"Usage: hemul [OPTION...] \n"
			"hemul -- A log-replay emulator\n"
			"Replays files at certain intervals\n"
			"\n"
			"  -d, --debuglevel=level     Debug-level\n"
			"  -D, --documentation        Output full documentation, then exit\n"
			"  -p, --period=ptime         Period-time in uS. A negative value means purely\n"
			"                             event-driven output.\n"
			"                             Default period-time is ["xstr(DEF_PERIOD)"] uS\n"
			"  -P, --pipe                 Output is a named pipe.\n"
			"                             Created if missing, requires -o\n"
			"  -E, --echo                 Also echo to std-out. Usefull when outdata goes\n"
			"                             to something unvisible (file, pipe or socket \n"
			"                             e.t.a.). Recommended when debugging the consumer\n"
			"  -n, --linenum=sepstr       Each output is perpended with line-number.\n"
			"                             String <sepstr> is the separator to the rest\n"
			"                             of the line.\n"
			"  -o, --outfile=file-name    Output is sent to this file-name.\n"
			"  -i, --infile=file-name     Input is received from this file-name.\n"
			"  -v, --verbose              Produce verbose output. This flag lets you see how\n"
			"  -R, --regex=string         Regex string to identify time-stamp\n"
			"  -r, --regi=idx             Sub expression to use as time-stamp\n"
			"                             Defaults to ["xstr(DEF_SUBEXPR)"]\n"
			"  -B, --buffered=size        Emulate buffered output of size <size>\n"
			"  -b, --buffered=timeout     Emulated buffered output with timeout of <timeout>\n"
			"                             Defaults to ["xstr(DEF_BUFFER_TIMEOUT)"] uS\n"
			"  -F, --format=string        Format string describing the time-stamp. Follows \n"
			"                             same convention as format for the 'date' command\n"
			"  -h, --help                 Give this help list\n"
			"  -u, --usage                Give a short usage message\n"
			"  -V, --version              Print program version\n"
			"\n"
			"Mandatory or optional arguments to long options are also mandatory or optional\n"
			"for any corresponding short options.\n"
			"\n"
			"Read the manual by passing the -D option\n"
			"\n"
			"Report bugs to <ambrmi09@gmail.com>.\n");
		fflush(file);
	}

	if (file && flags & HELP_VERSION) {
		fprintf(file, "%s\n", program_version);
		fflush(file);
	}

	if (file && flags & HELP_TRY) {
		fprintf(file, "%s",
			"Try `hemul --help' or `hemul --usage' for more information.\n");
		fflush(file);
	}

	if (file && flags & HELP_EXIT)
		exit(0);

	if (file && flags & HELP_EXIT_ERR)
		exit(1);
}

extern struct mtimemod_settings mtimemod_settings;

/* Parse a single option. */
static void parse_opt(
	int key,
	char *arg,
	struct hemul_args *hemul_args
){
	extern const char hemul_doc[];

	switch (key) {
		case 'D':
			printf("%s\n",hemul_doc);
			exit(0);
			break;
		case 'u':
			help(stdout, HELP_USAGE | HELP_EXIT);
			break;
		case 'h':
			help(stdout, HELP_LONG | HELP_EXIT);
			break;
		case 'v':
			hemul_args->verbose = 1;
			mtimemod_settings.verbose = 1;
			break;
		case 'V':
			help(stdout, HELP_VERSION | HELP_EXIT);
			break;
		case 'P':
			hemul_args->piped_output = 1;
			break;
		case 'E':
			hemul_args->echo = 1;
			break;

		case 'd':
			hemul_args->debuglevel = arg ? atoi (arg) : 0;
			mtimemod_settings.debuglevel = hemul_args->debuglevel;
			break;
		case 'n':
			hemul_args->linenumb = arg;
			break;
		case 'p':
			hemul_args->ptime = arg ? atoi (arg) : -1;
			break;
		case 'o':
			hemul_args->ofilename = arg;
			break;
		case 'i':
			hemul_args->ifilename = arg;
			break;
		case 'R':
			hemul_args->ts_regex.str = arg;
			break;
		case 'r':
			hemul_args->ts_regex.idx = arg ? atoi (arg) : -1;
			break;
		case 'B':
			hemul_args->buffer_size = arg ? atoi (arg) : -1;
			break;
		case 'b':
			hemul_args->buffer_timeout = arg ? atoi (arg) : -1;
			break;
		case 'F':
			hemul_args->ts_format = arg;
			break;


		case '?':
			/* getopt_long already printed an error message. */
			help(stderr, HELP_TRY | HELP_EXIT_ERR);
			break;
		default:
			fprintf(stderr, "hemul: unrecognized option '-%c'\n", (char)key);
			help(stderr, HELP_TRY | HELP_EXIT_ERR);
			break;
		}
}

static struct option long_options[] = {
	{"verbose",       no_argument,       0, 'v'},
	{"version",       no_argument,       0, 'V'},
	{"linenum",       required_argument, 0, 'n'},
	{"debuglevel",    required_argument, 0, 'd'},
	{"period",        required_argument, 0, 'p'},
	{"pipe",          no_argument,       0, 'P'},
	{"echo",          no_argument,       0, 'E'},
	{"outfile",       required_argument, 0, 'o'},
	{"infile",        required_argument, 0, 'i'},
	{"regex",         required_argument, 0, 'R'},
	{"regi",          required_argument, 0, 'r'},
	{"buffsize",      required_argument, 0, 'B'},
	{"bufftime",      required_argument, 0, 'b'},
	{"format",        required_argument, 0, 'F'},
	{"documentation", no_argument,       0, 'D'},
	{"usage",         no_argument,       0, 'u'},
	{"help",          no_argument,       0, 'h'},
	{"version",       no_argument,       0, 'V'},
	{0, 0, 0, 0}
};

struct hemul_args hemul_args = {
	.ptime            = DEF_PERIOD,
	.debuglevel       = 0,
	.linenumb         = NULL,
	.piped_output     = 0,
	.ofilename        = NULL,
	.ifilename        = NULL,
	.buffer_size      = -1,
	.buffer_timeout   = DEF_BUFFER_TIMEOUT,
	.ts_regex         = {
		.str          = NULL,
		.idx          = DEF_SUBEXPR,
	},
	.ts_format        = NULL,
	.echo             = 0,
};

int opt_errno = 0;
#define OPT_CHECK( E )                                                   \
	if ( (E) ) {                                                         \
		opt_errno = EINVAL;                                              \
		fprintf(stderr,"hemul: options combination impossible or "       \
			"does not make sense:\n");                                   \
		fprintf(stderr," failed check--> "str(E)"\n");                   \
	}

int main(int argc, char **argv) {
	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "DuhvVPd:p:o:i:R:r:B:b:F:n:E",
			long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		parse_opt(c, optarg, &hemul_args);
	}

	INFO(("\nhemul arguments:\n"));
	INFO((  "================\n"));
	INFO(("Period time:                       %d uS\n",
		hemul_args.ptime));
	INFO(("Debug level:                       %d\n",
		hemul_args.debuglevel));
	INFO(("Piped output:                      %d\n",
		hemul_args.piped_output));
	INFO(("Outfile-name:                      %s\n",
		hemul_args.ofilename));
	INFO(("Infile-name:                       %s\n",
		hemul_args.ifilename));
	INFO(("Emulated buffered size:            %d\n",
		hemul_args.buffer_size));
	INFO(("Emulated buffered output time-out: %d uS\n",
		hemul_args.buffer_timeout));
	INFO(("In-line time-stamp regex:         %s\n",
		hemul_args.ts_regex.str));
	INFO(("In-line time-stamp regex index:   %d\n",
		hemul_args.ts_regex.idx));
	INFO(("In-line time-stamp format:         %s\n",
		hemul_args.ts_format));
	INFO(("Stdout echo:         %s\n",
		hemul_args.echo));
	INFO(("\n"));

	OPT_CHECK(hemul_args.ts_format && !hemul_args.ts_regex.str);
#if defined(BUFFER_TIMEOUT_CC_DEFINED)
	OPT_CHECK(hemul_args.buffer_timeout>0 && hemul_args.buffer_size<0);
#endif
	OPT_CHECK(hemul_args.piped_output && !hemul_args.ofilename);
	OPT_CHECK(hemul_args.ts_regex.str && (hemul_args.ptime>0));

	if (opt_errno){
		errno = opt_errno;
		perror("hemul: option check");
		fflush(stderr);
		help(stderr, HELP_TRY | HELP_EXIT_ERR);
	}
	/* Handle any remaining command line arguments (not options). */
	if (optind < argc) {
		errno = EINVAL;
		perror("hemul: to many parameters");
		fflush(stderr);
		help(stderr, HELP_TRY | HELP_EXIT_ERR);
	}

	assert_ext(hemul_init()==0);
	assert_ext(hemul_run()==0);
	assert_ext(hemul_fini()==0);
	get_exit()(0);
	return 0;
}

