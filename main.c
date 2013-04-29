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

#include "assert_np.h"
#include "hemul.h"


#ifndef DEF_PERIOD
#define DEF_PERIOD -1
#endif

#define xstr(S) str(S)
#define str(S) #S

const char *program_version =
"hemul 0.01";

#define HELP_USAGE     1
#define HELP_LONG      2
#define HELP_VERSION   4
#define HELP_TRY       8
#define HELP_EXIT     16
#define HELP_EXIT_ERR 32

void help(FILE* file, int flags) {
	if (file && flags & HELP_USAGE) {
		fprintf(file, "%s",
			"Usage: hemul [-DlvhuV] [-d level] [-p ptime]\n"
			"            [--debuglevel=level] [--documentation]\n"
			"            [--period=ptime] [--outfile=file-name] [--verbose]\n"
			"            [--help] [--usage] [--version]\n");
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
			"  -p, --period=ptime         Period-time in uS. -1 equals event-driven output.\n"
			"                             Default period-time is [100000] uS\n"
			"  -P, --pipe                 Output is a named pipe.\n"
			"                             Created if missing, requires -o\n"
			"  -o, --outfile=file-name    Output is sent to this file-name.\n"
			"  -i, --infile=file-name     Input is received from this file-name.\n"
			"  -v, --verbose              Produce verbose output. This flag lets you see how\n"
			"  -R, --regex=string         Regex string to identify time-stamp\n"
			"  -r, --regi=idx             Sub expression to use as time-stamp\n"
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
	struct arguments *arguments
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
			arguments->verbose = 1;
			mtimemod_settings.verbose = 1;
			break;
		case 'V':
			help(stdout, HELP_VERSION | HELP_EXIT);
			break;
		case 'P':
			arguments->piped_output = 1;
			break;

		case 'd':
			arguments->debuglevel = arg ? atoi (arg) : 0;
			mtimemod_settings.debuglevel = arguments->debuglevel;
			break;
		case 'p':
			arguments->ptime = arg ? atoi (arg) : -1;
			break;
		case 'o':
			arguments->ofilename = arg;
			break;
		case 'i':
			arguments->ifilename = arg;
			break;
		case 'R':
			arguments->ts_regex.str = arg;
			break;
		case 'r':
			arguments->ts_regex.idx = arg ? atoi (arg) : -1;
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
	{"cersion",       no_argument,       0, 'V'},
	{"debuglevel",    required_argument, 0, 'd'},
	{"period",        required_argument, 0, 'p'},
	{"pipe",          no_argument,       0, 'P'},
	{"outfile",       required_argument, 0, 'o'},
	{"infile",        required_argument, 0, 'i'},
	{"regex",         required_argument, 0, 'R'},
	{"regi",          required_argument, 0, 'r'},
	{"documentation", no_argument,       0, 'D'},
	{"usage",         no_argument,       0, 'u'},
	{"help",          no_argument,       0, 'h'},
	{"version",       no_argument,       0, 'V'},
	{0, 0, 0, 0}
};

struct arguments arguments = {
	.ptime          = DEF_PERIOD,
	.debuglevel     = 0,
	.piped_output   = 0,
	.ofilename      = NULL,
	.ifilename      = NULL,
	.ts_format      = NULL,
	.ts_regex       = {
		.str        = NULL,
		.idx        = 1,
	},
};

int main(int argc, char **argv) {
	while (1) {
		int option_index = 0;
		int c = getopt_long(argc, argv, "DuhvVPd:p:o:i:R:r:",
			long_options, &option_index);
		/* Detect the end of the options. */
		if (c == -1)
			break;
		parse_opt(c, optarg, &arguments);
	}

	/* Handle any remaining command line arguments (not options). */
	if (optind < argc) {
		perror("hemul: to many parameters\n");
		fflush(stderr);
		help(stderr, HELP_TRY | HELP_EXIT_ERR);
	}

	assert_ext(hemul_init()==0);
	assert_ext(hemul_run()==0);
	assert_ext(hemul_fini()==0);
	return 0;
}

