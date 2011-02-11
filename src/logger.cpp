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
#include "logger.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************/
/* function: getLogger											*/
/* purpose: get a new logger									*/
/* args: void													*/
/* returns: logger&												*/
/****************************************************************/
logger& getLogger(void) {
	static logger l;
	return l;
}

/****************************************************************/
/* function: logger::logger										*/
/* purpose: constructor for the logger class					*/
/* args: void													*/
/****************************************************************/
logger::logger(void) {
	maxEntries = 0;
	numEntries = 0;
	lastPrinted = 0;
	entries = NULL;
}

/****************************************************************/
/* function: logger::~logger									*/
/* purpose: constructor for the logger class					*/
/* args: void													*/
/****************************************************************/
logger::~logger(void) {
	flush();
	clean();
	return;
}

/****************************************************************/
/* function: logger::clean										*/
/* purpose: clean out the logger construct properly				*/
/* args: void													*/
/* returns: void												*/
/****************************************************************/
void logger::clean(void) {
	for (unsigned int i = 0; i < numEntries; i++)
		FREE(entries[i].message);
	FREE(entries);
	numEntries = maxEntries = lastPrinted = 0;
	entries = NULL;
	return;
}

/****************************************************************/
/* function: logger::record										*/
/* purpose: record a message into the log						*/
/* args: const char*											*/
/* returns: bool												*/
/*		1 = added message sucessfuly							*/
/*		0 = message log at max size								*/
/****************************************************************/
bool logger::record(const char* msg) {
	int length = strlen(msg);

	if (numEntries == maxEntries && !resize())
		return false;

	entries[numEntries].message = (char *)malloc(sizeof(char) * (length + 1));
	memcpy(entries[numEntries].message, msg, sizeof(char) * length);
	entries[numEntries].message[length] = '\0';
	numEntries++;

	return true;
}

/****************************************************************/
/* function: logger::print										*/
/* purpose: print out the contents of the logger				*/
/* args: void													*/
/* returns: void												*/
/****************************************************************/
void logger::print(void) {
	for (unsigned int i = 0; i < numEntries; i++)
		printf("%s",entries[i].message);

	return;
}

/****************************************************************/
/* function: logger::flush										*/
/* purpose: print out any queued messages and remove them		*/
/* args: void													*/
/* returns: void												*/
/****************************************************************/
void logger::flush(void) {
	unsigned int i = 0;

	for (i = lastPrinted; i < numEntries; i++) {
		printf("%s",entries[i].message);
		FREE(entries[i].message);
	}

	lastPrinted = 0;
	numEntries = 0;
	return;
}

/****************************************************************/
/* function: logger::resize										*/
/* purpose: resize the active log								*/
/* args: void													*/
/* returns: bool												*/
/*		1 = resized sucessfuly									*/
/*		0 = failed to resize log (out of memory)				*/
/****************************************************************/
bool logger::resize(void) {
	_ENTRY *newEntries = NULL;

	newEntries = (_ENTRY *)malloc(sizeof(_ENTRY) * (maxEntries + LOGGER_RESIZE_AMOUNT));
	if (newEntries == NULL) {
		fprintf(stderr, "E: Unable to resize logger array!");
		return false;
	}

	maxEntries += LOGGER_RESIZE_AMOUNT;

	for (unsigned int i = 0; i < numEntries; i++)
		newEntries[i] = entries[i];

	FREE(entries);

	entries = newEntries;

	return true;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

