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
/* wav.cpp														*/
/****************************************************************/
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "riff.hpp"
#include "wav.hpp"
#include "file_compression.h"
#include "global.hpp"
#include "buffer.hpp"
#include "util.hpp"

/****************************************************************/
/* function: wav::wav											*/
/* purpose: constructor for the wav class						*/
/* args: void													*/
/****************************************************************/
wav::wav(void) {
	memset(&riff, 0, sizeof(_RIFF));
	memset(&fmt, 0, sizeof(_FMT));
	memset(&data, 0, sizeof(_DATA));
	buff_loc = 0;
	max_buff_loc = 0;
	bps = NULL;
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
		char temp[5];
		memcpy(temp,riff.Format,4);
		temp[5] = 0;
		fprintf(stderr,"\tFormat == %s\n",temp);
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
	if (bytencmp(fmt.SubchunkID, (BYTE*)"fmt ", 4) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid FMT header: SubchunkID != 'fmt '\n");
		char temp[5];
		memcpy(temp,fmt.SubchunkID,4);
		temp[5] = 0;
		fprintf(stderr,"\tSubchunkID == %s\n",temp);
		#endif
		return false;
	} else if (fmt.AudioFormat != WAVE_FORMAT_PCM) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid FMT header: AudioFormat != '1' (PCM)\n");
		fprintf(stderr,"\tAudioFormat == %u\n",(unsigned int)fmt.AudioFormat);
		#endif
		return false;
	} else if (fmt.BitsPerSample != 16 && fmt.BitsPerSample != 8 && fmt.BitsPerSample != 24 && fmt.BitsPerSample != 32) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid FMT header: Bits per sample = %u\n",(unsigned int)fmt.BitsPerSample);
		fprintf(stderr,"\tBits per sample == %u\n",(unsigned int)fmt.BitsPerSample);
		fprintf(stderr,"\tExpected Bits per sample to be '8', '16', '24', or 32\n");
		#endif
		return false;
	} else if (fmt.NumChannels != 2) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid FMT header: Num channels != '2'\n");
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
	if (bytencmp(data.SubchunkID, (BYTE*)"data", 4) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid DATA header: SubchunkID != 'data'\n");
		char temp[5];
		memcpy(temp,data.SubchunkID,4);
		temp[5] = 0;
		fprintf(stderr,"\tSubchunkID == %s\n",temp);
		#endif
		return false;
	} else if (data.SubchunkSize == 0) {
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
/* function: wav::readRIFF										*/
/* purpose: reads the riff header from a wav file				*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
bool wav::readRIFF(FILE* inFile) {
	if (fread(riff.ChunkID, sizeof(BYTE), 4, inFile) &&
		fread(&riff.ChunkSize, sizeof(DWORD), 1, inFile) &&
		fread(riff.Format, sizeof(BYTE), 4, inFile))
	{
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Read RIFF header\n");
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to read RIFF header: Could not read bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: wav::readFMT										*/
/* purpose: reads the fmt header from a wav file				*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
bool wav::readFMT(FILE* inFile) {
	if (fread(fmt.SubchunkID, sizeof(BYTE), 4, inFile) &&
		fread(&fmt.SubchunkSize, sizeof(DWORD), 1, inFile) &&
		fread(&fmt.AudioFormat, sizeof(SHORT), 1, inFile) &&
		fread(&fmt.NumChannels, sizeof(SHORT), 1, inFile) &&
		fread(&fmt.SampleRate, sizeof(DWORD), 1, inFile) &&
		fread(&fmt.ByteRate, sizeof(DWORD), 1, inFile) &&
		fread(&fmt.BlockAlign, sizeof(SHORT), 1, inFile) &&
		fread(&fmt.BitsPerSample, sizeof(SHORT), 1, inFile))
	{
		// Need to read extra stuff
		if (fmt.SubchunkSize-16 != 0) {
			fread(&fmt.ExtraFormatBytes, sizeof(SHORT), 1, inFile);
			if (fmt.ExtraFormatBytes > 0) {
				if ((fmt.ExtraFormat = (BYTE*)malloc(fmt.ExtraFormatBytes)) == NULL) {
					#ifdef _DEBUGOUTPUT
					fprintf(stderr,"E: Failed to read FMT header: Could not get memory for extra bytes\n");
					#endif
					return false;
				}
				fread(fmt.ExtraFormat, (fmt.ExtraFormatBytes), 1, inFile);
			}
		}

		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Read FMT header\n");
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to read FMT header: Could not read bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: wav::readDATA										*/
/* purpose: reads the data from a wav file						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
bool wav::readDATA(FILE* inFile) {
	DWORD size;
	BYTE id[4];

	fread(id, sizeof(BYTE), 4, inFile);
	fread(&size, sizeof(DWORD), 1, inFile);

	if (bytencmp(id, (BYTE*)"data", 4) == 0) {
		data.SubchunkID[0] = 'd'; data.SubchunkID[1] = 'a'; data.SubchunkID[2] = 't'; data.SubchunkID[3] = 'a';
		data.SubchunkSize = size;

		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Read DATA header\n");
		#endif
		return true;
	}

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to read DATA header: Could not locate");
	#endif
	return false;
}

/****************************************************************/
/* function: wav::read											*/
/* purpose: reads a wav file into memory						*/
/* args: const char[]											*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
bool wav::read(FILE *inFile) {
	/* read and validate wave header (RIFF Chunk), format chunk, and DATA */
	if ( (readRIFF(inFile) && validRIFF() && readFMT(inFile) && validFMT() && readDATA(inFile) && validDATA())) {
		max_buff_loc = (data.SubchunkSize / (fmt.BitsPerSample/8));
		bps = &fmt.BitsPerSample;
		return true;
	}
	return false;
}

