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

static FLAC__uint64 total_samples;
static unsigned sample_rate, channels, bps;

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
		return 0x00;
	}

	ret_val = inwav.encode("temp.wav", inputDATA, "temp_encoded.wav");
	if (ret_val == 0x00) {
		safeRemove("temp.wav");
		safeRemove("temp_encoded.wav");
		return 0x00;
	}
	safeRemove("temp.wav");

	if (!wavToFlac("temp_encoded.wav", outputFLAC)) {
		safeRemove("temp_encoded.wav");
		safeRemove(outputFLAC);
		return 0x00;
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
	bool torf = true;

	LOG("Converting from FLAC to WAV...\n");

	foutputWAV = open_file(outputWAV, "wb");
	if (foutputWAV == NULL) { return false; }

	decoder = FLAC__stream_decoder_new();
	if (decoder == NULL) {
		LOG_DEBUG("E: Failed to allocate memory for FLAC decoder\n");
		close(foutputWAV);
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
	close(foutputWAV);

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
	if (!decoder || !client_data) {
		return;
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
	if (!decoder) {
		decoder = NULL;
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
	if (!decoder||!status||!client_data) return;
}

/****************************************************************/
/* function: flac::wavToFlac									*/
/* purpose: convert the wav file to a flac file				 	*/
/* args: const char[], const char[]								*/
/* returns: bool												*/
/****************************************************************/
bool flac::wavToFlac(const char inputWAV[], const char outputFLAC[]) {
	FLAC__byte buffer[READSIZE * 2 * 2];
	FLAC__int32 pcm[READSIZE * 2];
	FLAC__bool torf = true;
	FLAC__StreamEncoder *encoder = NULL;
	FLAC__StreamEncoderInitStatus init_status;
	FILE *finputWAV;
	unsigned total_samples = 0, sample_rate = 0, channels = 0, bps = 0;

	LOG("Converting from WAV to FLAC...\n");

	finputWAV = open_file(inputWAV, "rb");
	if (finputWAV == NULL) { return false; }

	// read wav header and validate it
	if (fread(buffer, 1, 44, finputWAV) != 44) {
		LOG_DEBUG("E: Failed to read WAV header\n");
		return false;
	}

	if (memcmp(buffer, "RIFF", 4) || memcmp(buffer+8, "WAVEfmt \020\000\000\000\001\000\002\000", 16) || memcmp(buffer+32, "\004\000\020\000data", 8)) {
		LOG_DEBUG("E: Invalid/unsupported WAVE file, only 16bps stereo\n");
		close(finputWAV);
		return false;
	}

	sample_rate = ((((((unsigned)buffer[27] << 8) | buffer[26]) << 8) | buffer[25]) << 8) | buffer[24];
	channels = 2;
	bps = 16;
	total_samples = (((((((unsigned)buffer[43] << 8) | buffer[42]) << 8) | buffer[41]) << 8) | buffer[40]) / 4;
   
	// allocate the encoder
	if ((encoder = FLAC__stream_encoder_new()) == NULL) {
		LOG_DEBUG("E: Problem allocating the encoder\n");
		close(finputWAV);
		return false;
	}

	torf &= FLAC__stream_encoder_set_verify(encoder, true);
	torf &= FLAC__stream_encoder_set_compression_level(encoder, 5);
	torf &= FLAC__stream_encoder_set_channels(encoder, channels);
	torf &= FLAC__stream_encoder_set_bits_per_sample(encoder, bps);
	torf &= FLAC__stream_encoder_set_sample_rate(encoder, sample_rate);
	torf &= FLAC__stream_encoder_set_total_samples_estimate(encoder, total_samples);

	// initialize encoder
	if (torf) {
		init_status = FLAC__stream_encoder_init_file(encoder, outputFLAC, NULL, NULL);
		if (init_status != FLAC__STREAM_ENCODER_INIT_STATUS_OK) {
			LOG_DEBUG("E: Failed to initialize encoder\n");
			LOG_DEBUG("\tE: State: %s\n", FLAC__StreamEncoderInitStatusString[init_status]);
			torf = false;
		}
	}

	// read blocks of samples from WAVE file and feed to encoder
	if (torf) {
		size_t left = (size_t)total_samples;
		while (torf && left) {
			size_t need = (left>READSIZE? (size_t)READSIZE : (size_t)left);
			if (fread(buffer, channels*(bps/8), need, finputWAV) != need) {
				LOG_DEBUG("E: Failed to read from WAVE file\n");
				torf = false;
			} else {
				// convert the packed little-endian 16-bit PCM samples from WAVE into an interleaved FLAC__int32 buffer
				size_t i;
				for (i = 0; i < need*channels; i++) {
					// inefficient but simple and works on big- or little-endian machines
					pcm[i] = (FLAC__int32)(((FLAC__int16)(FLAC__int8)buffer[2*i+1] << 8) | (FLAC__int16)buffer[2*i]);
				}
				// feed samples to encoder
				torf = FLAC__stream_encoder_process_interleaved(encoder, pcm, need);
			}
			left -= need;
		}
	}

	torf &= FLAC__stream_encoder_finish(encoder);

	if (!torf) {
		LOG_DEBUG("E: FLAC encoding failed\n");
		LOG_DEBUG("\tE: State: %s\n", FLAC__StreamEncoderStateString[FLAC__stream_encoder_get_state(encoder)]);
	}

	FLAC__stream_encoder_delete(encoder);
	close(finputWAV);

	return torf;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
