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
#include "sha2_util.hpp"
#include "sha2.h"
#include <stdlib.h>
#include <string.h>

/****************************************************************/
/* function: sha2_key											*/
/* purpose: take the input string and return its sha2 sum	 	*/
/* args: const char *											*/
/* returns: unsigned char *										*/
/****************************************************************/
unsigned char *sha2_key(const char *input_string) {
	unsigned char *hash = (unsigned char *)malloc(32*sizeof(unsigned char));
	if(hash == NULL) {
		return NULL;
	}
	sha2((const unsigned char *)input_string,strlen(input_string),hash,0);
	return hash;
}

/****************************************************************/
/* function: sha2_sum											*/
/* purpose: take the input file and return its sha2 sum		 	*/
/* args: const char *											*/
/* returns: unsigned char *										*/
/****************************************************************/
unsigned char *sha2_sum(const char *path) {
	unsigned char *hash = (unsigned char *)malloc(32*sizeof(unsigned char));
	if(hash == NULL) {
		return NULL;
	}
	sha2_file(path,hash,0);
	return hash;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

