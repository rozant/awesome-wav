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

/****************************************************************/
/* function: encrypt_file										*/
/* purpose: encrypt a file with AES								*/
/* args: const char *, const char * unsigned char *				*/
/* returns: int													*/
/* notes: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int encrypt_file(const char *filename, const char *destfile, unsigned char *key) {
	unsigned char buffer[1024], digest[32], IV[16];
	unsigned int foo = 0, keylen = sizeof(key);
	FILE *fout = NULL, *fin = NULL;
	aes_context aes_ctx;
    sha2_context sha_ctx;
	#ifdef _WIN32
	__int64 filesize, offset;
	#else
	off_t filesize, offset;
	#endif

	/* open our files */
	fout = fopen(destfile,"wb");
	if(fout == NULL) {
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		return false;
	}
	fin = fopen(filename,"rb");
	if(fin == NULL) {
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		return false;
	}

	/* determine filesize */
	if(determine_filesize(fin,&filesize) == 0) {
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
        return false;
	}

	/* generate the IV */
	generateIV(IV,filename,filesize);
	/* write the IV to the begining of the encrypted file */
	if(fwrite(IV, 1, 16, fout) != 16) {
		fprintf(stderr, "fwrite(%d bytes) failed\n", 16);
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
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
			secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
			fclose(fout);
			fclose(fin);
			remove(destfile);
			return false;
		}
		for( foo = 0; foo < 16; foo++ ) {
			buffer[foo] = (unsigned char)( buffer[foo] ^ IV[foo] );
		}

		aes_crypt_ecb( &aes_ctx, AES_ENCRYPT, buffer, buffer );
		sha2_hmac_update( &sha_ctx, buffer, 16 );

		if( fwrite( buffer, 1, 16, fout ) != 16 ) {
			fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
			secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
			fclose(fout);
			fclose(fin);
			remove(destfile);
			return false;
		}
		memcpy( IV, buffer, 16 );
	}
	/* finalize everything and write to the file */
	sha2_hmac_finish( &sha_ctx, digest );
	if( fwrite( digest, 1, 32, fout ) != 32 ) {
		fprintf( stderr, "fwrite(%d bytes) failed\n", 16 );
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
		remove(destfile);
		return false;
	}

	/* return sucessfully */
	secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
	fclose(fout);
	fclose(fin);
	return true;
}

/****************************************************************/
/* function: decrpyt_file										*/
/* purpose: decrypt a file										*/
/* args: const char *, const char *, unsigned char *			*/
/* returns: int													*/
/****************************************************************/
int decrypt_file(const char *filename, const char *destfile, unsigned char *key) {
	unsigned char buffer[1024], digest[32], IV[16], tmp[16];
	int foo = 0, keylen = sizeof(key), n = 0, lastn = 0;
	FILE *fout = NULL, *fin = NULL;
	aes_context aes_ctx;
	sha2_context sha_ctx;
	#ifdef _WIN32
	__int64 filesize, offset;
	#else
	off_t filesize, offset;
	#endif

	/* open our files */
	fout = fopen(destfile,"wb");
	if(fout == NULL) {
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		return false;
	}
	fin = fopen(filename,"rb");
	if(fin == NULL) {
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		return false;
	}

	/* determine filesize */
	if(determine_filesize(fin,&filesize) == 0) {
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
        return false;
	}

	/* make sure that the file could possibly be what it is supposed to be */
	if(filesize < 48) {
		fprintf( stderr, "File too short to be encrypted.\n" );
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
        return false;
	}
	if((filesize & 0x0F) != 0) {
		fprintf( stderr, "File size not a multiple of 16.\n" );
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
        return false;
	}
	/* subtract IV and HMAC sizes */
	filesize -= 48;

	/* read the IV in */
	if( fread( buffer, 1, 16, fin ) != 16 ) {
		fprintf( stderr, "fread(%d bytes) failed\n", 16 );
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
        return false;
	}
	memcpy( IV, buffer, 16 );
	lastn = IV[15] & 0x0F;

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
	aes_setkey_dec( &aes_ctx, digest, 256 );
	sha2_hmac_starts( &sha_ctx, digest, 32, 0 );

	for(offset = 0; offset < filesize; offset += 16) {
		if(fread( buffer, 1, 16, fin ) != 16) {
			fprintf(stderr, "fread(%d bytes) failed\n", 16);
			secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
			fclose(fout);
			fclose(fin);
		    return false;
		}
		memcpy(tmp, buffer, 16);

		sha2_hmac_update(&sha_ctx, buffer, 16);
		aes_crypt_ecb(&aes_ctx, AES_DECRYPT, buffer, buffer);

		for(foo = 0; foo < 16; foo++) {
			buffer[foo] = (unsigned char)( buffer[foo] ^ IV[foo] );
		}

		memcpy(IV, tmp, 16);

		n = ( lastn > 0 && offset == filesize - 16 ) ? lastn : 16;

		if(fwrite(buffer, 1, n, fout) != (size_t)n) {
			fprintf( stderr, "fwrite(%d bytes) failed\n", n );
			secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
			fclose(fout);
			fclose(fin);
		    return false;
		}
	}
	/* finish up */
	sha2_hmac_finish( &sha_ctx, digest );

	/* verify that everything worked correctly and verify the file */
	if(fread(buffer, 1, 32, fin) != 32) {
		fprintf( stderr, "fread(%d bytes) failed\n", 32 );
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
		return false;
	}
	if(memcmp(digest, buffer, 32) != 0) {
		fprintf( stderr, "HMAC check failed: wrong key or file corrupted.\n" );
		secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
		fclose(fout);
		fclose(fin);
		return false;
	}

	/* return sucessfully */
	secure_exit(buffer,digest,IV,&aes_ctx,&sha_ctx);
	fclose(fout);
	fclose(fin);
	return true;
}

