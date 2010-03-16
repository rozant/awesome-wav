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
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "riff.hpp"
#include "wav.hpp"
#include "buffer.hpp"
#include "byteOperations.hpp"
#include "fileOperations.hpp"

using namespace std;

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
		cout << "E: Invalid RIFF header: Format != 'WAVE'" << endl;
		char temp[5];
		memcpy(temp,riff.Format,4);
		temp[5] = 0;
		cout << "\tFORMAT == " << temp << endl;
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Valid RIFF header" << endl;
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
		cout << "E: Invalid FMT header: SubchunkID != 'fmt '" << endl;
		#endif
		return false;
	} else if (fmt.AudioFormat != 1) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid FMT header: AudioFormat != '1' (PCM)" << endl;
		#endif
		return false;
	} else if (fmt.BitsPerSample != 16 && fmt.BitsPerSample != 8 && fmt.BitsPerSample != 24) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid FMT header: Bits per sample = " << fmt.BitsPerSample << endl;
		cout << "\tExpected Bits per sample to be '8', '16', or '24'" << endl;
		#endif
		return false;
	} else if (fmt.NumChannels != 2) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid FMT header: Num channels != '2'" << endl;
		#endif
		return false;
	}

	#ifdef _DEBUGOUTPUT
	cout << "S: Valid FMT header" << endl;
	cout << "\t S: Bits per sample: " << fmt.BitsPerSample << endl;
	cout << "\t S: Block align: " << fmt.BlockAlign << endl;
	cout << "\t S: Byte rate: " << fmt.ByteRate << endl;
	cout << "\t S: Num channels: " << fmt.NumChannels << endl;
	cout << "\t S: Sample rate: " << fmt.SampleRate << endl;
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
		cout << "E: Invalid DATA header: SubchunkID != 'data'" << endl;
		#endif
		return false;
	} else if (data.SubchunkSize == 0) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid DATA header: No DATA" << endl;
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Valid DATA header" << endl;
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
		cout << "S: Read RIFF header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to read RIFF header: Could not read bytes" << endl;
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
					cout << "E: Failed to read FMT header: Could not get memory for extra bytes" << endl;
					#endif
					return false;
				}
				fread(fmt.ExtraFormat, (fmt.ExtraFormatBytes), 1, inFile);
			}
		}

		#ifdef _DEBUGOUTPUT
		cout << "S: Read FMT header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to read FMT header: Could not read bytes" << endl;
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
		cout << "S: Read DATA header" << endl;
		#endif
		return true;
	}

	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to read DATA header: Could not locate" << endl;
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
		cout << "E: Failed to write RIFF header: FILE not open" << endl;
		#endif
		return false;
	}

	if (fwrite(riff.ChunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&riff.ChunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(riff.Format, sizeof(BYTE), 4, outFile))
	{
		#ifdef _DEBUGOUTPUT
		cout << "S: Wrote RIFF header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to write RIFF header: Could not write bytes" << endl;
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
		cout << "E: Failed to write FMT header: FILE not open" << endl;
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
		cout << "S: Wrote FMT header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to write FMT header: Could not write bytes" << endl;
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
		cout << "E: Failed to write DATA header: FILE not open" << endl;
		#endif
		return false;
	}

	if (fwrite(data.SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&data.SubchunkSize, sizeof(DWORD), 1, outFile))
	{
		#ifdef _DEBUGOUTPUT
		cout << "S: Wrote DATA header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to write DATA header: Could not write bytes" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: encode												*/
/* purpose: encode data into the audio file that is in ram	 	*/
/* args: const char[], const char[], const char[]				*/
/* returns: DWORD												*/
/****************************************************************/
DWORD wav::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[]) {
	struct stat inputDATAStat;
	BYTE *wavBuffer = NULL, *dataBuffer = NULL;
	DWORD maxSize = 0,  bitsInFile = 0, bytesPerSample = 0;
	BYTE bitsUsed = 0;
	size_t wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;

	/* Open up all of our files */
	FILE* fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) {
		return false;
	}
	FILE* fInputDATA = open(inputDATA, "rb");
	if (fInputDATA == NULL) {
		close(fInputWAV);
		return false;
	}
	FILE* fOutputWAV = open(outputWAV, "wb");
	if (fOutputWAV == NULL) {
		close(fInputWAV); close(fInputDATA);
		return false;
	}


	/* read and validate wave header (RIFF Chunk), and format chunk */
	if (!read(fInputWAV)) {
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);
		return false;
	}


	/* Get size of inputDATA */
	if (stat(inputDATA, &inputDATAStat) != 0) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to determine input data file size" << endl;
		#endif
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Determined input data file size (" << setprecision(3) << (((double)inputDATAStat.st_size) / 1048576.0) << " MB)" << endl;
	#endif


	/* Can the file fit? */
	bytesPerSample = (fmt.BitsPerSample/8);
	maxSize = (data.SubchunkSize / bytesPerSample) - 1; /* Only doing the lower part of the sample */
	if (fmt.BitsPerSample == 8) maxSize >>= 2; /* stupid for the 8-bit files */

	if ((DWORD)inputDATAStat.st_size > maxSize) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Data file is too large (Want to store " << (((double)inputDATAStat.st_size) / 1048576.0) << " MB. Can fit " << (((double)maxSize) / 1048576.0) << " MB.)" << endl;
		#endif
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Data fits (Storing " << (((double)inputDATAStat.st_size) / 1048576.0) << " MB. Can fit " << (((double)maxSize) / 1048576.0) << " MB.)" << endl;
	#endif


	/* determine how many bits are to be eaten per sample */
	bitsInFile = inputDATAStat.st_size * 8;
	if (fmt.BitsPerSample == 8) { maxSize <<= 1; } /* stupid for the 8-bit files */

	if(maxSize >= bitsInFile) {
		bitsUsed = 0; // 1 bits
	} else if ((maxSize * 2) >= bitsInFile) {
		bitsUsed = 1; // 2 bits
	} else if ((maxSize * 4) >= bitsInFile) {
		bitsUsed = 2; // 4 bits
	} else if ((maxSize * 8) >= bitsInFile && fmt.BitsPerSample != 8) {
		bitsUsed = 3; // 8 bits
	} else {
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);
		#ifdef _DEBUGOUTPUT
		cout << "E: The world is in trouble... this should never happen." << endl;
		#endif
		return false;
	}


	/* eat 2 bits initially to store how many bits are eaten */ 
	BYTE* datum = (BYTE*)calloc(bytesPerSample, sizeof(BYTE));
	fread(datum, sizeof(BYTE), bytesPerSample, fInputWAV);
	*datum = (((*datum) >> 2) << 2);
	*datum += bitsUsed;
	#ifdef _DEBUGOUTPUT
	cout << "S: Bits stored per sample: " << pow(2.0, bitsUsed) << endl;
	#endif


	/* Write our headers and how many bits used */
	if (!writeRIFF(fOutputWAV) || !writeFMT(fOutputWAV) || !writeDATA(fOutputWAV)) {	
		close(fInputWAV); close(fInputDATA); close(fOutputWAV);
		return false;
	}
	fwrite(datum, sizeof(BYTE), bytesPerSample, fOutputWAV);
	free(datum);


	/* Calculate the size of our buffers */
	maxWavBufferSize = 1024 * (bytesPerSample);
	maxDataBufferSize = 1024 / (8 / ((int)pow(2.0, bitsUsed)));


	/* Get memory for our buffers */
	if ((wavBuffer = (BYTE*)calloc(maxWavBufferSize, sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to get memory for WAV buffer" << endl;
		#endif
		return false;
	}
	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize,sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to get memory for DATA buffer" << endl;
		#endif
		return false;
	}


	/* read into the buffers, process, and write */
	wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
	dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);

	 while (wavBufferSize != 0) {
		if ((dataBufferSize != 0) && !encode(bitsUsed, bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
			close(fInputWAV); close(fInputDATA); close(fOutputWAV);
			free(wavBuffer); free(dataBuffer);
			return false;
		}

		fwrite(wavBuffer, sizeof(BYTE), wavBufferSize, fOutputWAV);

 		wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
		dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);
	}

	#ifdef _DEBUGOUTPUT
	cout << "S: Number of bytes stored: " << inputDATAStat.st_size << endl;
	#endif

	close(fInputWAV); close(fInputDATA); close(fOutputWAV);
	free(wavBuffer); free(dataBuffer);
	return inputDATAStat.st_size;
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
		cout << "E: Invalid WAV buffer size" << endl;
		#endif
		return false;
	}
	if ((dataBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid DATA buffer size" << endl;
		#endif
		return false;
	}

	switch (bitsUsed) {
		case 0: // 1 bit
			while (count < wavBufferSize) {
				tempByte = *currPos_DataBuffer;
				for (char i = 7; i >= 0; i--) {
					setBit(*currPos_WavBuffer, 0, getBit(tempByte,i));
					currPos_WavBuffer += bytesPerSample;
					count += bytesPerSample;
				}
				currPos_DataBuffer++;
			}
			break;
		case 1: // 2 bits
			while (count < wavBufferSize) {	
				tempByte = *currPos_DataBuffer;
				for (char i = 3; i >= 0; i--) {
					setBit(*currPos_WavBuffer, 1, getBit(tempByte, i*2 + 1));
					setBit(*currPos_WavBuffer, 0, getBit(tempByte, i*2));
					currPos_WavBuffer += bytesPerSample;
					count += bytesPerSample;
				}
				currPos_DataBuffer++;
			}
			break;
		case 2: // 4 bits
			while (count < wavBufferSize) {
				tempByte = *currPos_DataBuffer;
				for (char i = 1; i >= 0; i--) {
					setBit(*currPos_WavBuffer, 3, getBit(tempByte, i*4 + 3));
					setBit(*currPos_WavBuffer, 2, getBit(tempByte, i*4 + 2));
					setBit(*currPos_WavBuffer, 1, getBit(tempByte, i*4 + 1));
					setBit(*currPos_WavBuffer, 0, getBit(tempByte, i*4));
					currPos_WavBuffer += bytesPerSample;
					count += bytesPerSample;
				}
				currPos_DataBuffer++;
			}
			break;
		case 3: // 8 bits
			while (count < wavBufferSize) {
				*currPos_WavBuffer = *currPos_DataBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count += bytesPerSample;
			}
			break;
		default:
			#ifdef _DEBUGOUTPUT
			cout << "E: Invalid number of bits used." << endl;
			#endif
			return false;
	}
	return true;
}


