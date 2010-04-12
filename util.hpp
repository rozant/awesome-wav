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
#include <stdio.h>
#include "global.hpp"

/* non inline functions */
int bytencmp(const BYTE* b1, const BYTE* b2, size_t n);
FILE* open(const char *filename, const char *mode);
bool close(FILE *aFile);

/****************************************************************/
/* function: setBit												*/
/* purpose: sets the bit at a specific position					*/
/* args: BYTE&, const char, const bool							*/
/****************************************************************/
inline void setBit(BYTE &b, const char index, const bool torf) {
	BYTE bitMask = (1 << index);

	if (torf) // Set bit to 1
		b |= bitMask; 
	else // Set bit to 0
		b &= ~bitMask;
}

/****************************************************************/
/* function: getBit												*/
/* purpose: gets the bit at a specific position					*/
/* args: const BYTE, const char									*/
/* returns: bool												*/
/*		1 = bit is a 1											*/
/*		0 = bit is a 0											*/
/****************************************************************/
inline bool getBit(const BYTE b, const char index) {
	return ((1 << index) & b);
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

