/* ============================================================================
 *  LConfig.h
 * ============================================================================

 *  Author:         (c) 2001-2005 Camil Demetrescu
 *  License:        See the end of this file for license information
 *  Created:        November 28, 2001
 *  Module:         LL

 *  Last changed:   $Date: 2007/04/13 14:10:05 $
 *  Changed by:     $Author: ribbi $
 *  Revision:       $Revision: 1.2 $
*/

#ifndef __LConfig__
#define __LConfig__


/* COMPONENT ID */
#define LConfig_ID  0x800C
 
/* Supported platforms */
#define __LL_Win32_x86__       0
#define __LL_MacOSX_PowerPC__  1
#define __LL_Linux_x86__       2
#define __LL_Solaris_SPARC__   3
#define __LL_MacOS9_PowerPC__  4
#define __LL_LVM__             5

/* Current platform */
#ifndef __LL_PLATFORM__
#ifdef __LVM__
#define __LL_PLATFORM__       __LL_LVM__
#else
#define __LL_PLATFORM__       __LL_Linux_x86__
#endif
#endif

/* Enable Debug Mode */
#define __LL_DEBUG__

/* ----------------------------------------------------------------------------
 *  AUTO-SETUP
 * ----------------------------------------------------------------------------
*/

/* Supported OS CORE API */
#define __LL_Win32__           0
#define __LL_POSIX__           1

/* Supported CPU */
#define __LL_x86__             0
#define __LL_SPARC__           1
#define __LL_PowerPC__         2
#define __LL_LVM_VCPU__        3

/* Newline format */
#define __LL_CR__              0
#define __LL_LF__              1
#define __LL_CRLF__            2

/* Directory separator */
#define __LL_COLON__           ':'
#define __LL_SLASH__           '/'
#define __LL_BACKSLASH__       '\\'

/* Config current OS CORE, OS GUI, newline format and CPU */
#if __LL_PLATFORM__ == __LL_Win32_x86__
#define __LL_CPU__     __LL_x86__
#define __LL_OS_CORE__ __LL_POSIX__
#define __LL_NEWLINE__ __LL_CRLF__
#define __LL_DIR_SEP__ __LL_BACKSLASH__
#endif

#if __LL_PLATFORM__ == __LL_MacOSX_PowerPC__
#define __LL_CPU__     __LL_PowerPC__
#define __LL_OS_CORE__ __LL_POSIX__
#define __LL_NEWLINE__ __LL_LF__
#define __LL_DIR_SEP__ __LL_SLASH__
#endif

#if __LL_PLATFORM__ == __LL_Linux_x86__
#define __LL_CPU__     __LL_x86__
#define __LL_OS_CORE__ __LL_POSIX__
#define __LL_NEWLINE__ __LL_LF__
#define __LL_DIR_SEP__ __LL_SLASH__
#endif

#if __LL_PLATFORM__ == __LL_Solaris_SPARC__
#define __LL_CPU__     __LL_SPARC__
#define __LL_OS_CORE__ __LL_POSIX__
#define __LL_NEWLINE__ __LL_LF__
#define __LL_DIR_SEP__ __LL_SLASH__
#endif

#if __LL_PLATFORM__ == __LL_LVM__
#define __LL_CPU__     __LL_x86__
#define __LL_OS_CORE__ __LL_POSIX__
#define __LL_NEWLINE__ __LL_LF__
#define __LL_DIR_SEP__ __LL_SLASH__
#endif

/* Little endian CPU */
#if __LL_CPU__ == __LL_x86__
#define __LL_LITTLE_ENDIAN__
#endif

#endif


/* Copyright (C) 2001-2005 Camil Demetrescu

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
