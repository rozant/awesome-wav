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
#include "riff.hpp"
#include "wav.hpp"
#include "global.hpp"
#include "util.hpp"
#include "logger.hpp"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

/****************************************************************/
/* function: wav::wav                                           */
/* purpose: constructor for the wav class                       */
/* args: void                                                   */
/****************************************************************/
wav::wav(void) {
    memset(&riff, 0, sizeof(_RIFF));
    memset(&fmt, 0, sizeof(_FMT));
    memset(&data, 0, sizeof(_DATA));
    fact = NULL;
    peak = NULL;
    argt = NULL;
    threads = NULL;
    num_threads = 1;
}

/****************************************************************/
/* function: wav::~wav                                          */
/* purpose: destructor for the wav class                        */
/* args: void                                                   */
/****************************************************************/
wav::~wav(void) {
    clean();
    return;
}

/****************************************************************/
/* function: wav::clean                                         */
/* purpose: effective destructor for the wav class              */
/* args: void                                                   */
/* returns: void                                                */
/****************************************************************/
void wav::clean(void) {
    FREE(fact);
    if (peak != NULL) {
        FREE(peak->peak);
        FREE(peak->bit_align);
        FREE(peak);
    }
    peak = NULL;
    if (argt != NULL) {
        free(argt);
    }
    argt = NULL;
    if (threads != NULL) {
        free(threads);
    }
    threads = NULL;
    return;
}

/****************************************************************/
/* function: wav::validWAV                                      */
/* purpose: checks if the WAV file is valid                     */
/* args: void                                                   */
/* returns: bool                                                */
/*        1 = valid header                                      */
/*        0 = invalid header                                    */
/****************************************************************/
bool wav::validWAV(void) const {
    return (validRIFF() && validFMT() && validFACT() && validDATA());
}

/****************************************************************/
/* function: wav::validRIFF                                     */
/* purpose: checks if the RIFF header is valid                  */
/* args: void                                                   */
/* returns: bool                                                */
/*        1 = valid header                                      */
/*        0 = invalid header                                    */
/****************************************************************/
bool wav::validRIFF(void) const {
    if (memcmp(riff.ChunkID, "RIFF", 4) != 0) {
        LOG_DEBUG("E: Invalid RIFF header: Format != 'RIFF'\n");
        LOG_DEBUG("\tFormat == %s\n", (char *)riff.ChunkID);
        return false;
    }
    if (memcmp(riff.Format, "WAVE", 4) != 0) {
        LOG_DEBUG("E: Invalid RIFF header: Format != 'WAVE'\n");
        LOG_DEBUG("\tFormat == %s\n", (char *)riff.Format);
        return false;
    }
    LOG_DEBUG("S: Valid RIFF header\n");
    return true;
}

/****************************************************************/
/* function: wav::validFMT                                      */
/* purpose: checks if the FMT header is valid                   */
/* args: void                                                   */
/* returns: bool                                                */
/*        1 = valid header                                      */
/*        0 = invalid header                                    */
/****************************************************************/
bool wav::validFMT(void) const {
    if (memcmp(fmt.SubchunkID, "fmt ", 4) != 0) {
        LOG_DEBUG("E: Invalid FMT header: Format != 'fmt '\n");
        LOG_DEBUG("\tFormat == %s\n", (char *)fmt.SubchunkID);
        return false;
    }
    // check encoding method
    if (fmt.AudioFormat != WAVE_FORMAT_PCM && fmt.AudioFormat != WAVE_FORMAT_IEEE_FLOAT && fmt.AudioFormat != WAV_FORMAT_EXTENSIBLE) {
        LOG_DEBUG("E: Invalid FMT header: AudioFormat != '%d' (PCM) or '%d' (IEEE FLOAT)\n", WAVE_FORMAT_PCM, WAVE_FORMAT_IEEE_FLOAT);
        LOG_DEBUG("\tAudioFormat == %u\n", (unsigned int)fmt.AudioFormat);
        return false;
    }
    // check bits per sample
    if (fmt.AudioFormat == WAVE_FORMAT_PCM) {
        if (fmt.BitsPerSample != 8 && fmt.BitsPerSample != 16 && fmt.BitsPerSample != 24 && fmt.BitsPerSample != 32) {
            LOG_DEBUG("E: Invalid FMT header: Bits per sample = %u\n", (unsigned int)fmt.BitsPerSample);
            LOG_DEBUG("\tBits per sample == %u\n", (unsigned int)fmt.BitsPerSample);
            LOG_DEBUG("\tExpected Bits per sample to be '8', '16', '24', or 32\n");
            return false;
        }
    } else if (fmt.AudioFormat == WAVE_FORMAT_IEEE_FLOAT) {
        if (fmt.BitsPerSample != 32 && fmt.BitsPerSample != 64) {
            LOG_DEBUG("E: Invalid FMT header: Bits per sample = %u\n", (unsigned int)fmt.BitsPerSample);
            LOG_DEBUG("\tBits per sample == %u\n", (unsigned int)fmt.BitsPerSample);
            LOG_DEBUG("\tExpected Bits per sample to be '8', '16', '24', or 32\n");
            return false;
        }
    }
    // check channel count
    if (fmt.NumChannels > 2) {
        LOG_DEBUG("E: Invalid FMT header: Num channels > 2\n");
        LOG_DEBUG("\tNumChannels == %u\n", (unsigned int)fmt.NumChannels);
        return false;
    }

    #ifdef _DEBUGOUTPUT
    LOG_DEBUG("S: Valid FMT header\n");
    LOG_DEBUG("\tS: Bits per sample: %u\n", (unsigned int)fmt.BitsPerSample);
    LOG_DEBUG("\tS: Block align: %u\n", (unsigned int)fmt.BlockAlign);
    LOG_DEBUG("\tS: Byte rate: %u\n", (unsigned int)fmt.ByteRate);
    LOG_DEBUG("\tS: Num channels: %u\n", (unsigned int)fmt.NumChannels);
    LOG_DEBUG("\tS: Sample rate: %u\n", (unsigned int)fmt.SampleRate);
    if (fmt.SubchunkSize-16 != 0) {
        LOG_DEBUG("\tS: Extra format bytes: %u\n", (unsigned int)fmt.ExtraFormatBytes);
        if (fmt.ExtraFormatBytes == 22) {
            LOG_DEBUG("\tS: Valid bits per sample: %u\n", (unsigned int)fmt.ValidBitsPerSample);
            LOG_DEBUG("\tS: Channel mask: %u\n", (unsigned int)fmt.ChannelMask);
            LOG_DEBUG("\tS: Sub format: %s\n", (char *)fmt.SubFormat);
        }
    }
    #endif
    return true;
}

