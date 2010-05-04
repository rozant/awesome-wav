/****************************************************************/
/* The following File is mostly code acquired from the zlib		*/
/* examples of use. zlib licence can be found in				*/
/* ./docs/zlib-license.txt										*/
/****************************************************************/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#ifdef _WIN32
#include "../win32/include/zlib.h"
#else
#include <zlib.h>
#endif
#include "file_compression.h"
#include "../global.hpp"

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

/****************************************************************/
/* function: def												*/
/* purpose: compress a with a compression level				 	*/
/* args: FILE *, FILE *, const int								*/
/* returns: int													*/
/****************************************************************/
int def(FILE *source, FILE *dest, const int level) {
	int ret = 0, flush = 0;
	unsigned int have = 0;
	z_stream strm;
	unsigned char in[Z_CHUNK];
	unsigned char out[Z_CHUNK];

	/* allocate deflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	ret = deflateInit(&strm, level);
	if (ret != Z_OK) {
		return ret;
	}

	/* compress until end of file */
	do {
		strm.avail_in = fread(in, 1, Z_CHUNK, source);
		if (ferror(source)) {
			(void)deflateEnd(&strm);
			return Z_ERRNO;
		}
		flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
		strm.next_in = in;

		/* run deflate() on input until output buffer not full, finish
		   compression if all of source has been read in */
		do {
			strm.avail_out = Z_CHUNK;
			strm.next_out = out;
								 /* no bad return value */
			ret = deflate(&strm, flush);
								 /* state not clobbered */
			assert(ret != Z_STREAM_ERROR);
			have = Z_CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)deflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
								 /* all input will be used */
		assert(strm.avail_in == 0);
		/* done when last data in file processed */
	} while (flush != Z_FINISH);
	assert(ret == Z_STREAM_END); /* stream will be complete */

	/* clean up and return */
	(void)deflateEnd(&strm);
	return Z_OK;
}

/****************************************************************/
/* function: inf												*/
/* purpose: decompress a file 									*/
/* args: FILE *, FILE *											*/
/* returns: int													*/
/****************************************************************/
int inf(FILE *source, FILE *dest) {
	int ret;
	unsigned int have;
	z_stream strm;
	unsigned char in[Z_CHUNK];
	unsigned char out[Z_CHUNK];

	/* allocate inflate state */
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	ret = inflateInit(&strm);
	if (ret != Z_OK) {
		return ret;
	}

	/* decompress until deflate stream ends or end of file */
	do {
		strm.avail_in = fread(in, 1, Z_CHUNK, source);
		if (ferror(source)) {
			(void)inflateEnd(&strm);
			return Z_ERRNO;
		}
		if (strm.avail_in == 0)
			break;
		strm.next_in = in;

		/* run inflate() on input until output buffer not full */
		do {
			strm.avail_out = Z_CHUNK;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
								 /* state not clobbered */
			assert(ret != Z_STREAM_ERROR);
			switch (ret) {
				case Z_NEED_DICT:
								 /* and fall through */
					ret = Z_DATA_ERROR;
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return ret;
			}
			have = Z_CHUNK - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
				(void)inflateEnd(&strm);
				return Z_ERRNO;
			}
		} while (strm.avail_out == 0);
		/* done when inflate() says it's done */
	} while (ret != Z_STREAM_END);

	/* clean up and return */
	(void)inflateEnd(&strm);
	return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}

/****************************************************************/
/* function: zerr												*/
/* purpose: report a zlib or i/o error						 	*/
/* args: const int												*/
/* returns: const char *										*/
/****************************************************************/
const char * zerr(const int ret) {
	switch (ret) {
		case Z_ERRNO:
			if (ferror(stdin))
				return gettext("error reading stdin");
			if (ferror(stdout))
				return gettext("error writing stdout");
			break;
		case Z_STREAM_ERROR:
			return gettext("invalid compression level");
			break;
		case Z_DATA_ERROR:
			return gettext("invalid or incomplete deflate data");
			break;
		case Z_MEM_ERROR:
			return gettext("out of memory");
			break;
		case Z_VERSION_ERROR:
			return gettext("zlib version mismatch!");
		default:
			break;
	}
	return gettext("unknown zlib error");
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

