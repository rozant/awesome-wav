/*****************************************************************
* This program is free software; you can redistribute it and/or  *
* modify it under the terms of the GNU General Public License    *
* version 2 as published by the Free Software Foundation.        *
*                                                                *
* This program is distributed in the hope that it will be        *
* useful, but WITHOUT ANY WARRANTY; without even the implied     *
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR        *
* PURPOSE.  See the GNU General Public License for more details. *
*                                                                *
* You should have received a copy of the GNU General Public      *
* License along with this program; if not, write to the Free     *
* Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,  *
* USA.                                                           *
*****************************************************************/
#include "util.hpp"
#include "logger.hpp"
#include <fcntl.h>

/****************************************************************/
/* function: open_file                                          */
/* purpose: open a file.                                        */
/* args: const char *, const char *                             */
/* returns: int                                                 */
/*        fd     = opened correctly                             */
/*        -1 = opened incorrectly                               */
/****************************************************************/
int open_file(const char *filename, const char *mode) {
    int aFile;

    if( mode[0] == 'r' ) {
        aFile = open(filename, O_RDWR | O_CREAT);
    } else if ( mode[0] == 'w' ) {
        aFile = open(filename, O_CREAT|O_WRONLY|O_TRUNC,S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH);
    } else {
        aFile = -1;
    }

    if (aFile == -1) {
        #ifdef _DEBUGOUTPUT
        LOG_DEBUG("E: Failed to open %s with mode %s\n", filename, mode);
        #else
        LOG("Failed to open %s with mode %s\n", filename, mode);
        #endif
    } else {
        LOG_DEBUG("S: Opened %s with mode %s\n", filename, mode);
    }

    return aFile;
}

/****************************************************************/
/* function: close_file                                         */
/* purpose: close an open file.                                 */
/* args: int                                                    */
/* returns: bool                                                */
/*        1 = closed correctly                                  */
/*        0 = closed incorrectly, or already closed             */
/****************************************************************/
bool close_file(int aFile) {
    if (aFile) {
        if (close(aFile)) {
            LOG_DEBUG("E: Failed to close file\n");
            return false;
        } else {
            LOG_DEBUG("S: Closed file\n");
            return true;
        }
    }
    LOG_DEBUG("E: File already closed\n");
    return false;
}

/****************************************************************/
/* function: safeRemove                                         */
/* purpose: overwrites a file with 0s before removing it        */
/* args: const char *                                           */
/* returns: int                                                 */
/*        0 = overwrote and removed file successfully           */
/****************************************************************/
int safeRemove(const char *filename) {
    long int fileSize = 0, bufferSize = 128 * BUFFER_MULT;
    int8 buffer[128 * BUFFER_MULT];
    int result = 1;
    FILE *aFile;

    aFile = fopen(filename, "rb+");
    
    if (aFile) {
        fseek(aFile, 0, SEEK_END);
        fileSize = ftell(aFile);
        fseek(aFile, 0, SEEK_SET);

        memset(buffer, 0, bufferSize);

        while (fileSize > 0) {
            if (fileSize - bufferSize > 0) {
                fileSize -= bufferSize;
            } else {
                bufferSize = fileSize;
                fileSize = 0;
            }

            fwrite(buffer, sizeof(int8), bufferSize, aFile);
        }

        fclose(aFile);

        result = remove(filename);
    }

    if (result == 0) {
        LOG_DEBUG("S: Successfully overwrote and removed %s\n", filename);
    } else {
        LOG_DEBUG("E: Failed to overwrite and remove %s\n", filename);
    }

    return result;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

