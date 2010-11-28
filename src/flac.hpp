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
#ifndef __flac_hpp__
#define __flac_hpp__
#include "global.hpp"
#include "flac/FLAC/metadata.h"
#include "flac/FLAC/stream_encoder.h"

// error code defines
#define FLAC_SUCCESS		 1
#define FLAC_READ_FAIL		-1
#define FLAC_WRITE_FAIL		-2
#define FLAC_VALID_FAIL		-3
#define FLAC_FILE_CLOSED	-4

#define READSIZE 1024


static FLAC__uint64 total_samples;
static unsigned sample_rate;
static unsigned channels;
static unsigned bps;

/****************************************************************/
/* class: flac													*/
/* purpose: contain an entire flac file in ram.					*/
/****************************************************************/
class flac {
	private:
	public:
		// constructors
		flac(void);
		// destructor
		~flac(void);
		// manipulation
		unsigned long int encode(const char[], const char[], const char[]);
		bool decode(const char[], const char[], const int32&);
		bool wavToFlac(const char[], const char[]);
		bool flacToWav(const char[], const char[]);
};

FLAC__bool write_little_endian_uint16(FILE *f, FLAC__uint16 x);
FLAC__bool write_little_endian_int16(FILE *f, FLAC__int16 x);
FLAC__bool write_little_endian_uint32(FILE *f, FLAC__uint32 x);

FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data);
void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data);
void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data);


#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

