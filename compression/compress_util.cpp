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
#include "file_compression.h"
#include <stdio.h>

/****************************************************************/
/* function: compress_file										*/
/* purpose: compress a file										*/
/* args: const char *,  const char *,  const char				*/
/* returns: int													*/
/****************************************************************/
int compress_file(const char *filename, const char *destfile, const char level) {
	FILE *fin = NULL, *fout = NULL;

	/* open our files */
	fin = fopen(filename,"rb");
	if(fin == NULL) {
		return -1;
	}
	fout = fopen(destfile,"wb");
	if(fout == NULL) {
		fclose(fin);
		return -1;
	}

	if( def(fin, fout, level) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Could not compress data file.\n");
		#endif
		fclose(fin);
		fclose(fout);
		remove(destfile);
		return -1;
	}

	fclose(fin);
	fclose(fout);
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Compressed input data.\n");
	#endif
	return 0;
}

/****************************************************************/
/* function: decompress_file									*/
/* purpose: decompress a file									*/
/* args: const char *,  const char *							*/
/* returns: int													*/
/****************************************************************/
int decompress_file(const char *filename, const char *destfile) {
	FILE *fin = NULL, *fout = NULL;

	/* open our files */
	fin = fopen(filename,"rb");
	if(fin == NULL) {
		return -1;
	}
	fout = fopen(destfile,"wb");
	if(fout == NULL) {
		fclose(fin);
		return -1;
	}

	if( inf(fin, fout) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Could not decompress data file.\n");
		#endif
		fclose(fin);
		fclose(fout);
		remove(destfile);
		return -1;
	}

	fclose(fin);
	fclose(fout);
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: Decompressed input data.\n");
	#endif
	if( remove("data.z") == -1) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Could not remove temporary file data.z\n");
		#endif
	}
	return 0;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

