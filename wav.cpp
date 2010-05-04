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
#include <math.h>
#include <string.h>
#include "riff.hpp"
#include "wav.hpp"
#include "global.hpp"
#include "util.hpp"
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
/****************************************************************/
void wav::clean(void) {
	if(fact != NULL) {
		free(fact);
		fact = NULL;
	}
	if(peak != NULL) {
		if(peak->peak != NULL) {
			free(peak->peak);
			peak->peak = NULL;
		}
		if(peak->bit_align != NULL) {
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
	if (!(validRIFF() && validFMT() && validFACT() && validDATA())) {
		return false;
	}
	return true;
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
	if (bytencmp(riff.Format, (BYTE*)"WAVE", 4) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid RIFF header: Format != 'WAVE'\n");
		fprintf(stderr,"\tFormat == %s\n",(char*)riff.Format);
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Valid RIFF header\n");
	#endif
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
	/* check encoding method */
	if (fmt.AudioFormat != WAVE_FORMAT_PCM && fmt.AudioFormat != WAVE_FORMAT_IEEE_FLOAT) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid FMT header: AudioFormat != '%d' (PCM) or '%d' (IEEE FLOAT)\n",WAVE_FORMAT_PCM, WAVE_FORMAT_IEEE_FLOAT);
		fprintf(stderr,"\tAudioFormat == %u\n",(unsigned int)fmt.AudioFormat);
		#endif
		return false;
	}
	/* check bits per sample */
	if (fmt.AudioFormat == WAVE_FORMAT_PCM) {
		if (fmt.BitsPerSample != 16 && fmt.BitsPerSample != 8 && fmt.BitsPerSample != 24 && fmt.BitsPerSample != 32) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Invalid FMT header: Bits per sample = %u\n",(unsigned int)fmt.BitsPerSample);
			fprintf(stderr,"\tBits per sample == %u\n",(unsigned int)fmt.BitsPerSample);
			fprintf(stderr,"\tExpected Bits per sample to be '8', '16', '24', or 32\n");
			#endif
			return false;
		}
	} else if (fmt.AudioFormat == WAVE_FORMAT_IEEE_FLOAT) {
		if (fmt.BitsPerSample != 32 && fmt.BitsPerSample != 64) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Invalid FMT header: Bits per sample = %u\n",(unsigned int)fmt.BitsPerSample);
			fprintf(stderr,"\tBits per sample == %u\n",(unsigned int)fmt.BitsPerSample);
			fprintf(stderr,"\tExpected Bits per sample to be '8', '16', '24', or 32\n");
			#endif
			return false;
		}
	}
	/* check channel count */
	if (fmt.NumChannels > 2) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid FMT header: Num channels > 2\n");
		fprintf(stderr,"\tNumChannels == %u\n",(unsigned int)fmt.NumChannels);
		#endif
		return false;
	}

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Valid FMT header\n");
	fprintf(stderr,"\tS: Bits per sample: %u\n",(unsigned int)fmt.BitsPerSample);
	fprintf(stderr,"\tS: Block align: %u\n",(unsigned int)fmt.BlockAlign);
	fprintf(stderr,"\tS: Byte rate: %u\n",(unsigned int)fmt.ByteRate);
	fprintf(stderr,"\tS: Num channels: %u\n",(unsigned int)fmt.NumChannels);
	fprintf(stderr,"\tS: Sample rate: %u\n",(unsigned int)fmt.SampleRate);
	if (fmt.SubchunkSize-16 != 0) {
		fprintf(stderr,"\tS: Extra format bytes: %u\n",(unsigned int)fmt.ExtraFormatBytes);
		if(fmt.ExtraFormatBytes == 22) {
			fprintf(stderr,"\tS: Valid bits per sample: %u\n",(unsigned int)fmt.ValidBitsPerSample);
			fprintf(stderr,"\tS: Channel mask: %u\n",(unsigned int)fmt.ChannelMask);
			fprintf(stderr,"\tS: Sub format: %s\n",(char *)fmt.SubFormat);
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
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Valid FACT header\n");
	#endif
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
	if (data.SubchunkSize == 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid DATA header: No DATA\n");
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Valid DATA header\n");
	#endif
	return true;
}

/****************************************************************/
/* function: encode												*/
/* purpose: open the files ment for encoding				 	*/
/* args: const char[], const char[], const char[], const char	*/
/* returns: DWORD												*/
/****************************************************************/
DWORD wav::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[], const char compressionLevel) {
	FILE *fInputWAV, *fInputDATA, *fOutputWAV;
	DWORD ret_val = 0;

	/* Open up our input files */
	fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) { return false; }
	fInputDATA = open(inputDATA, "rb");
	if (fInputDATA == NULL) { close(fInputWAV); return false; }

	/* read and validate wave header (RIFF Chunk), and format chunk */
	if (!(RIFFread(fInputWAV,this) && validWAV())) { close(fInputWAV); close(fInputDATA); return false; }

	/* open up output file */
	fOutputWAV = open(outputWAV, "wb");
	if (fOutputWAV == NULL) { close(fInputWAV); close(fInputDATA); return false; }

	ret_val = encode(fInputWAV, fInputDATA, fOutputWAV);
	close(fInputWAV); close(fInputDATA); close(fOutputWAV);

	/* clean up */
	clean();
	return ret_val;
}

