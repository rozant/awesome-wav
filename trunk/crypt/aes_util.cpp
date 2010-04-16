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
#include "aes_util.hpp"
#include "sha2_util.hpp"
#include "sha2.h"
#include <stdio.h>
#include <string.h>

/****************************************************************/
/* function: generatIV											*/
/* purpose: generate the IV for AES encryption					*/
/* args: unsigned char *[16], const char * 						*/
/* returns: int													*/
/* nores: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int generateIV(unsigned char IV[16], const char *filename) {
	unsigned char buffer[1024];
	unsigned char digest[32];
	int foo = 0, lastn = 0;
	sha2_context sha_ctx;

	for( foo = 0; foo < 8; foo++ ) {
		buffer[foo] = (unsigned char)(filesize >> (foo << 3));
	}

	sha2_starts(&sha_ctx, 0);
	sha2_update(&sha_ctx, buffer, 8);
	sha2_update(&sha_ctx, (unsigned char *)filename, strlen(filename));
	sha2_finish(&sha_ctx, digest);

	memcpy(IV, digest, 16);

	lastn = (int)(filesize & 0x0F);

	IV[15] = (unsigned char)((IV[15] & 0xF0) | lastn);
	return true;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/
