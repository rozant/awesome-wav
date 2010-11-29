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
#ifndef __global_hpp__
#define __global_hpp__

//version numbers for software
#define POLARSSL_VER	"0.14.0-gpl"
#define AWESOME_VER		"1.5.0 alpha"
#define QLZ_VER			"1.4.1"

// buffer multipliers
#define BUFFER_MULT 1024
#define Z_MULT 2
#define C_BUFF 10240
#define LOGGER_RESIZE_AMOUNT 128

// gettext makes life a little easier sometimes
#ifndef gettext
#define gettext(Msgid) ((const char *) (Msgid))
#endif

// typedefs to make things look nice
#if defined _WIN32 || defined __MINGW32__ || defined __CYGWIN__
	typedef unsigned __int32 int32; 
	typedef unsigned __int16 int16;
	typedef unsigned __int8 int8;
#else
	#include <stdint.h>
	typedef uint32_t int32;
	typedef uint16_t int16;
	typedef uint8_t int8;
#endif

// make freeing easier
#ifndef FREE
#define FREE(p)	{ free(p); (p) = NULL; }
#endif

// the normal log
#ifndef LOG
#define LOG(...) { char c[1024]; sprintf(c, __VA_ARGS__); getLogger().record(c); getLogger().flush();  }
#endif

// the debug log
#ifdef _DEBUGOUTPUT
	#ifndef LOG_DEBUG
	#define LOG_DEBUG(...)  { char c[1024]; sprintf(c, __VA_ARGS__); getLogger().record(c);  }
	#endif
#endif
#ifndef _DEBUGOUTPUT
	#ifndef LOG_DEBUG
	#define LOG_DEBUG(...)  
	#endif
#endif

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