/****************************************************************/
/* function: encode												*/
/* purpose: do all necessary calculations and handle buffering 	*/
/* prerequisites: files are open; header data has been read		*/
/* args: FILE* FILE* FILE*										*/
/* returns: DWORD												*/
/****************************************************************/
DWORD wav::encode(FILE *fInputWAV, FILE *fInputDATA, FILE *fOutputWAV) {
	BYTE *wavBuffer = NULL, *dataBuffer = NULL;
	DWORD dataSize = 0, maxSize = 0, bytesPerSample = (fmt.BitsPerSample/8);
	BYTE bitsUsed = 0;
	size_t wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;

	/* Get size of data file we want to encode */
	fseek(fInputDATA, 0, SEEK_END);
	dataSize = ftell(fInputDATA);
	fseek(fInputDATA, 0, SEEK_SET);
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Determined input data file size (%.*f MB)\n",3,byteToMB(dataSize));
	#endif

	/* get the maximum number of bytes the wav file could hold */
	maxSize = getMaxBytesEncoded(fmt.BitsPerSample, data.SubchunkSize);
	if (dataSize > maxSize) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Data file is too large (Want to store %.*f MB - Can fit %.*f MB)\n",3,byteToMB(dataSize),3,byteToMB(maxSize));
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data fits (Storing %.*f MB - Can Fit %.*f MB)\n",3,byteToMB(dataSize),3,byteToMB(maxSize));
	#endif

	/* get the minimum number of bits the wav file could encode per sample */
	bitsUsed = getMinBitsEncodedPS(fmt.BitsPerSample, dataSize, maxSize);
	if (bitsUsed == 0 || bitsUsed > (fmt.BitsPerSample >> 1)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: This should never happen %d\n",(int)bitsUsed);
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data file could fit at %d bits per sample\n",(int)bitsUsed);
	#endif

	/* Write our headers and how many bits used */
	if( !RIFFwrite(fOutputWAV,this)) {
		return false;
	}

	/* Calculate the size of our buffers */
	maxWavBufferSize = BUFFER_MULT*(1024 * bytesPerSample);
	maxDataBufferSize = BUFFER_MULT*(128 * bitsUsed);

	/* Get memory for our buffers */
	if ((wavBuffer = (BYTE*)calloc(maxWavBufferSize, sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for WAV buffer\n");
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for WAV buffer\n",(unsigned int)maxWavBufferSize);
	#endif

	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize,sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for DATA buffer\n");
		#endif
		free(wavBuffer);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for DATA buffer\n",(unsigned int)maxDataBufferSize);
	clock_t start = clock();
	#endif

	/* read into the buffers, process, and write */
	wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
	dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);

	/* while there is data in the buffer encode and write to the file*/
	while (wavBufferSize != 0) {
		/* encode and error out if it fails */
		if ((dataBufferSize != 0) && !encode(bitsUsed, bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
			free(wavBuffer); free(dataBuffer);
			return false;
		}
		/* write the changes to the file */
		fwrite(wavBuffer, sizeof(BYTE), wavBufferSize, fOutputWAV);
		/* get the next chunk of data */
 		wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
		dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);
	}

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Took %.3f seconds to encode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
	fprintf(stderr,"S: Number of bytes stored: %u\n",(unsigned int)dataSize);
	#endif
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

	if ((wavBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid WAV buffer size\n");
		#endif
		return false;
	}
	if ((dataBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid DATA buffer size\n");
		#endif
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
				/* first byte */
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
				if( count < dataBufferSize ) {
					/* second byte */
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer -= (bytesPerSample-1);
					currPos_DataBuffer++;
					count++;
					if( count < dataBufferSize ) {
						/* third byte */
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
				/* first byte */
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				/* second byte */
				if(count < dataBufferSize) {
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer += (bytesPerSample - 1);
					currPos_DataBuffer++;
					count++;
				}
			}
			break;
		case 32:
			while (count < dataBufferSize) {
				/* first byte */
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				/* second byte */
				if(count < dataBufferSize) {
					*currPos_WavBuffer = *currPos_DataBuffer;
					currPos_WavBuffer++;
					currPos_DataBuffer++;
					count++;
					/* third byte */
					if(count < dataBufferSize) {
						*currPos_WavBuffer = *currPos_DataBuffer;
						currPos_WavBuffer++;
						currPos_DataBuffer++;
						count++;
						/* fourth byte */
						if(count < dataBufferSize) {
							*currPos_WavBuffer = *currPos_DataBuffer;
							currPos_WavBuffer += (bytesPerSample - 3);
							currPos_DataBuffer++;
							count++;
						}
					}
				}
			}
			break;
		default:
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Invalid number of bits used (%hu)\n",(unsigned short)bitsUsed);
			#endif
			return false;
	}
	return true;
}


/****************************************************************/
/* function: decode												*/
/* purpose: open the files ment for decoding				 	*/
/* args: const char[], const char[], const DWORD&, const char	*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(const char inputWAV[], const char outputDATA[], const DWORD& fileSize, const char compress) {
	FILE *fInputWAV, *fOutputDATA;
	bool ret_val = 0;

	/* Open up our input file */
	fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) { return false; }

	/* read and validate wave header (RIFF Chunk), and format chunk */
	if (!(RIFFread(fInputWAV,this) && validWAV())) { close(fInputWAV);  return false; }

	/* open up our output file */
	fOutputDATA = open(outputDATA, "wb");
	if (fOutputDATA == NULL) { close(fInputWAV); return false; }

	ret_val = decode(fInputWAV, fOutputDATA, fileSize);

	/* clean up */
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
	DWORD maxSize = 0, bytesPerSample = (fmt.BitsPerSample/8);
	BYTE bitsUsed = 0x00;
	size_t count = 0, wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;

	/* get the maximum number of bytes the wav file could hold */
	maxSize = getMaxBytesEncoded(fmt.BitsPerSample, data.SubchunkSize);
	if (fileSize > maxSize) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Data file is too large (Want to retrieve %.*f MB - Can retrieve %.*f MB)\n",3,byteToMB(fileSize),3,byteToMB(maxSize));
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data fits (Retrieving %.*f MB - Can retrieve %.*f MB)\n",3,byteToMB(fileSize),3,byteToMB(maxSize));
	#endif

	/* get the minimum number of bits the wav file could encode per sample */
	bitsUsed = getMinBitsEncodedPS(fmt.BitsPerSample, fileSize, maxSize);
	if (bitsUsed == 0 || bitsUsed > (fmt.BitsPerSample >> 1)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: This should never happen %d\n",(int)bitsUsed);
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data file could fit at %d bits per sample\n",(int)bitsUsed);
	#endif

	/* Calculate the size of our buffers */
	maxWavBufferSize = BUFFER_MULT*(1024 * bytesPerSample);
	maxDataBufferSize = BUFFER_MULT*(128 * bitsUsed);

	/* Get memory for our buffers */
	if ((wavBuffer = (BYTE*)calloc(maxWavBufferSize, sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for WAV buffer\n");
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for WAV buffer\n",(unsigned int)maxWavBufferSize);
	#endif

	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize,sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for DATA buffer\n");
		#endif
		free(wavBuffer);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for DATA buffer\n",(unsigned int)maxDataBufferSize);
	clock_t start = clock();
	#endif

	/* read into the buffers, process, and write */
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

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Took %.3f seconds to decode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
	fprintf(stderr,"S: Number of bytes retrieved: %u\n",(unsigned int)count);
	#endif
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

	if ((wavBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid WAV buffer size\n");
		#endif
		return false;
	}
	if ((dataBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid DATA buffer size\n");
		#endif
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
				/* first byte */
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
				if( count < dataBufferSize ) {
					/* second byte */
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer -= (bytesPerSample-1);
					currPos_DataBuffer++;
					count++;
					if( count < dataBufferSize ) {
						/* third byte */
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
				/* first byte */
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				/* second byte */
				if(count < dataBufferSize) {
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer += (bytesPerSample - 1);
					currPos_DataBuffer++;
					count++;
				}
			}
			break;
		case 32:
			while (count < dataBufferSize) {
				/* first byte */
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
				/* second byte */
				if(count < dataBufferSize) {
					*currPos_DataBuffer = *currPos_WavBuffer;
					currPos_WavBuffer++;
					currPos_DataBuffer++;
					count++;
					/* third byte */
					if(count < dataBufferSize) {
						*currPos_DataBuffer = *currPos_WavBuffer;
						currPos_WavBuffer++;
						currPos_DataBuffer++;
						count++;
						/* fourth byte */
						if(count < dataBufferSize) {
							*currPos_DataBuffer = *currPos_WavBuffer;
							currPos_WavBuffer += (bytesPerSample - 3);
							currPos_DataBuffer++;
							count++;
						}
					}
				}
			}
			break;
		default:
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Invalid number of bits used (%hu)\n",(unsigned short)bitsUsed);
			#endif
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
	DWORD maxSize, bytesPerSample = (bitsPerSample/8);

	/* allows the use of only the bottom half of the bits per sample */
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
		case 64:
			maxSize = (subchunkSize / bytesPerSample) << 2;
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
	if (fileSize == 0 || maxSize == 0)
		return 0;

	double d_MinBPS = ((double)(bitsPerSample >> 1) / ((double)maxSize / (double)fileSize));
	int i_MinBPS = (int)d_MinBPS;
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
	else
		return 0;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

