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
#ifndef __riff_struct_hpp__
#define __riff_struct_hpp__
#include <stdlib.h>
#include "global.hpp"
/* defines useful for RIFF formatting */
#define WAVE_FORMAT_UNKNOWN    	0x0000
#define	WAVE_FORMAT_PCM			0x0001	
#define WAVE_FORMAT_IEEE_FLOAT 	0x0003

/****************************************************************/
/* struct: _RIFF												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* first chunk in RIFF formatted files.							*/
/* notes: be warned - the ChunkID may not be NULL terminated	*/
/****************************************************************/
struct _RIFF {
	BYTE ChunkID[5]; // "RIFF"
	DWORD ChunkSize; // file size - 8
	BYTE Format[5]; // "WAVE"
	_RIFF(void) { ChunkID[4] = 0; Format[4] = 0; return; }
	~_RIFF(void) { return; }
};
/****************************************************************/
/* struct: _FMT													*/
/* purpose: define a structure to make it easy to look at the 	*/
/* FMT chunk in RIFF formatted files.							*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/****************************************************************/
struct _FMT {
	BYTE SubchunkID[5]; // "fmt "
	DWORD SubchunkSize; // 16 + extra format bytes
	SHORT AudioFormat;
	SHORT NumChannels;
	DWORD SampleRate;
	DWORD ByteRate;
	SHORT BlockAlign;
	SHORT BitsPerSample;
	SHORT ExtraFormatBytes;
	SHORT ValidBitsPerSample;
	DWORD ChannelMask;
	BYTE SubFormat[17];
	_FMT(void) { SubchunkID[4] = 0; SubFormat[16] = 0; return; }
	~_FMT(void) { return; }
};
/****************************************************************/
/* struct: _FACT												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* FACT chunk in WAV (RIFF subset) formatted files.				*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/*		this chunk will not exist in PCM wav files.				*/
/****************************************************************/
struct _FACT {
	BYTE SubchunkID[5]; // "fact"
	DWORD SubchunkSize;	// 4
	DWORD SampleLength; // per channel
	_FACT(void) { SubchunkID[4] = 0; return; }
	~_FACT(void) { return; }
};
/****************************************************************/
/* struct: _PPEAK												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* position peak chunks in WAV (RIFF subset) formatted files.	*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/*		this chunk will not exist in PCM wav files.				*/
/****************************************************************/
struct _PPEAK {
	float Value;		// peak value
	DWORD Position;		// sample frame for peak
};
/****************************************************************/
/* struct: _PEAK												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* PEAK chunk in WAV (RIFF subset) formatted files.				*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/*		this chunk will not exist in PCM wav files.				*/
/****************************************************************/
struct _PEAK {
	BYTE SubchunkID[5]; // "PEAK"
	DWORD SubchunkSize; 
	DWORD Version;		// peak chunk version
	DWORD timestamp;	// UNIX timestamp of creation
	_PPEAK *peak;		// one for each channel
	_PEAK(void) { peak = NULL; SubchunkID[4] = 0; return; }
	~_PEAK(void) { free(peak); return; }
};
/****************************************************************/
/* struct: _DATA												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* DATA chunk in WAV (RIFF subset) formatted files.				*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/****************************************************************/
struct _DATA {
	BYTE SubchunkID[5];
	DWORD SubchunkSize;
	BYTE *Data;
	_DATA(void) { Data = NULL; SubchunkID[4] = 0; return; }
	~_DATA(void) { free(Data); return; }
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/
