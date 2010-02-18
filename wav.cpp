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
	DWORD size, offset;
	BYTE id[4];
	BYTE *junkData;

	do {
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
		/* SHOULD JUST SKIP JUNK */
		if ((junkData = (BYTE*)malloc(size*sizeof(BYTE))) == NULL) {
			#ifdef _DEBUGOUTPUT
			cout << "E: Failed to load DATA header: Could not get memory for unneeded data" << endl;
			#endif
			return false;
		}
		fread(junkData, sizeof(BYTE), size, inFile);
		free(junkData);
		offset = ftell(inFile);
	} while (offset < riff.ChunkSize);

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
	return (loadRIFF(inFile) && validRIFF() && loadFMT(inFile) && validFMT() && loadDATA(inFile) && validDATA());
}

/****************************************************************/
/* function: wav::saveRIFF										*/
/* purpose: saves the riff header to a file						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = saved correctly										*/
/*		0 = saved incorrectly or did not open					*/
/****************************************************************/
bool wav::saveRIFF(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to save RIFF header: FILE not open" << endl;
		#endif
		return false;
	}

	if (fwrite(riff.ChunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&riff.ChunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(riff.Format, sizeof(BYTE), 4, outFile))
	{
		#ifdef _DEBUGOUTPUT
		cout << "S: Saved RIFF header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to save RIFF header: Could not save bytes" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: wav::saveFMT										*/
/* purpose: saves the fmt header to a file						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = loaded correctly									*/
/*		0 = loaded incorrectly or did not open					*/
/****************************************************************/
bool wav::saveFMT(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to save FMT header: FILE not open" << endl;
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
		// Need to save extra stuff
		if (fmt.SubchunkSize-16 != 0) {
			fwrite(&fmt.ExtraFormatBytes, sizeof(SHORT), 1, outFile);
			if (fmt.ExtraFormatBytes > 0) {
				fwrite(fmt.ExtraFormat, fmt.ExtraFormatBytes, 1, outFile);
			}
		}
		#ifdef _DEBUGOUTPUT
		cout << "S: Saved FMT header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to save FMT header: Could not save bytes" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: wav::saveDATA										*/
/* purpose: saves the data header to a file						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = loaded correctly									*/
/*		0 = loaded incorrectly or did not open					*/
/****************************************************************/
bool wav::saveDATA(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to save DATA header: FILE not open" << endl;
		#endif
		return false;
	}

	if (fwrite(data.SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&data.SubchunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(data.Data, sizeof(BYTE), data.SubchunkSize, outFile))
	{
		#ifdef _DEBUGOUTPUT
		cout << "S: Saved DATA header" << endl;
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: Failed to save DATA header: Could not save bytes" << endl;
	#endif
	return false;
}

/****************************************************************/
/* function: wav::save											*/
/* purpose: saves a wav file from memory						*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = saved correctly										*/
/*		0 = saved incorrectly or did not open					*/
/****************************************************************/
bool wav::save(FILE* outFile) const {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to save WAV file: FILE not open" << endl;
		#endif
		return false;
	}

	return (saveRIFF(outFile) && saveFMT(outFile) && saveDATA(outFile));
}

/****************************************************************/
/* function: encode												*/
/* purpose: encode data into the audio file that is in ram	 	*/
/* args: const char[], const char[], const char[]				*/
/* returns: DWORD												*/
/****************************************************************/
DWORD wav::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[]) {
	struct stat inputDATAStat;
	DWORD maxSize = 0,  bitsInFile = 0, bytesPerSample = 0;
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

	if (!encode(fInputDATA, bitsUsed, bytesPerSample)) {	
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		return false;
	}

	#ifdef _DEBUGOUTPUT
	cout << "S: Number of bytes stored: " << inputDATAStat.st_size << endl;
	#endif

	if (!save(fOutputDATA)) {	
		close(fInputWAV); close(fInputDATA); close(fOutputDATA);
		return false;
	}
	/* BUFFING STUFF HERE */

	close(fInputWAV); close(fInputDATA); close(fOutputDATA);
	return inputDATAStat.st_size;
}

bool wav::encode(FILE* fInputDATA, BYTE bitsUsed, DWORD bytesPerSample/*BYTE *inputWavBuffer, BYTE *inputDataBuffer*/) {
	BYTE tempByte = 0;

	/* eat 2 bits initially to store how many bits are eaten */ 
	BYTE* datum = data.Data;
	*datum = (((*datum) >> 2) << 2);
	*datum += bitsUsed;
	datum += bytesPerSample;
	#ifdef _DEBUGOUTPUT
	cout << "S: Bits stored per sample: " << pow(2.0, bitsUsed) << endl;
	#endif

	/* overwrite the bits in the samples */
	if (bitsUsed == 0) { // 1 bit
		while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA)) {			
			for (int i = 7; i >= 0; i--) {
				setBit(*datum, 0, getBit(tempByte,i));
				datum += bytesPerSample;
			}		
		}
	} else if (bitsUsed == 1) { // 4 bits
		while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA)) {			
			for (int i = 3; i >= 0; i--) {
				setBit(*datum, 1, getBit(tempByte, i*2 + 1));
				setBit(*datum, 0, getBit(tempByte, i*2));
				datum += bytesPerSample;
			}		
		}
	}  else if (bitsUsed == 2) { // 4 bits
		while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA)) {			
			for (int i = 1; i >= 0; i--) {
				setBit(*datum, 3, getBit(tempByte, i*4 + 3));
				setBit(*datum, 2, getBit(tempByte, i*4 + 2));
				setBit(*datum, 1, getBit(tempByte, i*4 + 1));
				setBit(*datum, 0, getBit(tempByte, i*4));
				datum += bytesPerSample;
			}		
		}
	} else if (bitsUsed == 3) { // 8 bits
		while (fread(&tempByte, sizeof(BYTE), 1, fInputDATA)) {
			*datum = tempByte;
			datum += bytesPerSample;
		}
	} else {
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
	if (fEncodedWAV == NULL) {
		return false;
	}

	/* Load and validate wave header (RIFF Chunk), and format chunk */
	if (!load(fEncodedWAV)) {
		close(fEncodedWAV);
		return false;
	}

	return decode(outputDATA, fileSize);
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

	dataFile = fopen(filename, "wb");
	if (dataFile == NULL) {
		#ifdef _DEBUGOUTPUT
		cout << "E: Failed to open output file for decoding (" << filename << ")" << endl;
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	cout << "S: Opened output file for decoding (" << filename << ")" << endl;
	#endif

	// Grab the bits from each sample, build a byte, and output the bytes to a file
	if (bitsUsed == 0) { // 1 bit
		while (i < fileSize) {
			for (int j = 7; j >= 0; j--) {
				setBit(tempByte, j, getBit(*datum, 0));
				datum += bytesPerSample;
			}
			i++;
			fwrite(&tempByte, sizeof(BYTE), 1, dataFile);			
		}
	} else if (bitsUsed == 1) { // 2 bits
		while (i < fileSize) {			
			for (int j = 3; j >= 0; j--) {
				setBit(tempByte, j*2 + 1, getBit(*datum, 1));
				setBit(tempByte, j*2, getBit(*datum, 0));
				datum += bytesPerSample;
			}
			i++;
			fwrite(&tempByte, sizeof(BYTE), 1, dataFile);
		}
	}  else if (bitsUsed == 2) { // 4 bits
		while (i < fileSize) {			
			for (int j = 1; j >= 0; j--) {
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

	return true;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
