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
#ifndef __file_compression_h__
#define __file_compression_h__

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>

// chunk size for compression
#define Z_CHUNK (Z_MULT*16384)

// function prototypes
int def(FILE *source, FILE *dest, const int level);
int inf(FILE *source, FILE *dest);
const char * zerr(const int ret);

#ifdef __cplusplus
}
#endif 

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

