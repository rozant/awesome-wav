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
int RIFFread(int, T *);
template <class T>
int RIFFreadRIFF(int, T *);
template <class T>
int RIFFreadFMT(int, T *);
template <class T>
int RIFFreadFACT(int, T *);
template <class T>
int RIFFreadPEAK(int, T *);
template <class T>
int RIFFreadDATA(int, T *);

// write
template <class T>
int RIFFwrite(int, const T *);
template <class T>
int RIFFwriteRIFF(int, const T *);
template <class T>
int RIFFwriteFMT(int, const T *);
template <class T>
int RIFFwriteFACT(int, const T *);
template <class T>
int RIFFwritePEAK(int, const T *);
template <class T>
int RIFFwriteDATA(int, const T *);

/***************************Functions****************************/
/****************************************************************/
/* function: RIFFread											*/
/* purpose: reads a wav file into memory						*/
/* args: int, T *											    */
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFread(int inFile, T *input) {
	long int f_pos, f_data_pos = 0, l_pos, l_data_pos = 0, l_end_pos;
	bool read_fmt_chunk = false;
	char chunk_type[5];
	int32 chunk_size;
	int ret_val;

	chunk_type[4] = '\0';

	// figure out the file size
	l_end_pos = lseek(inFile, 0, SEEK_END);
	lseek(inFile, 0, SEEK_SET);

	// read riff chunk
	ret_val = RIFFreadRIFF(inFile, input);
	if (ret_val != RIFF_SUCCESS) {
		return ret_val;
	}

	l_pos = lseek(inFile, 0, SEEK_CUR); // get the current file position
    f_pos = l_pos;

	while (l_pos < l_end_pos) {
		if (!read(inFile, chunk_type, 4 * sizeof(char))) { // read the header type
			LOG_DEBUG("E: Failed to read header: Could not read bytes\n");
			return RIFF_READ_FAIL;
		}

		if (memcmp(chunk_type, "fact", 4) == 0) { // read fact chunk if needed
			lseek(inFile, f_pos, SEEK_SET);
			input->fact = (_FACT *)calloc(1, sizeof(_FACT));
			ret_val = RIFFreadFACT(inFile, input);
			if (ret_val != RIFF_SUCCESS) {
				return ret_val;
			}
		} else if (memcmp(chunk_type, "PEAK", 4) == 0) { // read peak chunk if needed
			lseek(inFile, f_pos, SEEK_SET);
			input->peak = (_PEAK *)calloc(1, sizeof(_PEAK));
			ret_val = RIFFreadPEAK(inFile, input);
			if (ret_val != RIFF_SUCCESS) {
				return ret_val;
			}
		} else if (memcmp(chunk_type, "fmt ", 4) == 0) { // read fmt chunk
			lseek(inFile, f_pos, SEEK_SET);
			ret_val = RIFFreadFMT(inFile, input);
			if (ret_val != RIFF_SUCCESS) {
				return ret_val;
			}
			read_fmt_chunk = true;
		} else if (memcmp(chunk_type, "data", 4) == 0) { // read data chunk
			if (!read_fmt_chunk) {
				break;
			}

			lseek(inFile, f_pos, SEEK_SET);
			ret_val = RIFFreadDATA(inFile, input);
			l_pos = lseek(inFile, 0, SEEK_CUR);
			if (ret_val != RIFF_SUCCESS || l_pos + (long int)input->data.SubchunkSize > l_end_pos) {
				LOG("Invalid data chunk size.\n");
				return ret_val;
			}

			 // skip over the data chunk
			f_data_pos = f_pos;	
			l_data_pos = lseek(inFile, 0, SEEK_CUR);
			lseek(inFile, (long int)input->data.SubchunkSize, SEEK_CUR);
		} else {
			LOG_DEBUG("W: Found unknown header '%s'\n", chunk_type);
			if (!read(inFile, &chunk_size, 1 * sizeof(int32))) {
				return RIFF_READ_FAIL;
			}
			l_pos = lseek(inFile, 0, SEEK_CUR);
            f_pos = l_pos;

			// skip over the unknown chunk	
			if (l_pos + (long int)chunk_size > l_end_pos) {
				LOG("Invalid unknown chunk size.\n");
				return RIFF_READ_FAIL;
			}
			
			lseek(inFile, (long int)chunk_size, SEEK_CUR);
		}

		l_pos = lseek(inFile, 0, SEEK_CUR); // get the current file position
        f_pos = l_pos;
	}

	if (!read_fmt_chunk) {
		LOG("Never found the fmt chunk.\n");
		return RIFF_READ_FAIL;
	}

	if (l_data_pos == 0) {
		LOG("Never found the data chunk.\n");
		return RIFF_READ_FAIL;
	}

    lseek(inFile, f_data_pos, SEEK_SET);

	return RIFF_SUCCESS;
}

