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
#include "riff.hpp"
#include "wav.hpp"
#include "global.hpp"
#include "util.hpp"
#include "logger.hpp"
#include <stdio.h>
#include <math.h>
#include <string.h>
#ifdef _DEBUGOUTPUT
#include <time.h>
#endif

/****************************************************************/
/* function: wav::wav											*/
/* purpose: constructor for the wav class						*/
/* args: void													*/
/****************************************************************/
wav::wav(void) {
	memset(&riff, 0, sizeof(_RIFF));
	memset(&fmt, 0, sizeof(_FMT));
	memset(&data, 0, sizeof(_DATA));
	fact = NULL;
	peak = NULL;
}

/****************************************************************/
/* function: wav::~wav											*/
/* purpose: destructor for the wav class						*/
/* args: void													*/
/****************************************************************/
wav::~wav(void) {
	clean();
	return;
}

/****************************************************************/
/* function: wav::clean											*/
/* purpose: eddective destructor for the wav class				*/
/* args: void													*/
/* returns: void												*/
/****************************************************************/
void wav::clean(void) {
	if (fact != NULL) {
		free(fact);
		fact = NULL;
	}
	if (peak != NULL) {
		if (peak->peak != NULL) {
			free(peak->peak);
			peak->peak = NULL;
		}
		if (peak->bit_align != NULL) {
			free(peak->bit_align);
			peak->bit_align = NULL;
		}
		free(peak);
		peak = NULL;
	}
	return;
}

/****************************************************************/
/* function: wav::validWAV										*/
/* purpose: checks if the WAV file is valid						*/
/* args: void													*/
/* returns: bool												*/
/*		1 = valid header										*/
/*		0 = invalid header										*/
/****************************************************************/
bool wav::validWAV(void) const {
	return (validRIFF() && validFMT() && validFACT() && validDATA());
}

/****************************************************************/
/* function: wav::validRIFF										*/
/* purpose: checks if the RIFF header is valid					*/
/* args: void													*/
/* returns: bool												*/
/*		1 = valid header										*/
/*		0 = invalid header										*/
/****************************************************************/
bool wav::validRIFF(void) const {
	if (memcmp(riff.ChunkID, "RIFF", 4) != 0) {
		LOG_DEBUG("E: Invalid RIFF header: Format != 'RIFF'\n");
		LOG_DEBUG("\tFormat == %s\n", (char *)riff.ChunkID);
		return false;
	}
	if (memcmp(riff.Format, "WAVE", 4) != 0) {
		LOG_DEBUG("E: Invalid RIFF header: Format != 'WAVE'\n");
		LOG_DEBUG("\tFormat == %s\n", (char *)riff.Format);
		return false;
	}
	LOG_DEBUG("S: Valid RIFF header\n");
	return true;
}