/****************************************************************/
/* function: wav::writeRIFF										*/
/* purpose: writes the riff header to a file					*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
bool wav::writeRIFF(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write RIFF header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(riff.ChunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&riff.ChunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(riff.Format, sizeof(BYTE), 4, outFile))
	{
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Wrote RIFF header\n");
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to write RIFF header: Could not write bytesn\n");
	#endif
	return false;
}

/****************************************************************/
/* function: wav::writeFMT										*/
/* purpose: writes the fmt header to a file						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
bool wav::writeFMT(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write FMT header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(fmt.SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&fmt.SubchunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(&fmt.AudioFormat, sizeof(SHORT), 1, outFile) &&
		fwrite(&fmt.NumChannels, sizeof(SHORT), 1, outFile) &&
		fwrite(&fmt.SampleRate, sizeof(DWORD), 1, outFile) &&
		fwrite(&fmt.ByteRate, sizeof(DWORD), 1, outFile) &&
		fwrite(&fmt.BlockAlign, sizeof(SHORT), 1, outFile) &&
		fwrite(&fmt.BitsPerSample, sizeof(SHORT), 1, outFile))
	{
		// Need to write extra stuff
		if (fmt.SubchunkSize-16 != 0) {
			fwrite(&fmt.ExtraFormatBytes, sizeof(SHORT), 1, outFile);
			if (fmt.ExtraFormatBytes > 0) {
				fwrite(fmt.ExtraFormat, fmt.ExtraFormatBytes, 1, outFile);
			}
		}
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Wrote FMT header\n");
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to write FMT header: Could not write bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: wav::writeDATA										*/
/* purpose: writes the data header to a file					*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
bool wav::writeDATA(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write DATA header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(data.SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&data.SubchunkSize, sizeof(DWORD), 1, outFile))
	{
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Wrote DATA header\n");
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to write DATA header: Could not write bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: encode												*/
/* purpose: open the files ment for encoding				 	*/
/* args: const char[], const char[], const char[], const char	*/
/* returns: DWORD												*/
/****************************************************************/
DWORD wav::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[], const char compressionLevel) {
	FILE *fInputWAV, *fInputDATA, *fOutputWAV, *fCompDATA;
	DWORD ret_val = 0;

	/* Open up our input files */
	fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) { return false; }
	fInputDATA = open(inputDATA, "rb");
	if (fInputDATA == NULL) { close(fInputWAV); return false; }

	/* read and validate wave header (RIFF Chunk), and format chunk */
	if (!read(fInputWAV)) { close(fInputWAV); close(fInputDATA); return false; }

	/* open up output file */
	fOutputWAV = open(outputWAV, "wb");
	if (fOutputWAV == NULL) { close(fInputWAV); close(fInputDATA); return false; }

	/* if we are compressing */
	if (compressionLevel > 0) {
		fCompDATA = open("data.z", "wb");

		/* open a temp file and compress */
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not open temp data file.\n");
			#endif
			close(fInputWAV); close(fInputDATA); close(fOutputWAV);
			return false;
		}
		if( def(fInputDATA, fCompDATA, compressionLevel) != 0) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not compress data file.\n");
			#endif
			close(fInputWAV); close(fInputDATA); close(fOutputWAV); close(fCompDATA);
			return false;
		}
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Compressed input data.\n");
		#endif

		/* clean up and reopen data.z in read mode */
		close(fInputDATA);
		close(fCompDATA);
		fInputDATA = open("data.z","rb");
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not re-open temp data file.\n");
			#endif
			close(fInputWAV); close(fInputDATA); close(fOutputWAV);
			return false;
		}
		
		ret_val = encode(fInputWAV, fInputDATA, fOutputWAV);
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);

		if (remove("data.z") == -1) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not remove temporary file data.z\n");
			#endif
		}
	} else {
		ret_val = encode(fInputWAV, fInputDATA, fOutputWAV);
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);
	}
	
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
	if (!writeRIFF(fOutputWAV) || !writeFMT(fOutputWAV) || !writeDATA(fOutputWAV)) { return false; }

	/* Calculate the size of our buffers */
	maxWavBufferSize = 1024 * bytesPerSample;
	maxDataBufferSize = 128 * bitsUsed;

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
	fprintf(stderr,"S: Number of bytes stored: %u\n",(unsigned int)dataSize);
	#endif
	free(wavBuffer); free(dataBuffer);
	return dataSize;
}


