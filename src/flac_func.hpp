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
#ifndef __flac_func_hpp__
#define __flac_func_hpp__
#include "global.hpp"
#include "flac_struct.hpp"
#include "logger.hpp"
#include <stdio.h>
#include <string.h>

/**********************Function Prototypes***********************/

// read
template <class T>
int FLACread(FILE *, T *);
template <class T>
int FLACreadSTREAM(FILE *, T *);

/***************************Functions****************************/

/****************************************************************/
/* function: FLACread											*/
/* purpose: reads a flac file into memory						*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int FLACread(FILE *inFile, T *input) {
	int ret_val;

	// read riff chunk
	ret_val = FLACreadSTREAM(inFile, input);
	if (!ret_val) {
		return ret_val;
	}

	return FLAC_SUCCESS;
}

/****************************************************************/
/* function: FLACreadSTREAM										*/
/* purpose: reads the stream chunk from a flac file				*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int FLACreadSTREAM(FILE *inFile, T *input) {
	// read
	if (fread(input->stream.StreamMarker, sizeof(BYTE), 4, inFile)) {
		LOG_DEBUG("S: Read STREAM header\n");
	} else {
		LOG_DEBUG("E: Failed to read STREAM header: Could not read bytes\n");
		return FLAC_READ_FAIL;
	}
	// basic validation
	if (memcmp(input->stream.StreamMarker, "fLaC", 4) != 0) {
		LOG_DEBUG("E: Invalid STREAM header: StreamMarker != 'fLaC'\n");
		LOG_DEBUG("\tStreamMarker == %s\n", (char*)input->stream.StreamMarker);
		return FLAC_VALID_FAIL;
	}
	return FLAC_SUCCESS;
}

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/