/****************************************************************/
/* function: wav::validFMT										*/
/* purpose: checks if the FMT header is valid					*/
/* args: void													*/
/* returns: bool												*/
/*		1 = valid header										*/
/*		0 = invalid header										*/
/****************************************************************/
bool wav::validFMT(void) const {
	if (memcmp(fmt.SubchunkID, "fmt ", 4) != 0) {
		LOG_DEBUG("E: Invalid FMT header: Format != 'fmt '\n");
		LOG_DEBUG("\tFormat == %s\n", (char *)fmt.SubchunkID);
		return false;
	}
	// check encoding method
	if (fmt.AudioFormat != WAVE_FORMAT_PCM && fmt.AudioFormat != WAVE_FORMAT_IEEE_FLOAT) {
		LOG_DEBUG("E: Invalid FMT header: AudioFormat != '%d' (PCM) or '%d' (IEEE FLOAT)\n", WAVE_FORMAT_PCM, WAVE_FORMAT_IEEE_FLOAT);
		LOG_DEBUG("\tAudioFormat == %u\n", (unsigned int)fmt.AudioFormat);
		return false;
	}
	// check bits per sample
	if (fmt.AudioFormat == WAVE_FORMAT_PCM) {
		if (fmt.BitsPerSample != 16 && fmt.BitsPerSample != 8 && fmt.BitsPerSample != 24 && fmt.BitsPerSample != 32) {
			LOG_DEBUG("E: Invalid FMT header: Bits per sample = %u\n", (unsigned int)fmt.BitsPerSample);
			LOG_DEBUG("\tBits per sample == %u\n", (unsigned int)fmt.BitsPerSample);
			LOG_DEBUG("\tExpected Bits per sample to be '8', '16', '24', or 32\n");
			return false;
		}
	} else if (fmt.AudioFormat == WAVE_FORMAT_IEEE_FLOAT) {
		if (fmt.BitsPerSample != 32 && fmt.BitsPerSample != 64) {
			LOG_DEBUG("E: Invalid FMT header: Bits per sample = %u\n", (unsigned int)fmt.BitsPerSample);
			LOG_DEBUG("\tBits per sample == %u\n", (unsigned int)fmt.BitsPerSample);
			LOG_DEBUG("\tExpected Bits per sample to be '8', '16', '24', or 32\n");
			return false;
		}
	}
	// check channel count
	if (fmt.NumChannels > 2) {
		LOG_DEBUG("E: Invalid FMT header: Num channels > 2\n");
		LOG_DEBUG("\tNumChannels == %u\n", (unsigned int)fmt.NumChannels);
		return false;
	}

	#ifdef _DEBUGOUTPUT
	LOG_DEBUG("S: Valid FMT header\n");
	LOG_DEBUG("\tS: Bits per sample: %u\n", (unsigned int)fmt.BitsPerSample);
	LOG_DEBUG("\tS: Block align: %u\n", (unsigned int)fmt.BlockAlign);
	LOG_DEBUG("\tS: Byte rate: %u\n", (unsigned int)fmt.ByteRate);
	LOG_DEBUG("\tS: Num channels: %u\n", (unsigned int)fmt.NumChannels);
	LOG_DEBUG("\tS: Sample rate: %u\n", (unsigned int)fmt.SampleRate);
	if (fmt.SubchunkSize-16 != 0) {
		LOG_DEBUG("\tS: Extra format bytes: %u\n", (unsigned int)fmt.ExtraFormatBytes);
		if (fmt.ExtraFormatBytes == 22) {
			LOG_DEBUG("\tS: Valid bits per sample: %u\n", (unsigned int)fmt.ValidBitsPerSample);
			LOG_DEBUG("\tS: Channel mask: %u\n", (unsigned int)fmt.ChannelMask);
			LOG_DEBUG("\tS: Sub format: %s\n", (char *)fmt.SubFormat);
		}
	}
	#endif
	return true;
}

/****************************************************************/
/* function: wav::validFACT										*/
/* purpose: checks if the FACT header is valid					*/
/* args: void													*/
/* returns: bool												*/
/*		1 = valid header										*/
/*		0 = invalid header										*/
/****************************************************************/
bool wav::validFACT(void) const {
	if (fact == NULL) {
		return true;
	}
	if (memcmp(fact->SubchunkID, "fact", 4) != 0) {
		LOG_DEBUG("E: Invalid FACT header: Format != 'fact'\n");
		LOG_DEBUG("\tFormat == %s\n", (char *)fact->SubchunkID);
		return false;
	}
	LOG_DEBUG("S: Valid FACT header\n");
	return true;
}

/****************************************************************/
/* function: wav::validDATA										*/
/* purpose: checks if the DATA header is valid					*/
/* args: void													*/
/* returns: bool												*/
/*		1 = valid header										*/
/*		0 = invalid header										*/
/****************************************************************/
bool wav::validDATA(void) const {
	if (memcmp(data.SubchunkID, "data", 4) != 0) {
		LOG_DEBUG("E: Invalid DATA header: Format != 'data'\n");
		LOG_DEBUG("\tFormat == %s\n", (char *)data.SubchunkID);
		return false;
	}
	if (data.SubchunkSize == 0) {
		LOG_DEBUG("E: Invalid DATA header: No DATA\n");
		return false;
	}
	LOG_DEBUG("S: Valid DATA header\n");
	return true;
}

