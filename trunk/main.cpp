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
/* main.cpp														*/
/****************************************************************/
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include "wav.hpp"
using namespace std;

#ifdef _DEBUG
char DEBUG_WAV[] = "song.wav";
char DEBUG_DATA[] = "data.txt";
char DEBUG_ENCODED_WAV[] = "E_song.wav";
char DEBUG_DECODED_DATA[] = "D_data.txt";
#endif

/****************************************************************/
/* function: main												*/
/* purpose: initial function for program.					 	*/
/* args: int, char**											*/
/* returns: int													*/
/****************************************************************/
int main(int argc, char* argv[]) {
	/* wav file definitaion */
	wav in_wav;
	DWORD size = 0x00;

	/* decide what to do */
	switch(argc) {
		/* no arguments, use something default like. FOR DEBUG PURPOSES ONLY*/
		#ifdef _DEBUG
		case 1:

			size = in_wav.encode(DEBUG_WAV, DEBUG_DATA, DEBUG_ENCODED_WAV);
			if (size == 0x00) {
				exit(EXIT_FAILURE);
			}
			
			if (!in_wav.decode(DEBUG_ENCODED_WAV, DEBUG_DECODED_DATA, size)) {
				exit(EXIT_FAILURE);
			}

			break;
		#endif
		/* 4 ARGUMENTS */
		case 5:
			if((strcmp(argv[1],"-e") == 0) || (strcmp(argv[1],"--encode") == 0)) {	/* ENCODE: Input Wav, Output Wav, Input Data */
				size = in_wav.encode(argv[2],argv[4],argv[3]);
				if (size == 0x00) {
					exit(EXIT_FAILURE);
				}
				//cout << "Data was sucessfully encoded into the specified file." << endl;
				//	<< "Enter this when trying to decode file: " << size << endl;
				cout << size << endl;
			} else if ((strcmp(argv[1],"-d") == 0) || (strcmp(argv[1],"--decode") == 0)) {	/* DECODE: Input Wav, Ouptut Data, Data Size */
				if (!in_wav.decode(argv[2], argv[3], (DWORD)atol(argv[4]))) {
					exit(EXIT_FAILURE);
				}
				//cout << "Data was sucessfully decoded from the specified file." << endl;
			}
			break;
		/* DEMONSTRATION: 5 Arguments, Input Wav, Output Wav, In Data, Out Data */
		case 6:
			if((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"--class") == 0)) {
				/* encode */
				size = in_wav.encode(argv[2],argv[4],argv[3]);
				if (size == 0x00) {
					exit(EXIT_FAILURE);
				}
				
				/* decode */
				if (!in_wav.decode(argv[3], argv[5], size)) {
					exit(EXIT_FAILURE);
				}
			}
			break;
		/* invalid use */
		default:
			cerr << "Useage: ls [-edc] [ARGUMETS]..." << endl
				<< "Encode data into a wav file, or decode data from a wav file." << endl << endl
				<< " -e, --encode\tencode arg3 into arg1 and store in arg2" << endl
				<< " -d, --decode\tdecode arg2 from arg1 using key arg3" << endl
				<< " -c, --class\tencode arg3 into arg1 and store in arg2,\n\t\tthen decode arg2 and store in arg4" << endl << endl;
			exit(EXIT_FAILURE);
			break;
	}
	/* its over! */
	exit(EXIT_SUCCESS);
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

