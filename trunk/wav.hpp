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
/****************************************************************/
/* This is the .hpp file that implements the wav class.	It does	*/
/* normal .h-ish stuff.  If you	do not know what this means,	*/
/* please find the nearest person that does and have them 		*/
/* escort you away from this computer immediately.				*/
/****************************************************************/
#ifndef __wav_hpp__
#define __wav_hpp__
#include "global.hpp"
#include "riff.hpp"

/****************************************************************/
/* class: wav													*/
/* purpose: contain an entire wav formatted file in ram.	 	*/
/****************************************************************/
class wav {
	private:
		_RIFF riff;
		_FMT fmt;
		_DATA data;;
		/* file operations */
		bool readRIFF(FILE*);
		bool readFMT(FILE*);
		bool readDATA(FILE*);
		bool read(FILE*);
		bool writeFMT(FILE*) const;
		bool writeRIFF(FILE*) const;
		bool writeDATA(FILE*) const;
		/* data integrity checks */
		bool validRIFF(void) const;
		bool validFMT(void) const;
		bool validDATA(void) const;
		/* data operations */
		DWORD getMaxBytesEncoded(const SHORT, const DWORD);
		BYTE getMinBitsEncodedPS(const SHORT, const DWORD, const DWORD);
		DWORD encode(FILE*, FILE*, FILE*);
		bool encode(const BYTE, const DWORD, BYTE *, const size_t, BYTE *, const size_t);
		bool decode(FILE*, FILE*, const DWORD&);
		bool decode(const BYTE, const DWORD, BYTE *, const size_t, BYTE *, const size_t);
	public:
		/* constructors */
		wav(void);
		/* destructor */
		~wav(void) { return; }
		/* manipulation */
		DWORD encode(const char[], const char[], const char[], const char);
		bool decode(const char[], const char[], const DWORD&, const char);
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/