/****************************************************************/
/* function: wav::validFACT                                     */
/* purpose: checks if the FACT header is valid                  */
/* args: void                                                   */
/* returns: bool                                                */
/*        1 = valid header                                      */
/*        0 = invalid header                                    */
/****************************************************************/
bool wav::validFACT(void) const {
    if (fact == NULL) {
        return true;
    }
    if (memcmp(fact->SubchunkID, "fact", 4) != 0) {
        LOG_DEBUG("E: Invalid FACT header: Format != 'fact'\n");
        LOG_DEBUG("\tFormat == %s\n", (char *)fact->SubchunkID);
        return false;
    }
    LOG_DEBUG("S: Valid FACT header\n");
    return true;
}

/****************************************************************/
/* function: wav::validDATA                                     */
/* purpose: checks if the DATA header is valid                  */
/* args: void                                                   */
/* returns: bool                                                */
/*        1 = valid header                                      */
/*        0 = invalid header                                    */
/****************************************************************/
bool wav::validDATA(void) const {
    if (memcmp(data.SubchunkID, "data", 4) != 0) {
        LOG_DEBUG("E: Invalid DATA header: Format != 'data'\n");
        LOG_DEBUG("\tFormat == %s\n", (char *)data.SubchunkID);
        return false;
    }
    if (data.SubchunkSize == 0) {
        LOG_DEBUG("E: Invalid DATA header: No DATA\n");
        return false;
    }
    LOG_DEBUG("S: Valid DATA header\n");
    return true;
}

/****************************************************************/
/* function: encode                                             */
/* purpose: open the files ment for encoding                    */
/* args: const char[], const char[], const char[]               */
/* returns: unsigned long int                                   */
/****************************************************************/
unsigned long int wav::encode(const char inputWAV[], const char inputDATA[], const char outputWAV[]) {
    unsigned long int ret_val = 0;
    int fInputWAV, fInputDATA, fOutputWAV;

    LOG("Opening input wave file...\n");
    // Open up our input wav file
    fInputWAV = open_file(inputWAV, "rb");
    if (fInputWAV == -1) { return false; }

    LOG("Validating input wave file...\n");
    // read and validate wave header (RIFF Chunk), and format chunk
    if (RIFFread(fInputWAV, this) != RIFF_SUCCESS || !validWAV()) { close_file(fInputWAV); return false; }

    LOG("Opening input data file...\n");
    // Open up our input data file
    fInputDATA = open_file(inputDATA, "rb");
    if (fInputDATA == -1) { close_file(fInputWAV); return false; }

    LOG("Opening output wav file...\n");
    // open up output wav file
    fOutputWAV = open_file(outputWAV, "wb");
    if (fOutputWAV == -1) { close_file(fInputWAV); close_file(fInputDATA); return false; }

    LOG("Encoding data...\n");
    ret_val = encode(fInputWAV, fInputDATA, fOutputWAV);
    close_file(fInputWAV); close_file(fInputDATA); close_file(fOutputWAV);

    // clean up
    clean();
    return ret_val;
}

