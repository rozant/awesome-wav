#ifndef __typedefs_hpp__
#define __typedefs_hpp__

/* typedefs to make things look nice */
#ifdef _WIN32
	typedef unsigned __int32 DWORD; 
	typedef unsigned __int16 SHORT;
	typedef unsigned __int8 BYTE;
#else
	typedef uint32_t DWORD; 
	typedef uint16_t SHORT;
	typedef uint8_t BYTE;
#endif

#endif