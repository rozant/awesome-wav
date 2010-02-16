/****************************************************************/
/* This is the .hpp file that implements the wav class.	It does	*/
/* normal .h-ish stuff.  If you	do not know what this means,	*/
/* please find the nearest person that does and have them 		*/
/* escort you away from this computer immediately.				*/
/****************************************************************/
#ifndef __wav_h__
#define __wav_h__
#include "riff.hpp"

/****************************************************************/
/* class: wav													*/
/* purpose: contain an entire wav formatted file in ram.	 	*/
/****************************************************************/
class wav {
	private:
		_RIFF riff;
		_FMT fmt;
		_DATA data;
		/* file operations */
		bool loadRIFF(FILE*);
		bool loadFMT(FILE*);
		bool loadDATA(FILE*);
		bool load(FILE*);
		bool saveRIFF(FILE*) const;
		bool saveFMT(FILE*) const;
		bool saveDATA(FILE*) const;
		bool save(FILE*) const;
		bool close(FILE*) const;
		/* data integrity checks */
		bool validRIFF(void) const;
		bool validFMT(void) const;
		bool validDATA(void) const;
	public:
		/* constructors */
		wav(void);
		wav(const char[]);
		/* destructor */
		~wav(void) { return; }
		/* file operations */
		bool load(const char[]);
		bool save(const char[]) const;
		/* manipulation */
		#ifdef _DEBUG
		bool doStuff(void);
		#endif
		DWORD encode(const char[]);
		bool decode(const char[], const DWORD&) const;
		/* data integrity checks */
		bool valid(void) const;
};

#endif
/****************************************************************/
/****************************************************************/
/****************************************************************/
