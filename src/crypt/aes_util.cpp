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
#include "../logger.hpp"
#include "../global.hpp"
#include "../util.hpp"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#ifndef _WIN32
#include <sys/types.h>
#include <unistd.h>
#endif

/****************************************************************/
/* function: encrypt_file_ecb									*/
/* purpose: encrypt a file with AES ECB							*/
/* args: const char *, const char * const unsigned char *		*/
/* returns: int													*/
/* notes: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int encrypt_file_ecb(const char *filename, const char *destfile, const unsigned char *key) {
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

	LOG("Encrypting file %s with AES ECB\n", filename);

	// open our files
	fin = fopen(filename, "rb");
	if (fin == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode rb\n", filename);
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		return AES_IFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode rb\n", filename);
	fout = fopen(destfile, "wb");
	if (fout == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode wb\n", destfile);
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fin);
		safeRemove(filename);
		return AES_OFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode wb\n", destfile);

	// determine filesize
	if (determine_filesize(fin, &filesize) == 0) {
		LOG_DEBUG("E: AES - Failed to determine filesize\n");
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		safeRemove(filename);
		safeRemove(destfile);
        return AES_FAIL;
	}
	LOG_DEBUG("S: AES - Filesize is %ld\n", filesize);

	// generate the IV
	generateIV(IV, filename,filesize);
	// write the IV to the begining of the encrypted file
	if (fwrite(IV, 1, 16, fout) != 16) {
		LOG_DEBUG("E: AES - Failed to write IV\n");
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		safeRemove(filename);
		safeRemove(destfile);
		return AES_WRITE_FAIL;
	}
	LOG_DEBUG("S: AES - Wrote IV\n");

	// hash the IV and the key together for setting up the aes context + hmac
	memset( digest, 0,  32 );
	memcpy( digest, IV, 16 );

	for (foo = 0; foo < 8192; ++foo) {
		sha2_starts(&sha_ctx, 0);
 		sha2_update(&sha_ctx, digest, 32);
		sha2_update(&sha_ctx, key, keylen);
		sha2_finish(&sha_ctx, digest);
	}

	aes_setkey_enc(&aes_ctx, digest, 256);
	sha2_hmac_starts(&sha_ctx, digest, 32, 0);

	// encrypt and write the encrypted data
	for (offset = 0; offset < filesize; offset += 16) {
		foo = (filesize - offset > 16) ? 16 : (int)(filesize - offset);

		if (fread(buffer, 1, foo, fin) != (size_t) foo) {
			LOG_DEBUG("E: AES - Failed to read data for encryption\n");
			LOG("Failed to encrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
			safeRemove(filename);
			safeRemove(destfile);
			return AES_READ_FAIL;
		}
		for (foo = 0; foo < 16; foo++) {
			buffer[foo] = (unsigned char)(buffer[foo] ^ IV[foo]);
		}

		aes_crypt_ecb(&aes_ctx, AES_ENCRYPT, buffer, buffer);
		sha2_hmac_update(&sha_ctx, buffer, 16);

		if (fwrite(buffer, 1, 16, fout) != 16) {
			LOG_DEBUG("E: AES - Failed to write encrypted data\n");
			LOG("Failed to encrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
			safeRemove(filename);
			safeRemove(destfile);
			return AES_WRITE_FAIL;
		}
		memcpy( IV, buffer, 16 );
	}
	LOG_DEBUG("S: AES - Wrote encrypted data\n");
	// finalize everything and write to the file
	sha2_hmac_finish(&sha_ctx, digest);
	if (fwrite(digest, 1, 32, fout) != 32) {
		LOG_DEBUG("E: AES - Failed to write end of file\n");
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		safeRemove(filename);
		safeRemove(destfile);
		return AES_WRITE_FAIL;
	}
	LOG_DEBUG("S: AES - Wrote end of file\n");

	// return sucessfully
	secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
	if (fclose(fout)) {
		LOG_DEBUG("E: AES - Failed to close encrypted file\n");
	}
	LOG_DEBUG("S: AES - Closed output file\n");
	if (fclose(fin)) {
		LOG_DEBUG("E: AES - Failed to close input file\n");
	}
	LOG_DEBUG("S: AES - Closed input file\n");
	LOG_DEBUG("S: AES - Data encrypted sucessfully\n");
	LOG("File %s was encrypted sucessfully.\n", filename);
	return AES_SUCCESS;
}

/****************************************************************/
/* function: encrypt_file_cbc									*/
/* purpose: encrypt a file with AES CBC							*/
/* args: const char *, const char * const unsigned char *		*/
/* returns: int													*/
/* notes: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int encrypt_file_cbc(const char *filename, const char *destfile, const unsigned char *key) {
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

	LOG("Encrypting file %s with AES CBC\n", filename);

	// open our files
	fin = fopen(filename, "rb");
	if (fin == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode rb\n", filename);
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		return AES_IFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode rb\n", filename);
	fout = fopen(destfile, "wb");
	if (fout == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode wb\n", destfile);
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fin);
		safeRemove(filename);
		return AES_OFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode wb\n", destfile);

	// determine filesize
	if (determine_filesize(fin, &filesize) == 0) {
		LOG_DEBUG("E: AES - Failed to determine filesize\n");
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		safeRemove(filename);
		safeRemove(destfile);
        return AES_FAIL;
	}
	LOG_DEBUG("S: AES - Filesize is %ld\n", filesize);

	// generate the IV
	generateIV(IV, filename,filesize);
	// write the IV to the begining of the encrypted file
	if (fwrite(IV, 1, 16, fout) != 16) {
		LOG_DEBUG("E: AES - Failed to write IV\n");
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		safeRemove(filename);
		safeRemove(destfile);
		return AES_WRITE_FAIL;
	}
	LOG_DEBUG("S: AES - Wrote IV\n");

	// hash the IV and the key together for setting up the aes context + hmac
	memset( digest, 0,  32 );
	memcpy( digest, IV, 16 );

	for (foo = 0; foo < 8192; ++foo) {
		sha2_starts(&sha_ctx, 0);
 		sha2_update(&sha_ctx, digest, 32);
		sha2_update(&sha_ctx, key, keylen);
		sha2_finish(&sha_ctx, digest);
	}

	aes_setkey_enc(&aes_ctx, digest, 256);
	sha2_hmac_starts(&sha_ctx, digest, 32, 0);

	// encrypt and write the encrypted data
	for (offset = 0; offset < filesize; offset += 16) {
		foo = (filesize - offset > 16) ? 16 : (int)(filesize - offset);

		if (fread(buffer, 1, foo, fin) != (size_t) foo) {
			LOG_DEBUG("E: AES - Failed to read data for encryption\n");
			LOG("Failed to encrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
			safeRemove(filename);
			safeRemove(destfile);
			return AES_READ_FAIL;
		}

		aes_crypt_cbc(&aes_ctx, AES_ENCRYPT, foo, IV, buffer, buffer);
		sha2_hmac_update(&sha_ctx, buffer, 16);

		if (fwrite(buffer, 1, 16, fout) != 16) {
			LOG_DEBUG("E: AES - Failed to write encrypted data\n");
			LOG("Failed to encrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
			safeRemove(filename);
			safeRemove(destfile);
			return AES_WRITE_FAIL;
		}
	}
	LOG_DEBUG("S: AES - Wrote encrypted data\n");
	// finalize everything and write to the file
	sha2_hmac_finish(&sha_ctx, digest);
	if (fwrite(digest, 1, 32, fout) != 32) {
		LOG_DEBUG("E: AES - Failed to write end of file\n");
		LOG("Failed to encrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		safeRemove(filename);
		safeRemove(destfile);
		return AES_WRITE_FAIL;
	}
	LOG_DEBUG("S: AES - Wrote end of file\n");

	// return sucessfully
	secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);

	if (fclose(fout)) {
		LOG_DEBUG("E: AES - Failed to close encrypted file\n");
	}
	LOG_DEBUG("S: AES - Closed output file\n");
	if (fclose(fin)) {
		LOG_DEBUG("E: AES - Failed to close input file\n");
	}
	LOG_DEBUG("S: AES - Closed input file\n");
	LOG_DEBUG("S: AES - Data encrypted sucessfully\n");
	LOG("File %s was encrypted sucessfully.\n", filename);
	return AES_SUCCESS;
}

/****************************************************************/
/* function: decrpyt_file_ecb									*/
/* purpose: decrypt a file encrypted using AES ECB				*/
/* args: const char *, const char *, unsigned char *			*/
/* returns: int													*/
/****************************************************************/
int decrypt_file_ecb(const char *filename, const char *destfile, const unsigned char *key) {
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

	LOG("Decrypting file %s encrypted with AES ECB\n", filename);

	// open our files
	fin = fopen(filename, "rb");
	if (fin == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode rb\n", filename);
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		return AES_IFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode rb\n", filename);
	fout = fopen(destfile, "wb");
	if (fout == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode wb\n", destfile);
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fin);
		return AES_OFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode wb\n",destfile);

	// determine filesize
	if (determine_filesize(fin, &filesize) == 0) {
		LOG_DEBUG("E: AES - Failed to determine filesize\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_FAIL;
	}
	LOG_DEBUG("S: AES - Filesize is %ld\n", filesize);


	// make sure that the file could possibly be what it is supposed to be
	if (filesize < 48) {
		LOG_DEBUG("E: AES - File too short to be encrypted\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_FAIL;
	}
	if ((filesize & 0x0F) != 0) {
		LOG_DEBUG("E: AES - File size not a multiple of 16\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_FAIL;
	}
	// subtract IV and HMAC sizes
	filesize -= 48;

	// read the IV in
	if (fread(buffer, 1, 16, fin) != 16) {
		LOG_DEBUG("E: AES - Failed to read the IV\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_READ_FAIL;
	}
	LOG_DEBUG("S: AES - Read the IV\n");
	memcpy(IV, buffer, 16);
	lastn = IV[15] & 0x0F;

	// hash the IV and the key together for setting up the aes context + hmac
	memset(digest, 0,  32);
	memcpy(digest, IV, 16);

	for (foo = 0; foo < 8192; ++foo) {
		sha2_starts(&sha_ctx, 0);
 		sha2_update(&sha_ctx, digest, 32);
		sha2_update(&sha_ctx, key, keylen);
		sha2_finish(&sha_ctx, digest);
	}

	aes_setkey_dec(&aes_ctx, digest, 256);
	sha2_hmac_starts(&sha_ctx, digest, 32, 0);

	for (offset = 0; offset < filesize; offset += 16) {
		if (fread(buffer, 1, 16, fin) != 16) {
			LOG_DEBUG("E: AES - Failed to read encrypted data\n");
			LOG("Failed to decrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
		    return AES_READ_FAIL;
		}
		memcpy(tmp, buffer, 16);

		sha2_hmac_update(&sha_ctx, buffer, 16);
		aes_crypt_ecb(&aes_ctx, AES_DECRYPT, buffer, buffer);

		for (foo = 0; foo < 16; foo++) {
			buffer[foo] = (unsigned char)(buffer[foo] ^ IV[foo]);
		}

		memcpy(IV, tmp, 16);

		n = (lastn > 0 && offset == filesize - 16) ? lastn : 16;

		if (fwrite(buffer, 1, n, fout) != (size_t)n) {
			LOG_DEBUG("E: AES - Failed to write decrypted data\n");
			LOG("Failed to decrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
		    return AES_WRITE_FAIL;
		}
	}
	// finish up
	sha2_hmac_finish(&sha_ctx, digest );

	// verify that everything worked correctly and verify the file
	if (fread(buffer, 1, 32, fin) != 32) {
		LOG_DEBUG("E: AES - Failed to read end of file\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		return AES_READ_FAIL;
	}
	if (memcmp(digest, buffer, 32) != 0) {
		LOG_DEBUG("E: AES - HMAC check failed: wrong key or file corrupted\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		return AES_FAIL;
	}
	LOG_DEBUG("S: AES - Decrypted data passed\n");

	// return sucessfully
	secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
	if (fclose(fout)) {
		LOG_DEBUG("E: AES - Failed to close decrypted file\n");
	}
	LOG_DEBUG("S: AES - Closed decrypted file\n");
	if (fclose(fin)) {
		LOG_DEBUG("E: AES - Failed to close input file\n");
	}
	LOG_DEBUG("S: AES - Closed input file\n");
	LOG_DEBUG("S: AES - Data decrypted sucessfully\n");
	LOG("File %s was decrypted sucessfully.\n", filename);
	return AES_SUCCESS;
}

/****************************************************************/
/* function: decrypt_file_cbc									*/
/* purpose: decrypt a file encrypted using AES ECB				*/
/* args: const char *, const char * const unsigned char *		*/
/* returns: int													*/
/* notes: IV = SHA-256( filesize || filename )[0..15]			*/
/****************************************************************/
int decrypt_file_cbc(const char *filename, const char *destfile, const unsigned char *key) {
	unsigned char buffer[1024], digest[32], IV[16];
	int foo = 0, keylen = sizeof(key), n = 16;
	FILE *fout = NULL, *fin = NULL;
	aes_context aes_ctx;
	sha2_context sha_ctx;
	#ifdef _WIN32
	__int64 filesize, offset;
	#else
	off_t filesize, offset;
	#endif

	LOG("Decrypting file %s encrypted with AES CBC\n", filename);

	// open our files
	fin = fopen(filename, "rb");
	if (fin == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode rb\n", filename);
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		return AES_IFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode rb\n", filename);
	fout = fopen(destfile, "wb");
	if (fout == NULL) {
		LOG_DEBUG("E: AES - Failed to open %s with mode wb\n", destfile);
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fin);
		return AES_OFILE_FAIL;
	}
	LOG_DEBUG("S: AES - Opened %s with mode wb\n",destfile);

	// determine filesize
	if (determine_filesize(fin, &filesize) == 0) {
		LOG_DEBUG("E: AES - Failed to determine filesize\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_FAIL;
	}
	LOG_DEBUG("S: AES - Filesize is %ld\n", filesize);

	// make sure that the file could possibly be what it is supposed to be
	if (filesize < 48) {
		LOG_DEBUG("E: AES - File too short to be encrypted\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_FAIL;
	}
	if ((filesize & 0x0F) != 0) {
		LOG_DEBUG("E: AES - File size not a multiple of 16\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_FAIL;
	}
	// subtract IV and HMAC sizes
	filesize -= 48;

	// read the IV in
	if (fread(buffer, 1, 16, fin) != 16) {
		LOG_DEBUG("E: AES - Failed to read the IV\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
        return AES_READ_FAIL;
	}
	LOG_DEBUG("S: AES - Read the IV\n");
	memcpy(IV, buffer, 16);

	// hash the IV and the key together for setting up the aes context + hmac
	memset(digest, 0,  32);
	memcpy(digest, IV, 16);

	for (foo = 0; foo < 8192; ++foo) {
		sha2_starts(&sha_ctx, 0);
 		sha2_update(&sha_ctx, digest, 32);
		sha2_update(&sha_ctx, key, keylen);
		sha2_finish(&sha_ctx, digest);
	}

	aes_setkey_dec(&aes_ctx, digest, 256);
	sha2_hmac_starts(&sha_ctx, digest, 32, 0);

	for (offset = 0; offset < filesize; offset += n) {
		if (fread(buffer, 1, n, fin) != (size_t)n) {
			LOG_DEBUG("E: AES - Failed to read encrypted data\n");
			LOG("Failed to decrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
		    return AES_READ_FAIL;
		}

		sha2_hmac_update(&sha_ctx, buffer, n);
		aes_crypt_cbc(&aes_ctx, AES_DECRYPT, n , IV, buffer, buffer);

		if (fwrite(buffer, 1, n, fout) != (size_t)n) {
			LOG_DEBUG("E: AES - Failed to write decrypted data\n");
			LOG("Failed to decrypt file %s\n", filename);
			secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
			fclose(fout);
			fclose(fin);
		    return AES_WRITE_FAIL;
		}
	}

	// finish up
	sha2_hmac_finish(&sha_ctx, digest );

	// verify that everything worked correctly and verify the file
	if (fread(buffer, 1, 32, fin) != 32) {
		LOG_DEBUG("E: AES - Failed to read end of file\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		return AES_READ_FAIL;
	}
	if (memcmp(digest, buffer, 32) != 0) {
		LOG_DEBUG("E: AES - HMAC check failed: wrong key or file corrupted\n");
		LOG("Failed to decrypt file %s\n", filename);
		secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
		fclose(fout);
		fclose(fin);
		return AES_FAIL;
	}
	LOG_DEBUG("S: AES - Decrypted data passed\n");

	// return sucessfully
	secure_exit(buffer, digest, IV, &aes_ctx, &sha_ctx);
	if (fclose(fout)) {
		LOG_DEBUG("E: AES - Failed to close decrypted file\n");
	}
	LOG_DEBUG("S: AES - Closed decrypted file\n");
	if (fclose(fin)) {
		LOG_DEBUG("E: AES - Failed to close input file\n");
	}
	LOG_DEBUG("S: AES - Closed input file\n");
	LOG_DEBUG("S: AES - Data decrypted sucessfully\n");
	LOG("File %s was decrypted sucessfully.\n", filename);
	return AES_SUCCESS;
}

/****************************************************************/
/* function: determine_filesize									*/
/* purpose: decrypt a file										*/
/* args: FILE *, __int64/off_t *								*/
/* returns: int													*/
/****************************************************************/
#ifdef _WIN32
int determine_filesize(FILE *fin, __int64 *filesize) {
	fseek(fin, 0, SEEK_END);
	*filesize = ftell(fin);
	fseek(fin, 0, SEEK_SET);
#else
int determine_filesize(FILE *fin, off_t *filesize) {
	*filesize = lseek(fileno(fin), 0, SEEK_END);
#endif
	if (*filesize < 0 || fseek(fin, 0, SEEK_SET) < 0) {
        return 0;
	}
	return 1;
}

/****************************************************************/
/* function: secure_exit										*/
/* purpose: destroy compromising data that is in ram			*/
/* args: unsigned char *,  unsigned char *,  unsigned char *	*/
/*		aes_context *, sha2_context *							*/
/* returns: void												*/
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
#ifdef _WIN32
int generateIV(unsigned char IV[16], const char *filename, __int64 filesize) {
#else
int generateIV(unsigned char IV[16], const char *filename, unsigned long int filesize) {
#endif
	unsigned char digest[32], buffer[8];
	int foo = 0;
	sha2_context sha_ctx;

	for (foo = 0; foo < 8; foo++ ) {
		buffer[foo] = (unsigned char)((filesize >> (foo << 3)) & 0xFF);
	}

	sha2_starts(&sha_ctx, 0);
	sha2_update(&sha_ctx, buffer, 8);
	sha2_update(&sha_ctx, (unsigned char *)filename, strlen(filename));
	sha2_finish(&sha_ctx, digest);

	memcpy(IV, digest, 16);

	foo = (int)(filesize & 0x0F);

	IV[15] = (unsigned char)((IV[15] & 0xF0) | foo);
    memset(digest, 0, 32);
	memset(buffer, 0, 8);
	return true;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