/****************************************************************/
/* function: RIFFreadRIFF										*/
/* purpose: reads the riff header from a wav file				*/
/* args: int, T *											    */
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadRIFF(int inFile, T *input) {
	// read
	if (read(inFile, input->riff.ChunkID, 4 * sizeof(int8)) &&
		read(inFile, &input->riff.ChunkSize, 1 * sizeof(int32)) &&
		read(inFile, input->riff.Format,4*sizeof(int8)))
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
/* args: int, T *											    */
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadFMT(int inFile, T *input) {
	if (read(inFile, input->fmt.SubchunkID, 4 * sizeof(int8)) &&
		read(inFile, &input->fmt.SubchunkSize, 1 * sizeof(int32)) &&
		read(inFile, &input->fmt.AudioFormat, 1 * sizeof(int16)) &&
		read(inFile, &input->fmt.NumChannels, 1 * sizeof(int16)) &&
		read(inFile, &input->fmt.SampleRate, 1 * sizeof(int32)) &&
		read(inFile, &input->fmt.ByteRate, 1 * sizeof(int32)) &&
		read(inFile, &input->fmt.BlockAlign, 1 * sizeof(int16)) &&
		read(inFile, &input->fmt.BitsPerSample, 1 * sizeof(int16)))
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
			if(read(inFile, &input->fmt.ExtraFormatBytes, 1 * sizeof(int16))) {
				if (input->fmt.ExtraFormatBytes == 22) {
					if (!(read(inFile, &input->fmt.ValidBitsPerSample, 1 * sizeof(int16)) &&
						read(inFile, &input->fmt.ChannelMask, 1 * sizeof(int32)) &&
						read(inFile, input->fmt.SubFormat, 16 * sizeof(int8))))
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
				LOG_DEBUG("E: Failed to read Extended FMT header\n");
			}
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
/* args: int, T *											    */
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadFACT(int inFile, T *input) {
	if (read(inFile, input->fact->SubchunkID, 4 * sizeof(int8)) &&
		read(inFile, &input->fact->SubchunkSize, 1 * sizeof(int32)) &&
		read(inFile, &input->fact->SampleLength, 1 * sizeof(int32)))
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
/* args: int, T *											    */
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadPEAK(int inFile, T *input) {
	unsigned int foo = 0;
	if (read(inFile, input->peak->SubchunkID, 4 * sizeof(int8)) &&
		read(inFile, &input->peak->SubchunkSize, 1 * sizeof(int32)) &&
		read(inFile, &input->peak->Version, 1 * sizeof(int32)) &&
		read(inFile, &input->peak->timestamp, 1 * sizeof(int32))) 
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
			if (!(read(inFile, &input->peak->peak[foo].Value, 1 * sizeof(float)) &&
				read(inFile, &input->peak->peak[foo].Position, 1 * sizeof(int32))))
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
			if (!read(inFile, input->peak->bit_align, 1 * sizeof(int16))) {
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
/* args: int, T *											    */
/* returns: int													*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
int RIFFreadDATA(int inFile, T *input) {
	if (read(inFile, input->data.SubchunkID, 4 *  sizeof(int8)) &&
		read(inFile, &input->data.SubchunkSize, 1 * sizeof(int32)))
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
/* args: int, T *											    */
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not write					*/
/****************************************************************/
template <class T>
int RIFFwrite(int outFile, const T *input) {
	int ret_val = RIFF_SUCCESS;
	if (outFile < 0) {
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

	return ret_val;
}

/****************************************************************/
/* function: RIFFwriteRIFF										*/
/* purpose: writes the riff header to a file					*/
/* args: int, const T *										    */
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteRIFF(int outFile, const T *input) {
	if (outFile < 0) {
		LOG_DEBUG("E: Failed to write RIFF header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (write(outFile, input->riff.ChunkID, 4 * sizeof(int8)) &&
		write(outFile, &input->riff.ChunkSize, 1 * sizeof(int32)) &&
		write(outFile, input->riff.Format, 4 * sizeof(int8)))
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
/* args: int, const T *										    */
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteFMT(int outFile, const T *input) {
	if (outFile < 0) {
		LOG_DEBUG("E: Failed to write FMT header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (write(outFile, input->fmt.SubchunkID, 4 * sizeof(int8)) &&
		write(outFile, &input->fmt.SubchunkSize, 1 * sizeof(int32)) &&
		write(outFile, &input->fmt.AudioFormat, 1 * sizeof(int16)) &&
		write(outFile, &input->fmt.NumChannels, 1 * sizeof(int16)) &&
		write(outFile, &input->fmt.SampleRate, 1 * sizeof(int32)) &&
		write(outFile, &input->fmt.ByteRate, 1 * sizeof(int32)) &&
		write(outFile, &input->fmt.BlockAlign, 1 * sizeof(int16)) &&
		write(outFile, &input->fmt.BitsPerSample, 1 * sizeof(int16)))
	{
		// Need to write extra stuff
		if (input->fmt.SubchunkSize-16 != 0) {
			write(outFile, &input->fmt.ExtraFormatBytes, 1 * sizeof(int16));
			if (input->fmt.ExtraFormatBytes > 0) {
				write(outFile, &input->fmt.ValidBitsPerSample, 1 * sizeof(int16));
				write(outFile, &input->fmt.ChannelMask, 1 * sizeof(int32));
				write(outFile, input->fmt.SubFormat, 16 * sizeof(int8));
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
/* args: int, const T *										    */
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteFACT(int outFile, const T *input) {
	if (outFile < 0) {
		LOG_DEBUG("E: Failed to write FACT header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (write(outFile, input->fact->SubchunkID, 4 * sizeof(int8)) &&
		write(outFile, &input->fact->SubchunkSize, 1 * sizeof(int32)) &&
		write(outFile, &input->fact->SampleLength, 1 * sizeof(int32)))
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
/* args: int, const T *										    */
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwritePEAK(int outFile, const T *input) {
	if (outFile < 0) {
		LOG_DEBUG("E: Failed to write PEAK header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	// write the peak chunk
	if (write(outFile, input->peak->SubchunkID, 4 * sizeof(int8)) &&
		write(outFile, &input->peak->SubchunkSize, 1 * sizeof(int32)) &&
		write(outFile, &input->peak->Version, 1 * sizeof(int32)) &&
		write(outFile, &input->peak->timestamp, 1 * sizeof(int32)) &&
		write(outFile, input->peak->peak, input->fmt.NumChannels * sizeof(_PPEAK)))
	{
		// if 64-bit align was used, write it
		if(input->peak->bit_align != NULL) {
			if(write(outFile, &input->peak->bit_align, 1 * sizeof(int16))) {
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
/* args: int, const T *										    */
/* returns: int													*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
int RIFFwriteDATA(int outFile, const T *input) {
	if (outFile < 0) {
		LOG_DEBUG("E: Failed to write DATA header: FILE not open\n");
		return RIFF_FILE_CLOSED;
	}

	if (write(outFile, input->data.SubchunkID, 4 * sizeof(int8)) &&
		write(outFile, &input->data.SubchunkSize, 1 * sizeof(int32)))
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

