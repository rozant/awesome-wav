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
/* function: wav::loadRIFF										*/
/* purpose: loads the riff header from a wav file				*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = loaded correctly									*/
/*		0 = loaded incorrectly or did not load					*/
/****************************************************************/
bool wav::loadRIFF(FILE* inFile) {
	if (fread(riff.ChunkID, sizeof(BYTE), 4, inFile) &&
		fread(&riff.ChunkSize, sizeof(DWORD), 1, inFile) &&
		fread(riff.Format, sizeof(BYTE), 4, inFile))
	{
		#ifdef _DEBUGOUTPUT
		cout << "S: Loaded RIFF header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to load RIFF header: Could not get bytes" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: wav::loadFMT										*/
/* purpose: loads the fmt header from a wav file				*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = loaded correctly									*/
/*		0 = loaded incorrectly or did not load					*/
/****************************************************************/
bool wav::loadFMT(FILE* inFile) {
	if (fread(fmt.SubchunkID, sizeof(BYTE), 4, inFile) &&
		fread(&fmt.SubchunkSize, sizeof(DWORD), 1, inFile) &&
		fread(&fmt.AudioFormat, sizeof(SHORT), 1, inFile) &&
		fread(&fmt.NumChannels, sizeof(SHORT), 1, inFile) &&
		fread(&fmt.SampleRate, sizeof(DWORD), 1, inFile) &&
		fread(&fmt.ByteRate, sizeof(DWORD), 1, inFile) &&
		fread(&fmt.BlockAlign, sizeof(SHORT), 1, inFile) &&
		fread(&fmt.BitsPerSample, sizeof(SHORT), 1, inFile))
	{
		// Need to get extra stuff
		if (fmt.SubchunkSize-16 != 0) {
			fread(&fmt.ExtraFormatBytes, sizeof(SHORT), 1, inFile);
			if (fmt.ExtraFormatBytes > 0) {
				if ((fmt.ExtraFormat = (BYTE*)malloc(fmt.ExtraFormatBytes)) == NULL) {
					#ifdef _DEBUGOUTPUT
					cout << "E: Failed to load FMT header: Could not get memory for extra bytes" << endl;
					#endif
					return false;
				}
				fread(fmt.ExtraFormat, (fmt.ExtraFormatBytes), 1, inFile);
			}
		}

		#ifdef _DEBUGOUTPUT
		cout << "S: Loaded FMT header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to load FMT header: Could not get bytes" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: wav::loadDATA										*/
/* purpose: loads the data from a wav file						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = loaded correctly									*/
/*		0 = loaded incorrectly or did not load					*/
/****************************************************************/
bool wav::loadDATA(FILE* inFile) {
	DWORD size;
	BYTE id[4];

	fread(id, sizeof(BYTE), 4, inFile);
	fread(&size, sizeof(DWORD), 1, inFile);

	if (bytencmp(id, (BYTE*)"data", 4) == 0) {
		data.SubchunkID[0] = 'd'; data.SubchunkID[1] = 'a'; data.SubchunkID[2] = 't'; data.SubchunkID[3] = 'a';

		if ((data.Data = (BYTE*)malloc(size*sizeof(BYTE))) == NULL) {
			#ifdef _DEBUGOUTPUT
			cout << "E: Failed to load DATA header: Could not get memory for data" << endl;
			#endif
			return false;
		}
		data.SubchunkSize = size;

		#ifdef _DEBUGOUTPUT
		cout << "S: Loaded DATA header" << endl;
		#endif
		return true;
	}

	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to load DATA header: Could not locate" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: wav::load											*/
/* purpose: loads a wav file into memory						*/
/* args: const char[]											*/
/* returns: bool												*/
/*		1 = loaded correctly									*/
/*		0 = loaded incorrectly or did not load					*/
/****************************************************************/
bool wav::load(FILE *inFile) {
	/* Load and validate wave header (RIFF Chunk), format chunk, and DATA */
	if ( (loadRIFF(inFile) && validRIFF() && loadFMT(inFile) && validFMT() && loadDATA(inFile) && validDATA())) {
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
		fwrite(&data.SubchunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(data.Data, sizeof(BYTE), data.SubchunkSize, outFile))
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
	//	BYTE* inputWavBuffer = NULL;
	DWORD maxSize = 0,  bitsInFile = 0, bytesPerSample = 0, buff_length = 0;
	BYTE bitsUsed = 0;

	FILE* fInputWAV = open(inputWAV, "rb");
	if (fInputWAV == NULL) {
		return false;
	}

	FILE* fInputDATA = open(inputDATA, "rb");
	if (fInputDATA == NULL) {
		close(fInputWAV);
		return false;
	}

	FILE* fOutputDATA = open(outputWAV, "wb");
	if (fOutputDATA == NULL) {
		close(fInputWAV); close(fInputDATA);
		return false;
	}

	/* Load and validate wave header (RIFF Chunk), and format chunk */
	if (!load(fInputWAV)) {
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		return false;
	}

	/* Get size of inputDATA */
	if (stat(inputDATA, &inputDATAStat) != 0) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to determine input data file size" << endl;
		#endif
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Determined input data file size (" << setprecision(3) << (((double)inputDATAStat.st_size) / 1048576.0) << " MB)" << endl;
	#endif

	/* Can the file fit? */
	bytesPerSample = (fmt.BitsPerSample/8);
	maxSize = (data.SubchunkSize / bytesPerSample) - 1; // Only doing the lower part of the sample
	if(fmt.BitsPerSample == 8) { maxSize >>= 2; } /* stupid for the 8-bit files */

	if ((DWORD)inputDATAStat.st_size > maxSize) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Data file is too large (Want to store " << (((double)inputDATAStat.st_size) / 1048576.0) << " MB. Can fit " << (((double)maxSize) / 1048576.0) << " MB.)" << endl;
		#endif
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
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
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		#ifdef _DEBUGOUTPUT
		cout << "E: The world is in trouble... this should never happen." << endl;
		#endif
		return false;
	}

	/* BUFFING STUFF HERE */
	fread(data.Data, sizeof(BYTE), data.SubchunkSize, fInputWAV);

	/* eat 2 bits initially to store how many bits are eaten */ 
	BYTE* datum = data.Data;
	*datum = (((*datum) >> 2) << 2);
	*datum += bitsUsed;
	datum += bytesPerSample;
	#ifdef _DEBUGOUTPUT
	cout << "S: Bits stored per sample: " << pow(2.0, bitsUsed) << endl;
	#endif

	if (!encode(fInputDATA, bitsUsed, bytesPerSample, datum, buff_length)) {
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		return false;
	}

	#ifdef _DEBUGOUTPUT
	cout << "S: Number of bytes stored: " << inputDATAStat.st_size << endl;
	#endif

	if (!writeRIFF(fOutputDATA) || !writeFMT(fOutputDATA) || !writeDATA(fOutputDATA)) {	
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		return false;
	}
	/* BUFFING STUFF HERE */

	close(fInputWAV); close(fInputDATA); close(fOutputDATA);
	//	free(inputWavBuffer);		BUFFER
	return inputDATAStat.st_size;
}


/****************************************************************/
/* function: encode												*/
/* purpose: encode data into the audio file using a buffer	 	*/
/* args: FILE* , BYTE, DWORD, BYTE*, DWORD						*/
/* returns: bool												*/
/* notes: the last BYTE* and DWORD are for the buffered wav		*/
/****************************************************************/
bool wav::encode(FILE* fInputDATA, BYTE bitsUsed, DWORD bytesPerSample, BYTE *wavBuffer, DWORD wavBufferSize) {
	BYTE tempByte = 0x00;
	DWORD count = 0x00;

	switch (bitsUsed) {
		case 0: // 1 bit
			while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA) /*&& count < wavBufferSize*/) {			
				for (char i = 7; i >= 0; i--) {
					setBit(*wavBuffer, 0, getBit(tempByte,i));
					wavBuffer += bytesPerSample;
					count += bytesPerSample;
				}								
			}
			break;
		case 1: // 2 bits
			while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA) /*&& count < wavBufferSize*/) {			
				for (char i = 3; i >= 0; i--) {
					setBit(*wavBuffer, 1, getBit(tempByte, i*2 + 1));
					setBit(*wavBuffer, 0, getBit(tempByte, i*2));
					wavBuffer += bytesPerSample;
					count += bytesPerSample;
				}			
			}
			break;
		case 2: // 4 bits
			while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA) /*&& count < wavBufferSize*/) {			
				for (char i = 1; i >= 0; i--) {
					setBit(*wavBuffer, 3, getBit(tempByte, i*4 + 3));
					setBit(*wavBuffer, 2, getBit(tempByte, i*4 + 2));
					setBit(*wavBuffer, 1, getBit(tempByte, i*4 + 1));
					setBit(*wavBuffer, 0, getBit(tempByte, i*4));
					wavBuffer += bytesPerSample;
					count += bytesPerSample;
				}	
			}
			break;
		case 3: // 8 bits
			while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA) /*&& count < wavBufferSize*/) {			
				*wavBuffer = tempByte;
				wavBuffer += bytesPerSample;	
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
/* args: const char[], const char[], const string&				*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(const char encodedWAV[], const char outputDATA[], const DWORD& fileSize) {
	FILE* fEncodedWAV = open(encodedWAV, "rb");
//	BYTE* inputWavBuffer = NULL;
	bool ret_val = false;
	if (fEncodedWAV == NULL) {
		return false;
	}

	/* Load and validate wave header (RIFF Chunk), and format chunk */
	if (!load(fEncodedWAV)) {
		close(fEncodedWAV);
		return false;
	}

	//	inputWavBuffer = (BYTE*)calloc(BUFFER_SIZE(fmt.BlockAlign / fmt.NumChannels),sizeof(BYTE));

	fread(data.Data, sizeof(BYTE), data.SubchunkSize, fEncodedWAV);
	close(fEncodedWAV);

	ret_val = decode(outputDATA, fileSize);

//	free(inputWavBuffer);
	return ret_val;
}