/****************************************************************/
/* function: encode                                             */
/* purpose: do all necessary calculations and handle buffering  */
/* prerequisites: files are open; header data has been read     */
/* args: int int int                                            */
/* returns: unsigned long int                                   */
/****************************************************************/
unsigned long int wav::encode(int fInputWAV, int fInputDATA, int fOutputWAV) {
    unsigned long int dataSize = 0, maxSize = 0;
    unsigned int foo = 0;
    int32 bytesPerSample = (fmt.BitsPerSample >> 3), initial_offset = 0, offset_block_size = 0;
    int16 num_wav_buffers = data.SubchunkSize / (BUFFER_MULT * (1024 * bytesPerSample));
    int8 bitsUsed = 0;
    bool enc_ret = true;

    // Get size of data file we want to encode
    dataSize = lseek(fInputDATA, 0, SEEK_END);
    lseek(fInputDATA, 0, SEEK_SET);

    LOG_DEBUG("S: Determined input data file size (%.*f MB)\n", 3, byteToMB(dataSize));
    if (dataSize == 0) {
        LOG("Data file is empty\n");
        return false;
    }

    // get the maximum number of bytes the wav file could hold
    maxSize = getMaxBytesEncoded(fmt.BitsPerSample, data.SubchunkSize);
    if (dataSize > maxSize) {
        LOG_DEBUG("E: Data file is too large (Want to store %.*f MB - Can fit %.*f MB)\n", 3, byteToMB(dataSize), 3, byteToMB(maxSize));
        return false;
    }
    LOG_DEBUG("S: Data fits (Storing %.*f MB - Can Fit %.*f MB)\n", 3, byteToMB(dataSize), 3, byteToMB(maxSize));

    // get the minimum number of bits the wav file could encode per sample
    bitsUsed = getMinBitsEncodedPS(fmt.BitsPerSample, dataSize, maxSize);
    if (bitsUsed == 0 || bitsUsed > (fmt.BitsPerSample >> 1)) {
        LOG_DEBUG("E: This should never happen... bitsused %d\n", (int)bitsUsed);
        return false;
    }
    LOG_DEBUG("S: Data file could fit at %d bits per sample\n", (int)bitsUsed);

    // Write our headers and how many bits used
    if (RIFFwrite(fOutputWAV,this) != RIFF_SUCCESS) {
        return false;
    }

    /* calculate start and end positions for wav data */
    if( num_wav_buffers * (BUFFER_MULT * (1024 * bytesPerSample)) < data.SubchunkSize) {
        num_wav_buffers += 1;
    }

    num_threads = 1;
    /* thread var */
    threads = (pthread_t *)calloc(sizeof(pthread_t),num_threads);
    argt = (thread_args *)calloc(sizeof(thread_args),num_threads);

    // set the initial offset
    initial_offset = lseek(fOutputWAV,0,SEEK_CUR);
    offset_block_size = data.SubchunkSize / num_threads;

    // set up thread arguments
    for(foo = 0; foo < num_threads; ++foo) {
        argt[foo].fInputWAV = fInputWAV;
        argt[foo].fInputDATA = fInputDATA;
        argt[foo].fOutputWAV = fOutputWAV;
        argt[foo].maxSize = maxSize;
        argt[foo].bytesPerSample = bytesPerSample;
        argt[foo].bitsUsed = bitsUsed;
        argt[foo].initial_offset = initial_offset;
        argt[foo].offset_block_size = offset_block_size;
        argt[foo].enc_ret = enc_ret;
    }

    getLogger().flush();

    #ifdef _DEBUGOUTPUT
    clock_t start = clock();
    #endif

    //--------------------- Parallel portion can start right here ---------------------
    for(foo = 0; foo < num_threads; ++foo) {
        if( pthread_create( &threads[foo], NULL, wav::parallel_encode_helper, this) < 0) {
            return false;
        }
    }
    for(foo = 0; foo < num_threads; ++foo) {
        pthread_join(threads[foo], NULL);
    }
    for(foo = 0; foo < num_threads; ++foo) {
        if (argt[foo].enc_ret == false) {
            return false;
        }
    }
    //--------------------- Parallel portion can end right here -----------------------

    LOG_DEBUG("S: Took %.3f seconds to encode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
    LOG_DEBUG("S: Number of bytes stored: %u\n", (unsigned int)dataSize);

    /* cleanup */
    free(argt);
    free(threads);
    threads = NULL;
    argt = NULL;
    return dataSize;
}

/****************************************************************/
/* function: parallel_encode                                    */
/* purpose: encode data into the audio file using a buffer in   */
/*  parallel                                                    */
/* args: int , int , int , const unsigned long int,             */
/*  const int32, const int8, const int32, const int32. bool     */
/* returns: void                                                */
/****************************************************************/
void *wav::parallel_encode(void) {
    unsigned long int currentSize = 0;
    unsigned int foo;
    size_t wavBufferSize = 0, maxWavBufferSize = 0, dataBufferSize = 0, maxDataBufferSize = 0;
    int32 wavDataLeft = 0, wav_in_offset = 0, wav_out_offset = 0, data_offset = 0;
    int8 *wavBuffer = NULL, *dataBuffer = NULL;
    thread_args *arg_s = NULL;
    bool endOfDataFile = false;

    for(foo = 0; foo < num_threads; ++foo) {
        if( pthread_equal(threads[foo],pthread_self()) ) {
            LOG_DEBUG("I: Thead argument data has been selected\n");
            arg_s = &argt[foo];
            getLogger().flush();
            break;
        }
    }

    wav_out_offset = (int32)arg_s->initial_offset;

    // Calculate the size of our buffers
    maxWavBufferSize = BUFFER_MULT * (1024 * argt->bytesPerSample);
    maxDataBufferSize = BUFFER_MULT * (128 * argt->bitsUsed);

    // Get memory for our buffers
    if ((wavBuffer = (int8*)calloc(maxWavBufferSize, sizeof(int8))) == NULL) {
        LOG_DEBUG("E: Failed to get memory for WAV buffer\n");
        arg_s->enc_ret = false;
        return &arg_s->enc_ret;
    }
    LOG_DEBUG("S: Got %u bytes for WAV buffer\n", (unsigned int)maxWavBufferSize);

    if ((dataBuffer = (int8*)calloc(maxDataBufferSize, sizeof(int8))) == NULL) {
        LOG_DEBUG("E: Failed to get memory for DATA buffer\n");
        free(wavBuffer);
        arg_s->enc_ret = false;
        return &arg_s->enc_ret;
    }
    LOG_DEBUG("S: Got %u bytes for DATA buffer\n", (unsigned int)maxDataBufferSize);

    wavDataLeft = arg_s->offset_block_size;

    // while there is data in the buffer encode and write to the file
    #ifdef _DEBUGOUTPUT
    clock_t event_start;
    clock_t event_end;
    unsigned long int reading_audio_data = 0;
    unsigned long int reading_file_data = 0;
    unsigned long int gen_rand_data = 0;
    unsigned long int encoding_data = 0;
    unsigned long int writing_data = 0;
    #endif
    for(;;) {
        // get the next chunk of song
        if (wavDataLeft <= 0) {
            break;
        }

        #ifdef _DEBUGOUTPUT
        event_start = clock();
        #endif
        wavBufferSize = pread(arg_s->fInputWAV, wavBuffer, (wavDataLeft < maxWavBufferSize) ? wavDataLeft : maxWavBufferSize, wav_in_offset);
        #ifdef _DEBUGOUTPUT
        event_end = clock();
        reading_audio_data += event_end - event_start;
        #endif

        if (wavBufferSize == 0) {
            LOG_DEBUG("E: Data subchunk size is bigger than the file\n");
            arg_s->enc_ret = false;
            return &arg_s->enc_ret;
        }
        wavDataLeft -= wavBufferSize;
        wav_in_offset += wavBufferSize;

        // get the next chunk of data
        if (!endOfDataFile) {
            #ifdef _DEBUGOUTPUT
            event_start = clock();
            #endif
            dataBufferSize = pread(arg_s->fInputDATA, dataBuffer, maxDataBufferSize, data_offset);
            #ifdef _DEBUGOUTPUT
            event_end = clock();
            reading_file_data += event_end - event_start;
            #endif
            if ( dataBufferSize < maxDataBufferSize ) {
                endOfDataFile = true;
                // seed the random number generator
                srand((unsigned int)time(NULL));
            }
            data_offset += dataBufferSize;
        }

        // no more data so generate random stuff
        if (endOfDataFile) {
            int8* currPos_DataBuffer = dataBuffer;
            size_t count = 0x00, offset = 0, increment = sizeof(int);

            #ifdef _DEBUGOUTPUT
            event_start = clock();
            #endif
            // copy music data to the data buffer
            if ( dataBufferSize < maxDataBufferSize ) {
                // the buffer is partiallly full from a read... only overwrite some of the buffer
                offset = dataBufferSize;
                memcpy(dataBuffer + dataBufferSize, wavBuffer, maxDataBufferSize - dataBufferSize);
            } else {
                memcpy(dataBuffer, wavBuffer, dataBufferSize);
            }

            // we continue encoding until the data buffer is empty so make sure its to correct size
            dataBufferSize = (currentSize + maxDataBufferSize > arg_s->maxSize) ? arg_s->maxSize - currentSize : maxDataBufferSize;

            if (dataBufferSize - offset >= increment) {
                currPos_DataBuffer += offset;
                count += offset;
                while (count <= dataBufferSize - increment) {
                    *currPos_DataBuffer = rand() & 0xFF;
                    currPos_DataBuffer += increment;
                    count += increment;
                }
            }
            #ifdef _DEBUGOUTPUT
            event_end = clock();
            gen_rand_data += event_end - event_start;
            #endif
        }

        // encode and error out if it fails
        #ifdef _DEBUGOUTPUT
        event_start = clock();
        #endif
        if (!encode(arg_s->bitsUsed, arg_s->bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
            free(wavBuffer); free(dataBuffer);
            arg_s->enc_ret = false;
            return &arg_s->enc_ret;
        }
        #ifdef _DEBUGOUTPUT
        event_end = clock();
        encoding_data += event_end - event_start;
        #endif

        #ifdef _DEBUGOUTPUT
        event_start = clock();
        #endif
        // write the changes to the file
        pwrite(arg_s->fOutputWAV, wavBuffer, wavBufferSize, wav_out_offset);
        #ifdef _DEBUGOUTPUT
        event_end = clock();
        writing_data += event_end - event_start;
        #endif
        wav_out_offset += wavBufferSize;
        currentSize += maxDataBufferSize;
    }

    LOG_DEBUG("S: Extended Time Data (in seconds)\nTime Reading Audio: %lf\tTime Reading Data: %lf\tTime Generating Data: %lf\nTime Encoding Data: %lf\tTime Writing Data: %lf\n",(double)reading_audio_data/CLOCKS_PER_SEC,(double)reading_file_data/CLOCKS_PER_SEC,(double)gen_rand_data/CLOCKS_PER_SEC,(double)encoding_data/CLOCKS_PER_SEC,(double)writing_data/CLOCKS_PER_SEC);
    LOG_DEBUG("S: Extended Time Data (in clock_ticks)\nTime Reading Audio: %lu\tTime Reading Data: %lu\tTime Generating Data: %lu\nTime Encoding Data: %lu\tTime Writing Data: %lu\n",reading_audio_data,reading_file_data,gen_rand_data,encoding_data,writing_data);

    /* cleanup and exit */
    free(wavBuffer); free(dataBuffer);
    return &arg_s->enc_ret;
}

/****************************************************************/
/* function: encode                                             */
/* purpose: encode data into the audio file using a buffer      */
/* args: const int8, const int32, int8*, const size_t, int8*,   */
/*        const size_t                                          */
/* returns: bool                                                */
/****************************************************************/
bool wav::encode(const int8 bitsUsed, const int32 bytesPerSample, int8 *wavBuffer, const size_t wavBufferSize, int8 *dataBuffer, const size_t dataBufferSize) {
    int8 tempByte = 0x00;
    size_t count = 0x00;
    int8* currPos_WavBuffer = wavBuffer;
    int8* currPos_DataBuffer = dataBuffer;

    if (wavBufferSize == 0) {
        LOG_DEBUG("E: Invalid WAV buffer size\n");
        return false;
    }
    if (dataBufferSize == 0) {
        LOG_DEBUG("E: Invalid DATA buffer size\n");
        return false;
    }

    switch (bitsUsed) {
        case 1:
            while (count < dataBufferSize) {
                tempByte = *currPos_DataBuffer;
                for (char i = 7; i >= 0; i--) {
                    setBit(*currPos_WavBuffer, 0, getBit(tempByte,i));
                    currPos_WavBuffer += bytesPerSample;
                }
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 2:
            while (count < dataBufferSize) {    
                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;
                
                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte <<= 2;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte <<= 4;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte <<= 6;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                currPos_DataBuffer++;
                count++;
            }
            break;
        case 4:
            while (count < dataBufferSize) {
                tempByte = *currPos_DataBuffer;
                clearLower4Bits(*currPos_WavBuffer);
                tempByte >>= 4;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                tempByte = *currPos_DataBuffer;
                clearLower4Bits(*currPos_WavBuffer);
                clearUpper4Bits(tempByte);
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;    
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 8:
            while (count < dataBufferSize) {
                *currPos_WavBuffer = *currPos_DataBuffer;
                currPos_WavBuffer += bytesPerSample;
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 12: // Ugly and not very efficient
            while (count < dataBufferSize) {
                // first byte
                *currPos_WavBuffer = *currPos_DataBuffer;
                currPos_WavBuffer += bytesPerSample;
                currPos_DataBuffer++;
                count++;
                if (count < dataBufferSize) {
                    // second byte
                    *currPos_WavBuffer = *currPos_DataBuffer;
                    currPos_WavBuffer -= (bytesPerSample-1);
                    currPos_DataBuffer++;
                    count++;
                    if (count < dataBufferSize) {
                        // third byte
                        tempByte = *currPos_DataBuffer;
                        clearLower4Bits(*currPos_WavBuffer);
                        tempByte >>= 4;
                        *currPos_WavBuffer += tempByte;
                        currPos_WavBuffer += bytesPerSample;

                        tempByte = *currPos_DataBuffer;
                        clearLower4Bits(*currPos_WavBuffer);
                        clearUpper4Bits(tempByte);
                        *currPos_WavBuffer += tempByte;
                        currPos_WavBuffer += (bytesPerSample-1);    
                        currPos_DataBuffer++;
                        count++;
                    }
                }
            }
            break;
        case 16:
            while (count < dataBufferSize) {
                // first byte
                *currPos_WavBuffer = *currPos_DataBuffer;
                currPos_WavBuffer++;
                currPos_DataBuffer++;
                count++;
                // second byte
                if (count < dataBufferSize) {
                    *currPos_WavBuffer = *currPos_DataBuffer;
                    currPos_WavBuffer += (bytesPerSample - 1);
                    currPos_DataBuffer++;
                    count++;
                }
            }
            break;
        case 32:
            while (count < dataBufferSize) {
                // first byte
                *currPos_WavBuffer = *currPos_DataBuffer;
                currPos_WavBuffer++;
                currPos_DataBuffer++;
                count++;
                // second byte
                if (count < dataBufferSize) {
                    *currPos_WavBuffer = *currPos_DataBuffer;
                    currPos_WavBuffer++;
                    currPos_DataBuffer++;
                    count++;
                    // third byte
                    if (count < dataBufferSize) {
                        *currPos_WavBuffer = *currPos_DataBuffer;
                        currPos_WavBuffer++;
                        currPos_DataBuffer++;
                        count++;
                        // fourth byte
                        if (count < dataBufferSize) {
                            *currPos_WavBuffer = *currPos_DataBuffer;
                            currPos_WavBuffer += (bytesPerSample - 3);
                            currPos_DataBuffer++;
                            count++;
                        }
                    }
                }
            }
            break;
        case 48:
            while (count < dataBufferSize) {
                // first byte
                *currPos_WavBuffer = *currPos_DataBuffer;
                currPos_WavBuffer++;
                currPos_DataBuffer++;
                count++;
                // second byte
                if (count < dataBufferSize) {
                    *currPos_WavBuffer = *currPos_DataBuffer;
                    currPos_WavBuffer++;
                    currPos_DataBuffer++;
                    count++;
                    // third byte
                    if (count < dataBufferSize) {
                        *currPos_WavBuffer = *currPos_DataBuffer;
                        currPos_WavBuffer++;
                        currPos_DataBuffer++;
                        count++;
                        // fourth byte
                        if (count < dataBufferSize) {
                            *currPos_WavBuffer = *currPos_DataBuffer;
                            currPos_WavBuffer++;
                            currPos_DataBuffer++;
                            count++;
                            // fifth byte
                            if (count < dataBufferSize) {
                                *currPos_WavBuffer = *currPos_DataBuffer;
                                currPos_WavBuffer++;
                                currPos_DataBuffer++;
                                count++;
                                // sixth byte
                                if (count < dataBufferSize) {
                                    *currPos_WavBuffer = *currPos_DataBuffer;
                                    currPos_WavBuffer += (bytesPerSample - 5);
                                    currPos_DataBuffer++;
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            LOG_DEBUG("E: Invalid number of bits used (%hu)\n", (unsigned short)bitsUsed);
            return false;
    }
    return true;
}

/****************************************************************/
/* function: encode_offset                                      */
/* purpose: encode data into the audio file using a buffer      */
/* args: const int8, const int32, int8*, const size_t, int8*,   */
/*        const size_t, const unsigned char                     */
/* returns: bool                                                */
/****************************************************************/
bool wav::encode_offset(const int8 bitsUsed, const int32 bytesPerSample, int8 *wavBuffer, const size_t wavBufferSize, int8 *dataBuffer, const size_t dataBufferSize, const unsigned char off) {
    int8 tempByte = 0x00;
    size_t count = 0x00;
    int8* currPos_WavBuffer = wavBuffer;
    int8* currPos_DataBuffer = dataBuffer;

    if (wavBufferSize == 0) {
        LOG_DEBUG("E: Invalid WAV buffer size\n");
        return false;
    }
    if (dataBufferSize == 0) {
        LOG_DEBUG("E: Invalid DATA buffer size\n");
        return false;
    }
    if(off > 7) {
        LOG_DEBUG("E: Invalid data offset specified\n");
        return false;
    }

    switch (bitsUsed) {
        case 1:    // change 0 to off to support the offset, yay easy one.
            while (count < dataBufferSize) {
                tempByte = *currPos_DataBuffer;
                for (char i = 7; i >= 0; i--) {
                    setBit(*currPos_WavBuffer, off, getBit(tempByte,i));
                    currPos_WavBuffer += bytesPerSample;
                }
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 2:
            while (count < dataBufferSize) {    
                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;
                
                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte <<= 2;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte <<= 4;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                clearLower2Bits(*currPos_WavBuffer);
                tempByte = *currPos_DataBuffer;
                tempByte <<= 6;
                tempByte >>= 6;
                *currPos_WavBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                currPos_DataBuffer++;
                count++;
            }
            break;
        default:
            LOG_DEBUG("E: Invalid number of bits used (%hu)\n", (unsigned short)bitsUsed);
            return false;
    }
    return true;
}

/****************************************************************/
/* function: decode                                             */
/* purpose: open the files ment for decoding                    */
/* args: const char[], const char[], const int32&               */
/* returns: bool                                                */
/****************************************************************/
bool wav::decode(const char inputWAV[], const char outputDATA[], const int32& fileSize) {
    int fInputWAV, fOutputDATA;
    bool ret_val = 0;

    LOG("Opening input wave file...\n");
    // Open up our input file
    fInputWAV = open_file(inputWAV, "rb");
    if (fInputWAV == -1) { return false; }

    LOG("Validating input wave file...\n");
    // read and validate wave header (RIFF Chunk), and format chunk
    if (RIFFread(fInputWAV, this) != RIFF_SUCCESS || !validWAV()) { close_file(fInputWAV);  return false; }

    LOG("Opening output data file...\n");
    // open up our output file
    fOutputDATA = open_file(outputDATA, "wb");
    if (fOutputDATA == -1) { close_file(fInputWAV); return false; }

    LOG("Decoding data...\n");
    ret_val = decode(fInputWAV, fOutputDATA, fileSize);

    // clean up
    close_file(fInputWAV); close_file(fOutputDATA);
    clean();
    return ret_val;
}

/****************************************************************/
/* function: decode                                             */
/* purpose: do all necessary calculations and handle buffering  */
/* prerequisites: files are open; header data has been read     */
/* args: const char[], const char[], const int32&               */
/* returns: bool                                                */
/****************************************************************/
bool wav::decode(int fInputWAV, int fOutputDATA, const int32& fileSize) {
    size_t count = 0;
    int32 maxSize = 0, bytesPerSample = (fmt.BitsPerSample >> 3), initial_offset = 0;
    int8 bitsUsed = 0x00;

    if (fileSize == 0) {
        LOG("File size is 0\n");
        return false;
    }

    // get the maximum number of bytes the wav file could hold
    maxSize = getMaxBytesEncoded(fmt.BitsPerSample, data.SubchunkSize);
    if (fileSize > maxSize) {
        LOG_DEBUG("E: Data file is too large (Want to retrieve %.*f MB - Can retrieve %.*f MB)\n", 3, byteToMB(fileSize), 3, byteToMB(maxSize));
        return false;
    }
    LOG_DEBUG("S: Data fits (Retrieving %.*f MB - Can retrieve %.*f MB)\n", 3, byteToMB(fileSize), 3, byteToMB(maxSize));

    // get the minimum number of bits the wav file could encode per sample
    bitsUsed = getMinBitsEncodedPS(fmt.BitsPerSample, fileSize, maxSize);
    if (bitsUsed == 0 || bitsUsed > (fmt.BitsPerSample >> 1)) {
        LOG_DEBUG("E: This should never happen... bitsused %d\n", (int)bitsUsed);
        return false;
    }
    LOG_DEBUG("S: Data file could fit at %d bits per sample\n", (int)bitsUsed);

    #ifdef _DEBUGOUTPUT
    clock_t start = clock();
    #endif

    // Set the initial offset
    initial_offset = lseek(fInputWAV,0,SEEK_CUR);

    //--------------------- Parallel portion can start right here ---------------------
    if( !(count = parallel_decode(fInputWAV, fOutputDATA, bytesPerSample, fileSize, bitsUsed, initial_offset)) ) {
        return false;
    }
    //--------------------- Parallel portion can end right here -----------------------

    LOG_DEBUG("S: Took %.3f seconds to decode.\n", ((double)clock() - start) / CLOCKS_PER_SEC );
    LOG_DEBUG("S: Number of bytes retrieved: %u\n", (unsigned int)count);

    return true;
}

/****************************************************************/
/* function: parallel_decode                                    */
/* purpose: ddecode data from the audio file using a buffer in  */
/*  parallel                                                    */
/* args: int , int , const unsigned long int, const int32,      */
/*  const int8, const int32                                     */
/* returns: bool                                                */
/****************************************************************/
size_t wav::parallel_decode(int fInputWAV, int fOutputDATA, const int32 &bytesPerSample, const int32& fileSize, const int8 &bitsUsed, const int32 &initial_offset) {
    size_t count = 0, wavBufferSize, maxWavBufferSize, dataBufferSize, maxDataBufferSize;
    int32 wav_in_offset = initial_offset, data_out_offset = 0;
    int8 *wavBuffer = NULL, *dataBuffer = NULL;

    // Calculate the size of our buffers
    maxWavBufferSize = BUFFER_MULT * (1024 * bytesPerSample);
    maxDataBufferSize = BUFFER_MULT * (128 * bitsUsed);

    // Get memory for our buffers
    if ((wavBuffer = (int8*)calloc(maxWavBufferSize, sizeof(int8))) == NULL) {
        LOG_DEBUG("E: Failed to get memory for WAV buffer\n");
        return false;
    }
    LOG_DEBUG("S: Got %u bytes for WAV buffer\n", (unsigned int)maxWavBufferSize);

    if ((dataBuffer = (int8*)calloc(maxDataBufferSize, sizeof(int8))) == NULL) {
        LOG_DEBUG("E: Failed to get memory for DATA buffer\n");
        free(wavBuffer);
        return false;
    }
    LOG_DEBUG("S: Got %u bytes for DATA buffer\n", (unsigned int)maxDataBufferSize);

    #ifdef _DEBUGOUTPUT
    clock_t event_start;
    clock_t event_end;
    unsigned long int reading_audio_data = 0;
    unsigned long int decoding_data = 0;
    unsigned long int writing_file_data = 0;
    #endif

    // read into the buffers, process, and write
    count = 0;
    for(;;) {
        #ifdef _DEBUGOUTPUT
        event_start = clock();
        #endif
        wavBufferSize = pread(fInputWAV, wavBuffer, maxWavBufferSize, wav_in_offset);
        #ifdef _DEBUGOUTPUT
        event_end = clock();
        reading_audio_data += event_end - event_start;
        #endif
        wav_in_offset += wavBufferSize; 

        if (count + maxDataBufferSize > fileSize) {
            dataBufferSize = fileSize - count;
            count = fileSize;
        } else {
            dataBufferSize = maxDataBufferSize;
            count += maxDataBufferSize;
        }

        #ifdef _DEBUGOUTPUT
        event_start = clock();
        #endif
        if (!decode(bitsUsed, bytesPerSample, wavBuffer, wavBufferSize, dataBuffer, dataBufferSize)) {
            free(wavBuffer); free(dataBuffer);
            return false;
        }
        #ifdef _DEBUGOUTPUT
        event_end = clock();
        decoding_data += event_end - event_start;
        #endif

        #ifdef _DEBUGOUTPUT
        event_start = clock();
        #endif
        pwrite(fOutputDATA, dataBuffer, dataBufferSize, data_out_offset);
        #ifdef _DEBUGOUTPUT
        event_end = clock();
        writing_file_data += event_end - event_start;
        #endif
        data_out_offset +=  dataBufferSize;

        if (count == fileSize)
            break;
    }

    LOG_DEBUG("S: Extended Time Data (in seconds)\nTime Reading Audio: %lf\tTime Decoding Data: %lf\tTime Writing Data: %lf\n",(double)reading_audio_data/CLOCKS_PER_SEC,(double)decoding_data/CLOCKS_PER_SEC,(double)writing_file_data/CLOCKS_PER_SEC);
    LOG_DEBUG("S: Extended Time Data (in clock_ticks)\nTime Reading Audio: %lu\tTime Decoding Data: %lu\tTime Writing Data: %lu\n",reading_audio_data,decoding_data,writing_file_data);

    /* cleanup and exit */
    free(wavBuffer); free(dataBuffer);
    return count;
}

/****************************************************************/
/* function: decode                                             */
/* purpose: decode data from the audio file that is in ram      */
/* args: const int8, const int32, int8*, const size_t, int8*,   */
/*    const size_t                                              */
/* returns: bool                                                */
/****************************************************************/
bool wav::decode(const int8 bitsUsed, const int32 bytesPerSample, int8 *wavBuffer, const size_t wavBufferSize, int8 *dataBuffer, const size_t dataBufferSize) {
    int8 tempByte = 0x00;
    size_t count = 0x00;
    int8* currPos_WavBuffer = wavBuffer;
    int8* currPos_DataBuffer = dataBuffer;

    if (wavBufferSize == 0) {
        LOG_DEBUG("E: Invalid WAV buffer size\n");
        return false;
    }
    if (dataBufferSize == 0) {
        LOG_DEBUG("E: Invalid DATA buffer size\n");
        return false;
    }

    // Grab the bits from each sample, build a byte, and output the bytes to a file
    switch (bitsUsed) {
        case 1:
            while (count < dataBufferSize) {
                for (char j = 7; j >= 0; j--) {
                    setBit(tempByte, j, getBit(*currPos_WavBuffer, 0));
                    currPos_WavBuffer += bytesPerSample;
                    
                }
                *currPos_DataBuffer = tempByte;
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 2:
            while (count < dataBufferSize) {
                *currPos_DataBuffer = 0;

                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;
                
                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                tempByte >>= 2;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                tempByte >>= 4;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                tempByte >>= 6;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                currPos_DataBuffer++;
                count++;
            }
            break;
        case 4:
            while (count < dataBufferSize) {
                tempByte = *currPos_WavBuffer;
                *currPos_DataBuffer = 0;
                tempByte <<= 4;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                tempByte = *currPos_WavBuffer;
                clearUpper4Bits(tempByte);
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 8:
            while (count < dataBufferSize) {
                *currPos_DataBuffer = *currPos_WavBuffer;
                currPos_WavBuffer += bytesPerSample;
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 12: // Ugly and not very efficient
            while (count < dataBufferSize) {
                // first byte
                *currPos_DataBuffer = *currPos_WavBuffer;
                currPos_WavBuffer += bytesPerSample;
                currPos_DataBuffer++;
                count++;
                if (count < dataBufferSize) {
                    // second byte
                    *currPos_DataBuffer = *currPos_WavBuffer;
                    currPos_WavBuffer -= (bytesPerSample-1);
                    currPos_DataBuffer++;
                    count++;
                    if (count < dataBufferSize) {
                        // third byte
                        tempByte = *currPos_WavBuffer;
                        *currPos_DataBuffer = 0;
                        tempByte <<= 4;
                        *currPos_DataBuffer += tempByte;
                        currPos_WavBuffer += bytesPerSample;

                        tempByte = *currPos_WavBuffer;
                        clearUpper4Bits(tempByte);
                        *currPos_DataBuffer += tempByte;
                        currPos_WavBuffer += (bytesPerSample-1);
                        currPos_DataBuffer++;
                        count++;
                    }
                }
            }
            break;
        case 16:
            while (count < dataBufferSize) {
                // first byte
                *currPos_DataBuffer = *currPos_WavBuffer;
                currPos_WavBuffer++;
                currPos_DataBuffer++;
                count++;
                // second byte
                if (count < dataBufferSize) {
                    *currPos_DataBuffer = *currPos_WavBuffer;
                    currPos_WavBuffer += (bytesPerSample - 1);
                    currPos_DataBuffer++;
                    count++;
                }
            }
            break;
        case 32:
            while (count < dataBufferSize) {
                // first byte
                *currPos_DataBuffer = *currPos_WavBuffer;
                currPos_WavBuffer++;
                currPos_DataBuffer++;
                count++;
                // second byte
                if (count < dataBufferSize) {
                    *currPos_DataBuffer = *currPos_WavBuffer;
                    currPos_WavBuffer++;
                    currPos_DataBuffer++;
                    count++;
                    // third byte
                    if (count < dataBufferSize) {
                        *currPos_DataBuffer = *currPos_WavBuffer;
                        currPos_WavBuffer++;
                        currPos_DataBuffer++;
                        count++;
                        // fourth byte
                        if (count < dataBufferSize) {
                            *currPos_DataBuffer = *currPos_WavBuffer;
                            currPos_WavBuffer += (bytesPerSample - 3);
                            currPos_DataBuffer++;
                            count++;
                        }
                    }
                }
            }
            break;
        case 48:
            while (count < dataBufferSize) {
                // first byte
                *currPos_DataBuffer = *currPos_WavBuffer;
                currPos_WavBuffer++;
                currPos_DataBuffer++;
                count++;
                // second byte
                if (count < dataBufferSize) {
                    *currPos_DataBuffer = *currPos_WavBuffer;
                    currPos_WavBuffer++;
                    currPos_DataBuffer++;
                    count++;
                    // third byte
                    if (count < dataBufferSize) {
                        *currPos_DataBuffer = *currPos_WavBuffer;
                        currPos_WavBuffer++;
                        currPos_DataBuffer++;
                        count++;
                        // fourth byte
                        if (count < dataBufferSize) {
                            *currPos_DataBuffer = *currPos_WavBuffer;
                            currPos_WavBuffer++;
                            currPos_DataBuffer++;
                            count++;
                            // fifth byte
                            if (count < dataBufferSize) {
                                *currPos_DataBuffer = *currPos_WavBuffer;
                                currPos_WavBuffer++;
                                currPos_DataBuffer++;
                                count++;
                                // sixth byte
                                if (count < dataBufferSize) {
                                    *currPos_DataBuffer = *currPos_WavBuffer;
                                    currPos_WavBuffer += (bytesPerSample - 5);
                                    currPos_DataBuffer++;
                                    count++;
                                }
                            }
                        }
                    }
                }
            }
            break;
        default:
            LOG_DEBUG("E: Invalid number of bits used (%hu)\n", (unsigned short)bitsUsed);
            return false;
    }
    return true;
}

/****************************************************************/
/* function: decode_offset                                      */
/* purpose: decode data from the audio file that is in ram      */
/* args: const int8, const int32, int8*, const size_t, int8*,   */
/*        const size_t, const unsigned char                     */
/* returns: bool                                                */
/****************************************************************/
bool wav::decode_offset(const int8 bitsUsed, const int32 bytesPerSample, int8 *wavBuffer, const size_t wavBufferSize, int8 *dataBuffer, const size_t dataBufferSize, const unsigned char off) {
    int8 tempByte = 0x00;
    size_t count = 0x00;
    int8* currPos_WavBuffer = wavBuffer;
    int8* currPos_DataBuffer = dataBuffer;

    if (wavBufferSize == 0) {
        LOG_DEBUG("E: Invalid WAV buffer size\n");
        return false;
    }
    if (dataBufferSize == 0) {
        LOG_DEBUG("E: Invalid DATA buffer size\n");
        return false;
    }
    if(off > 7) {
        LOG_DEBUG("E: Invalid data offset specified\n");
        return false;
    }

    // Grab the bits from each sample, build a byte, and output the bytes to a file
    switch (bitsUsed) {
        case 1: // also yay easy one
            while (count < dataBufferSize) {
                for (char j = 7; j >= 0; j--) {
                    setBit(tempByte, j, getBit(*currPos_WavBuffer, off));
                    currPos_WavBuffer += bytesPerSample;
                    
                }
                *currPos_DataBuffer = tempByte;
                currPos_DataBuffer++;
                count++;
            }
            break;
        case 2:
            while (count < dataBufferSize) {
                *currPos_DataBuffer = 0;

                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;
                
                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                tempByte >>= 2;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                tempByte >>= 4;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                tempByte = *currPos_WavBuffer;
                tempByte <<= 6;
                tempByte >>= 6;
                *currPos_DataBuffer += tempByte;
                currPos_WavBuffer += bytesPerSample;

                currPos_DataBuffer++;
                count++;
            }
            break;
        default:
            LOG_DEBUG("E: Invalid number of bits used (%hu)\n", (unsigned short)bitsUsed);
            return false;
    }
    return true;
}

/****************************************************************/
/* function: getMaxBytesEncoded                                 */
/* purpose: calculate max number of bytes a WAV can encode      */
/* args: const int16, const int32                               */
/* returns: int32                                               */
/****************************************************************/
int32 wav::getMaxBytesEncoded(const int16 bitsPerSample, const int32 subchunkSize) {
    int32 maxSize, bytesPerSample = (bitsPerSample >> 3);

    // allows the use of only the bottom half of the bits per sample
    switch (bitsPerSample) {
        case 8:
            maxSize = (subchunkSize / bytesPerSample) >> 1;
            break;
        case 16:
            maxSize = (subchunkSize / bytesPerSample);
            break;
        case 24:
            maxSize = (subchunkSize / bytesPerSample);
            maxSize += maxSize >> 1;
            break;
        case 32:
            maxSize = (subchunkSize / bytesPerSample) << 1;
            break;
        case 64: // allows use of bottom 48 bits because 16 bits is enough audio data
            maxSize = (subchunkSize / bytesPerSample) << 2;
            maxSize += ((subchunkSize / bytesPerSample) << 1);
            break;
        default:
            maxSize = 0;
            break;
    }
    return maxSize;
}

/****************************************************************/
/* function: getMinBitsEncodedPS                                */
/* purpose: calculate min number of bits possibly encoded       */
/*            per sample                                        */
/* args: const int16, const int32, const int32                  */
/* returns: int8                                                */
/****************************************************************/
int8 wav::getMinBitsEncodedPS(const int16 bitsPerSample, const int32 fileSize, const int32 maxSize) {
    double d_MinBPS = 0.0;
    int i_MinBPS = 0;

    if (fileSize == 0 || maxSize == 0)
        return 0;

    // 64 bits is allowed 3/4 of the bits for encoding. we need to account for that
    if(bitsPerSample != 64) {
        d_MinBPS = ((double)(bitsPerSample >> 1) / ((double)maxSize / (double)fileSize));
    } else {
        d_MinBPS = ((double)((bitsPerSample >> 2)*3) / ((double)maxSize / (double)fileSize));
    }

    // convert to int
    i_MinBPS = (int)d_MinBPS;
    // make sure it rounded up
    if (d_MinBPS > (double)i_MinBPS)
        i_MinBPS++;

    if (i_MinBPS == 1)
        return 1;
    else if (i_MinBPS == 2)
        return 2;
    else if (i_MinBPS <= 4)
        return 4;
    else if (i_MinBPS <= 8)
        return 8;
    else if (i_MinBPS <= 12)
        return 12;
    else if (i_MinBPS <= 16)
        return 16;
    else if (i_MinBPS <= 32)
        return 32;
    else if (i_MinBPS <= 48)
        return 48;
    else
        return 0;
}

/****************************************************************/
/****************************************************************/
/****************************************************************/

