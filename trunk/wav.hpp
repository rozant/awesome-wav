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
		/* data integrity checks */
		bool validRIFF(void) const;
		bool validFMT(void) const;
		bool validDATA(void) const;
		/* data operations */
		bool encode(/*BYTE *, BYTE **/ FILE*, BYTE, DWORD);
	public:
		/* constructors */
		wav(void);
		/* destructor */
		~wav(void) { return; }
		/* manipulation */
		DWORD encode(const char[], const char[], const char[]);
		bool decode(const char[], const DWORD&) const;
		bool decode(const char[], const char[], const DWORD&);
};

#endif
/****************************************************************/
/****************************************************************/
/****************************************************************/