/****************************************************************/
/* function: decode												*/
/* purpose: decode data from the audio file that is in ram	 	*/
/* args: const char[], const string&							*/
/* returns: bool												*/
/****************************************************************/
bool wav::decode(const char filename[], const DWORD& fileSize) const {
	DWORD maxSize = 0, bitsInFile = 0, i = 0, bytesPerSample = (fmt.BlockAlign / fmt.NumChannels);
	BYTE bitsUsed = 0, tempByte = 0;
	FILE* dataFile;

	// Check first two bits to see how many are used per sample
	BYTE* datum = data.Data;
	setBit(tempByte, 0, getBit(*datum, 0));
	setBit(tempByte, 1, getBit(*datum, 1));
	bitsUsed = tempByte;
	datum += bytesPerSample;

	/* Could the file fit? */
	maxSize = (data.SubchunkSize / bytesPerSample) - 1; // Only doing the lower part of the sample
	if(fmt.BitsPerSample == 8) {
		maxSize >>= 1;
	}
	bitsInFile = fileSize * 8;
	if (maxSize * ((BYTE)pow(2.0, bitsUsed)) < bitsInFile) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Data file could not fit at " << pow(2.0, bitsUsed) << " bits per sample" << endl;
		cout << "\t (Wanted to retrieve " << (((double)fileSize) / 1048576.0) << " MB. Could fit " << (((double)(maxSize / (8 / (BYTE)pow(2.0, bitsUsed)))) / 1048576.0) << " MB.)" << endl;
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Data file could fit at " << pow(2.0, bitsUsed) << " bits per sample" << endl;
	cout << "\t (Retrieving " << (((double)fileSize) / 1048576.0) << " MB. Could fit " << (((double)(maxSize / (8 / (BYTE)pow(2.0, bitsUsed)))) / 1048576.0) << " MB.)" << endl;
	#endif

	dataFile = open(filename, "wb");
	if (dataFile == NULL) {
		return false;
	}

	// Grab the bits from each sample, build a byte, and output the bytes to a file
	if (bitsUsed == 0) { // 1 bit
		while (i < fileSize) {
			for (char j = 7; j >= 0; j--) {
				setBit(tempByte, j, getBit(*datum, 0));
				datum += bytesPerSample;
			}
			i++;
			fwrite(&tempByte, sizeof(BYTE), 1, dataFile);			
		}
	} else if (bitsUsed == 1) { // 2 bits
		while (i < fileSize) {			
			for (char j = 3; j >= 0; j--) {
				setBit(tempByte, j*2 + 1, getBit(*datum, 1));
				setBit(tempByte, j*2, getBit(*datum, 0));
				datum += bytesPerSample;
			}
			i++;
			fwrite(&tempByte, sizeof(BYTE), 1, dataFile);
		}
	}  else if (bitsUsed == 2) { // 4 bits
		while (i < fileSize) {			
			for (char j = 1; j >= 0; j--) {
				setBit(tempByte, j*4 + 3, getBit(*datum, 3));
				setBit(tempByte, j*4 + 2, getBit(*datum, 2));
				setBit(tempByte, j*4 + 1, getBit(*datum, 1));
				setBit(tempByte, j*4, getBit(*datum, 0));
				datum += bytesPerSample;
			}
			i++;
			fwrite(&tempByte, sizeof(BYTE), 1, dataFile);
		}
	} else if (bitsUsed == 3 && fmt.BitsPerSample != 8) { // 8 bits
		while (i < fileSize) {
			tempByte = *datum;
			fwrite(&tempByte, sizeof(BYTE), 1, dataFile);
			datum += bytesPerSample;
			i++;
		}
	} else {
		#ifdef _DEBUGOUTPUT
		cout << "E: Invalid number of bits per sample" << endl;
		#endif
		close(dataFile);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Number of bytes retrieved: " << fileSize << endl;
	#endif
	close(dataFile);
	return true;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
