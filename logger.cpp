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

logger& getLogger() {
	static logger l;
	return l;
}


logger::logger(void) {
	maxEntries = 0;
	numEntries = 0;
	entries = NULL;
}

logger::~logger(void) {
	clean();
}

void logger::clean(void) {
	for (DWORD i = 0; i < numEntries; i++)
		FREE(entries[i].message);
	FREE(entries);
}

bool logger::record(char* msg) {
	int length = strlen(msg);

	if (numEntries == maxEntries && !resize())
		return false;

	entries[numEntries].message = (char *)malloc(length + 1);
	memcpy(entries[numEntries].message, msg, length);
	entries[numEntries].message[length] = '\0';
	numEntries++;

	return true;
}

void logger::print() {
	for (DWORD i = 0; i < numEntries; i++)
		printf(entries[i].message);
}

bool logger::resize(void) {
	_ENTRY *newEntries = NULL;

	newEntries = (_ENTRY *)malloc(sizeof(_ENTRY) * (maxEntries + LOGGER_RESIZE_AMOUNT));
	if (newEntries == NULL) {
		fprintf(stderr, "E: Unable to resize logger array!");
		return false;
	}

	maxEntries += LOGGER_RESIZE_AMOUNT;

	for (DWORD i = 0; i < numEntries; i++)
		newEntries[i] = entries[i];

	FREE(entries);

	entries = newEntries;

	return true;
}