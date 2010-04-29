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
#include "aes.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

int encrypt_file(const char *filename, const char *destfile, unsigned char *key) {
	unsigned char buffer[1024], digest[32], IV[16];
	int foo = 0, keylen = sizeof(key);
	FILE *fout = NULL, *fin = NULL;
    aes_context aes_ctx;
    sha2_context sha_ctx;
	#ifdef _WIN32
	LARGE_INTEGER li_size;
	__int64 filesize, offset;
	#else
	off_t filesize, offset;
	#endif

	fout = fopen(destfile,"w");
	if(fout == NULL) {
		return false;
	}
	fin = fopen(filename,"r");
	if(fin == NULL) {
		fclose(fout);
		return false;
	}

	#ifdef _WIN32
	/* win32 large file suport */
	li_size.QuadPart = 0;
	li_size.LowPart  =
		SetFilePointer((HANDLE) _get_osfhandle( _fileno( fin ) ), li_size.LowPart, &li_size.HighPart, FILE_END);

	if(li_size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
        fprintf(stderr, "Filesize fails\n");
		fclose(fout);
		fclose(fin);
        return false;
    }

    filesize = li_size.QuadPart;
	#else
    if(( filesize = lseek( fileno( fin ), 0, SEEK_END )) < 0) {
        fprintf(stderr,"Filesize fails\n");
        fclose(fout);
		fclose(fin);
		return false;
    }
	#endif

	/* generate the IV */
	generateIV(IV,filename,filesize);
	/* write the IV to the begining of the encrypted file */
	if(fwrite(IV, 1, 16, fout) != 16) {
		fprintf(stderr, "fwrite(%d bytes) failed\n", 16);
		fclose(fout);
		fclose(fin);
		remove(destfile);
		return false;
	}

	/* hash the IV and the key together for setting up the aes context + hmac */
	memset( digest, 0,  32 );
	memcpy( digest, IV, 16 );

	for(foo = 0; foo < 8192; ++foo) {
		sha2_starts(&sha_ctx, 0);
 		sha2_update(&sha_ctx, digest, 32);
		sha2_update(&sha_ctx, (const unsigned char *)key, keylen);
		sha2_finish(&sha_ctx, digest);
	}

	memset(key, 0, keylen);
	aes_setkey_enc( &aes_ctx, digest, 256 );
	sha2_hmac_starts( &sha_ctx, digest, 32, 0 );

	/* encrypt and write the encrypted data */
	for( offset = 0; offset < filesize; offset += 16 ) {
		foo = ( filesize - offset > 16 ) ? 16 : (int)( filesize - offset );

		if( fread( buffer, 1, foo, fin ) != (size_t) foo ) {
			fprintf( stderr, "fread(%d bytes) failed\n", foo );
			return false;
		}
		for( foo = 0; foo < 16; foo++ ) {
			buffer[foo] = (unsigned char)( buffer[foo] ^ IV[foo] );
		}

		aes_crypt_ecb( &aes_ctx, AES_ENCRYPT, buffer, buffer );
		sha2_hmac_update( &sha_ctx, buffer, 16 );

		if( fwrite( buffer, 1, 16, fout ) != 16 ) {
			fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
			return false;
		}
		memcpy( IV, buffer, 16 );
	}
	return true;
}

/****************************************************************/
/* function: generatIV											*/
/* purpose: generate the IV for AES encryption					*/
/* args: unsigned char *[16], const char * 						*/
/* returns: int													*/
/* nores: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int generateIV(unsigned char IV[16], const char *filename, unsigned long int filesize) {
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
