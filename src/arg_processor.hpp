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
#ifndef __arg_processor_hpp__
#define __arg_processor_hpp__

// defines for all of the formats the program supports
enum opt_formats {
	UNKNOWN,
	WAV,
	FLAC
};

// defines for all of the modes the program supports
enum opt_modes {
	NONE,
	ENCODE,
	DECODE,
	TEST,
	VERSION
};

// defines for all of the formats the program supports
enum enc_methods {
	ECB = 1,
	CBC = 2
};

// structure for holding options
struct opts {
	char *input_file;						// input song name
	char *output_file;						// output song name
	char *data;								// data file name
	char *test_out;							// test data output
	unsigned char enc_method;				// encryption method
	unsigned char *enc_key;					// encryption key
	opt_formats format;						// song file format
	opt_modes mode;							// encode or decode
	unsigned char comp;						// should data be/is data compressed
    char threads;                           // number of threads to use
};

// function prototypes
int arg_processor(const int argc, const char **argv, opts *options);
void opt_init(opts *);
void opt_clean(opts *);

#endif
/****************************************************************/
/****************************************************************/
/****************************************************************/

