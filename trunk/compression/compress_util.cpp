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
#include "compress_util.hpp"
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
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to open %s with mode rb\n",filename);
		#endif
		printf("Failed to compress file %s\n",filename);
		return COMU_IFILE_FAIL;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Opened %s with mode rb\n",filename);
	#endif
	fout = fopen(destfile,"wb");
	if(fout == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to open %s with mode wb\n",destfile);
		#endif
		printf("Failed to compress file %s\n",filename);
		fclose(fin);
		return COMU_OFILE_FAIL;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Opened %s with mode wb\n",destfile);
	#endif

	/* compress the file */
	if( def(fin, fout, level) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Could not compress data file.\n");
		#endif
		printf("Failed to compress file %s\n",filename);
		fclose(fin);
		fclose(fout);
		remove(destfile);
		return COMU_FAIL;
	}

	/* close files and exit */
	if(fclose(fout)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to close encrypted file\n");
		#endif
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Closed output file\n");
	#endif
	if(fclose(fin)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to close input file\n");
		#endif
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Closed input file\n");
	#endif
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Compressed input data.\n");
	#endif
	printf("File %s was compressed sucessfully.\n",filename);
	return COMU_SUCCESS;
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
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to open %s with mode rb\n",filename);
		#endif
		printf("Failed to decompress file %s\n",filename);
		return COMU_IFILE_FAIL;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Opened %s with mode rb\n",filename);
	#endif
	fout = fopen(destfile,"wb");
	if(fout == NULL) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to open %s with mode wb\n",destfile);
		#endif
		printf("Failed to decompress file %s\n",filename);
		fclose(fin);
		return COMU_OFILE_FAIL;
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Opened %s with mode wb\n",destfile);
	#endif

	/* decompress the file */
	if( inf(fin, fout) != 0) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Could not decompress data file.\n");
		#endif
		printf("Failed to decompress file %s\n",filename);
		fclose(fin);
		fclose(fout);
		remove(destfile);
		return COMU_FAIL;
	}

	/* close files and exit */
	if(fclose(fout)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to close encrypted file\n");
		#endif
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Closed output file\n");
	#endif
	if(fclose(fin)) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Failed to close input file\n");
		#endif
	}
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Closed input file\n");
	#endif
	#ifdef _DEBUGOUTPUT
	fprintf(stderr,"S: ZLIB - Decompressed input data.\n");
	#endif
	if( remove("data.z") == -1) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: ZLIB - Could not remove temporary file data.z\n");
		#endif
	}
	printf("File %s was decompressed sucessfully.\n",filename);
	return COMU_SUCCESS;
}

/****************************************************************/
/* function: comp_err											*/
/* purpose: report a compress_util error					 	*/
/* args: const int												*/
/* returns: const char *										*/
/****************************************************************/
const char *comp_err(const int ret) {
	switch (ret) {
		case COMU_SUCCESS:
			return gettext("compress util did not fail");
		case COMU_FAIL:
			return gettext("zlib could not compress/decompress file");
		case COMU_IFILE_FAIL:
			return gettext("compress util could not open input file");
		case COMU_OFILE_FAIL:
			return gettext("compress util could not open output file");
		default:
			break;
	}
	return gettext("unknown compress util error");
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