/****************************************************************/
/* function: encode												*/
/* purpose: encode data into the audio file using a buffer	 	*/
/* args: BYTE, DWORD, BYTE*, size_t, BYTE*, size_t				*/
/* returns: bool												*/
/****************************************************************/
bool wav::encode(BYTE bitsUsed, DWORD bytesPerSample, BYTE *wavBuffer, size_t wavBufferSize, BYTE *dataBuffer, size_t dataBufferSize) {
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
				tempByte = *currPos_DataBuffer;
				for (char i = 3; i >= 0; i--) {
					setBit(*currPos_WavBuffer, 1, getBit(tempByte, i*2 + 1));
					setBit(*currPos_WavBuffer, 0, getBit(tempByte, i*2));
					currPos_WavBuffer += bytesPerSample;
				}
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 4:
			while (count < dataBufferSize) {
				tempByte = *currPos_DataBuffer;
				for (char i = 1; i >= 0; i--) {
					setBit(*currPos_WavBuffer, 3, getBit(tempByte, i*4 + 3));
					setBit(*currPos_WavBuffer, 2, getBit(tempByte, i*4 + 2));
					setBit(*currPos_WavBuffer, 1, getBit(tempByte, i*4 + 1));
					setBit(*currPos_WavBuffer, 0, getBit(tempByte, i*4));
					currPos_WavBuffer += bytesPerSample;
				}
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
				/* second byte */
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer -= (bytesPerSample-1);
				currPos_DataBuffer++;
				count++;
				/* third byte */
				tempByte = *currPos_DataBuffer;
				setBit(*currPos_WavBuffer, 3, getBit(tempByte, 3));
				setBit(*currPos_WavBuffer, 2, getBit(tempByte, 2));
				setBit(*currPos_WavBuffer, 1, getBit(tempByte, 1));
				setBit(*currPos_WavBuffer, 0, getBit(tempByte, 0));
				currPos_WavBuffer += bytesPerSample;
				setBit(*currPos_WavBuffer, 3, getBit(tempByte, 7));
				setBit(*currPos_WavBuffer, 2, getBit(tempByte, 6));
				setBit(*currPos_WavBuffer, 1, getBit(tempByte, 5));
				setBit(*currPos_WavBuffer, 0, getBit(tempByte, 4));
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
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
	FILE *fInputWAV, *fOutputDATA, *fCompDATA;
	bool ret_val = 0;

	/* Open up our input file */
	fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) { return false; }

	/* read and validate wave header (RIFF Chunk), and format chunk */
	if (!read(fInputWAV)) { close(fInputWAV);  return false; }

	/* open up our output file */
	fOutputDATA = open(outputDATA, "wb");
	if (fOutputDATA == NULL) { close(fInputWAV); return false; }

	/* if we are compressing */
	if (compress > 0) {
		fCompDATA = open("data.z", "wb");

		/* open the temp file */
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not open temp data file.\n");
			#endif
			close(fInputWAV); close(fOutputDATA);
			return false;
		}

		ret_val = decode(fInputWAV, fCompDATA, fileSize);
		close(fCompDATA);
		if (!ret_val) { close(fInputWAV); close(fOutputDATA); return ret_val; }

		/* re-open the temp file in read mode */
		fCompDATA = open("data.z", "rb");
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not re-open temp data file.\n");
			#endif
			close(fInputWAV); close(fOutputDATA);
			return false;
		}

		/* decompress */
		if( inf(fCompDATA, fOutputDATA) != 0) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not decompress data file.\n");
			#endif
			close(fInputWAV); close(fOutputDATA); close(fCompDATA);
			return false;
		}
		close(fCompDATA);
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Decompressed input data.\n");
		#endif

		if( remove("data.z") == -1) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not remove temporary file data.z\n");
			#endif
		}
	} else {
		ret_val = decode(fInputWAV, fOutputDATA, fileSize);
	}

	close(fInputWAV); close(fOutputDATA);
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
	DWORD maxSize = 0, bytesPerSample = (fmt.BitsPerSample/8), count = 0;
	BYTE bitsUsed = 0x00;
	size_t wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;

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
	maxWavBufferSize = 1024 * bytesPerSample;
	maxDataBufferSize = 128 * bitsUsed;

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
	fprintf(stderr,"S: Number of bytes retrieved: %u\n",(unsigned int)count);
	#endif
	free(wavBuffer); free(dataBuffer);
	return true;
}

