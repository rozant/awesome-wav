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
#include "cd_da.hpp"
#include "./compression/compress_util.hpp"
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
	fprintf(stderr,"Useage: %s [-edcs(aes key)] arg1 arg2 arg3\n",prog_name);
	fprintf(stderr,"Encode data into a wav file, or decode data from a wav file.\n\n");
	fprintf(stderr,"  -e\tencode arg3 into arg1 and store in arg2\n");
	fprintf(stderr,"  -d\tdecode arg2 from arg1 using key arg3\n");
	fprintf(stderr,"  -c\tenable data compression.  If decoding, assume retrieved data is compressed\n");
	fprintf(stderr,"  -aes\tenable data encryption.  must be followed by the key.\n");
	fprintf(stderr,"\tdefaults to -c6. valid options are -c1 through -c9, from low to high compression\n");
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
	int ret = 0;
	char *temp_str = NULL;
	char *file_name = NULL;
	opts options;
	/* wav file definitaion */
	wav in_wav;
	/* set up the options struct */
	opt_init(&options);

	/* decide what to do */
	if (arg_processor(argc,(const char **)argv,&options) == EXIT_FAILURE) {
		usage(argv[0]);
		opt_clean(&options);
		exit(EXIT_FAILURE);
	}

	/* if we are encoding or decoding, do the right thing */
	switch(options.mode) {
		case ENCODE:
			/* if compression is enabled */
			if(options.comp > 0) {
				file_name = (char *)malloc((strlen(options.data)+3)*sizeof(char));
				memcpy(file_name,options.data,strlen(options.data));
				strcat(file_name,".z");
				ret = compress_file(options.data,file_name,options.comp);
				if( ret != COMU_SUCCESS) {
					printf("%s\n",comp_err(ret));
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			/* in encryption is enabled */
			if(options.enc_key != NULL) {
				file_name = (char *)malloc((strlen(options.data)+5)*sizeof(char));
				memcpy(file_name,options.data,strlen(options.data));
				strcat(file_name,".aes");
				if(encrypt_file(options.data,file_name,options.enc_key) != AES_SUCCESS) {	
					if(options.comp > 0) { remove(options.data); }
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if(options.comp > 0) { remove(options.data); }
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			/* encode */
			size = in_wav.encode(options.input_file,options.data,options.output_file);
			/* cleanup */
			if(options.enc_key != NULL) { remove(options.data); }
			/* if failed, cleanup */
			if(size == 0x00) {
				printf("Failed to encode data.\n");
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			/* success */
			printf("Data was sucessfully encoded into the specified file.\n");
			printf("The Decode key is: %u\n",(unsigned int)size);
			break;
		case DECODE:
			/* if compression is enabled */
			if(options.comp > 0) { free(temp_str); temp_str = (char *) calloc(7,sizeof(char)); memcpy(temp_str,"data.z",6); }
			/* if encryption is enabled */
			if(options.enc_key != NULL) { free(temp_str); temp_str = (char *)calloc(9,sizeof(char)); memcpy(temp_str,"data.aes",8); }
			/* if neither is enabled */
			if(options.comp == 0 && options.enc_key == NULL) { temp_str = options.output_file; }
			/* decode */
			temp = in_wav.decode(options.input_file,temp_str,(DWORD)atol(options.data));
			if (!temp) {
				printf("Failed to decode data.\n");
				remove(temp_str);
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			/* if encryption is enabled */
			if(options.enc_key != NULL) {
				if(options.comp > 0) { free(temp_str); temp_str = (char *) calloc(7,sizeof(char)); memcpy(temp_str,"data.z",6);
				} else { free(temp_str); temp_str = options.output_file; }
				if(decrypt_file("data.aes",temp_str,options.enc_key) != AES_SUCCESS) {
					remove("data.aes");
					remove(temp_str);
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				remove("data.aes");
			}
			/* if compression is enabled */
			if(options.comp > 0) {
				ret = decompress_file("data.z",options.output_file);
				if(ret != COMU_SUCCESS) {
					printf("%s\n",comp_err(ret));
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
			}
			printf("Data was sucessfully decoded from the specified file.\n");
			break;
		case TEST:
			/* encode file */
			/* if compression is enabled */
			if(options.comp > 0) {
				file_name = (char *)malloc((strlen(options.data)+3)*sizeof(char));
				memcpy(file_name,options.data,strlen(options.data));
				strcat(file_name,".z");
				ret = compress_file(options.data,file_name,options.comp);
				if( ret != COMU_SUCCESS) {
					printf("%s\n",comp_err(ret));
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			/* in encryption is enabled */
			if(options.enc_key != NULL) {
				file_name = (char *)malloc((strlen(options.data)+5)*sizeof(char));
				memcpy(file_name,options.data,strlen(options.data));
				strcat(file_name,".aes");
				if(encrypt_file(options.data,file_name,options.enc_key) != AES_SUCCESS) {	
					if(options.comp > 0) { remove(options.data); }
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if(options.comp > 0) { remove(options.data); }
				free(options.data);
				options.data = file_name;
				file_name = NULL;
			}
			/* encode */
			size = in_wav.encode(options.input_file,options.data,options.output_file);
			/* cleanup */
			if(options.enc_key != NULL) { remove(options.data); }
			/* if failed, cleanup */
			if(size == 0x00) {
				printf("Failed to encode data.\n");
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			printf("Data was sucessfully encoded into the specified file.\n");
			/* decode file */
			/* if compression is enabled */
			if(options.comp > 0) { free(temp_str); temp_str = (char *) calloc(7,sizeof(char)); memcpy(temp_str,"data.z",6); }
			/* if encryption is enabled */
			if(options.enc_key != NULL) { free(temp_str); temp_str = (char *)calloc(9,sizeof(char)); memcpy(temp_str,"data.aes",8); }
			/* if neither is enabled */
			if(options.comp == 0 && options.enc_key == NULL) { temp_str = options.test_out; }
			/* decode */
			temp = in_wav.decode(options.output_file,temp_str,size);
			if (!temp) {
				printf("Failed to decode data.\n");
				remove(temp_str);
				opt_clean(&options);
				exit(EXIT_FAILURE);
			}
			/* if encryption is enabled */
			if(options.enc_key != NULL) {
				if(options.comp > 0) { free(temp_str); temp_str = (char *) calloc(7,sizeof(char)); memcpy(temp_str,"data.z",6);
				} else { free(temp_str); temp_str = options.test_out; }
				if(decrypt_file("data.aes",temp_str,options.enc_key) != AES_SUCCESS) {
					remove("data.aes");
					remove(temp_str);
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				remove("data.aes");
			}
			/* if compression is enabled */
			if(options.comp > 0) {
				ret = decompress_file("data.z",options.test_out);
				if(ret != COMU_SUCCESS) {
					printf("%s\n",comp_err(ret));
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
			}
			printf("Data was sucessfully decoded from the specified file.\n");
			break;
		default:
			fprintf(stderr,"E: mode was not set.\n");
			opt_clean(&options);
			exit(EXIT_FAILURE);
			break;
	}

	/* cleanup */
	if(options.comp != 0 || options.enc_key != NULL) {
		free(temp_str);
	}
	opt_clean(&options);
	/* its over! */
	exit(EXIT_SUCCESS);
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

