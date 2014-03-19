#ifndef OCCAFFSTRING_H
#define OCCAFFSTRING_H

/*
 * The code in the header file was adapted from petsc.  The license information
 * for petsc can be found below.
 *
 *     Licensing Notification
 *
 *       Permission to use, reproduce, prepare derivative works, and to
 *       redistribute to others this software, derivatives of this software, and
 *       future versions of this software as well as its documentation is hereby
 *       granted, provided that this notice is retained thereon and on all
 *       copies or modifications. This permission is perpetual, world-wide, and
 *       provided on a royalty-free basis. UChicago Argonne, LLC and all other
 *       contributors make no representations as to the suitability and
 *       operability of this software for any purpose. It is provided "as is"
 *       without express or implied warranty.
 *
 *      Authors: http://www.mcs.anl.gov/petsc/miscellaneous/index.html
 *
 *
 *      - Mathematics and Computer Science Division
 *      - Argonne National Laboratory
 *      - Argonne IL 60439
 *
 *
 *       Portions of this software are copyright by UChicago Argonne, LLC.
 *       Argonne National Laboratory with facilities in the state of Illinois,
 *       is owned by The United States Government, and operated by UChicago
 *       Argonne, LLC under provision of a contract with the Department of
 *       Energy.
 *
 *     DISCLAIMER
 *
 *       PORTIONS OF THIS SOFTWARE WERE PREPARED AS AN ACCOUNT OF WORK SPONSORED
 *       BY AN AGENCY OF THE UNITED STATES GOVERNMENT. NEITHER THE UNITED STATES
 *       GOVERNMENT NOR ANY AGENCY THEREOF, NOR THE UNIVERSITY OF CHICAGO, NOR
 *       ANY OF THEIR EMPLOYEES OR OFFICERS, MAKES ANY WARRANTY, EXPRESS OR
 *       IMPLIED, OR ASSUMES ANY LEGAL LIABILITY OR RESPONSIBILITY FOR THE
 *       ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY INFORMATION, APPARATUS,
 *       PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE WOULD NOT
 *       INFRINGE PRIVATELY OWNED RIGHTS. REFERENCE HEREIN TO ANY SPECIFIC
 *       COMMERCIAL PRODUCT, PROCESS, OR SERVICE BY TRADE NAME, TRADEMARK,
 *       MANUFACTURER, OR OTHERWISE, DOES NOT NECESSARILY CONSTITUTE OR IMPLY
 *       ITS ENDORSEMENT, RECOMMENDATION, OR FAVORING BY THE UNITED STATES
 *       GOVERNMENT OR ANY AGENCY THEREOF. THE VIEW AND OPINIONS OF AUTHORS
 *       EXPRESSED HEREIN DO NOT NECESSARILY STATE OR REFLECT THOSE OF THE
 *       UNITED STATES GOVERNMENT OR ANY AGENCY THEREOF.
 *
 */

#include <stdlib.h>
#include <string.h>

#define OCCAF_NULL_CHARACTER_Fortran ((char *) 0)

/* --------------------------------------------------------------------*/
/*
    This lets us map the str-len argument either, immediately following
    the char argument (DVF on Win32) or at the end of the argument list
    (general unix compilers)
*/
#if defined(OCCAF_HAVE_FORTRAN_MIXED_STR_ARG)
#define OCCAF_MIXED_LEN(len) ,int len
#define OCCAF_END_LEN(len)
#define OCCAF_MIXED_LEN_CALL(len) ,len
#define OCCAF_END_LEN_CALL(len)
#define OCCAF_MIXED_LEN_PROTO ,int
#define OCCAF_END_LEN_PROTO
#else
#define OCCAF_MIXED_LEN(len)
#define OCCAF_END_LEN(len)   ,int len
#define OCCAF_MIXED_LEN_CALL(len)
#define OCCAF_END_LEN_CALL(len)   ,len
#define OCCAF_MIXED_LEN_PROTO
#define OCCAF_END_LEN_PROTO   ,int
#endif

/* --------------------------------------------------------------------*/
#define OCCAF_CHAR char*
#define OCCAF_FIXCHAR(a,n,b) \
do {\
  if (a == OCCAF_NULL_CHARACTER_Fortran) { \
    b = a = 0; \
  } else { \
    while((n > 0) && (a[n-1] == ' ')) n--; \
    b = malloc((n+1)*sizeof(char)); \
    if(b==NULL) abort(); \
    strncpy(b,a,n); \
    b[n] = '\0'; \
  } \
} while (0)

#define OCCAF_FREECHAR(a,b) \
do {\
  if (a != b) free(b);\
} while (0)

#define OCCAF_FIXRETURNCHAR(flg,a,n)               \
do {\
  if (flg) {                                   \
    int __i;                                   \
    for (__i=0; __i<n && a[__i] != 0; __i++) ; \
    for (; __i<n; __i++) a[__i] = ' ' ; \
  }\
} while (0)

#endif
