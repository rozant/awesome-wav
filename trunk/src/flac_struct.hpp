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
#ifndef __flac_struct_hpp__
#define __flac_struct_hpp__
#include "global.hpp"
#include <stdlib.h>

// error code defines
#define FLAC_SUCCESS		 1
#define FLAC_READ_FAIL		-1
#define FLAC_WRITE_FAIL		-2
#define FLAC_VALID_FAIL		-3
#define FLAC_FILE_CLOSED	-4

struct FLAC_METADATA_BLOCK_STREAMINFO {
};

struct FLAC_METADATA_BLOCK_PADDING {
};

struct FLAC_METADATA_BLOCK_APPLICATION {
};

struct FLAC_SEEKPOINT {
	FLAC_SEEKPOINT(void) { return; }
	~FLAC_SEEKPOINT(void) { return; }
};

struct FLAC_METADATA_BLOCK_SEEKTABLE {
};

struct FLAC_METADATA_BLOCK_VORBIS_COMMENT {
};

struct FLAC_CUESHEET_INDEX {
	FLAC_CUESHEET_INDEX(void) { return; }
	~FLAC_CUESHEET_INDEX(void) { return; }
};

struct FLAC_CUESHEET_TRACK {
	FLAC_CUESHEET_TRACK(void) { return; }
	~FLAC_CUESHEET_TRACK(void) { return; }
};

struct FLAC_METADATA_BLOCK_CUESHEET {
};

struct FLAC_METADATA_BLOCK_PICTURE {
};

struct FLAC_FRAME_HEADER {
	SHORT SyncCode_BlockingStrategy;
	BYTE BlockSize_SampleRate;
	BYTE ChannelAssignment_SampleSize;
	BYTE *Extra;
	BYTE CRC8;
	FLAC_FRAME_HEADER(void) { Extra = NULL; return; }
	~FLAC_FRAME_HEADER(void) { free(Extra); return; }
};

struct FLAC_RICE_PARTITION {
	FLAC_RICE_PARTITION(void) { return; }
	~FLAC_RICE_PARTITION(void) { return; }
};

struct FLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE {
	FLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE(void) { return; }
	~FLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE(void) { return; }
};

struct FLAC_RICE2_PARTITION {
	FLAC_RICE2_PARTITION(void) { return; }
	~FLAC_RICE2_PARTITION(void) { return; }
};

struct FLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2 {
	FLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2(void) { return; }
	~FLAC_RESIDUAL_CODING_METHOD_PARTITIONED_RICE2(void) { return; }
};

struct FLAC_RESIDUAL {
	FLAC_RESIDUAL(void) { return; }
	~FLAC_RESIDUAL(void) { return; }
};

struct FLAC_SUBFRAME_HEADER {
	BYTE SubFrameType_WastedBitsFlag;
	BYTE *WastedBits;
	FLAC_SUBFRAME_HEADER(void) { WastedBits = NULL; return; }
	~FLAC_SUBFRAME_HEADER(void) { free(WastedBits); return; }
};

struct FLAC_SUBFRAME_CONSTANT {
	FLAC_SUBFRAME_CONSTANT(void) { return; }
	~FLAC_SUBFRAME_CONSTANT(void) { return; }
};

struct FLAC_SUBFRAME_FIXED {
	FLAC_SUBFRAME_FIXED(void) { return; }
	~FLAC_SUBFRAME_FIXED(void) { return; }
};

struct FLAC_SUBFRAME_LPC {
	FLAC_SUBFRAME_LPC(void) { return; }
	~FLAC_SUBFRAME_LPC(void) { return; }
};

struct FLAC_SUBFRAME_VERBATIM {
	FLAC_SUBFRAME_VERBATIM(void) { return; }
	~FLAC_SUBFRAME_VERBATIM(void) { return; }
};

struct FLAC_SUBFRAME {
	FLAC_SUBFRAME(void) { return; }
	~FLAC_SUBFRAME(void) { return; }
};

struct FLAC_FRAME_FOOTER {
	SHORT CRC16;
	FLAC_FRAME_FOOTER(void) { return; }
	~FLAC_FRAME_FOOTER(void) { return; }
};

struct FLAC_FRAME {
	FLAC_FRAME_HEADER FrameHeader;
	FLAC_SUBFRAME *Subframe;
	BYTE *Padding;
	FLAC_FRAME_FOOTER FrameFooter;
	FLAC_FRAME(void) { Subframe = NULL; Padding = NULL; return; }
	~FLAC_FRAME(void) { free(Subframe); free(Padding); return; }
};

struct FLAC_METADATA_BLOCK_HEADER {
	BYTE BlockType;
	DWORD DataLength;
	FLAC_METADATA_BLOCK_HEADER(void) { return; }
	~FLAC_METADATA_BLOCK_HEADER(void) { return; }
};

union FLAC_METADATA_BLOCK_DATA {
	FLAC_METADATA_BLOCK_STREAMINFO StreamInfo;
	FLAC_METADATA_BLOCK_PADDING Padding;
	FLAC_METADATA_BLOCK_APPLICATION Application;
	FLAC_METADATA_BLOCK_SEEKTABLE SeekTable;
	FLAC_METADATA_BLOCK_VORBIS_COMMENT Comment;
	FLAC_METADATA_BLOCK_CUESHEET Cuesheet;
	FLAC_METADATA_BLOCK_PICTURE Picture;
};

struct FLAC_METADATA_BLOCK {
	FLAC_METADATA_BLOCK_HEADER Header;
	FLAC_METADATA_BLOCK_DATA Data;
	FLAC_METADATA_BLOCK(void) { return; }
	~FLAC_METADATA_BLOCK(void) { return; }
};

struct FLAC_STREAM {
	BYTE StreamMarker[5]; // "fLaC"
	FLAC_METADATA_BLOCK StreamInfo;
	FLAC_METADATA_BLOCK *ExtraInfo;
	FLAC_FRAME *AudioFrames;
	FLAC_STREAM(void) { StreamMarker[4] = 0; ExtraInfo = NULL; AudioFrames = NULL; return; }
	~FLAC_STREAM(void) { free(ExtraInfo); free(AudioFrames); return; }
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

