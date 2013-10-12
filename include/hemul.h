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

/* Public interface for HEMUL */

#ifndef hemul_h
#define hemul_h
#include <pthread.h>
#include <regex.h>
#include <stdint.h>

struct ts_regex {
	char *str;			/* The string originally describing the regex. Note:
						   Might be invalid data depending on usage (as it's
						   used only once to compile the regex below). */
	regex_t rgx;		/* Compiled version of the regex. This data must
						   always be reliable */
	int idx;			/* Which subexpression contain the time-stamp */
};

/* General hemul_args */
struct hemul_args
{
	int verbose;
	int ptime;
	int debuglevel;
	int piped_output;
	int echo;
	char *ofilename;
	char *ifilename;
	struct ts_regex ts_regex;
	int buffer_size;
	int buffer_timeout;
	char *linenumb;
	char *ts_format; /* Time Format parse-able by *strptime*/
};
extern struct hemul_args hemul_args;

int hemul_init( void );
int hemul_fini( void );
int hemul_run( void );

#endif /* hemul_h */

