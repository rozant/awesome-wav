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
#ifndef __riff_func_hpp__
#define __riff_func_hpp__
#include "global.hpp"
#include "riff.hpp"
#include "logger.hpp"
#include <stdio.h>
#include <string.h>

/**********************Function Prototypes***********************/

// read
template <class T>
int RIFFread(FILE *, T *);
template <class T>
int RIFFreadRIFF(FILE *, T *);
template <class T>
int RIFFreadFMT(FILE *, T *);
template <class T>
int RIFFreadFACT(FILE *, T *);
template <class T>
int RIFFreadPEAK(FILE *, T *);
template <class T>
int RIFFreadDATA(FILE *, T *);

// write
template <class T>
int RIFFwrite(FILE *, const T *);
template <class T>
int RIFFwriteRIFF(FILE *, const T *);
template <class T>
int RIFFwriteFMT(FILE *, const T *);
template <class T>
int RIFFwriteFACT(FILE *, const T *);
template <class T>
int RIFFwritePEAK(FILE *, const T *);
template <class T>
int RIFFwriteDATA(FILE *, const T *);

/***************************Functions****************************/

/****************************************************************/
/* function: RIFFread											*/
/* purpose: reads a wav file into memory						*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFread(FILE *inFile, T *input) {
	int ret_val;
	char temp[4];
	fpos_t pos;

	// read riff chunk
	ret_val = RIFFreadRIFF(inFile, input);
	if (ret_val != RIFF_SUCCESS) {
		return ret_val;
	}

	// read fmt chunk when it appears
	ret_val = RIFFreadFMT(inFile, input);
	if (ret_val != RIFF_SUCCESS) {
		return ret_val;
	}

	// read fact chunk if needed
	fgetpos(inFile, &pos);
	fread(temp, sizeof(char), 4, inFile);
	if (memcmp(temp, "fact", 4) == 0) {
		fsetpos(inFile, &pos);
		input->fact = (_FACT *)calloc(1, sizeof(_FACT));
		ret_val = RIFFreadFACT(inFile, input);
		if (ret_val != RIFF_SUCCESS) {
			return ret_val;
		}
	} else {
		fsetpos(inFile, &pos);
	}
	// read peak chunk if needed
	fgetpos(inFile, &pos);
	fread(temp, sizeof(char), 4, inFile);
	if (memcmp(temp, "PEAK", 4) == 0) {
		fsetpos(inFile, &pos);
		input->peak = (_PEAK *)calloc(1, sizeof(_PEAK));
		ret_val = RIFFreadPEAK(inFile, input);
		if (ret_val != RIFF_SUCCESS) {
			return ret_val;
		}
	} else {
		fsetpos(inFile, &pos);
	}
	// read data chunk
	ret_val = RIFFreadDATA(inFile, input);
	if (ret_val != RIFF_SUCCESS) {
		return ret_val;
	}
	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFreadRIFF										*/
/* purpose: reads the riff header from a wav file				*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadRIFF(FILE *inFile, T *input) {
	// read
	if (fread(input->riff.ChunkID, sizeof(int8), 4, inFile) &&
		fread(&input->riff.ChunkSize, sizeof(int32), 1, inFile) &&
		fread(input->riff.Format, sizeof(int8), 4, inFile))
	{
		LOG_DEBUG("S: Read RIFF header\n");
	} else {
		LOG_DEBUG("E: Failed to read RIFF header: Could not read bytes\n");
		return RIFF_READ_FAIL;
	}
	// basic validation
	if (memcmp(input->riff.ChunkID, "RIFF", 4) != 0) {
		LOG_DEBUG("E: Invalid RIFF header: ChunkID != 'RIFF'\n");
		LOG_DEBUG("\tChunkID == %s\n", (char*)input->riff.ChunkID);
		return RIFF_VALID_FAIL;
	} else if (input->riff.ChunkSize == 0) {
		LOG_DEBUG("E: Invalid RIFF header: Chunk Size canot be 0\n");
		return RIFF_VALID_FAIL;
	}
	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFreadFMT										*/
