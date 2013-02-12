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

/****************************************************************/
/* function: open_file                                          */
/* purpose: open a file.                                        */
/* args: const char *, const char *                             */
/* returns: FILE *                                              */
/*        *     = opened correctly                              */
/*        NULL = opened incorrectly                             */
/****************************************************************/
FILE* open_file(const char *filename, const char *mode) {
    FILE* aFile = fopen(filename, mode);

    if (aFile == NULL) {
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
/* args: FILE *                                                 */
/* returns: bool                                                */
/*        1 = closed correctly                                  */
/*        0 = closed incorrectly, or already closed             */
/****************************************************************/
bool close_file(FILE *aFile) {
    if (aFile) {
        if (fclose(aFile)) {
            LOG_DEBUG("E: Failed to close file\n");
            return false;
        } else {
            aFile = NULL;
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

