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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _DEBUG
char DEBUG_WAV[] = "./The Blood Of Cu Chulainn-8_Bit_PCM.wav";
char DEBUG_DATA[] = "./test_input-4b-8kbps-21791276.txt";
char DEBUG_ENCODED_WAV[] = "E_song.wav";
char DEBUG_DECODED_DATA[] = "D_data.txt";
#endif

/****************************************************************/
/* function: usage												*/
/* purpose: display the usage of the this program			 	*/
/* args: const char*											*/
/* returns: void												*/
/****************************************************************/
void usage(const char prog_name[]) {
	fprintf(stderr,"Useage: %s [-edcs] arg1 arg2 arg3\n",prog_name);
	fprintf(stderr,"Encode data into a wav file, or decode data from a wav file.\n\n");
	fprintf(stderr,"  -e\tencode arg3 into arg1 and store in arg2\n");
	fprintf(stderr,"  -d\tdecode arg2 from arg1 using key arg3\n");
	fprintf(stderr,"  -c\tenable data compression.  If decoding, assume retrieved data is compressed\n");
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
	unsigned long int size = 0x00, temp = 0;
	opts options;
	/* wav file definitaion */
	wav in_wav;
	/* set up the options struct */
	opt_init(&options);

	/* decide what to do */
	if(argc == 1) {
		#ifdef _DEBUG
		size = in_wav.encode(DEBUG_WAV, DEBUG_DATA, DEBUG_ENCODED_WAV,options.comp);
		if (size == 0x00) {
			opt_clean(&options);
			exit(EXIT_FAILURE);
		}

		if (!in_wav.decode(DEBUG_ENCODED_WAV, DEBUG_DECODED_DATA, size,options.comp)) {
			opt_clean(&options);
			exit(EXIT_FAILURE);
		}
		#else
		usage(argv[0]);
		opt_clean(&options);
		exit(EXIT_FAILURE);
		#endif
	} else {
		if (arg_processor(argc,(const char **)argv,&options) == EXIT_FAILURE) {
			usage(argv[0]);
			opt_clean(&options);
			exit(EXIT_FAILURE);
		}

		/* if we are encoding or decoding, do the right thing */
		switch(options.mode) {
			case ENCODE:
				if(options.comp > 0) {
					if( compress_file(options.data,"data.z",options.comp) < 0) {
						opt_clean(&options);
						exit(EXIT_FAILURE);
					}
					free(options.data);
					options.data = (char *)calloc(7,sizeof(char));
					memcpy(options.data,"data.z",6);
				}
				size = in_wav.encode(options.input_file,options.data,options.output_file,options.comp);
				if(size == 0x00) {
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if(options.comp > 0) {
					remove("data.z");
				}
				printf("Data was sucessfully encoded into the specified file.\n");
				printf("The Decode key is: %u\n",(unsigned int)size);
				break;
			case DECODE:
				if (!in_wav.decode(options.input_file,options.output_file,(DWORD)atol(options.data),options.comp)) {
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				printf("Data was sucessfully decoded from the specified file.\n");
				break;
			case TEST:
				/* encode file */
				if(options.comp > 0) {
					if( compress_file(options.data,"data.z",options.comp) < 0) {
						opt_clean(&options);
						exit(EXIT_FAILURE);
					}
					free(options.data);
					options.data = (char *)calloc(7,sizeof(char));
					memcpy(options.data,"data.z",6);
				}
				if( (size = in_wav.encode(options.input_file,options.data,options.output_file,options.comp)) == 0x00) {
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if(options.comp > 0) {
					remove("data.z");
				}
				printf("Data was sucessfully encoded into the specified file.\n");
				/* decode file */
				if( options.comp > 0) {
					temp = in_wav.decode(options.output_file,"data.z",size,options.comp);
				} else {
					temp = in_wav.decode(options.output_file,options.test_out,size,options.comp);
				}
				if(temp != true) {
					opt_clean(&options);
					exit(EXIT_FAILURE);
				}
				if(options.comp > 0) {
					if(decompress_file("data.z",options.test_out) < 0) {
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
	}

	/* cleanup */
	opt_clean(&options);
	/* its over! */
	exit(EXIT_SUCCESS);
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