/****************************************************************/
/* function: encode												*/
/* purpose: open the files ment for encoding				 	*/
/* args: const char[], const char[], const char[]				*/
/* returns: unsigned long int									*/
/****************************************************************/
unsigned long int wav::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[]) {
	unsigned long int ret_val = 0;
	FILE *fInputWAV, *fInputDATA, *fOutputWAV;

	LOG("Opening input wave file...\n");
	// Open up our input wav file
	fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) { return false; }

	LOG("Validating input wave file...\n");
	// read and validate wave header (RIFF Chunk), and format chunk
	if (!(RIFFread(fInputWAV, this) && validWAV())) { close(fInputWAV); return false; }

	LOG("Opening input data file...\n");
	// Open up our input data file
	fInputDATA = open(inputDATA, "rb");
	if (fInputDATA == NULL) { close(fInputWAV); return false; }

	LOG("Opening output wav file...\n");
	// open up output wav file
	fOutputWAV = open(outputWAV, "wb");
	if (fOutputWAV == NULL) { close(fInputWAV); close(fInputDATA); return false; }

	LOG("Encoding data...\n");
	ret_val = encode(fInputWAV, fInputDATA, fOutputWAV);
	close(fInputWAV); close(fInputDATA); close(fOutputWAV);

	// clean up
	clean();
	return ret_val;
}

/****************************************************************/
/* function: encode												*/
/* purpose: do all necessary calculations and handle buffering 	*/
/* prerequisites: files are open; header data has been read		*/
/* args: FILE* FILE* FILE*										*/
/* returns: unsigned long int									*/
/****************************************************************/
unsigned long int wav::encode(FILE *fInputWAV, FILE *fInputDATA, FILE *fOutputWAV) {
	unsigned long int dataSize = 0, maxSize = 0;
	DWORD bytesPerSample = (fmt.BitsPerSample >> 3);
	BYTE *wavBuffer = NULL, *dataBuffer = NULL;
	BYTE bitsUsed = 0;
	size_t wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;

	// Get size of data file we want to encode
	fseek(fInputDATA, 0, SEEK_END);
	dataSize = ftell(fInputDATA);
	fseek(fInputDATA, 0, SEEK_SET);
	LOG_DEBUG("S: Determined input data file size (%.*f MB)\n", 3, byteToMB(dataSize));

	// get the maximum number of bytes the wav file could hold
	maxSize = getMaxBytesEncoded(fmt.BitsPerSample, data.SubchunkSize);
	if (dataSize > maxSize) {
		LOG_DEBUG("E: Data file is too large (Want to store %.*f MB - Can fit %.*f MB)\n", 3, byteToMB(dataSize), 3, byteToMB(maxSize));
		return false;
	}
	LOG_DEBUG("S: Data fits (Storing %.*f MB - Can Fit %.*f MB)\n", 3, byteToMB(dataSize), 3, byteToMB(maxSize));

	// get the minimum number of bits the wav file could encode per sample
	bitsUsed = getMinBitsEncodedPS(fmt.BitsPerSample, dataSize, maxSize);
	if (bitsUsed == 0 || bitsUsed > (fmt.BitsPerSample >> 1)) {
		LOG_DEBUG("E: This should never happen %d\n", (int)bitsUsed);
		return false;
	}
	LOG_DEBUG("S: Data file could fit at %d bits per sample\n", (int)bitsUsed);

	// Write our headers and how many bits used
	if (!RIFFwrite(fOutputWAV,this)) {
		return false;
	}

	// Calculate the size of our buffers
	maxWavBufferSize = BUFFER_MULT * (1024 * bytesPerSample);
	maxDataBufferSize = BUFFER_MULT * (128 * bitsUsed);

	// Get memory for our buffers
	if ((wavBuffer = (BYTE*)calloc(maxWavBufferSize, sizeof(BYTE))) == NULL) {
		LOG_DEBUG("E: Failed to get memory for WAV buffer\n");
		return false;
	}
	LOG_DEBUG("S: Got %u bytes for WAV buffer\n", (unsigned int)maxWavBufferSize);

	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize, sizeof(BYTE))) == NULL) {
		LOG_DEBUG("E: Failed to get memory for DATA buffer\n");
		free(wavBuffer);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	LOG_DEBUG("S: Got %u bytes for DATA buffer\n", (unsigned int)maxDataBufferSize);
	clock_t start = clock();
	#endif

	// read into the buffers, process, and write
	wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
	dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);

	// while there is data in the buffer encode and write to the file
	while (wavBufferSize != 0) {
		// encode and error out if it fails
		if ((dataBufferSize != 0) && !encode(bitsUsed, bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
			free(wavBuffer); free(dataBuffer);
			return false;
		}
		// write the changes to the file
		fwrite(wavBuffer, sizeof(BYTE), wavBufferSize, fOutputWAV);
		// get the next chunk of data
 		wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
		dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);
	}

	LOG_DEBUG("S: Took %.3f seconds to encode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
	LOG_DEBUG("S: Number of bytes stored: %u\n", (unsigned int)dataSize);
	free(wavBuffer); free(dataBuffer);
	return dataSize;
}

