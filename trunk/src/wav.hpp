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
#ifndef __wav_hpp__
#define __wav_hpp__
#include "global.hpp"
#include "riff.hpp"

/* structure to pass arguments to parallel encode */
typedef struct {
    int fInputWAV;
    int fInputDATA;
    int fOutputWAV;
    unsigned long int maxSize;
    int32 bytesPerSample;
    int8 bitsUsed;
    int32 wav_out_init_offset;
    int32 wav_in_init_offset;
    int32 wav_in_block_size;
    int32 data_init_offset;
    bool enc_ret;
} thread_args;

/****************************************************************/
/* class: wav                                                   */
/* purpose: contain an entire wav formatted file in ram.        */
/****************************************************************/
class wav {
    private:
        _RIFF riff;
        _FMT fmt;
        _FACT *fact;
        _PEAK *peak;
        _DATA data;
        _RIFF_UNKNOWN_CHUNKS unknown0;
        thread_args *argt;
        pthread_t *threads;
        int8 num_threads;
        // file operations
        template <class T>
        friend int RIFFread(int, T *);
        template <class T>
        friend int RIFFreadRIFF(int, T *);
        template <class T>
        friend int RIFFreadFMT(int, T *);
        template <class T>
        friend int RIFFreadFACT(int, T *);
        template <class T>
        friend int RIFFreadPEAK(int, T *);
        template <class T>
        friend int RIFFreadDATA(int, T *);
        template <class T>
        friend int RIFFwrite(int, const T *);
        template <class T>
        friend int RIFFwriteRIFF(int, const T *);
        template <class T>
        friend int RIFFwriteFMT(int, const T *);
        template <class T>
        friend int RIFFwriteFACT(int, const T *);
        template <class T>
        friend int RIFFwritePEAK(int, const T *);
        template <class T>
        friend int RIFFwriteDATA(int, const T *);
        // parallel operations
//        friend void *parallel_encode(void *);
        // data integrity checks
        bool validWAV(void) const;
        bool validRIFF(void) const;
        bool validFMT(void) const;
        bool validFACT(void) const;
        bool validDATA(void) const;
        // data operations
        int32 getMaxBytesEncoded(const int16, const int32);
        int8 getMinBitsEncodedPS(const int16, const int32, const int32);
        unsigned long int encode(int, int, int);
        bool encode(const int8, const int32, int8 *, const size_t, int8 *, const size_t);
//        void parallel_encode(int, int, int, const unsigned long int&, const int32&, const int8&, const int32&, const int32&, bool&);
        void *parallel_encode(void);
        static void *parallel_encode_helper(void *context) { return ((wav *)context)->parallel_encode(); }
        bool encode_offset(const int8, const int32, int8 *, const size_t, int8 *, const size_t, const unsigned char);


        bool decode(int, int, const int32&);
        size_t parallel_decode(int, int, const int32&, const int32&, const int8&, const int32&);
        bool decode(const int8, const int32, int8 *, const size_t, int8 *, const size_t);
        bool decode_offset(const int8, const int32, int8 *, const size_t, int8 *, const size_t, const unsigned char);
        // other things
        void clean(void);
    public:
        // constructors
        wav(void);
        // destructor
        ~wav(void);
        // manipulation
        unsigned long int encode(const char[], const char[], const char[]);
        bool decode(const char[], const char[], const int32&);
};

#endif

/****************************************************************/
/****************************************************************/
/****************************************************************/