/****************************************************************/
/* function: determine_filesize									*/
/* purpose: decrypt a file										*/
/* args: FILE *, __int64/off_t *								*/
/* returns: int													*/
/****************************************************************/
#ifdef _WIN32
int determine_filesize(FILE *fin, __int64 *filesize) {
	LARGE_INTEGER li_size;
	li_size.QuadPart = 0;
	li_size.LowPart  =
		SetFilePointer((HANDLE) _get_osfhandle( _fileno( fin ) ), li_size.LowPart, &li_size.HighPart, FILE_END);

	if(li_size.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR) {
        fprintf(stderr, "Filesize fails\n");
        return 0;
    }

    *filesize = li_size.QuadPart;
#else
int determine_filesize(FILE *fin, off_t *filesize) {
    if(( *filesize = lseek( fileno( fin ), 0, SEEK_END )) < 0) {
        fprintf(stderr,"Filesize fails\n");
		return 0;
    }
#endif
	if(fseek(fin, 0, SEEK_SET) < 0) {
		fprintf( stderr, "fseek(0,SEEK_SET) failed\n" );
		return 0;
    }
	return 1;
}

/****************************************************************/
/* function: secure_exit										*/
/* purpose: destroy compromising data that is in ram			*/
/* args: unsigned char *,  unsigned char *,  unsigned char *	*/
/*		aes_context *, sha2_context *							*/
/* returns: int													*/
/****************************************************************/
void secure_exit(unsigned char buffer[1024], unsigned char digest[32], unsigned char IV[16], aes_context *aes_ctx, sha2_context *sha_ctx) {
    memset(buffer, 0, 1024);
    memset(digest, 0, 32);
	memset(IV, 0, 16);
    memset(aes_ctx, 0, sizeof(aes_context));
    memset(sha_ctx, 0, sizeof(sha2_context));
	return;
}

/****************************************************************/
/* function: generateIV											*/
/* purpose: generate the IV for AES encryption					*/
/* args: unsigned char *[16], const char *, unsigned long int	*/
/* returns: int													*/
/* nores: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int generateIV(unsigned char IV[16], const char *filename, unsigned long int filesize) {
	unsigned char digest[32], buffer[8];
	int foo = 0;
	sha2_context sha_ctx;

	for( foo = 0; foo < 8; foo++ ) {
		buffer[foo] = (unsigned char)(filesize >> (foo << 3));
	}

	sha2_starts(&sha_ctx, 0);
	sha2_update(&sha_ctx, buffer, 8);
	sha2_update(&sha_ctx, (unsigned char *)filename, strlen(filename));
	sha2_finish(&sha_ctx, digest);

	memcpy(IV, digest, 16);

	foo = (int)(filesize & 0x0F);

	IV[15] = (unsigned char)((IV[15] & 0xF0) | foo);
    memset(digest,0,32);
	memset(buffer,0,8);
	return true;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

