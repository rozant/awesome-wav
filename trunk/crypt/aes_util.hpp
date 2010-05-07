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
#ifndef __aes_util_hpp__
#define __aes_util_hpp__
#include "sha2.h"
#include "aes.h"
#include <stdio.h>
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

/* compress_util return codes */
enum aes_util_ret_code {
	AES_WRITE_FAIL = -4,
	AES_READ_FAIL = -3,
	AES_FILE_FAIL = -2,
	AES_FAIL = -1,
	AES_SUCCESS = 0
};

/* function prototypes */
int encrypt_file(const char *, const char *, const unsigned char *);
int decrypt_file(const char *, const char *, const unsigned char *);
#ifdef _WIN32
int determine_filesize(FILE *, __int64 *);
#else
int determine_filesize(FILE *, off_t *);
#endif
void secure_exit(unsigned char buffer[1024], unsigned char digest[32], unsigned char IV[16], aes_context *, sha2_context *);
int generateIV(unsigned char *, const char *, unsigned long int);

#endif
/****************************************************************/
/****************************************************************/
/****************************************************************/

