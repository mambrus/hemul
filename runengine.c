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
#include <regex.h>
#include <mqueue.h>

/* More than 3 should really not be needed, adding a few just in case */
#define MAX_SUBEXP 5


/* This thread takes what it's got in buffer when buf-len is reached. */
void *output_pumper(void* inarg) {
	mqd_t q;
	int s,rc;
	char msg_buf[1024];


	assert_ext((q = mq_open( Q_OUTPUT_NAME , O_RDONLY, NULL, 0 )) > 0);
	while (1) {

		rc = mq_receive(q, msg_buf, 1024/*MYMSGSIZE*/, NULL);
		fputs(stdout,"hello\n");

	}
}
