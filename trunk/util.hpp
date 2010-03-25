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
#ifdef _DEBUGOUTPUT
#include <ostream>
#endif
#include <stdio.h>
#include "typedefs.hpp"

/****************************************************************/
/* function: setBit												*/
/* purpose: sets the bit at a specific position					*/
/* args: BYTE&, const char, const bool&							*/
/****************************************************************/
void setBit(BYTE &b, const char index, const bool &torf) {
	BYTE bitMask = 1;
	bitMask <<= index;

	if (torf) // Set bit to 1
		b |= bitMask; 
	else // Set bit to 0
		b &= ~bitMask;
}

/****************************************************************/
/* function: getBit												*/
/* purpose: gets the bit at a specific position					*/
/* args: const BYTE&, const char								*/
/* returns: bool												*/
/*		1 = bit is a 1											*/
/*		0 = bit is a 0											*/
/****************************************************************/
bool getBit(const BYTE &b, const char index) {
	BYTE bitMask = 1;
	bitMask <<= index;

	if (bitMask & b)
		return true;
	return false;
}

/****************************************************************/
/* function: bytencmp											*/
/* purpose: compares two bytes									*/
/* args: const BYTE *, const BYTE *, size_t						*/
/* returns: int													*/
/****************************************************************/
int bytencmp(const BYTE* b1, const BYTE* b2, size_t n) {
	while(n--)
		if(*b1++!=*b2++)
			return *(BYTE*)(b1 - 1) - *(BYTE*)(b2 - 1);
	return 0;
}

/****************************************************************/
/* function: open												*/
/* purpose: open a file.										*/
/* args: const char *, const char * 							*/
/* returns: FILE *												*/
/*		*	 = opened correctly									*/
/*		NULL = opened incorrectly								*/
/****************************************************************/
FILE* open(const char *filename, const char *mode) {
	FILE* aFile = NULL;
	aFile = fopen(filename, mode);

	#ifdef _DEBUGOUTPUT
	if (aFile == NULL)
		std::cout << "E: Failed to open " << filename << " with mode " << mode << std::endl;
	else
		std::cout << "S: Opened " << filename << " with mode " << mode << std::endl;
	#endif

	return aFile;
}

/****************************************************************/
/* function: close												*/
/* purpose: close an open file.									*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = closed correctly									*/
/*		0 = closed incorrectly, or already closed				*/
/****************************************************************/
bool close(FILE *aFile) {
	if( aFile) {
		if ( fclose( aFile ) ) {
			#ifdef _DEBUGOUTPUT
			std::cout << "E: Failed to close file" << std::endl;
			#endif
			return false;
		} else {
			#ifdef _DEBUGOUTPUT
			std::cout << "S: Closed file" << std::endl;
			#endif
			return true;
		}
	}
	#ifdef _DEBUGOUTPUT
	std::cout << "E: File already closed" << std::endl;
	#endif
	return false;
}

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

