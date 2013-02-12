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
FILE* open_file(const char *filename, const char *mode);
bool close(FILE *aFile);
int safeRemove(const char *filename);

/****************************************************************/
/* function: setBit												*/
/* purpose: sets the bit at a specific position					*/
/* args: int8&, const int8, const bool							*/
/****************************************************************/
inline void setBit(int8 &b, const int8 index, const bool torf) {
	if (torf) // Set bit to 1
		b |= (1 << index); 
	else // Set bit to 0
		b &= ~(1 << index);
}

/****************************************************************/
/* function: getBit												*/
/* purpose: gets the bit at a specific position					*/
/* args: const int8, const int8									*/
/* returns: bool												*/
/*		1 = bit is a 1											*/
/*		0 = bit is a 0											*/
/****************************************************************/
inline bool getBit(const int8 b, const int8 index) {
	return (bool)((1 << index) & b);
}

/****************************************************************/
/* function: clearLower2Bits									*/
/* purpose: clears the 2 lower bits in a int8					*/
/* args: int8&													*/
/****************************************************************/
inline void clearLower2Bits(int8 &b) {
	b &= 0xFC; // 252
}

/****************************************************************/
/* function: clearLower2Bits_off								*/
/* purpose: clears the 2 lower bits in a int8, with offset		*/
/* args: int8&, const char										*/
/* returns: int													*/
/****************************************************************/
inline int clearLower2Bits_off(int8 &b, const char off) {
	switch (off) {
		case 0:
			b &= 0xFC; // 252
			break;
		case 1:
			b &= 0xF9; // 249
			break;
		case 2:
			b &= 0xF3; // 243
			break;
		case 3:
			b &= 0xE7; // 231
			break;
		case 4:
			b &= 0xCF; // 207
			break;
		case 5:
			b &= 0x9F; // 159
			break;
		case 6:
			b &= 0x3F; // 63
			break;
		default:
			return -1;
			break;
	}
	return 0;
}

/****************************************************************/
/* function: clearLower4Bits									*/
/* purpose: clears the 4 lower bits in a int8					*/
/* args: int8&													*/
/****************************************************************/
inline void clearLower4Bits(int8 &b) {
	b &= 0xF0; // 240
}

/****************************************************************/
/* function: clearUpper4Bits									*/
/* purpose: clears the 4 upper bits in a int8					*/
/* args: int8&													*/
/****************************************************************/
inline void clearUpper4Bits(int8 &b) {
	b &= 0x0F; // 15
}

/****************************************************************/
/* function: byteToMB											*/
/* purpose: convers bytes to megabytes							*/
/* args: const int32											*/
/* returns: double												*/
/****************************************************************/
#ifdef _DEBUGOUTPUT
inline double byteToMB(const int32 bytes) {
	return bytes / 1048576.0;
}
#endif

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

