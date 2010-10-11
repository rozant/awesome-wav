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
#ifndef __util_hpp__
#define __util_hpp__
#include "global.hpp"
#include <stdio.h>
#include <string.h>

// non inline functions
FILE* open(const char *filename, const char *mode);
bool close(FILE *aFile);
int safeRemove(const char *filename);

/****************************************************************/
/* function: setBit												*/
/* purpose: sets the bit at a specific position					*/
/* args: BYTE&, const BYTE, const bool							*/
/****************************************************************/
inline void setBit(BYTE &b, const BYTE index, const bool torf) {
	if (torf) // Set bit to 1
		b |= (1 << index); 
	else // Set bit to 0
		b &= ~(1 << index);
}

/****************************************************************/
/* function: getBit												*/
/* purpose: gets the bit at a specific position					*/
/* args: const BYTE, const BYTE									*/
/* returns: bool												*/
/*		1 = bit is a 1											*/
/*		0 = bit is a 0											*/
/****************************************************************/
inline bool getBit(const BYTE b, const BYTE index) {
	return (bool)((1 << index) & b);
}

/****************************************************************/
/* function: clearLower2Bits									*/
/* purpose: clears the 2 lower bits in a BYTE					*/
/* args: BYTE&													*/
/****************************************************************/
inline void clearLower2Bits(BYTE &b) {
	b &= 0xFC; // 252
}

/****************************************************************/
/* function: clearLower4Bits									*/
/* purpose: clears the 4 lower bits in a BYTE					*/
/* args: BYTE&													*/
/****************************************************************/
inline void clearLower4Bits(BYTE &b) {
	b &= 0xF0; // 240
}

/****************************************************************/
/* function: clearUpper4Bits									*/
/* purpose: clears the 4 upper bits in a BYTE					*/
/* args: BYTE&													*/
/****************************************************************/
inline void clearUpper4Bits(BYTE &b) {
	b &= 0x0F; // 15
}

/****************************************************************/
/* function: byteToMB											*/
/* purpose: convers bytes to megabytes							*/
/* args: const DWORD											*/
/* returns: double												*/
/****************************************************************/
#ifdef _DEBUGOUTPUT
inline double byteToMB(const DWORD bytes) {
	return bytes / 1048576.0;
}
#endif

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

