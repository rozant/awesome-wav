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
#include <stdio.h>
#include <string.h>
#include "global.hpp"
#include "riff.hpp"

/**********************Function Prototypes***********************/
template <class T>
bool RIFFread(FILE *, T *);
template <class T>
bool RIFFreadRIFF(FILE *, T *);
template <class T>
bool RIFFreadFMT(FILE *, T *);
template <class T>
bool RIFFreadFACT(FILE *, T *);
template <class T>
bool RIFFreadDATA(FILE *, T *);
template <class T>
bool RIFFwrite(FILE *, const T *);
template <class T>
bool RIFFwriteRIFF(FILE *, const T *);
template <class T>
bool RIFFwriteFMT(FILE *, const T *);
template <class T>
bool RIFFwriteFACT(FILE *, const T *);
template <class T>
bool RIFFwriteDATA(FILE *, const T *);
/***************************Functions****************************/
/****************************************************************/
/* function: RIFFread											*/
/* purpose: reads a wav file into memory						*/
/* args: FILE *, T *											*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
bool RIFFread(FILE *inFile, T *input) {
	char temp[4];
	fpos_t pos;
	/* read wave wav file chunks */
	if ( (RIFFreadRIFF(inFile,input) && RIFFreadFMT(inFile,input))) {
		fgetpos(inFile,&pos);
		fgets(temp,4,inFile);
		if (memcmp(temp, "fact", 4) == 0) {
			fsetpos(inFile,&pos);
			input->fact = (_FACT *)malloc(sizeof(_FACT));
			if(!RIFFreadFACT(inFile,input)) {
				return false;
			}
		} else {
			fsetpos(inFile,&pos);
		}
		if (RIFFreadDATA(inFile,input)) {
			return true;
		}
	}
	return false;
}
/****************************************************************/
/* function: RIFFreadRIFF										*/
/* purpose: reads the riff header from a wav file				*/
/* args: FILE *, T *											*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
bool RIFFreadRIFF(FILE *inFile, T *input) {
	if (fread(input->riff.ChunkID, sizeof(BYTE), 4, inFile) &&
		fread(&input->riff.ChunkSize, sizeof(DWORD), 1, inFile) &&
		fread(input->riff.Format, sizeof(BYTE), 4, inFile))
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
/* function: RIFFreadFMT										*/
/* purpose: reads the fmt header from a wav file				*/
/* args: FILE *, T *											*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
bool RIFFreadFMT(FILE *inFile, T *input) {
	if (fread(input->fmt.SubchunkID, sizeof(BYTE), 4, inFile) &&
		fread(&input->fmt.SubchunkSize, sizeof(DWORD), 1, inFile) &&
		fread(&input->fmt.AudioFormat, sizeof(SHORT), 1, inFile) &&
		fread(&input->fmt.NumChannels, sizeof(SHORT), 1, inFile) &&
		fread(&input->fmt.SampleRate, sizeof(DWORD), 1, inFile) &&
		fread(&input->fmt.ByteRate, sizeof(DWORD), 1, inFile) &&
		fread(&input->fmt.BlockAlign, sizeof(SHORT), 1, inFile) &&
		fread(&input->fmt.BitsPerSample, sizeof(SHORT), 1, inFile))
	{
		// Need to read extra stuff
		if (input->fmt.SubchunkSize-16 != 0) {
			fread(&input->fmt.ExtraFormatBytes, sizeof(SHORT), 1, inFile);
			if (input->fmt.ExtraFormatBytes == 22) {
				fread(&input->fmt.ValidBitsPerSample, sizeof(SHORT), 1, inFile);
				fread(&input->fmt.ChannelMask, sizeof(DWORD), 1, inFile);
				fread(input->fmt.SubFormat, sizeof(BYTE), 16, inFile);
			} else if (input->fmt.ExtraFormatBytes != 0) {
				#ifdef _DEBUGOUTPUT
				fprintf(stderr,"E: Invalid FMT header. Incorrect number of extra format bits.\n");
				fprintf(stderr,"\tExtra format bytes == %u\n",(unsigned int)input->fmt.ExtraFormatBytes);
				#endif
				return false;
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
/* function: RIFFreadFACT										*/
/* purpose: reads the fact chunk from a wav file				*/
/* args: FILE *, T *											*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
bool RIFFreadFACT(FILE *inFile, T *input) {
	if (fread(input->fact->SubchunkID, sizeof(BYTE), 4, inFile) &&
		fread(&input->fact->SubchunkSize, sizeof(DWORD), 1, inFile) &&
		fread(&input->fact->SampleLength, sizeof(DWORD), 1, inFile))
	{
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Read FACT header\n");
		#endif
		return true;
	}

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to read FACT header: Could not read bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: RIFFreadDATA										*/
/* purpose: reads the data from a wav file						*/
/* args: FILE *, T *											*/
/* returns: bool												*/
/*		1 = read correctly										*/
/*		0 = read incorrectly or did not read					*/
/****************************************************************/
template <class T>
bool RIFFreadDATA(FILE *inFile, T *input) {
	if (fread(input->data.SubchunkID, sizeof(BYTE), 4, inFile) &&
		fread(&input->data.SubchunkSize, sizeof(DWORD), 1, inFile))
	{
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Read DATA header\n");
		#endif
		return true;
	}

	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to read DATA header: Could not read bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: RIFFwrite											*/
/* purpose: writes a wav file to disk							*/
/* args: FILE *, T *											*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not write					*/
/****************************************************************/
template <class T>
bool RIFFwrite(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write RIFF header: FILE not open\n");
		#endif
		return false;
	}
	if (!RIFFwriteRIFF(outFile,input) || !RIFFwriteFMT(outFile,input)) { 
		return false;
	}
	if (input->fact != NULL) {
		if (!RIFFwriteFACT(outFile,input)) {
			return false;
		}
	}
	if (!RIFFwriteDATA(outFile,input)) {
		return false;
	}
	return true;
}