/****************************************************************/
/* function: decode												*/
/* purpose: decode data from the audio file that is in ram	 	*/
/* args: const char[], const char[], const DWORD&				*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(const char inputWAV[], const char outputDATA[], const DWORD& fileSize) {
	BYTE *wavBuffer = NULL, *dataBuffer = NULL;
	DWORD maxSize = 0, bitsInFile = 0, bytesPerSample = 0, count = 0;
	BYTE bitsUsed = 0x00;
	size_t wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;
	
	/* Open up all of our files */
	FILE* fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) {
		return false;
	}
	FILE* fOutputDATA = open(outputDATA, "wb");
	if (fOutputDATA == NULL) {
		close(fInputWAV);
		return false;
	}


	/* read and validate wave header (RIFF Chunk), and format chunk */
	if (!read(fInputWAV)) {
		close(fInputWAV); close(fOutputDATA);
		return false;
	}

	bytesPerSample = (fmt.BitsPerSample/8);


	/* Check first two bits to see how many are used per sample */ 
	BYTE* datum = (BYTE*)calloc(bytesPerSample, sizeof(BYTE));
	fread(datum, sizeof(BYTE), bytesPerSample, fInputWAV);
	setBit(bitsUsed, 0, getBit(*datum, 0));
	setBit(bitsUsed, 1, getBit(*datum, 1));
	free(datum);
	#ifdef _DEBUGOUTPUT
	cout << "S: Bits stored per sample: " << pow(2.0, bitsUsed) << endl;
	#endif
	

	/* Could the file fit? */
	maxSize = (data.SubchunkSize / bytesPerSample) - 1; /* Only doing the lower part of the sample */
	if (fmt.BitsPerSample == 8) maxSize >>= 1;

	bitsInFile = fileSize * 8;
	if (maxSize * ((BYTE)pow(2.0, bitsUsed)) < bitsInFile) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Data file could not fit at " << pow(2.0, bitsUsed) << " bits per sample" << endl;
		cout << "\t (Wanted to retrieve " << (((double)fileSize) / 1048576.0) << " MB. Could fit " << (((double)(maxSize / (8 / (BYTE)pow(2.0, bitsUsed)))) / 1048576.0) << " MB.)" << endl;
		#endif
		close(fInputWAV); close(fOutputDATA);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Data file could fit at " << pow(2.0, bitsUsed) << " bits per sample" << endl;
	cout << "\t (Retrieving " << (((double)fileSize) / 1048576.0) << " MB. Could fit " << (((double)(maxSize / (8 / (BYTE)pow(2.0, bitsUsed)))) / 1048576.0) << " MB.)" << endl;
	#endif


	/* Calculate the size of our buffers */
	maxWavBufferSize = 1024 * bytesPerSample;
	maxDataBufferSize = 1024 / (8 / ((int)pow(2.0, bitsUsed)));


	/* Get memory for our buffers */
	if ((wavBuffer = (BYTE*)calloc(maxWavBufferSize, sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to get memory for WAV buffer" << endl;
		#endif
		return false;
	}
	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize,sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to get memory for DATA buffer" << endl;
		#endif
		return false;
	}


	/* read into the buffers, process, and write */
	wavBufferSize = fread(wavBuffer, sizeof(BYTE), maxWavBufferSize, fInputWAV);
	count = dataBufferSize = maxDataBufferSize;

	while (count <= fileSize) {
		if (!decode(bitsUsed, bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
			close(fInputWAV); close(fOutputDATA);
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
	cout << "S: Number of bytes retrieved: " << fileSize << endl;
	#endif
	close(fInputWAV); close(fOutputDATA);
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
		cout << "E: Invalid WAV buffer size" << endl;
		#endif
		return false;
	}
	if ((dataBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid DATA buffer size" << endl;
		#endif
		return false;
	}


	// Grab the bits from each sample, build a byte, and output the bytes to a file
	switch (bitsUsed) {
		case 0: // 1 bit
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
		case 1: // 2 bits
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
		case 2: // 4 bits
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
		case 3: // 8 bits
			while (count < dataBufferSize) {
				*currPos_DataBuffer = *currPos_WavBuffer;
				currPos_WavBuffer += bytesPerSample;
				currPos_DataBuffer++;
				count++;
			}
			break;
		default:
			#ifdef _DEBUGOUTPUT
			cout << "E: Invalid number of bits per sample" << endl;
			#endif
			return false;
	}
	return true;
}


/****************************************************************/
/****************************************************************/
/****************************************************************/
