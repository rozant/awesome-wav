/*****************************************************************
* This program is free software; you can redistribute it and/or	 *
* modify it under the terms of the GNU General Public License	 *
* version 2 as published by the Free Software Foundation.		 *
*																 *
* This program is distributed in the hope that it will be		 *
* useful, but WITHOUT ANY WARRANTY; without even the implied	 *
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR		 *
* PURPOSE.  See the GNU General Public License for more details. *
*																 *
* You should have received a copy of the GNU General Public 	 *
* License along with this program; if not, write to the Free 	 *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,	 *
* USA.															 *
*****************************************************************/
#ifndef __buffer_hpp__
#define __buffer_hpp__

#include "global.hpp"

/* macro for defining the size of the buffer */
#define BUFFER_SIZE(x) 1024*x

template <class T>
void next_chunk(T *input, BYTE * buffer, DWORD * buff_len) {
	SHORT *bps = input->bps;
	if(buffer == NULL) {
		buffer = (BYTE*)calloc(BUFFER_SIZE((*bps/8)),sizeof(BYTE));
	}
	return;
}

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

