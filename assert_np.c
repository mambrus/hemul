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


#include "assert_np.h"

#ifdef NDEBUG
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifndef ASSERT_ERROR_FILE
#define ASSERT_ERROR_FILE stderr
#endif

void _assertfail(char *assertstr, char *filestr, int line) {

		fprintf(ASSERT_ERROR_FILE,"assert_ext: \"%s\" %s:%d\n",
			assertstr, filestr, line);
		fflush(ASSERT_ERROR_FILE);
		_exit(-1);
}
#endif //#ifdef NDEBUG