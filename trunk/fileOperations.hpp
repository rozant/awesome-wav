#include <iostream>
using namespace std;

/****************************************************************/
/* function: open												*/
/* purpose: open a file.										*/
/* args: const char *, const char * 							*/
/* returns: FILE *												*/
/*		*	 = opened correctly									*/
/*		NULL = opened incorrectly								*/
/****************************************************************/
FILE* open(const char *filename, const char *mode) {
	FILE* aFile = NULL;
	aFile = fopen(filename, mode);

	#ifdef _DEBUGOUTPUT
	if (aFile == NULL)
		cout << "E: Failed to open " << filename << " with mode " << mode << endl;
	else
		cout << "S: Opened " << filename << " with mode " << mode << endl;
	#endif

	return aFile;
}

/****************************************************************/
/* function: close												*/
/* purpose: close an open file.									*/
/* args: FILE *													*/
/* returns: bool												*/
/*		1 = closed correctly									*/
/*		0 = closed incorrectly, or already closed				*/
/****************************************************************/
bool close(FILE *aFile) {
	if( aFile) {
		if ( fclose( aFile ) ) {
			#ifdef _DEBUGOUTPUT
			cout << "E: Failed to close file" << endl;
			#endif
			return false;
		} else {
			#ifdef _DEBUGOUTPUT
			cout << "S: Closed file" << endl;
			#endif
			return true;
		}
	}
	#ifdef _DEBUGOUTPUT
	cout << "E: File already closed" << endl;
	#endif
	return false;
}