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

/* Local interface & stuff for HEMUL  */

#ifndef hemul_h
#define hemul_h
#include <pthread.h>
#include <regex.h>
#include <stdint.h>

#ifndef DBGLVL
#define DBGLVL 3
#endif

#define Q_OUTPUT_NAME "/tmp/q/out"

#define INFO_TO( F, FNKN, S ) \
	{ \
		if (arguments.verbose) { \
			FNKN S; \
			fflush( F ); \
		} \
	}

#if ( DBGLVL == 0 )
#  define INFO( S ) ((void)0)
#elif ( DBGLVL == 1 )
#  define INFO( S ) INFO_TO( stdout, printf, S )
#elif ( DBGLVL == 2 )
#  define INFO( S ) INFO_TO( stdout, printf, S )
#elif ( DBGLVL == 3 )
#  define eprintf(...) fprintf (stderr, __VA_ARGS__)
#  define INFO( S ) INFO_TO( stderr, eprintf, S )
#else
#  error bad value of DBGLVL
#endif

struct ts_regex {
	char *str;			/* The string originally describing the regex. Note:
						   Might be invalid data depending on usage (as it's
						   used only once to compile the regex below). */
	regex_t rgx;		/* Compiled version of the regex. This data must
						   always be reliable */
	int idx;			/* Which subexpression contain the time-stamp */
};

/* General arguments */
struct arguments
{
	int verbose;
	int ptime;        
	int debuglevel;
	int piped_output;
	char *ofilename;
	char *ifilename;
	char *ts_format; /* Time Format parse-able by *strptime*/
	struct ts_regex ts_regex;
};
extern struct arguments arguments;

/* Globals */
struct mod_hemul
{
	pthread_t tout;
	int pipe_created;
	struct ts_regex *ts_regex;
	FILE *in;
	FILE *out;
};
extern struct mod_hemul mod_hemul;

int hemul_init( void );
int hemul_fini( void );
int hemul_run( void );

#endif /* hemul_h */

