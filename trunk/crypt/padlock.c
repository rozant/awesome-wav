/*
 *  VIA PadLock support functions
 *
 *  Copyright (C) 2006-2010, Paul Bakker <polarssl_maintainer at polarssl.org>
 *  All rights reserved.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
/*
 *  This implementation is based on the VIA PadLock Programming Guide:
 *
 *  http://www.via.com.tw/en/downloads/whitepapers/initiatives/padlock/
 *  programming_guide.pdf
 */

#include "./config.h"

#if defined(POLARSSL_PADLOCK_C)

#include "./aes.h"
#include "./padlock.h"

#if defined(POLARSSL_HAVE_X86)

#include <string.h>

/*
 * PadLock detection routine
 */
int padlock_supports( int feature )
{
    static int flags = -1;
    int ebx, edx;

    if( flags == -1 )
    {
        asm( "movl  %%ebx, %0           \n"     \
             "movl  $0xC0000000, %%eax  \n"     \
             "cpuid                     \n"     \
             "cmpl  $0xC0000001, %%eax  \n"     \
             "movl  $0, %%edx           \n"     \
             "jb    unsupported         \n"     \
             "movl  $0xC0000001, %%eax  \n"     \
             "cpuid                     \n"     \
             "unsupported:              \n"     \
             "movl  %%edx, %1           \n"     \
             "movl  %2, %%ebx           \n"
             : "=m" (ebx), "=m" (edx)
             :  "m" (ebx)
             : "eax", "ecx", "edx" );

        flags = edx;
    }

    return( flags & feature );
}

/*
 * PadLock AES-ECB block en(de)cryption
 */
int padlock_xcryptecb( aes_context *ctx,
                       int mode,
                       const unsigned char input[16],
                       unsigned char output[16] )
{
    int ebx;
    unsigned long *rk;
    unsigned long *blk;
    unsigned long *ctrl;
    unsigned char buf[256];

    rk  = ctx->rk;
    blk = PADLOCK_ALIGN16( buf );
    memcpy( blk, input, 16 );

     ctrl = blk + 4;
    *ctrl = 0x80 | ctx->nr | ( ( ctx->nr + ( mode^1 ) - 10 ) << 9 );

    asm( "pushfl; popfl         \n"     \
         "movl    %%ebx, %0     \n"     \
         "movl    $1, %%ecx     \n"     \
         "movl    %2, %%edx     \n"     \
         "movl    %3, %%ebx     \n"     \
         "movl    %4, %%esi     \n"     \
         "movl    %4, %%edi     \n"     \
         ".byte  0xf3,0x0f,0xa7,0xc8\n" \
         "movl    %1, %%ebx     \n"
         : "=m" (ebx)
         :  "m" (ebx), "m" (ctrl), "m" (rk), "m" (blk)
         : "ecx", "edx", "esi", "edi" );

    memcpy( output, blk, 16 );

    return( 0 );
}

#endif

#endif