/* purpose: reads the fmt header from a wav file				*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadFMT(FILE *inFile, T *input) {
	if (fread(input->fmt.SubchunkID, sizeof(int8), 4, inFile) &&
		fread(&input->fmt.SubchunkSize, sizeof(int32), 1, inFile) &&
		fread(&input->fmt.AudioFormat, sizeof(int16), 1, inFile) &&
		fread(&input->fmt.NumChannels, sizeof(int16), 1, inFile) &&
		fread(&input->fmt.SampleRate, sizeof(int32), 1, inFile) &&
		fread(&input->fmt.ByteRate, sizeof(int32), 1, inFile) &&
		fread(&input->fmt.BlockAlign, sizeof(int16), 1, inFile) &&
		fread(&input->fmt.BitsPerSample, sizeof(int16), 1, inFile))
	{
		// basic validation
		if (memcmp(input->fmt.SubchunkID, "fmt ", 4) != 0) {
			LOG_DEBUG("E: Invalid FMT header: SubchunkID != 'fmt '\n");
			LOG_DEBUG("\tSubchunkID == %s\n", (char*)input->fmt.SubchunkID);
			return RIFF_VALID_FAIL;
		} else if (input->fmt.SubchunkSize != 16 && input->fmt.SubchunkSize != 18 && input->fmt.SubchunkSize != 40) {
			LOG_DEBUG("E: Invalid FMT header: invalid SubchunkSize\n");
			return RIFF_VALID_FAIL;
		}
		// Need to read extra stuff
		if (input->fmt.SubchunkSize-16 != 0) {
			fread(&input->fmt.ExtraFormatBytes, sizeof(int16), 1, inFile);
			if (input->fmt.ExtraFormatBytes == 22) {
				if (!(fread(&input->fmt.ValidBitsPerSample, sizeof(int16), 1, inFile) &&
					fread(&input->fmt.ChannelMask, sizeof(int32), 1, inFile) &&
					fread(input->fmt.SubFormat, sizeof(int8), 16, inFile)))
				{
					LOG_DEBUG("E: Failed to read FMT header: Could not read bytes\n");
					return RIFF_READ_FAIL;
				}
			} else if (input->fmt.ExtraFormatBytes != 0) {
				LOG_DEBUG("E: Invalid FMT header. Incorrect number of extra format bits.\n");
				LOG_DEBUG("\tExtra format bytes == %u\n", (unsigned int)input->fmt.ExtraFormatBytes);
				return RIFF_VALID_FAIL;
			}
			LOG_DEBUG("S: Read Extended FMT header\n");
		} else {
			LOG_DEBUG("S: Read FMT header\n");
		}
	} else {
		LOG_DEBUG("E: Failed to read FMT header: Could not read bytes\n");
		return RIFF_READ_FAIL;
	}
	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFreadFACT										*/
/* purpose: reads the fact chunk from a wav file				*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadFACT(FILE *inFile, T *input) {
	if (fread(input->fact->SubchunkID, sizeof(int8), 4, inFile) &&
		fread(&input->fact->SubchunkSize, sizeof(int32), 1, inFile) &&
		fread(&input->fact->SampleLength, sizeof(int32), 1, inFile))
	{
		LOG_DEBUG("S: Read FACT header\n");
	} else {
		LOG_DEBUG("E: Failed to read FACT header: Could not read bytes\n");
		return RIFF_READ_FAIL;
	}
	// basic validation
	if (memcmp(input->fact->SubchunkID, "fact", 4) != 0) {
		LOG_DEBUG("E: Invalid FACT header: SubchunkID != 'fact'\n");
		LOG_DEBUG("\tSubchunkID == %s\n", (char*)input->fact->SubchunkID);
		return RIFF_VALID_FAIL;
	} else if (input->fact->SubchunkSize != 4) {
		LOG_DEBUG("E: Invalid FACT chunk size\n");
		return RIFF_VALID_FAIL;
	}
	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFreadPEAK										*/
