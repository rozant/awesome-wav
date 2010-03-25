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
/****************************************************************/
/* This is the .hpp file that implements the structures used	*/
/* in the RIFF format.  It does normal .h-ish stuff.  If you	*/
/* do not know what this means, please find the nearest person	*/
/* that does and have them escort you away from this computer	*/
/* immediately.													*/
/****************************************************************/
#ifndef __riff_hpp__
#define __riff_hpp__
#include <stdlib.h>
#include "typedefs.hpp"
/* defines useful for RIFF formatting */
#define FORMAT_PCM 1



/****************************************************************/
/* struct: _RIFF												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* first chunk in RIFF formatted files.							*/
/* notes: be warned - the ChunkID may not be NULL terminated	*/
/****************************************************************/
struct _RIFF {
	BYTE ChunkID[4]; // "RIFF"
	DWORD ChunkSize; // file size - 8
	BYTE Format[4]; // "WAVE"
	_RIFF(void) { return; }
	~_RIFF(void) { return; }
};
/****************************************************************/
/* struct: _FMT													*/
/* purpose: define a structure to make it easy to look at the 	*/
/* second chunk in RIFF formatted files.						*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/****************************************************************/
struct _FMT {
	BYTE SubchunkID[4]; // "fmt "
	DWORD SubchunkSize; // 16 + extra format bytes
	SHORT AudioFormat;
	SHORT NumChannels;
	DWORD SampleRate;
	DWORD ByteRate;
	SHORT BlockAlign;
	SHORT BitsPerSample;
	SHORT ExtraFormatBytes;
	BYTE *ExtraFormat;
	_FMT(void) { return; }
	~_FMT(void) {
		free(ExtraFormat);
		return;
	}
};
/****************************************************************/
/* struct: _DATA												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* third chunk in WAV (RIFF subset) formatted files.			*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/****************************************************************/
struct _DATA {
	BYTE SubchunkID[4];
	DWORD SubchunkSize;
	BYTE *Data;
	_DATA(void) { return; }
	~_DATA(void) {
		free(Data);
		return;
	}
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/
