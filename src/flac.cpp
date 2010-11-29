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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "flac.hpp"
#include "global.hpp"
#include "util.hpp"
#include "logger.hpp"
#include "wav.hpp"

/****************************************************************/
/* function: flac::flac											*/
/* purpose: constructor for the flac class						*/
/* args: void													*/
/****************************************************************/
flac::flac(void) {
	total_samples = 0;
	sample_rate = 0;
	channels = 0;
	bps = 0;
}

/****************************************************************/
/* function: flac::~flac										*/
/* purpose: destructor for the flac class						*/
/* args: void													*/
/****************************************************************/
flac::~flac(void) {
	return;
}

/****************************************************************/
/* function: flac::encode										*/
/* purpose: convert flac to wav, encode, then wav to flac	 	*/
/* args: const char[], const char[], const char[]				*/
/* returns: unsigned long int									*/
/****************************************************************/
unsigned long int flac::encode(const char inputFLAC[], const char inputDATA[], const char outputFLAC[]) {
	unsigned long int ret_val = 0;
	wav inwav;

	if (!flacToWav(inputFLAC, "temp.wav")) {
		safeRemove("temp.wav");
		return false;
	}

	ret_val = inwav.encode("temp.wav", inputDATA, "temp_encoded.wav");
	if (ret_val == 0x00) {
		safeRemove("temp.wav");
		safeRemove("temp_encoded.wav");
		return ret_val;
	}
	safeRemove("temp.wav");

	if (!wavToFlac("temp_encoded.wav", outputFLAC)) {
		safeRemove("temp_encoded.wav");
		safeRemove("outputFLAC");
		return false;
	}
	safeRemove("temp_encoded.wav");

	return ret_val;
}

/****************************************************************/
/* function: flac::decode										*/
/* purpose: convert flac to wav and then decode				 	*/
/* args: const char[], const char[], const int32&				*/
/* returns: bool												*/
/****************************************************************/
bool flac::decode(const char inputFLAC[], const char outputDATA[], const int32& fileSize) {
	wav inwav;

	if (!flacToWav(inputFLAC, "temp_out.wav")) {
		return false;
	}

	if (!inwav.decode("temp_out.wav", outputDATA, fileSize)) {
		return false;
	}

	return true;
}

/****************************************************************/
/* function: flac::flacToWav									*/
/* purpose: convert the flac file to a wav file				 	*/
/* args: const char[], const char[]								*/
/* returns: bool												*/
/****************************************************************/
bool flac::flacToWav(const char inputFLAC[], const char outputWAV[]) {
	FLAC__StreamDecoder *decoder = NULL;
	FLAC__StreamDecoderInitStatus init_status;
	FILE *foutputWAV;
	bool torf;

	LOG("Converting from FLAC to WAV...\n");

	foutputWAV = open(outputWAV, "wb");
	if (foutputWAV == NULL) { return false; }

	decoder = FLAC__stream_decoder_new();
	if (decoder == NULL) {
		LOG_DEBUG("E: Failed to allocate memory for FLAC decoder\n");
		return false;
	}

	(void)FLAC__stream_decoder_set_md5_checking(decoder, true);

	init_status = FLAC__stream_decoder_init_file(decoder, inputFLAC, &write_callback, &metadata_callback, &error_callback, foutputWAV);
	if (init_status != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
		LOG_DEBUG("E: FLAC decoder initialization failed: %s\n", FLAC__StreamDecoderInitStatusString[init_status]);
		torf = false;
	}
	
	if (torf) {
		torf = FLAC__stream_decoder_process_until_end_of_stream(decoder);
	}

	if (!torf) {
		LOG_DEBUG("E: FLAC decoding failed\n");
		LOG_DEBUG("\tE: State: %s\n", FLAC__StreamDecoderStateString[FLAC__stream_decoder_get_state(decoder)]);
	}

	FLAC__stream_decoder_delete(decoder);
	fclose(foutputWAV);

	return torf;
}