/* purpose: reads the peak chunk from a wav file				*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadPEAK(FILE *inFile, T *input) {
	unsigned int foo = 0;
	if (fread(input->peak->SubchunkID, sizeof(int8), 4, inFile) &&
		fread(&input->peak->SubchunkSize, sizeof(int32), 1, inFile) &&
		fread(&input->peak->Version, sizeof(int32), 1, inFile) &&
		fread(&input->peak->timestamp, sizeof(int32), 1, inFile)) 
	{
		// make sure peak size is valid
		if (input->peak->SubchunkSize != (2*sizeof(int32) + input->fmt.NumChannels * sizeof(_PPEAK)) &&
			input->peak->SubchunkSize != (2*sizeof(int32) + input->fmt.NumChannels * sizeof(_PPEAK) + sizeof(int16)))
		{
			LOG_DEBUG("E: Invalid PEAK chunk size\n");
			return RIFF_VALID_FAIL;
		}

		// allocate and read in the peak data for each channel
		input->peak->peak = (_PPEAK *)malloc(input->fmt.NumChannels * sizeof(_PPEAK));
		if (input->peak->peak == NULL) {
			LOG_DEBUG("E: Failed to read PEAK header: Could not allocate memory\n");
			return RIFF_READ_FAIL;
		}
		for (foo = 0; foo < input->fmt.NumChannels; ++foo) {
			if (!(fread(&input->peak->peak[foo].Value, sizeof(float), 1, inFile) &&
				fread(&input->peak->peak[foo].Position, sizeof(int32), 1, inFile)))
			{
				LOG_DEBUG("E: Failed to read PEAK header: Could not read bytes\n");
				return RIFF_READ_FAIL;
			}
		}

		// read the 64-bit align if it exists
		if (input->peak->SubchunkSize == (2*sizeof(int32) + input->fmt.NumChannels * sizeof(_PPEAK) + sizeof(int16))) {
			input->peak->bit_align = (int16 *)malloc(sizeof(int16));
			if (input->peak->bit_align == NULL) {
				LOG_DEBUG("E: Failed to read PEAK header: Could not allocate memory\n");
				return RIFF_READ_FAIL;
			}
			if (!fread(input->peak->bit_align, sizeof(int16), 1, inFile)) {
				LOG_DEBUG("E: Failed to read PEAK header: Could not read bytes\n");
				return RIFF_READ_FAIL;
			}
		}
		LOG_DEBUG("S: Read PEAK header\n");
	} else {
		LOG_DEBUG("E: Failed to read PEAK header: Could not read bytes\n");
		return RIFF_READ_FAIL;
	}

	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFreadDATA										*/
/* purpose: reads the data from a wav file						*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadDATA(FILE *inFile, T *input) {
	if (fread(input->data.SubchunkID, sizeof(int8), 4, inFile) &&
		fread(&input->data.SubchunkSize, sizeof(int32), 1, inFile))
	{
		LOG_DEBUG("S: Read DATA header\n");
	} else {
		LOG_DEBUG("E: Failed to read DATA header: Could not read bytes\n");
		return RIFF_READ_FAIL;
	}
	// basic validation
	if (memcmp(input->data.SubchunkID, (int8*)"data", 4) != 0) {
		LOG_DEBUG("E: Invalid DATA header: SubchunkID != 'data'\n");
		LOG_DEBUG("\tSubchunkID == %s\n", (char*)input->data.SubchunkID);
		return RIFF_VALID_FAIL;
	}
	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFwrite											*/
