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
#include "util.hpp"

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
		fprintf(stderr,"E: Failed to open %s with mode %s\n",filename,mode);
	else
		fprintf(stderr,"S: Opened %s with mode %s\n",filename,mode);
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
			fprintf(stderr,"E: Failed to close file\n");
			#endif
			return false;
		} else {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"S: Closed file\n");
			#endif
			return true;
		}
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: File already closed\n");
	#endif
	return false;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