/****************************************************************/
/* function: metadata_callback									*/
/* purpose: flac decoder metadata callback					 	*/
/* args: const FLAC__StreamDecoder *,							*/
/*		const FLAC__StreamMetadata,	void *						*/
/* returns: void												*/
/****************************************************************/
void metadata_callback(const FLAC__StreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data) {
	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO) {
		total_samples = metadata->data.stream_info.total_samples;
		sample_rate = metadata->data.stream_info.sample_rate;
		channels = metadata->data.stream_info.channels;
		bps = metadata->data.stream_info.bits_per_sample;
	}
}

/****************************************************************/
/* function: write_callback										*/
/* purpose: flac decoder write callback						 	*/
/* args: const FLAC__StreamDecoder *, const FLAC__Frame *,		*/
/*		const FLAC__int32 * const buffer[], void *				*/
/* returns: FLAC__StreamDecoderWriteStatus						*/
/****************************************************************/
FLAC__StreamDecoderWriteStatus write_callback(const FLAC__StreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *client_data) {
	const FLAC__uint32 total_size = (FLAC__uint32)(total_samples * channels * (bps/8));
	FILE *f = (FILE*)client_data;
	size_t i;

	if (total_samples == 0) {
		LOG_DEBUG("E: Total samples not defined in STREAMINFO\n");
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	if (channels != 2) {
		LOG_DEBUG("E: Converter only supports 2 channels - have %d channels\n", channels);
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	if (bps != 16) {
		LOG_DEBUG("E: Converter only supports 16 bps - have %d bps\n", bps);
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}

	// write WAVE header
	if (frame->header.number.sample_number == 0) {
		if ( fwrite("RIFF", 1, 4, f) < 4 || !write_little_endian_uint32(f, total_size + 36) || fwrite("WAVEfmt ", 1, 8, f) < 8 ||
			!write_little_endian_uint32(f, 16) || !write_little_endian_uint16(f, 1) || !write_little_endian_uint16(f, (FLAC__uint16)channels) ||
			!write_little_endian_uint32(f, sample_rate) || !write_little_endian_uint32(f, sample_rate * channels * (bps/8)) ||
			!write_little_endian_uint16(f, (FLAC__uint16)(channels * (bps/8))) || !write_little_endian_uint16(f, (FLAC__uint16)bps) ||
			fwrite("data", 1, 4, f) < 4 || !write_little_endian_uint32(f, total_size))
		{
			LOG_DEBUG("E: Failed to write WAV header\n");
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}

	// write decoded PCM samples
	for (i = 0; i < frame->header.blocksize; i++) {
		if (!write_little_endian_int16(f, (FLAC__int16)buffer[0][i]) || !write_little_endian_int16(f, (FLAC__int16)buffer[1][i])) {
			LOG_DEBUG("E: Failed to write data chunk\n");
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
		}
	}

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

/****************************************************************/
/* function: error_callback										*/
/* purpose: flac decoder error callback						 	*/
/* args: const FLAC__StreamDecoder *, const FLAC__Frame,		*/
/*		FLAC__StreamDecoderErrorStatus, void *					*/
/* returns: FLAC__StreamDecoderWriteStatus						*/
/****************************************************************/
void error_callback(const FLAC__StreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data) {
	LOG_DEBUG("E: Error callback: %s\n", FLAC__StreamDecoderErrorStatusString[status]);
}

bool flac::wavToFlac(const char inputWAV[], const char outputFLAC[]) {
	FLAC__byte buffer[READSIZE * 2 * 2]; // we read the WAVE data into here
	unsigned total_samples = 0;
	static FLAC__int32 pcm[READSIZE * 2];
	FLAC__bool ok = true;
	FLAC__StreamEncoder *encoder = 0;
	FLAC__StreamEncoderInitStatus init_status;
	FLAC__StreamMetadata *metadata[2];
	FLAC__StreamMetadata_VorbisComment_Entry entry;
	FILE *fin;
	unsigned sample_rate = 0;
	unsigned channels = 0;
	unsigned bps = 0;

	LOG("Converting from WAV to FLAC...\n");

	if ((fin = fopen(inputWAV, "rb")) == NULL) {
		fprintf(stderr, "ERROR: opening %s for output\n", inputWAV);
		return false;
	}

	// read wav header and validate it
	if (
		fread(buffer, 1, 44, fin) != 44 ||
		memcmp(buffer, "RIFF", 4) ||
		memcmp(buffer+8, "WAVEfmt \020\000\000\000\001\000\002\000", 16) ||
		memcmp(buffer+32, "\004\000\020\000data", 8)
	) {
		fprintf(stderr, "ERROR: invalid/unsupported WAVE file, only 16bps stereo WAVE in canonical form allowed\n");
		fclose(fin);
		return false;
	}
	sample_rate = ((((((unsigned)buffer[27] << 8) | buffer[26]) << 8) | buffer[25]) << 8) | buffer[24];
	channels = 2;
	bps = 16;
	total_samples = (((((((unsigned)buffer[43] << 8) | buffer[42]) << 8) | buffer[41]) << 8) | buffer[40]) / 4;
   
	// allocate the encoder
	if ((encoder = FLAC__stream_encoder_new()) == NULL) {
		fprintf(stderr, "ERROR: allocating encoder\n");
		fclose(fin);
		return false;
	}

	ok &= FLAC__stream_encoder_set_verify(encoder, true);
	ok &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	ok &= FLAC__stream_encoder_set_channels(encoder, channels);
	ok &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	ok &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
	ok &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

	// now add some metadata; we'll add some tags and a padding block
	if (ok) {
		if (
			(metadata[0] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_VORBIS_COMMENT)) == NULL ||
			(metadata[1] = FLAC__metadata_object_new(FLAC__METADATA_TYPE_PADDING)) == NULL ||
			!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "ARTIST", "Some Artist") ||
			!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false) ||
			!FLAC__metadata_object_vorbiscomment_entry_from_name_value_pair(&entry, "YEAR", "1984") ||
			!FLAC__metadata_object_vorbiscomment_append_comment(metadata[0], entry, false)
		) {
			fprintf(stderr, "ERROR: out of memory or tag error\n");
			ok = false;
		}

		metadata[1]->length = 1234; // set the padding length

		ok = FLAC__stream_encoder_set_metadata(encoder, metadata, 2);
	}

	// initialize encoder
	if (ok) {
		init_status = FLAC__stream_encoder_init_file(encoder, outputFLAC, NULL, NULL);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			fprintf(stderr, "ERROR: initializing encoder: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
			ok = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (ok) {
		size_t left = (size_t)total_samples;
		while(ok && left) {
			size_t need = (left>READSIZE? (size_t)READSIZE : (size_t)left);
			if (fread(buffer, channels*(bps/8), need, fin) != need) {
				fprintf(stderr, "ERROR: reading from WAVE file\n");
				ok = false;
			}
			else {
				// convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer for libFLAC
				size_t i;
				for(i = 0; i < need*channels; i++) {
					// inefficient but simple and works on big- or little-endian machines
					pcm[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buffer[2*i+1] << 8) | (FLAC__int16)buffer[2*i]);
				}
				// feed samples to encoder
				ok = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
			}
			left -= need;
		}
	}

	ok &= FLAC__stream_encoder_finish(encoder);

	fprintf(stderr, "encoding: %s\n", ok? "succeeded" : "FAILED");
	fprintf(stderr, "   state: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);

	// now that encoding is finished, the metadata can be freed
	FLAC__metadata_object_delete(metadata[0]);
	FLAC__metadata_object_delete(metadata[1]);

	FLAC__stream_encoder_delete(encoder);
	fclose(fin);

	return true;
}


FLAC__bool write_little_endian_uint16(FILE *f, FLAC__uint16 x) {
	return fputc(x, f) != EOF && fputc(x >> 8, f) != EOF;
}

FLAC__bool write_little_endian_int16(FILE *f, FLAC__int16 x) {
	return write_little_endian_uint16(f, (FLAC__uint16)x);
}

FLAC__bool write_little_endian_uint32(FILE *f, FLAC__uint32 x) {
	return fputc(x, f) != EOF && fputc(x >> 8, f) != EOF && fputc(x >> 16, f) != EOF && fputc(x >> 24, f) != EOF;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