/* purpose: writes a wav file to disk							*/
/* args: FILE *, T *											*/
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not write					*/
/****************************************************************/
template <class T>
int RIFFwrite(FILE *outFile, const T *input) {
	int ret_val = 0;
	if (outFile == NULL) {
		LOG_DEBUG("E: Failed to write RIFF header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}
	// write riff header
	ret_val = RIFFwriteRIFF(outFile,input);
	if (!ret_val) { 
		return ret_val;
	}
	// write format header
	ret_val = RIFFwriteFMT(outFile,input);
	if (!ret_val) {
		return ret_val;
	}
	// write fact header if needed
	if (input->fact != NULL) {
		ret_val = RIFFwriteFACT(outFile,input);
		if (!ret_val) {
			return ret_val;
		}
	}
	// write peak header if needed
	if (input->peak != NULL) {
		ret_val = RIFFwritePEAK(outFile,input);
		if (!ret_val) {
			return ret_val;
		}
	}
	// write data header
	ret_val = RIFFwriteDATA(outFile,input);
	if (!ret_val) {
		return ret_val;
	}
	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFwriteRIFF										*/
/* purpose: writes the riff header to a file					*/
/* args: FILE *, const T *										*/
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteRIFF(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		LOG_DEBUG("E: Failed to write RIFF header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (fwrite(input->riff.ChunkID, sizeof(int8), 4, outFile) &&
		fwrite(&input->riff.ChunkSize, sizeof(int32), 1, outFile) &&
		fwrite(input->riff.Format, sizeof(int8), 4, outFile))
	{
		LOG_DEBUG("S: Wrote RIFF header\n");
		return RIFF_SUCCESS;
	}
	LOG_DEBUG("E: Failed to write RIFF header: Could not write bytesn\n");
	return RIFF_WRITE_FAIL;
}

/****************************************************************/
/* function: RIFFwriteFMT										*/
/* purpose: writes the fmt header to a file						*/
/* args: FILE *, const T *										*/
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteFMT(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		LOG_DEBUG("E: Failed to write FMT header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (fwrite(input->fmt.SubchunkID, sizeof(int8), 4, outFile) &&
		fwrite(&input->fmt.SubchunkSize, sizeof(int32), 1, outFile) &&
		fwrite(&input->fmt.AudioFormat, sizeof(int16), 1, outFile) &&
		fwrite(&input->fmt.NumChannels, sizeof(int16), 1, outFile) &&
		fwrite(&input->fmt.SampleRate, sizeof(int32), 1, outFile) &&
		fwrite(&input->fmt.ByteRate, sizeof(int32), 1, outFile) &&
		fwrite(&input->fmt.BlockAlign, sizeof(int16), 1, outFile) &&
		fwrite(&input->fmt.BitsPerSample, sizeof(int16), 1, outFile))
	{
		// Need to write extra stuff
		if (input->fmt.SubchunkSize-16 != 0) {
			fwrite(&input->fmt.ExtraFormatBytes, sizeof(int16), 1, outFile);
			if (input->fmt.ExtraFormatBytes > 0) {
				fwrite(&input->fmt.ValidBitsPerSample, sizeof(int16), 1, outFile);
				fwrite(&input->fmt.ChannelMask, sizeof(int32), 1, outFile);
				fwrite(input->fmt.SubFormat, sizeof(int8), 16, outFile);
			}
		}
		LOG_DEBUG("S: Wrote FMT header\n");
		return RIFF_SUCCESS;
	}
	LOG_DEBUG("E: Failed to write FMT header: Could not write bytes\n");
	return RIFF_WRITE_FAIL;
}

/****************************************************************/
/* function: RIFFwriteFACT										*/
/* purpose: writes the FACT header to a file					*/
/* args: FILE *, const T *										*/
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteFACT(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		LOG_DEBUG("E: Failed to write FACT header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (fwrite(input->fact->SubchunkID, sizeof(int8), 4, outFile) &&
		fwrite(&input->fact->SubchunkSize, sizeof(int32), 1, outFile) &&
		fwrite(&input->fact->SampleLength, sizeof(int32), 1, outFile))
	{
		LOG_DEBUG("S: Wrote FACT header\n");
		return RIFF_SUCCESS;
	}
	LOG_DEBUG("E: Failed to write FACT header: Could not write bytes\n");
	return RIFF_WRITE_FAIL;
}

/****************************************************************/
/* function: RIFFwritePEAK										*/
/* purpose: writes the PEAK header to a file					*/
/* args: FILE *, const T *										*/
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwritePEAK(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		LOG_DEBUG("E: Failed to write PEAK header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	// write the peak chunk
	if (fwrite(input->peak->SubchunkID, sizeof(int8), 4, outFile) &&
		fwrite(&input->peak->SubchunkSize, sizeof(int32), 1, outFile) &&
		fwrite(&input->peak->Version, sizeof(int32), 1, outFile) &&
		fwrite(&input->peak->timestamp, sizeof(int32), 1, outFile) &&
		fwrite(input->peak->peak, sizeof(_PPEAK), input->fmt.NumChannels,outFile))
	{
		// if 64-bit align was used, write it
		if(input->peak->bit_align != NULL) {
			if(fwrite(&input->peak->bit_align, sizeof(int16), 1, outFile)) {
				LOG_DEBUG("S: Wrote PEAK header\n");
				return RIFF_SUCCESS;
			} else {
				LOG_DEBUG("E: Failed to write PEAK header: Could not write bytes\n");
				return RIFF_WRITE_FAIL;
			}
		}
		LOG_DEBUG("S: Wrote PEAK header\n");
		return RIFF_SUCCESS;
	}
	LOG_DEBUG("E: Failed to write PEAK header: Could not write bytes\n");
	return RIFF_WRITE_FAIL;
}

/****************************************************************/
/* function: RIFFwriteDATA										*/
/* purpose: writes the data header to a file					*/
/* args: FILE *, const T *										*/
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteDATA(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		LOG_DEBUG("E: Failed to write DATA header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (fwrite(input->data.SubchunkID, sizeof(int8), 4, outFile) &&
		fwrite(&input->data.SubchunkSize, sizeof(int32), 1, outFile))
	{
		LOG_DEBUG("S: Wrote DATA header\n");
		return RIFF_SUCCESS;
	}
	LOG_DEBUG("E: Failed to write DATA header: Could not write bytes\n");
	return RIFF_WRITE_FAIL;
}

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

