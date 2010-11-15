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
#include "global.hpp"
#include <stdlib.h>

/****************************************************************/
/* struct: _RIFF_UNKNOWN_CHUNKS									*/
/* purpose: store unknown chunks here							*/
/****************************************************************/
struct _RIFF_UNKNOWN_CHUNKS {
	char *data;
	_RIFF_UNKNOWN_CHUNKS(void) { data = NULL; return; }
	~_RIFF_UNKNOWN_CHUNKS(void) { free(data); data = NULL; }
};

/****************************************************************/
/* struct: _RIFF												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* first chunk in RIFF formatted files.							*/
/* notes: be warned - the ChunkID may not be NULL terminated	*/
/****************************************************************/
struct _RIFF {
	int8 ChunkID[5]; // "RIFF"
	int32 ChunkSize; // file size - 8
	int8 Format[5];
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
	int8 SubchunkID[5]; // "fmt "
	int32 SubchunkSize; // 16 + extra format bytes
	int16 AudioFormat;
	int16 NumChannels;
	int32 SampleRate;
	int32 ByteRate;
	int16 BlockAlign;
	int16 BitsPerSample;
	int16 ExtraFormatBytes;
	int16 ValidBitsPerSample;
	int32 ChannelMask;
	int8 SubFormat[17];
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
	int8 SubchunkID[5]; // "fact"
	int32 SubchunkSize;	// 4
	int32 SampleLength; // per channel
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
	int32 Position;		// sample frame for peak
};
/****************************************************************/
/* struct: _PEAK												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* PEAK chunk in WAV (RIFF subset) formatted files.				*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/*		this chunk will not exist in PCM wav files.				*/
/****************************************************************/
struct _PEAK {
	int8 SubchunkID[5]; // "PEAK"
	int32 SubchunkSize;
	int32 Version;		// peak chunk version
	int32 timestamp;	// UNIX timestamp of creation
	_PPEAK *peak;		// one for each channel
	int16 *bit_align;	// space for the 64-bit alignment variable
	_PEAK(void) { bit_align = NULL; peak = NULL; SubchunkID[4] = 0; return; }
	~_PEAK(void) { free(peak); free(bit_align); return; }
};
/****************************************************************/
/* struct: _DATA												*/
/* purpose: define a structure to make it easy to look at the 	*/
/* DATA chunk in WAV (RIFF subset) formatted files.				*/
/* notes: be warned - the SubchunkID may not be NULL terminated	*/
/****************************************************************/
struct _DATA {
	int8 SubchunkID[5];	// "data"
	int32 SubchunkSize;	// size of the byte array
	int8 *Data;
	_DATA(void) { Data = NULL; SubchunkID[4] = 0; return; }
	~_DATA(void) { free(Data); return; }
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