/****************************************************************/
/* function: RIFFwriteRIFF										*/
/* purpose: writes the riff header to a file					*/
/* args: FILE *, const T *										*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
bool RIFFwriteRIFF(FILE* outFile, const T *input) {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write RIFF header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(input->riff.ChunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&input->riff.ChunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(input->riff.Format, sizeof(BYTE), 4, outFile))
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
/* function: RIFFwriteFMT										*/
/* purpose: writes the fmt header to a file						*/
/* args: FILE *, const T *										*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
bool RIFFwriteFMT(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write FMT header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(input->fmt.SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&input->fmt.SubchunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(&input->fmt.AudioFormat, sizeof(SHORT), 1, outFile) &&
		fwrite(&input->fmt.NumChannels, sizeof(SHORT), 1, outFile) &&
		fwrite(&input->fmt.SampleRate, sizeof(DWORD), 1, outFile) &&
		fwrite(&input->fmt.ByteRate, sizeof(DWORD), 1, outFile) &&
		fwrite(&input->fmt.BlockAlign, sizeof(SHORT), 1, outFile) &&
		fwrite(&input->fmt.BitsPerSample, sizeof(SHORT), 1, outFile))
	{
		// Need to write extra stuff
		if (input->fmt.SubchunkSize-16 != 0) {
			fwrite(&input->fmt.ExtraFormatBytes, sizeof(SHORT), 1, outFile);
			if (input->fmt.ExtraFormatBytes > 0) {
				fwrite(&input->fmt.ValidBitsPerSample, sizeof(SHORT), 1, outFile);
				fwrite(&input->fmt.ChannelMask, sizeof(DWORD), 1, outFile);
				fwrite(input->fmt.SubFormat, sizeof(BYTE), 16, outFile);
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
/* function: RIFFwriteFACT										*/
/* purpose: writes the FACT header to a file					*/
/* args: FILE *, const T *										*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
bool RIFFwriteFACT(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write FACT header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(input->fact->SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&input->fact->SubchunkSize, sizeof(DWORD), 1, outFile) &&
		fwrite(&input->fact->SampleLength, sizeof(DWORD), 1, outFile))
	{
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"S: Wrote FACT header\n");
		#endif
		return true;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"E: Failed to write FACT header: Could not write bytes\n");
	#endif
	return false;
}

/****************************************************************/
/* function: RIFFwriteDATA										*/
/* purpose: writes the data header to a file					*/
/* args: FILE *, const T *										*/
/* returns: bool												*/
/*		1 = wrote correctly										*/
/*		0 = wrote incorrectly or did not open					*/
/****************************************************************/
template <class T>
bool RIFFwriteDATA(FILE *outFile, const T *input) {
	if (outFile == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Failed to write DATA header: FILE not open\n");
		#endif
		return false;
	}

	if (fwrite(input->data.SubchunkID, sizeof(BYTE), 4, outFile) &&
		fwrite(&input->data.SubchunkSize, sizeof(DWORD), 1, outFile))
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

#endif
/****************************************************************/
/****************************************************************/
/****************************************************************/
