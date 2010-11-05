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
#include "arg_processor.hpp"
#include "global.hpp"
#include "wav.hpp"
#include "flac.hpp"
#include "logger.hpp"
#include "util.hpp"
#ifndef _NZLIB
#include "./compression/compress_util.hpp"
#endif
#include "./compression/compress_util2.hpp"
#include "./crypt/aes_util.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************/
/* function: usage												*/
/* purpose: display the usage of the this program			 	*/
/* args: const char*											*/
/* returns: void												*/
/****************************************************************/
void usage(const char prog_name[]) {
	LOG("Useage: %s [-edcs(aes key)] arg1 arg2 arg3\n", prog_name);
	LOG("Encode data into a wav file, or decode data from a wav file.\n\n");
	LOG("  -wav\tsong is a wav file\n");
	LOG("  -flac\tsong is a flac file\n");
	LOG("  -e\tencode arg3 into arg1 and store in arg2\n");
	LOG("  -d\tdecode arg2 from arg1 using key arg3\n");
	LOG("  -c\tenable data compression with qlz\n");
	LOG("\t -If decoding, assume retrieved data is compressed\n");
#ifndef NZLIB
	LOG("  -zlib\tenable data compression with zlib\n");
	LOG("\t -If decoding, assume retrieved data is compressed\n");
	LOG("\t -Defaults to -zlib6\n");
	LOG("\t -Valid options are -zlib1 through -zlib9, from low to high compression\n");
#endif
	LOG("  -aes\tenable data encryption.  must be followed by the key.\n");
	return;
}

/****************************************************************/
/* function: version_info										*/
/* purpose: display the known version information			 	*/
/* args: void													*/
/* returns: void												*/
/****************************************************************/
void version_info(void) {
	LOG("awesome-wav version: %s\n",AWESOME_VER);
	LOG("polarssl version: %s\n",POLARSSL_VER);
	LOG("qlz version: %s\n",QLZ_VER);
	return;
}

