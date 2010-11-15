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
#include "logger.hpp"
#include "./crypt/sha2_util.hpp"
#ifdef _DEBUGOUTPUT
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/****************************************************************/
/* function: arg_processor										*/
/* purpose: process the arguments and store options in a struct	*/
/* args: const int, const char **, opts *						*/
/* returns: int													*/
/****************************************************************/
int arg_processor(const int argc, const char **argv, opts *options) {
	int arg_count = 0;

	// for all of the argumets
	for (int foo = 1; foo < argc; foo++) {
		if (argv[foo][0] == '-') {								// if it is an option do stuff
			if (strcmp(argv[foo],"-e") == 0) {					// if encoding, set the mode to encode
				LOG_DEBUG("S: Setting mode to 'ENCODE'\n");
				options->mode = ENCODE;
			} else if (strcmp(argv[foo],"-d") == 0) {			// if decoding, set the mode to decode
				LOG_DEBUG("S: Setting mode to 'DECODE'\n");
				options->mode = DECODE;
			} else if (strcmp(argv[foo],"-t") == 0) {			// if testing, set the mode to test
				LOG_DEBUG("S: Setting mode to 'TEST'\n");
				options->mode = TEST;
			} else if (strcmp(argv[foo],"--version") == 0) {	// if wanting to display version number
				LOG_DEBUG("S: Setting mode to 'VERSION'\n");
				options->mode = VERSION;
				break;
			} else if (strncmp(argv[foo],"-c",2) == 0) {			// compress with qlz
				if ( strlen(argv[foo]) == 3 ) {
					if (isdigit(argv[foo][2])) {
						LOG_DEBUG("S: Setting ZLIB compression level %d\n",atoi(&argv[foo][2]));
						options->comp = (char)atoi(&argv[foo][2]);
					} else {
						LOG_DEBUG("S: Setting ZLIB compression level 6\n");
						options->comp = 6;
					}
				} else {
					LOG_DEBUG("S: Setting ZLIB compression level 6\n");
					options->comp = 6;
				}
			} else if (strcmp(argv[foo], "-aes") == 0) {		// encrypt with AES
				if (++foo >= argc) { return EXIT_FAILURE; }
				LOG_DEBUG("S: Setting AES encryption\n");
				options->enc_key = sha2_key(argv[foo]);
			} else {											// invalid option
				LOG_DEBUG("E: Invalid option '%s'.\n", argv[foo]);
				return EXIT_FAILURE;
			}
		} else {
			// otherwise assume it is a file name
			switch(arg_count) {
				case 0:						// arg 1
					options->input_file = (char *)calloc(strlen(argv[foo])+1,sizeof(char));
					memcpy(options->input_file, argv[foo], strlen(argv[foo]));
					break;
				case 1:						// arg 2
					options->output_file = (char *)calloc(strlen(argv[foo])+1,sizeof(char));
					memcpy(options->output_file, argv[foo], strlen(argv[foo]));
					break;
				case 2:						// arg 3
					options->data = (char *)calloc(strlen(argv[foo])+1,sizeof(char));
					memcpy(options->data, argv[foo], strlen(argv[foo]));
					break;
				case 3:						// arg 4
					options->test_out = (char *)calloc(strlen(argv[foo])+1,sizeof(char));
					memcpy(options->test_out, argv[foo], strlen(argv[foo]));
					break;
				default:					// others
					break;
			}
			++arg_count;
		}
	}
	// check for arguemnt errors that have not been caught yet
	if (options->mode != VERSION) {
		if ((options->mode != TEST && arg_count != 3) || (options->mode == TEST && arg_count != 4)) {
			LOG_DEBUG("E: Incorrect number of arguments.\n");
			return EXIT_FAILURE;
		}
	}
	// its over
	return EXIT_SUCCESS;
}

/****************************************************************/
/* function: opt_clean											*/
/* purpose: free the stuff that opts use						*/
/* args: opts *													*/
/* returns: void												*/
/****************************************************************/
void opt_clean(opts *foo) {
	FREE(foo->input_file);
	FREE(foo->output_file);
	FREE(foo->data);
	FREE(foo->test_out);
	if (foo->enc_key != NULL) {
		memset(foo->enc_key,0,sizeof(foo->enc_key));
		FREE(foo->enc_key);
	}
	return;
}

/****************************************************************/
/* function: opt_init											*/
/* purpose: make sure all values are clear						*/
/* args: opts *													*/
/* returns: void												*/
/****************************************************************/
void opt_init(opts *foo) { 
	foo->input_file = foo->output_file = NULL;
	foo->data = foo->test_out = NULL;
	foo->enc_key = NULL;
	foo->mode = ENCODE; 
	foo->comp = 0; 
	return;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

