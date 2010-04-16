/**
 * \file config.h
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
 *
 * This set of compile-time options may be used to enable
 * or disable features selectively, and reduce the global
 * memory footprint.
 */
#ifndef POLARSSL_CONFIG_H
#define POLARSSL_CONFIG_H

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

/*
 * Uncomment if native integers are 8-bit wide.
 *
#define POLARSSL_HAVE_INT8
 */

/*
 * Uncomment if native integers are 16-bit wide.
 *
#define POLARSSL_HAVE_INT16
 */

/*
 * Uncomment if the compiler supports long long.
 *
#define POLARSSL_HAVE_LONGLONG
 */

/*
 * Uncomment to enable the use of assembly code.
 *
 * Requires support for asm() in compiler.
 *
 * Used in:
 *      library/timing.c
 *      library/padlock.c
 *      include/polarssl/bn_mul.h
 *
 */
#define POLARSSL_HAVE_ASM

/*
 * Uncomment if the CPU supports SSE2 (IA-32 specific).
 *
#define POLARSSL_HAVE_SSE2
 */

/*
 * Enable the checkup functions (*_self_test).
 */
#define POLARSSL_SELF_TEST

/*
 * Enable the prime-number generation code.
 */
#define POLARSSL_GENPRIME

/*
 * Uncomment this macro to store the AES tables in ROM.
 *
#define POLARSSL_AES_ROM_TABLES
 */

/*
 * Module:  library/aes.c
 * Caller:  library/ssl_tls.c
 *
 * This module enables the following ciphersuites:
 *      SSL_RSA_AES_128_SHA
 *      SSL_RSA_AES_256_SHA
 *      SSL_EDH_RSA_AES_256_SHA
 */
#define POLARSSL_AES_C

/*
 * Module:  library/padlock.c
 * Caller:  library/aes.c
 *
 * This modules adds support for the VIA PadLock on x86.
 */
#define POLARSSL_PADLOCK_C

/*
 * Module:  library/sha2.c
 * Caller:
 *
 * This module adds support for SHA-224 and SHA-256.
 */
#define POLARSSL_SHA2_C

#endif /* config.h */