/****************************************************************/
/* function: decode												*/
/* purpose: decode data from the audio file that is in ram	 	*/
/* args: BYTE, DWORD, BYTE*, size_t, BYTE*, size_t				*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(BYTE bitsUsed, DWORD bytesPerSample, BYTE *wavBuffer, size_t wavBufferSize, BYTE *dataBuffer, size_t dataBufferSize) {
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
				for (char j = 3; j >= 0; j--) {
					setBit(tempByte, j*2 + 1, getBit(*currPos_WavBuffer, 1));
					setBit(tempByte, j*2, getBit(*currPos_WavBuffer, 0));
					currPos_WavBuffer += bytesPerSample;
				}
				*currPos_DataBuffer = tempByte;
				currPos_DataBuffer++;
				count++;
			}
			break;
		case 4:
			while (count < dataBufferSize) {			
				for (char j = 1; j >= 0; j--) {
					setBit(tempByte, j*4 + 3, getBit(*currPos_WavBuffer, 3));
					setBit(tempByte, j*4 + 2, getBit(*currPos_WavBuffer, 2));
					setBit(tempByte, j*4 + 1, getBit(*currPos_WavBuffer, 1));
					setBit(tempByte, j*4, getBit(*currPos_WavBuffer, 0));
					currPos_WavBuffer += bytesPerSample;
				}
				*currPos_DataBuffer = tempByte;
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
				/* second byte */
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer -= (bytesPerSample-1);
				currPos_DataBuffer++;
				count++;
				/* third byte */
				setBit(tempByte, 3, getBit(*currPos_WavBuffer, 3));
				setBit(tempByte, 2, getBit(*currPos_WavBuffer, 2));
				setBit(tempByte, 1, getBit(*currPos_WavBuffer, 1));
				setBit(tempByte, 0, getBit(*currPos_WavBuffer, 0));
				currPos_WavBuffer += bytesPerSample;
				setBit(tempByte, 7, getBit(*currPos_WavBuffer, 3));
				setBit(tempByte, 6, getBit(*currPos_WavBuffer, 2));
				setBit(tempByte, 5, getBit(*currPos_WavBuffer, 1));
				setBit(tempByte, 4, getBit(*currPos_WavBuffer, 0));
				*currPos_DataBuffer = tempByte;
				currPos_WavBuffer++;
				currPos_DataBuffer++;
				count++;
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
	else
		return 0;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