/****************************************************************/
/* function: encode												*/
/* purpose: encode data into the audio file using a buffer	 	*/
/* args: const BYTE, const DWORD, BYTE*, const size_t, BYTE*,	*/
/*		const size_t											*/
/* returns: bool												*/
/****************************************************************/
bool wav::encode(const BYTE bitsUsed, const DWORD bytesPerSample, BYTE *wavBuffer, const size_t wavBufferSize, BYTE *dataBuffer, const size_t dataBufferSize) {
	BYTE tempByte = 0x00;
	size_t count = 0x00;
	BYTE* currPos_WavBuffer = wavBuffer;
	BYTE* currPos_DataBuffer = dataBuffer;

	if (wavBufferSize == 0) {
		LOG_DEBUG("E: Invalid WAV buffer size\n");
		return false;
	}
	if (dataBufferSize == 0) {
		LOG_DEBUG("E: Invalid DATA buffer size\n");
		return false;
	}

	switch (bitsUsed) {
		case 1:
			while (count < dataBufferSize) {
				tempByte = *currPos_DataBuffer;
				for (char i = 7; i >= 0; i--) {
					setBit(*currPos_WavBuffer, 0, getBit(tempByte,i));
					currPos_WavBuffer += bytesPerSample;
				}
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 2:
			while (count < dataBufferSize) {	
				clearLower2Bits(*currPos_WavBuffer);
				tempByte = *currPos_DataBuffer;
				tempByte >>= 6;
				*currPos_WavBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;
				
				clearLower2Bits(*currPos_WavBuffer);
				tempByte = *currPos_DataBuffer;
				tempByte <<= 2;
				tempByte >>= 6;
				*currPos_WavBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				clearLower2Bits(*currPos_WavBuffer);
				tempByte = *currPos_DataBuffer;
				tempByte <<= 4;
				tempByte >>= 6;
				*currPos_WavBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				clearLower2Bits(*currPos_WavBuffer);
				tempByte = *currPos_DataBuffer;
				tempByte <<= 6;
				tempByte >>= 6;
				*currPos_WavBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				currPos_DataBuffer++;
				count++;
			}
			break;
		case 4:
			while (count < dataBufferSize) {
				tempByte = *currPos_DataBuffer;
				clearLower4Bits(*currPos_WavBuffer);
				tempByte >>= 4;
				*currPos_WavBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				tempByte = *currPos_DataBuffer;
				clearLower4Bits(*currPos_WavBuffer);
				clearUpper4Bits(tempByte);
				*currPos_WavBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;	
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 8:
			while (count < dataBufferSize) {
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 12: // Ugly and not very efficient
			while (count < dataBufferSize) {
				// first byte
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
				if (count < dataBufferSize) {
					// second byte
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer -= (bytesPerSample-1);
					currPos_DataBuffer++;
					count++;
					if (count < dataBufferSize) {
						// third byte
						tempByte = *currPos_DataBuffer;
						clearLower4Bits(*currPos_WavBuffer);
						tempByte >>= 4;
						*currPos_WavBuffer += tempByte;
						currPos_WavBuffer += bytesPerSample;

						tempByte = *currPos_DataBuffer;
						clearLower4Bits(*currPos_WavBuffer);
						clearUpper4Bits(tempByte);
						*currPos_WavBuffer += tempByte;
						currPos_WavBuffer += (bytesPerSample-1);	
						currPos_DataBuffer++;
						count++;
					}
				}
			}
			break;
		case 16:
			while (count < dataBufferSize) {
				// first byte
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				// second byte
				if (count < dataBufferSize) {
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer += (bytesPerSample - 1);
					currPos_DataBuffer++;
					count++;
				}
			}
			break;
		case 32:
			while (count < dataBufferSize) {
				// first byte
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				// second byte
				if (count < dataBufferSize) {
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer++;
					currPos_DataBuffer++;
					count++;
					// third byte
					if (count < dataBufferSize) {
						*currPos_WavBuffer = *currPos_DataBuffer;
						currPos_WavBuffer++;
						currPos_DataBuffer++;
						count++;
						// fourth byte
						if (count < dataBufferSize) {
							*currPos_WavBuffer = *currPos_DataBuffer;
							currPos_WavBuffer += (bytesPerSample - 3);
							currPos_DataBuffer++;
							count++;
						}
					}
				}
			}
			break;
		case 48:
			while (count < dataBufferSize) {
				// first byte
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				// second byte
				if (count < dataBufferSize) {
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer++;
					currPos_DataBuffer++;
					count++;
					// third byte
					if (count < dataBufferSize) {
						*currPos_WavBuffer = *currPos_DataBuffer;
						currPos_WavBuffer++;
						currPos_DataBuffer++;
						count++;
						// fourth byte
						if (count < dataBufferSize) {
							*currPos_WavBuffer = *currPos_DataBuffer;
							currPos_WavBuffer++;
							currPos_DataBuffer++;
							count++;
							// fifth byte
							if (count < dataBufferSize) {
								*currPos_WavBuffer = *currPos_DataBuffer;
								currPos_WavBuffer++;
								currPos_DataBuffer++;
								count++;
								// sixth byte
								if (count < dataBufferSize) {
									*currPos_WavBuffer = *currPos_DataBuffer;
									currPos_WavBuffer += (bytesPerSample - 5);
									currPos_DataBuffer++;
									count++;
								}
							}
						}
					}
				}
			}
			break;
		default:
			LOG_DEBUG("E: Invalid number of bits used (%hu)\n", (unsigned short)bitsUsed);
			return false;
	}
	return true;
}

/****************************************************************/
/* function: decode												*/
/* purpose: open the files ment for decoding				 	*/
/* args: const char[], const char[], const DWORD&				*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(const char inputWAV[], const char outputDATA[], const DWORD& fileSize) {
	FILE *fInputWAV, *fOutputDATA;
	bool ret_val = 0;

	LOG("Opening input wave file...\n");
	// Open up our input file
	fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) { return false; }

	LOG("Validating input wave file...\n");
	// read and validate wave header (RIFF Chunk), and format chunk
	if (!(RIFFread(fInputWAV, this) && validWAV())) { close(fInputWAV);  return false; }

	LOG("Opening output data file...\n");
	// open up our output file
	fOutputDATA = open(outputDATA, "wb");
	if (fOutputDATA == NULL) { close(fInputWAV); return false; }

	LOG("Decoding data...\n");
	ret_val = decode(fInputWAV, fOutputDATA, fileSize);

	// clean up
	close(fInputWAV); close(fOutputDATA);
	clean();
	return ret_val;
}

/****************************************************************/
/* function: decode												*/
/* purpose: do all necessary calculations and handle buffering 	*/
/* prerequisites: files are open; header data has been read		*/
/* args: const char[], const char[], const DWORD&				*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(FILE* fInputWAV, FILE* fOutputDATA, const DWORD& fileSize) {
	BYTE *wavBuffer = NULL, *dataBuffer = NULL;
	DWORD maxSize = 0, bytesPerSample = (fmt.BitsPerSample >> 3);
	BYTE bitsUsed = 0x00;
	size_t count = 0, wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;

	// get the maximum number of bytes the wav file could hold
	maxSize = getMaxBytesEncoded(fmt.BitsPerSample, data.SubchunkSize);
	if (fileSize > maxSize) {
		LOG_DEBUG("E: Data file is too large (Want to retrieve %.*f MB - Can retrieve %.*f MB)\n", 3, byteToMB(fileSize), 3, byteToMB(maxSize));
		return false;
	}
	LOG_DEBUG("S: Data fits (Retrieving %.*f MB - Can retrieve %.*f MB)\n", 3, byteToMB(fileSize), 3, byteToMB(maxSize));

	// get the minimum number of bits the wav file could encode per sample
	bitsUsed = getMinBitsEncodedPS(fmt.BitsPerSample, fileSize, maxSize);
	if (bitsUsed == 0 || bitsUsed > (fmt.BitsPerSample >> 1)) {
		LOG_DEBUG("E: This should never happen %d\n", (int)bitsUsed);
		return false;
	}
	LOG_DEBUG("S: Data file could fit at %d bits per sample\n", (int)bitsUsed);

	// Calculate the size of our buffers
	maxWavBufferSize = BUFFER_MULT * (1024 * bytesPerSample);
	maxDataBufferSize = BUFFER_MULT * (128 * bitsUsed);

	// Get memory for our buffers
	if ((wavBuffer = (BYTE*)calloc(maxWavBufferSize, sizeof(BYTE))) == NULL) {
		LOG_DEBUG("E: Failed to get memory for WAV buffer\n");
		return false;
	}
	LOG_DEBUG("S: Got %u bytes for WAV buffer\n", (unsigned int)maxWavBufferSize);

	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize, sizeof(BYTE))) == NULL) {
		LOG_DEBUG("E: Failed to get memory for DATA buffer\n");
		free(wavBuffer);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	LOG_DEBUG("S: Got %u bytes for DATA buffer\n", (unsigned int)maxDataBufferSize);
	clock_t start = clock();
	#endif

	// read into the buffers, process, and write
	wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
	count = dataBufferSize = maxDataBufferSize;

	while (count <= fileSize) {
		if (!decode(bitsUsed, bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
			free(wavBuffer); free(dataBuffer);
			return false;
		}

		fwrite(dataBuffer, sizeof(BYTE), dataBufferSize, fOutputDATA);

		if (count == fileSize)
			break;
	
 		wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
		
		if (count + maxDataBufferSize > fileSize) {
			dataBufferSize = fileSize - count;
			count = fileSize;
		} else {
			dataBufferSize = maxDataBufferSize;
			count += maxDataBufferSize;
		}
	}

	LOG_DEBUG("S: Took %.3f seconds to decode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
	LOG_DEBUG("S: Number of bytes retrieved: %u\n", (unsigned int)count);
	free(wavBuffer); free(dataBuffer);
	return true;
}

/****************************************************************/
/* function: decode												*/
/* purpose: decode data from the audio file that is in ram	 	*/
/* args: const BYTE, const DWORD, BYTE*, const size_t, BYTE*,	*/
/*		const size_t											*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(const BYTE bitsUsed, const DWORD bytesPerSample, BYTE *wavBuffer, const size_t wavBufferSize, BYTE *dataBuffer, const size_t dataBufferSize) {
	BYTE tempByte = 0x00;
	size_t count = 0x00;
	BYTE* currPos_WavBuffer = wavBuffer;
	BYTE* currPos_DataBuffer = dataBuffer;

	if (wavBufferSize == 0) {
		LOG_DEBUG("E: Invalid WAV buffer size\n");
		return false;
	}
	if (dataBufferSize == 0) {
		LOG_DEBUG("E: Invalid DATA buffer size\n");
		return false;
	}

	// Grab the bits from each sample, build a byte, and output the bytes to a file
	switch (bitsUsed) {
		case 1:
			while (count < dataBufferSize) {
				for (char j = 7; j >= 0; j--) {
					setBit(tempByte, j, getBit(*currPos_WavBuffer, 0));
					currPos_WavBuffer += bytesPerSample;
					
				}
				*currPos_DataBuffer = tempByte;
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 2:
			while (count < dataBufferSize) {
				*currPos_DataBuffer = 0;

				tempByte = *currPos_WavBuffer;
				tempByte <<= 6;
				*currPos_DataBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;
				
				tempByte = *currPos_WavBuffer;
				tempByte <<= 6;
				tempByte >>= 2;
				*currPos_DataBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				tempByte = *currPos_WavBuffer;
				tempByte <<= 6;
				tempByte >>= 4;
				*currPos_DataBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				tempByte = *currPos_WavBuffer;
				tempByte <<= 6;
				tempByte >>= 6;
				*currPos_DataBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				currPos_DataBuffer++;
				count++;
			}
			break;
		case 4:
			while (count < dataBufferSize) {
				tempByte = *currPos_WavBuffer;
				*currPos_DataBuffer = 0;
				tempByte <<= 4;
				*currPos_DataBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;

				tempByte = *currPos_WavBuffer;
				clearUpper4Bits(tempByte);
				*currPos_DataBuffer += tempByte;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 8:
			while (count < dataBufferSize) {
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 12: // Ugly and not very efficient
			while (count < dataBufferSize) {
				// first byte
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
				if (count < dataBufferSize) {
					// second byte
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer -= (bytesPerSample-1);
					currPos_DataBuffer++;
					count++;
					if (count < dataBufferSize) {
						// third byte
						tempByte = *currPos_WavBuffer;
						*currPos_DataBuffer = 0;
						tempByte <<= 4;
						*currPos_DataBuffer += tempByte;
						currPos_WavBuffer += bytesPerSample;

						tempByte = *currPos_WavBuffer;
						clearUpper4Bits(tempByte);
						*currPos_DataBuffer += tempByte;
						currPos_WavBuffer += (bytesPerSample-1);
						currPos_DataBuffer++;
						count++;
					}
				}
			}
			break;
		case 16:
			while (count < dataBufferSize) {
				// first byte
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				// second byte
				if (count < dataBufferSize) {
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer += (bytesPerSample - 1);
					currPos_DataBuffer++;
					count++;
				}
			}
			break;
		case 32:
			while (count < dataBufferSize) {
				// first byte
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				// second byte
				if (count < dataBufferSize) {
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer++;
					currPos_DataBuffer++;
					count++;
					// third byte
					if (count < dataBufferSize) {
						*currPos_DataBuffer = *currPos_WavBuffer;
						currPos_WavBuffer++;
						currPos_DataBuffer++;
						count++;
						// fourth byte
						if (count < dataBufferSize) {
							*currPos_DataBuffer = *currPos_WavBuffer;
							currPos_WavBuffer += (bytesPerSample - 3);
							currPos_DataBuffer++;
							count++;
						}
					}
				}
			}
			break;
		case 48:
			while (count < dataBufferSize) {
				// first byte
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				// second byte
				if (count < dataBufferSize) {
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer++;
					currPos_DataBuffer++;
					count++;
					// third byte
					if (count < dataBufferSize) {
						*currPos_DataBuffer = *currPos_WavBuffer;
						currPos_WavBuffer++;
						currPos_DataBuffer++;
						count++;
						// fourth byte
						if (count < dataBufferSize) {
							*currPos_DataBuffer = *currPos_WavBuffer;
							currPos_WavBuffer++;
							currPos_DataBuffer++;
							count++;
							// fifth byte
							if (count < dataBufferSize) {
								*currPos_DataBuffer = *currPos_WavBuffer;
								currPos_WavBuffer++;
								currPos_DataBuffer++;
								count++;
								// sixth byte
								if (count < dataBufferSize) {
									*currPos_DataBuffer = *currPos_WavBuffer;
									currPos_WavBuffer += (bytesPerSample - 5);
									currPos_DataBuffer++;
									count++;
								}
							}
						}
					}
				}
			}
			break;
		default:
			LOG_DEBUG("E: Invalid number of bits used (%hu)\n", (unsigned short)bitsUsed);
			return false;
	}
	return true;
}

/****************************************************************/
/* function: getMaxBytesEncoded									*/
/* purpose: calculate max number of bytes a WAV can encode	 	*/
/* args: const SHORT, const DWORD								*/
/* returns: DWORD												*/
/****************************************************************/
DWORD wav::getMaxBytesEncoded(const SHORT bitsPerSample, const DWORD subchunkSize) {
	DWORD maxSize, bytesPerSample = (bitsPerSample >> 3);

	// allows the use of only the bottom half of the bits per sample
	switch (bitsPerSample) {
		case 8:
			maxSize = (subchunkSize / bytesPerSample) >> 1;
			break;
		case 16:
			maxSize = (subchunkSize / bytesPerSample);
			break;
		case 24:
			maxSize = (subchunkSize / bytesPerSample);
			maxSize += maxSize >> 1;
			break;
		case 32:
			maxSize = (subchunkSize / bytesPerSample) << 1;
			break;
		case 64: // allows use of bottom 48 bits because 16 bits is enough audio data
			maxSize = (subchunkSize / bytesPerSample) << 2;
			maxSize += ((subchunkSize / bytesPerSample) << 1);
			break;
		default:
			maxSize = 0;
			break;
	}
	return maxSize;
}

/****************************************************************/
/* function: getMinBitsEncodedPS								*/
/* purpose: calculate min number of bits possibly encoded		*/
/*			per sample					 						*/
/* args: const SHORT, const DWORD, const DWORD					*/
/* returns: BYTE												*/
/****************************************************************/
BYTE wav::getMinBitsEncodedPS(const SHORT bitsPerSample, const DWORD fileSize, const DWORD maxSize) {
	double d_MinBPS = 0.0;
	int i_MinBPS = 0;

	if (fileSize == 0 || maxSize == 0)
		return 0;

	// 64 bits is allowed 3/4 of the bits for encoding. we need to account for that
	if(bitsPerSample != 64) {
		d_MinBPS = ((double)(bitsPerSample >> 1) / ((double)maxSize / (double)fileSize));
	} else {
		d_MinBPS = ((double)((bitsPerSample >> 2)*3) / ((double)maxSize / (double)fileSize));
	}

	i_MinBPS = (int)d_MinBPS;
	if (d_MinBPS > i_MinBPS)
		i_MinBPS++;

	if (i_MinBPS == 1)
		return 1;
	else if (i_MinBPS == 2)
		return 2;
	else if (i_MinBPS <= 4)
		return 4;
	else if (i_MinBPS <= 8)
		return 8;
	else if (i_MinBPS <= 12)
		return 12;
	else if (i_MinBPS <= 16)
		return 16;
	else if (i_MinBPS <= 32)
		return 32;
	else if (i_MinBPS <= 48)
		return 48;
	else
		return 0;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

