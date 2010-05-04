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
#ifndef __cd_da_hpp__
#define __cd_da_hpp__
#include "global.hpp"
#include <stdio.h>

/****************************************************************/
/* class: cd_da													*/
/* purpose: enable reading, writing, encoding, and decoding	of	*/
/*			PCM data										 	*/
/****************************************************************/
class cd_da {
	private:
		DWORD bitsPerSample;
		/* data operations */
		DWORD getMaxBytesEncoded(const DWORD);
		BYTE getMinBitsEncodedPS(const DWORD, const DWORD);
		DWORD encode(FILE*, FILE*, FILE*);
		bool encode(const BYTE, const DWORD, BYTE *, const size_t, BYTE *, const size_t);
		bool decode(FILE*, FILE*, const DWORD&);
		bool decode(const BYTE, const DWORD, BYTE *, const size_t, BYTE *, const size_t);
	public:
		/* constructors */
		cd_da(void);
		/* destructor */
		~cd_da(void);
		/* manipulation */
		DWORD encode(const char[], const char[], const char[], const char);
		bool decode(const char[], const char[], const DWORD&, const char);
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

