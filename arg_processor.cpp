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
#ifdef _DEBUGOUTPUT
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "arg_processor.hpp"

/****************************************************************/
/* function: arg_processor										*/
/* purpose: process the arguments and store options in a struct	*/
/* args: const int, const char **, opts *						*/
/* returns: int													*/
/****************************************************************/
int arg_processor(const int argc, const char **argv, opts *options) {
	int foo = 0;
	unsigned char arg_count = 0;

	/* for all of the argumets */
	for (foo = 1; foo < argc; ++foo) {
		/* if it is an option do stuff */
		if (argv[foo][0] == '-') {
			if(strlen(argv[foo]) == 2) {
				switch(argv[foo][1]) {
					case 'e':				/* encode (default) */
						options->mode = ENCODE;
						break;
					case 'd':				/* decode */
						options->mode = DECODE;
						break;
					case 't':				/* testing */
						options->mode = TEST;
						break;
					case 'c':				/* data compression */
						options->comp = 6;
						break;
					default:
						#ifdef _DEBUGOUTPUT
						fprintf(stderr,"E: Invalid option '%s'.\n",argv[foo]);
						#endif
						return EXIT_FAILURE;
						break;
				}
			} else {
				#ifdef _DEBUGOUTPUT
				fprintf(stderr,"E: Invalid option '%s'.\n",argv[foo]);
				#endif
				return EXIT_FAILURE;
			}
		} else {
			/* otherwise assume it is a file name */
			if (arg_count == 0) {
				options->input_file = (char *)calloc(strlen(argv[foo]),sizeof(char));
				strncpy(options->input_file,argv[foo],strlen(argv[foo]));	
			} else if (arg_count == 1) {
				options->output_file = (char *)calloc(strlen(argv[foo]),sizeof(char));
				strncpy(options->output_file,argv[foo],strlen(argv[foo]));	
			} else if (arg_count == 2) {
				options->data = (char *)calloc(strlen(argv[foo]),sizeof(char));
				strncpy(options->data,argv[foo],strlen(argv[foo]));
			} else if (arg_count == 3) {
				options->test_out = (char *)calloc(strlen(argv[foo]),sizeof(char));
				strncpy(options->test_out,argv[foo],strlen(argv[foo]));
			}
			++arg_count;
		}
	}
	if( (options->mode != TEST && arg_count != 3) || (options->mode == TEST && arg_count != 4) ) {
		#ifdef _DEBUGOUTPUT
		fprintf(stderr,"E: Incorrect number of arguments.\n");
		#endif
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

