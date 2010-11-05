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
#ifndef __flac_hpp__
#define __flac_hpp__
#include "global.hpp"
#include "flac_struct.hpp"
#include "flac_func.hpp"

/****************************************************************/
/* class: flac													*/
/* purpose: contain an entire flac file in ram.					*/
/****************************************************************/
class flac {
	private:
		FLAC_STREAM stream;
		// file operations
		template <class T>
		friend int FLACread(FILE*,T *);
		template <class T>
		friend int FLACreadSTREAM(FILE*,T *);
		// other things
		void clean(void);
		unsigned long int dostuff(void);
	public:
		// constructors
		flac(void);
		// destructor
		~flac(void);
		// manipulation
		unsigned long int encode(const char[], const char[], const char[]);
		bool decode(const char[], const char[], const DWORD&);
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

