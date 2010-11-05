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
#include "compress_util2.hpp"
#include "quicklz.h"
#include "../logger.hpp"
#include "../global.hpp"
#include "../util.hpp"
#include <stdio.h>
#include <stdlib.h>

/****************************************************************/
/* function: qlz_compress_file									*/
/* purpose: compress a file										*/
/* args: const char *,  const char *							*/
/* returns: int													*/
/****************************************************************/
int qlz_compress_file(const char *filename, const char *destfile) {
	FILE *fin = NULL, *fout = NULL;
	char *in_buff, *com_buff, *scratch;
	size_t read, com;

	LOG("Compressing file %s\n", filename);

	// open our files
	fin = fopen(filename, "rb");
	if (fin == NULL) {
		LOG_DEBUG("E: QLZ - Failed to open %s with mode rb\n", filename);
		LOG("Failed to compress file %s\n", filename);
		return QLZ_IFILE_FAIL;
	}
	LOG_DEBUG("S: QLZ - Opened %s with mode rb\n", filename);
	fout = fopen(destfile, "wb");
	if (fout == NULL) {
		LOG_DEBUG("E: QLZ - Failed to open %s with mode wb\n", destfile);
		LOG("Failed to compress file %s\n", filename);
		fclose(fin);
		safeRemove(filename);
		return QLZ_OFILE_FAIL;
	}
	LOG_DEBUG("S: QLZ - Opened %s with mode wb\n",destfile);

	// allocate buffers
	in_buff = (char*)malloc(C_BUFF);
	com_buff = (char*)malloc(C_BUFF+400);
	scratch = (char*)calloc(QLZ_SCRATCH_COMPRESS,sizeof(char));
	if(in_buff == NULL || com_buff == NULL || scratch == NULL) {
		LOG_DEBUG("S: QLZ - Faild to allocate buffers.\n");
		return QLZ_BUFF_MEM_FAIL;
	}
	// compress the file
	while((read = fread(in_buff, 1,C_BUFF, fin)) != 0) {
		com = qlz_compress(in_buff, com_buff, read, scratch);
		fwrite(com_buff, com, 1, fout);
	}

	// close files and exit
	if (fclose(fout)) {
		LOG_DEBUG("E: QLZ - Failed to close encrypted file\n");
	}
	LOG_DEBUG("S: QLZ - Closed output file\n");
	if (fclose(fin)) {
		LOG_DEBUG("E: QLZ - Failed to close input file\n");
	}
	LOG_DEBUG("S: QLZ - Closed input file\n");
	LOG_DEBUG("S: QLZ - Compressed input data.\n");
	LOG("File %s was compressed sucessfully.\n", filename);
	return QLZ_SUCCESS;
}

/****************************************************************/
/* function: qlz_decompress_file								*/
/* purpose: decompress a file									*/
/* args: const char *,  const char *							*/
/* returns: int													*/
/****************************************************************/
int qlz_decompress_file(const char *filename, const char *destfile) {
	FILE *fin = NULL, *fout = NULL;
	char *in_buff, *decom_buff, *scratch;
	size_t read, decom;

	LOG("Decompressing file %s\n", filename);

	// open our files
	fin = fopen(filename, "rb");
	if (fin == NULL) {
		LOG_DEBUG("E: QLZ - Failed to open %s with mode rb\n", filename);
		LOG("Failed to decompress file %s\n", filename);
		return QLZ_IFILE_FAIL;
	}
	LOG_DEBUG("S: QLZ - Opened %s with mode rb\n", filename);
	fout = fopen(destfile, "wb");
	if (fout == NULL) {
		LOG_DEBUG("E: QLZ - Failed to open %s with mode wb\n",destfile);
		LOG("Failed to decompress file %s\n", filename);
		fclose(fin);
		safeRemove(filename);
		return QLZ_OFILE_FAIL;
	}
	LOG_DEBUG("S: QLZ - Opened %s with mode wb\n", destfile);

	// allocate buffers
	in_buff = (char*)malloc(C_BUFF);
	decom_buff = (char*)malloc(C_BUFF+400);
	scratch = (char*)calloc(QLZ_SCRATCH_COMPRESS,sizeof(char));
	if(in_buff == NULL || decom_buff == NULL || scratch == NULL) {
		LOG_DEBUG("S: QLZ - Faild to allocate buffers.\n");
		return QLZ_BUFF_MEM_FAIL;
	}
	// decompress the file
	while((read = fread(in_buff, 1, 9, fin)) != 0) {
		read = qlz_size_compressed(in_buff);
		fread(in_buff + 9, 1, read - 9, fin);
		decom = qlz_decompress(in_buff, decom_buff, scratch);
		fwrite(decom_buff, decom, 1, fout);
	}

	// close files and exit
	if (fclose(fout)) {
		LOG_DEBUG("E: ZLIB - Failed to close encrypted file\n");
	}
	LOG_DEBUG("S: ZLIB - Closed output file\n");
	if (fclose(fin)) {
		LOG_DEBUG("E: ZLIB - Failed to close input file\n");
	}
	LOG_DEBUG("S: ZLIB - Closed input file\n");
	LOG_DEBUG("S: ZLIB - Decompressed input data.\n");
	if (safeRemove(filename) != 0) {
		LOG_DEBUG("E: ZLIB - Could not remove temporary file data.z\n");
	}
	LOG("File %s was decompressed sucessfully.\n", filename);
	return QLZ_SUCCESS;
}

/****************************************************************/
/* function: qlz_comp_err										*/
/* purpose: report a qlz_compress_util error				 	*/
/* args: const int												*/
/* returns: const char *										*/
/****************************************************************/
const char *qlz_comp_err(const int ret) {
	switch (ret) {
		case QLZ_SUCCESS:
			return gettext("qlz util did not fail");
		case QLZ_FAIL:
			return gettext("qlz util could not compress/decompress file");
		case QLZ_IFILE_FAIL:
			return gettext("qlz util could not open input file");
		case QLZ_OFILE_FAIL:
			return gettext("qlz util could not open output file");
		case QLZ_BUFF_MEM_FAIL:
			return gettext("qlz util could not allocate buffers");
		default:
			break;
	}
	return gettext("unknown compress util error");
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

