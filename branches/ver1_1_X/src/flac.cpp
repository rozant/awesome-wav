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
#include "flac.hpp"
#include "global.hpp"
#include "util.hpp"
#include "logger.hpp"
#include <stdio.h>

/****************************************************************/
/* function: flac::flac											*/
/* purpose: constructor for the flac class						*/
/* args: void													*/
/****************************************************************/
flac::flac(void) {
}

/****************************************************************/
/* function: flac::~flac										*/
/* purpose: destructor for the wav class						*/
/* args: void													*/
/****************************************************************/
flac::~flac(void) {
	clean();
	return;
}

/****************************************************************/
/* function: flac::clean										*/
/* purpose: effective destructor for the flac class				*/
/* args: void													*/
/* returns: void												*/
/****************************************************************/
void flac::clean(void) {
	return;
}

/****************************************************************/
/* function: encode												*/
/* purpose: open the files ment for encoding				 	*/
/* args: const char[], const char[], const char[]				*/
/* returns: unsigned long int									*/
/****************************************************************/
unsigned long int flac::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[]) {
	unsigned long int ret_val = 0;
	FILE *fInputFLAC;
	char *inputFLAC = (char *)"16_Bit_PCM.flac";

	LOG("Opening input wave file...\n");
	// Open up our input wav file
	fInputFLAC = open(inputFLAC, "rb");
	if (fInputFLAC == NULL) { return false; }

	FLACread(fInputFLAC, this);

	// clean up
	clean();
	return ret_val;
}

/****************************************************************/
/* function: decode												*/
/* purpose: open the files ment for decoding				 	*/
/* args: const char[], const char[], const int32&				*/
/* returns: bool												*/
/****************************************************************/
bool flac::decode(const char inputWAV[], const char outputDATA[], const int32& fileSize) {
	return false;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
