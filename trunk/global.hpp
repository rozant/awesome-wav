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

#ifndef gettext
#define gettext(Msgid) ((const char *) (Msgid))
#endif
#define BUFFER_MULT 16
#define Z_MULT 2
#define LOGGER_RESIZE_AMOUNT 128

/* typedefs to make things look nice */
#ifdef _WIN32
	typedef unsigned __int32 DWORD; 
	typedef unsigned __int16 SHORT;
	typedef unsigned __int8 BYTE;
#else
	#include <stdint.h>
	typedef uint32_t DWORD; 
	typedef uint16_t SHORT;
	typedef uint8_t BYTE;
#endif

#ifndef FREE
#define FREE(p)	{ if (p != NULL) { free(p); (p) = NULL; } }
#endif

#ifndef LOG
#define LOG(...) { char c[1024]; sprintf(c, __VA_ARGS__); getLogger().record(c);  }
#endif

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

