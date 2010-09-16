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

// defines for all of the modes the program supports
enum opt_modes {
	NONE = 0,	
	ENCODE = 1,
	DECODE = 2,
	TEST = 3
};

// structure for holding options
struct opts {
	char *input_file;						// input wav name
	char *output_file;						// output wav name
	char *data;								// data file name
	char *test_out;							// test data output
	unsigned char *enc_key;					// encryption key
	char mode;								// encode or decode
	char comp;								// should data be/is data compressed
};

// function prototypes
int arg_processor(const int argc, const char **argv, opts *options);
void opt_init(opts *);
void opt_clean(opts *);

#endif
/****************************************************************/
/****************************************************************/
/****************************************************************/

