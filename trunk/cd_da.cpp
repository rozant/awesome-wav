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
/* cd_da.cpp														*/
/****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "cd_da.hpp"
#include "file_compression.h"
#include "global.hpp"
#include "util.hpp"
#ifdef _DEBUGOUTPUT
#include <time.h>
#endif

/****************************************************************/
/* function: cd_da::cd_da										*/
/* purpose: constructor for the cd-da class						*/
/* args: void													*/
/****************************************************************/
cd_da::cd_da(void) {
	bitsPerSample = 16;
}

/****************************************************************/
/* function: cd_da::~cd_da										*/
/* purpose: destructor for the cd-da class						*/
/* args: void													*/
/****************************************************************/
cd_da::~cd_da(void) {
}

/****************************************************************/
/* function: encode												*/
/* purpose: open the files ment for encoding				 	*/
/* args: const char[], const char[], const char[], const char	*/
/* returns: DWORD												*/
/****************************************************************/
DWORD cd_da::encode(const char inputCDDA[], const char inputDATA[], const char outputCDDA[], const char compressionLevel) {
	FILE *fInputCDDA, *fInputDATA, *fOutputCDDA, *fCompDATA;
	DWORD ret_val = 0;

	/* Open up our input files */
	fInputCDDA = open(inputCDDA, "rb");
	if (fInputCDDA == NULL) { return false; }
	fInputDATA = open(inputDATA, "rb");
	if (fInputDATA == NULL) { close(fInputCDDA); return false; }

	/* open up output file */
	fOutputCDDA = open(outputCDDA, "wb");
	if (fOutputCDDA == NULL) { close(fInputCDDA); close(fInputDATA); return false; }

	/* if we are compressing */
	if (compressionLevel > 0) {
		fCompDATA = open("data.z", "wb");

		/* open a temp file and compress */
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not open temp data file.\n");
			#endif
			close(fInputCDDA); close(fInputDATA); close(fOutputCDDA);
			return false;
		}
		if( def(fInputDATA, fCompDATA, compressionLevel) != 0) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not compress data file.\n");
			#endif
			close(fInputCDDA); close(fInputDATA); close(fOutputCDDA); close(fCompDATA);
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
			close(fInputCDDA); close(fInputDATA); close(fOutputCDDA);
			return false;
		}
		
		ret_val = encode(fInputCDDA, fInputDATA, fOutputCDDA);
		close(fInputCDDA); close(fInputDATA); close(fOutputCDDA);

		if (remove("data.z") == -1) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not remove temporary file data.z\n");
			#endif
		}
	} else {
		ret_val = encode(fInputCDDA, fInputDATA, fOutputCDDA);
		close(fInputCDDA); close(fInputDATA); close(fOutputCDDA);
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
DWORD cd_da::encode(FILE *fInputCDDA, FILE *fInputDATA, FILE *fOutputCDDA) {
	BYTE *cddaBuffer = NULL, *dataBuffer = NULL;
	DWORD cddaSize = 0, dataSize = 0, maxSize = 0, bytesPerSample = (bitsPerSample/8);
	BYTE bitsUsed = 0;
	size_t cddaBufferSize, maxAudioBufferSize, dataBufferSize, maxDataBufferSize;

	/* Get size of cdda file we want to encode */
	fseek(fInputCDDA, 0, SEEK_END);
	cddaSize = ftell(fInputCDDA);
	fseek(fInputCDDA, 0, SEEK_SET);
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Determined cdda file size (%.*f MB)\n",3,byteToMB(cddaSize));
	#endif

	/* Get size of data file we want to encode */
	fseek(fInputDATA, 0, SEEK_END);
	dataSize = ftell(fInputDATA);
	fseek(fInputDATA, 0, SEEK_SET);
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Determined input data file size (%.*f MB)\n",3,byteToMB(dataSize));
	#endif

	/* get the maximum number of bytes the cdda file could hold */
	maxSize = getMaxBytesEncoded(cddaSize);
	if (dataSize > maxSize) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Data file is too large (Want to store %.*f MB - Can fit %.*f MB)\n",3,byteToMB(dataSize),3,byteToMB(maxSize));
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data fits (Storing %.*f MB - Can Fit %.*f MB)\n",3,byteToMB(dataSize),3,byteToMB(maxSize));
	#endif

	/* get the minimum number of bits the cdda file could encode per sample */
	bitsUsed = getMinBitsEncodedPS(dataSize, maxSize);
	if (bitsUsed == 0 || bitsUsed > (bitsPerSample >> 1)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: This should never happen %d\n",(int)bitsUsed);
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data file could fit at %d bits per sample\n",(int)bitsUsed);
	#endif

	/* Calculate the size of our buffers */
	maxAudioBufferSize = BUFFER_MULT*(1024 * bytesPerSample);
	maxDataBufferSize = BUFFER_MULT*(128 * bitsUsed);

	/* Get memory for our buffers */
	if ((cddaBuffer = (BYTE*)calloc(maxAudioBufferSize, sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for CDDA buffer\n");
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for CDDA buffer\n",(unsigned int)maxAudioBufferSize);
	#endif

	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize,sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for DATA buffer\n");
		#endif
		free(cddaBuffer);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for DATA buffer\n",(unsigned int)maxDataBufferSize);
	clock_t start = clock();
	#endif

	/* read into the buffers, process, and write */
	cddaBufferSize = fread(cddaBuffer, sizeof(BYTE), maxAudioBufferSize, fInputCDDA);
	dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);

	/* while there is data in the buffer encode and write to the file*/
	while (cddaBufferSize != 0) {
		/* encode and error out if it fails */
		if ((dataBufferSize != 0) && !encode(bitsUsed, bytesPerSample, cddaBuffer, cddaBufferSize, dataBuffer, dataBufferSize)) {
			free(cddaBuffer); free(dataBuffer);
			return false;
		}
		/* write the changes to the file */
		fwrite(cddaBuffer, sizeof(BYTE), cddaBufferSize, fOutputCDDA);
		/* get the next chunk of data */
 		cddaBufferSize = fread(cddaBuffer, sizeof(BYTE), maxAudioBufferSize, fInputCDDA);
		dataBufferSize = fread(dataBuffer, sizeof(BYTE), maxDataBufferSize, fInputDATA);
	}

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Took %.3f seconds to encode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
	fprintf(stderr,"S: Number of bytes stored: %u\n",(unsigned int)dataSize);
	#endif
	free(cddaBuffer); free(dataBuffer);
	return dataSize;
}


/****************************************************************/
/* function: encode												*/
/* purpose: encode data into the cdda file using a buffer	 	*/
/* args: const BYTE, const DWORD, BYTE*, const size_t, BYTE*,	*/
/*		const size_t											*/
/* returns: bool												*/
/****************************************************************/
bool cd_da::encode(const BYTE bitsUsed, const DWORD bytesPerSample, BYTE *cddaBuffer, const size_t cddaBufferSize, BYTE *dataBuffer, const size_t dataBufferSize) {
	BYTE tempByte = 0x00;
	size_t count = 0x00;
	BYTE* currPos_WavBuffer = cddaBuffer;
	BYTE* currPos_DataBuffer = dataBuffer;

	if ((cddaBufferSize == 0)) {
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
bool cd_da::decode(const char inputCDDA[], const char outputDATA[], const DWORD& fileSize, const char compress) {
	FILE *fInputCDDA, *fOutputDATA, *fCompDATA;
	bool ret_val = 0;

	/* Open up our input file */
	fInputCDDA = open(inputCDDA, "rb");
	if (fInputCDDA == NULL) { return false; }

	/* open up our output file */
	fOutputDATA = open(outputDATA, "wb");
	if (fOutputDATA == NULL) { close(fInputCDDA); return false; }

	/* if we are compressing */
	if (compress > 0) {
		fCompDATA = open("data.z", "wb");

		/* open the temp file */
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not open temp data file.\n");
			#endif
			close(fInputCDDA); close(fOutputDATA);
			return false;
		}

		ret_val = decode(fInputCDDA, fCompDATA, fileSize);
		close(fCompDATA);
		if (!ret_val) { close(fInputCDDA); close(fOutputDATA); return ret_val; }

		/* re-open the temp file in read mode */
		fCompDATA = open("data.z", "rb");
		if( fCompDATA == NULL ) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not re-open temp data file.\n");
			#endif
			close(fInputCDDA); close(fOutputDATA);
			return false;
		}

		/* decompress */
		if( inf(fCompDATA, fOutputDATA) != 0) {
			#ifdef _DEBUGOUTPUT
			fprintf(stderr,"E: Could not decompress data file.\n");
			#endif
			close(fInputCDDA); close(fOutputDATA); close(fCompDATA);
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
		ret_val = decode(fInputCDDA, fOutputDATA, fileSize);
	}

	/* clean up */
	close(fInputCDDA); close(fOutputDATA);
	return ret_val;
}

/****************************************************************/
/* function: decode												*/
/* purpose: do all necessary calculations and handle buffering 	*/
/* prerequisites: files are open; header data has been read		*/
/* args: const char[], const char[], const DWORD&				*/
/* returns: bool												*/
/****************************************************************/
bool cd_da::decode(FILE* fInputCDDA, FILE* fOutputDATA, const DWORD& fileSize) {
	BYTE *cddaBuffer = NULL, *dataBuffer = NULL;
	DWORD cddaSize = 0, maxSize = 0, bytesPerSample = (bitsPerSample/8), count = 0;
	BYTE bitsUsed = 0x00;
	size_t cddaBufferSize, maxAudioBufferSize, dataBufferSize, maxDataBufferSize;

	/* Get size of cdda file we want to encode */
	fseek(fInputCDDA, 0, SEEK_END);
	cddaSize = ftell(fInputCDDA);
	fseek(fInputCDDA, 0, SEEK_SET);
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Determined cdda file size (%.*f MB)\n",3,byteToMB(cddaSize));
	#endif

	/* get the maximum number of bytes the cdda file could hold */
	maxSize = getMaxBytesEncoded(cddaSize);
	if (fileSize > maxSize) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Data file is too large (Want to retrieve %.*f MB - Can retrieve %.*f MB)\n",3,byteToMB(fileSize),3,byteToMB(maxSize));
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data fits (Retrieving %.*f MB - Can retrieve %.*f MB)\n",3,byteToMB(fileSize),3,byteToMB(maxSize));
	#endif

	/* get the minimum number of bits the cdda file could encode per sample */
	bitsUsed = getMinBitsEncodedPS(fileSize, maxSize);
	if (bitsUsed == 0 || bitsUsed > (bitsPerSample >> 1)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: This should never happen %d\n",(int)bitsUsed);
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Data file could fit at %d bits per sample\n",(int)bitsUsed);
	#endif

	/* Calculate the size of our buffers */
	maxAudioBufferSize = BUFFER_MULT*(1024 * bytesPerSample);
	maxDataBufferSize = BUFFER_MULT*(128 * bitsUsed);

	/* Get memory for our buffers */
	if ((cddaBuffer = (BYTE*)calloc(maxAudioBufferSize, sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for CDDA buffer\n");
		#endif
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for CDDA buffer\n",(unsigned int)maxAudioBufferSize);
	#endif

	if ((dataBuffer = (BYTE*)calloc(maxDataBufferSize,sizeof(BYTE))) == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to get memory for DATA buffer\n");
		#endif
		free(cddaBuffer);
		return false;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Got %u bytes for DATA buffer\n",(unsigned int)maxDataBufferSize);
	clock_t start = clock();
	#endif

	/* read into the buffers, process, and write */
	cddaBufferSize = fread(cddaBuffer, sizeof(BYTE), maxAudioBufferSize, fInputCDDA);
	count = dataBufferSize = maxDataBufferSize;

	while (count <= fileSize) {
		if (!decode(bitsUsed, bytesPerSample, cddaBuffer, cddaBufferSize, dataBuffer, dataBufferSize)) {
			free(cddaBuffer); free(dataBuffer);
			return false;
		}

		fwrite(dataBuffer, sizeof(BYTE), dataBufferSize, fOutputDATA);

		if (count == fileSize)
			break;
	
 		cddaBufferSize = fread(cddaBuffer, sizeof(BYTE), maxAudioBufferSize, fInputCDDA);
		
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
	free(cddaBuffer); free(dataBuffer);
	return true;
}

/****************************************************************/
/* function: decode												*/
/* purpose: decode data from the cdda file that is in ram	 	*/
/* args: const BYTE, const DWORD, BYTE*, const size_t, BYTE*,	*/
/*		const size_t											*/
/* returns: bool												*/
/****************************************************************/
bool cd_da::decode(const BYTE bitsUsed, const DWORD bytesPerSample, BYTE *cddaBuffer, const size_t cddaBufferSize, BYTE *dataBuffer, const size_t dataBufferSize) {
	BYTE tempByte = 0x00;
	size_t count = 0x00;
	BYTE* currPos_WavBuffer = cddaBuffer;
	BYTE* currPos_DataBuffer = dataBuffer;

	if ((cddaBufferSize == 0)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Invalid CDDA buffer size\n");
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
/* purpose: calculate max number of bytes an cdda file can		*/
/*			encode												*/
/* args: const DWORD											*/
/* returns: DWORD												*/
/****************************************************************/
DWORD cd_da::getMaxBytesEncoded(const DWORD subchunkSize) {
	/* allow the use of only the bottom half of the bits per sample */
	return (subchunkSize / (bitsPerSample/8));
}

/****************************************************************/
/* function: getMinBitsEncodedPS								*/
/* purpose: calculate min number of bits possibly encoded		*/
/*			per sample					 						*/
/* args: const DWORD, const DWORD								*/
/* returns: BYTE												*/
/****************************************************************/
BYTE cd_da::getMinBitsEncodedPS(const DWORD fileSize, const DWORD maxSize) {
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
	else
		return 0;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
