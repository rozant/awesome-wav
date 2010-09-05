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
#include "logger.hpp"

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
	if (aFile == NULL) {
		LOG_DEBUG("E: Failed to open %s with mode %s\n", filename, mode);
	} else {
		LOG_DEBUG("S: Opened %s with mode %s\n", filename, mode);
	}
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
	if (aFile) {
		if (fclose(aFile)) {
			LOG_DEBUG("E: Failed to close file\n");
			return false;
		} else {
			LOG_DEBUG("S: Closed file\n");
			return true;
		}
	}
	LOG_DEBUG("E: File already closed\n");
	return false;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