/****************************************************************/
/* function: main												*/
/* purpose: initial function for program.					 	*/
/* args: int, char**											*/
/* returns: int													*/
/****************************************************************/
int main(int argc, char* argv[]) {
	unsigned long int size = 0x00, temp;
	int ret = EXIT_SUCCESS;
	char *temp_str = NULL;
	char *file_name = NULL;
	char *data_z = (char *)"data.z";
	char *data_aes = (char *)"data.aes";
	opts options;
	wav in_wav; // wav file definition
	flac in_flac; // flac file definition

	opt_init(&options); // set up the options struct

	// decide what to do
	if (arg_processor(argc, (const char **)argv, &options) == EXIT_FAILURE) {
		usage(argv[0]);
		opt_clean(&options);
		exit(EXIT_FAILURE);
	}

	// if we are encoding or decoding, do the right thing
	switch(options.mode) {
		case VERSION:
			version_info();
			break;
		case ENCODE:
			// if compression is enabled
			if (options.comp > 0) {
				file_name = (char *)calloc((strlen(options.data)+3), sizeof(char));
				memcpy(file_name, options.data, strlen(options.data));
				strcat(file_name, ".z");
				#ifndef _NZLIB
				if(options.comp < 10) {
					ret = compress_file(options.data, file_name, options.comp);
				} else {
				#endif
					ret = qlz_compress_file(options.data, file_name);
				#ifndef _NZLIB
				}
				#endif
				if (ret != 0) {
					#ifndef _NZLIB
					if(options.comp < 10) {
						LOG("%s\n", comp_err(ret));	
					} else {
					#endif
						LOG("%s\n", qlz_comp_err(ret));
					#ifndef _NZLIB
					}
					#endif
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			// in encryption is enabled
			if (options.enc_key != NULL) {
				file_name = (char *)calloc((strlen(options.data)+5), sizeof(char));
				memcpy(file_name, options.data, strlen(options.data));
				strcat(file_name, ".aes");
				if (encrypt_file_ecb(options.data, file_name, options.enc_key) != AES_SUCCESS) {	
					if (options.comp > 0) { safeRemove(options.data); }
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if (options.comp > 0) { safeRemove(options.data); }
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			// encode
			if (options.format == WAV) {
				size = in_wav.encode(options.input_file, options.data, options.output_file);
			} else if (options.format == FLAC) {
				size = in_flac.encode(options.input_file, options.data, options.output_file);
			}
			// cleanup
			if (options.enc_key != NULL || options.comp > 0) { safeRemove(options.data); }
			// if failed, cleanup
			if (size == 0x00) {
				LOG("Failed to encode data.\n");
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			// success
			LOG("Data was sucessfully encoded into the specified file.\n");
			LOG("The Decode key is: %lu\n", size);
			break;
		case DECODE:
			// if compression is enabled
			if (options.comp > 0) { temp_str = data_z; }
			// if encryption is enabled
			if (options.enc_key != NULL) { temp_str = data_aes; }
			// if neither is enabled
			if (options.comp == 0 && options.enc_key == NULL) { temp_str = options.output_file; }
			// decode
			if (options.format == WAV) {
				temp = in_wav.decode(options.input_file, temp_str, (DWORD)atol(options.data));
			} else if (options.format == FLAC) {
				temp = in_flac.decode(options.input_file, temp_str, (DWORD)atol(options.data));
			}
			if (!temp) {
				LOG("Failed to decode data.\n");
				safeRemove(temp_str);
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			// if encryption is enabled
			if (options.enc_key != NULL) {
				if (options.comp > 0) { temp_str = data_z;
				} else { temp_str = options.output_file; }
				if (decrypt_file_ecb(data_aes, temp_str, options.enc_key) != AES_SUCCESS) {
					safeRemove(data_aes);
					safeRemove(temp_str);
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				safeRemove("data.aes");
			}
			// if compression is enabled
			if (options.comp > 0) {
				#ifndef _NZLIB
				if(options.comp < 10) {
					ret = decompress_file(data_z, options.output_file);
				} else {
				#endif
					ret = qlz_decompress_file(data_z, options.output_file);
				#ifndef _NZLIB
				}
				#endif
				if (ret != 0) {
					#ifndef _NZLIB
					if(options.comp < 10) {
						LOG("%s\n", comp_err(ret));	
					} else {
					#endif
						LOG("%s\n", qlz_comp_err(ret));
					#ifndef _NZLIB
					}
					#endif
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
			}
			LOG("Data was sucessfully decoded from the specified file.\n");
			break;
		case TEST:
			// encode file
			// if compression is enabled
			if (options.comp > 0) {
				file_name = (char *)calloc((strlen(options.data)+3), sizeof(char));
				memcpy(file_name,options.data, strlen(options.data));
				strcat(file_name, ".z");
				#ifndef _NZLIB
				if(options.comp < 10) {
					ret = compress_file(options.data, file_name, options.comp);
				} else {
				#endif
					ret = qlz_compress_file(options.data, file_name);
				#ifndef _NZLIB
				}
				#endif
				if (ret != 0) {
					#ifndef _NZLIB
					if(options.comp < 10) {
						LOG("%s\n", comp_err(ret));	
					} else {
					#endif
						LOG("%s\n", qlz_comp_err(ret));
					#ifndef _NZLIB
					}
					#endif
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			// in encryption is enabled
			if (options.enc_key != NULL) {
				file_name = (char *)calloc((strlen(options.data)+5), sizeof(char));
				memcpy(file_name, options.data, strlen(options.data));
				strcat(file_name, ".aes");
				if (encrypt_file_ecb(options.data, file_name, options.enc_key) != AES_SUCCESS) {	
					if (options.comp > 0) { safeRemove(options.data); }
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if (options.comp > 0) { safeRemove(options.data); }
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			// encode
			if (options.format == WAV) {
				size = in_wav.encode(options.input_file, options.data, options.output_file);
			} else if (options.format == FLAC) {
				size = in_flac.encode(options.input_file, options.data, options.output_file);
			}
			// cleanup
			if (options.enc_key != NULL || options.comp > 0) { safeRemove(options.data); }
			// if failed, cleanup
			if (size == 0x00) {
				LOG("Failed to encode data.\n");
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			LOG("Data was sucessfully encoded into the specified file.\n");
			// decode file
			// if compression is enabled
			if (options.comp > 0) { temp_str = data_z; }
			// if encryption is enabled
			if (options.enc_key != NULL) { temp_str = data_aes; }
			// if neither is enabled
			if (options.comp == 0 && options.enc_key == NULL) { temp_str = options.test_out; }
			// decode
			if (options.format == WAV) {
				temp = in_wav.decode(options.output_file, temp_str, size);
			} else if (options.format == FLAC) {
				temp = in_flac.decode(options.output_file, temp_str, size);
			}
			if (!temp) {
				LOG("Failed to decode data.\n");
				safeRemove(temp_str);
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}

			// if encryption is enabled
			if (options.enc_key != NULL) {
				if (options.comp > 0) { temp_str = data_z;
				} else { temp_str = options.test_out; }
				if (decrypt_file_ecb(data_aes, temp_str, options.enc_key) != AES_SUCCESS) {
					safeRemove(data_aes);
					safeRemove(temp_str);
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				safeRemove(data_aes);
			}
			// if compression is enabled
			if (options.comp > 0) {
				#ifndef _NZLIB
				if(options.comp < 10) {
					ret = decompress_file(data_z, options.test_out);
				} else {
				#endif
					ret = qlz_decompress_file(data_z, options.test_out);
				#ifndef _NZLIB
				}
				#endif
				if (ret != 0) {
					#ifndef _NZLIB
					if(options.comp < 10) {
						LOG("%s\n", comp_err(ret));	
					} else {
					#endif
						LOG("%s\n", qlz_comp_err(ret));
					#ifndef _NZLIB
					}
					#endif
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
			}
			LOG("Data was sucessfully decoded from the specified file.\n");
			break;
		default:
			LOG_DEBUG("E: mode was not set.\n");
			opt_clean(&options);
			exit(EXIT_FAILURE);
			break;
	}	
	// cleanup
	opt_clean(&options);
	// its over!
	exit(EXIT_SUCCESS);
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

