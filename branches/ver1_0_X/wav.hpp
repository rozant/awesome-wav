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
		_FACT *fact;
		_PEAK *peak;
		_DATA data;
		// file operations
		template <class T>
		friend int RIFFread(FILE*,T *);
		template <class T>
		friend int RIFFreadRIFF(FILE *, T *);
		template <class T>
		friend int RIFFreadFMT(FILE *, T *);
		template <class T>
		friend int RIFFreadFACT(FILE *, T *);
		template <class T>
		friend int RIFFreadPEAK(FILE *, T *);
		template <class T>
		friend int RIFFreadDATA(FILE *, T *);
		template <class T>
		friend int RIFFwrite(FILE *, const T *);
		template <class T>
		friend int RIFFwriteRIFF(FILE *, const T *);
		template <class T>
		friend int RIFFwriteFMT(FILE *, const T *);
		template <class T>
		friend int RIFFwriteFACT(FILE *, const T *);
		template <class T>
		friend int RIFFwritePEAK(FILE *, const T *);
		template <class T>
		friend int RIFFwriteDATA(FILE *, const T *);
		// data integrity checks
		bool validWAV(void) const;
		bool validRIFF(void) const;
		bool validFMT(void) const;
		bool validFACT(void) const;
		bool validDATA(void) const;
		// data operations
		int32 getMaxBytesEncoded(const int16, const int32);
		int8 getMinBitsEncodedPS(const int16, const int32, const int32);
		unsigned long int encode(FILE*, FILE*, FILE*);
		bool encode(const int8, const int32, int8 *, const size_t, int8 *, const size_t);
		bool decode(FILE*, FILE*, const int32&);
		bool decode(const int8, const int32, int8 *, const size_t, int8 *, const size_t);
		// other things
		void clean(void);
	public:
		// constructors
		wav(void);
		// destructor
		~wav(void);
		// manipulation
		unsigned long int encode(const char[], const char[], const char[]);
		bool decode(const char[], const char[], const int32&);
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

