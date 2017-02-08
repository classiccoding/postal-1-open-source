////////////////////////////////////////////////////////////////////////////////
//
// Copyright 2016 RWS Inc, All Rights Reserved
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of version 2 of the GNU General Public License as published by
// the Free Software Foundation
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
//
////////////////////////////////////////////////////////////////////////////////
//
//	UnixSystem.h
// 
// History:
//		06/01/04 RCG    Added.

////////////////////////////////////////////////////////////////////////////////
//
//	This file provides typedefs, macros, pragmas, etc. for Unix systems.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef UNIXSYSTEM_H
#define UNIXSYSTEM_H

#include <SDL2/SDL.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#if defined(WIN32) || defined(__WIN32__) || defined(_MSC_VER)
//#warning I find your lack of POSIX disturbing. ;)

#ifndef snprintf
#define snprintf _snprintf
#endif
#ifndef vsnprintf
#define vsnprintf _vsnprintf
#endif
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif

#include <BaseTsd.h>

typedef SSIZE_T ssize_t;

#ifndef __WORDSIZE
#define __WORDSIZE WORDSIZE
#endif

# if __WORDSIZE == 64
#  define __INT64_C(c)	c ## L
#  define __UINT64_C(c)	c ## UL
# else
#  define __INT64_C(c)	c ## LL
#  define __UINT64_C(c)	c ## ULL
# endif

/* Limits of integral types.  */

/* Minimum of signed integral types.  */
# define INT8_MIN		(-128)
# define INT16_MIN		(-32767-1)
# define INT32_MIN		(-2147483647-1)
# define INT64_MIN		(-__INT64_C(9223372036854775807)-1)
/* Maximum of signed integral types.  */
# define INT8_MAX		(127)
# define INT16_MAX		(32767)
# define INT32_MAX		(2147483647)
# define INT64_MAX		(__INT64_C(9223372036854775807))

/* Maximum of unsigned integral types.  */
# define UINT8_MAX		(255)
# define UINT16_MAX		(65535)
# define UINT32_MAX		(4294967295U)
# define UINT64_MAX		(__UINT64_C(18446744073709551615))

#else
#include <sys/types.h>
#endif

#if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
#define SYS_ENDIAN_BIG 1
#else
#define SYS_ENDIAN_LITTLE 1
#endif

#include <map>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// For compilers that support partial paths in #include's, define this.  For
// those that don't, don't define it.
////////////////////////////////////////////////////////////////////////////////
#define PATHS_IN_INCLUDES


////////////////////////////////////////////////////////////////////////////////
// Usefull pragms
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Basic RSPiX Types
////////////////////////////////////////////////////////////////////////////////
#ifndef __RSPX_TYPES
#define __RSPX_TYPES

#define UNUSED1(a)                (void)(a)
#define UNUSED2(a,b)             UNUSED1(a),UNUSED1(b)
#define UNUSED3(a,b,c)           UNUSED1(a),UNUSED2(b,c)
#define UNUSED4(a,b,c,d)         UNUSED1(a),UNUSED3(b,c,d)
#define UNUSED5(a,b,c,d,e)       UNUSED1(a),UNUSED4(b,c,d,e)
#define UNUSED6(a,b,c,d,e,f)     UNUSED1(a),UNUSED5(b,c,d,e,f)
#define UNUSED7(a,b,c,d,e,f,g)   UNUSED1(a),UNUSED6(b,c,d,e,f,g)
#define UNUSED8(a,b,c,d,e,f,g,h) UNUSED1(a),UNUSED7(b,c,d,e,f,g,h)

#define VA_NUM_ARGS_IMPL(_1,_2,_3,_4,_5, _6, _7, _8, N,...) N
#define VA_NUM_ARGS(...) VA_NUM_ARGS_IMPL(__VA_ARGS__,8, 7, 6, 5, 4, 3, 2, 1)

#define UNUSED_IMPL_(nargs) UNUSED ## nargs
#define UNUSED_IMPL(nargs) UNUSED_IMPL_(nargs)
#define UNUSED(...) UNUSED_IMPL( VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__ )

#define UNHANDLED_SWITCH    default: TRACE("Unhandled switch\n"); break

#define UINT8_MIN 0
#define UINT16_MIN 0
#define UINT32_MIN 0
#define UINT64_MIN 0

  static_assert(sizeof(uintptr_t) == sizeof(void*), "your compiler is broken!");

	// 128-bit got a little trickier...
	#ifdef SYS_ENDIAN_LITTLE
		typedef struct {	uint64_t	lo;
                        int64_t	hi;} int128_t;
		typedef struct {	uint64_t	lo;
                        uint64_t	hi;} uint128_t;
	#else	// defined(SYS_ENDIAN_BIG)

		typedef struct {	int64_t	hi;
                        uint64_t	lo;} int128_t;
		typedef struct {	uint64_t	hi;
                        uint64_t	lo;} uint128_t;
	#endif

	// These pixel types take the endian order of the system into account.
	typedef uint8_t RPixel;
	typedef uint16_t RPixel16;
	typedef struct
		{
		uint8_t	u8Red;
		uint8_t	u8Green;
		uint8_t	u8Blue;
		} RPixel24;
	typedef struct
		{
		uint8_t	u8Alpha;
		uint8_t	u8Red;
		uint8_t	u8Green;
		uint8_t	u8Blue;
		} RPixel32;
	inline bool operator==(const RPixel24& lhs, const RPixel24& rhs)
		{ return ((lhs.u8Blue == rhs.u8Blue) && (lhs.u8Green == rhs.u8Green) && (lhs.u8Red == rhs.u8Red)) ? true : false; }
	inline bool operator==(const RPixel32& lhs, const RPixel32& rhs)
		{ return ((lhs.u8Blue == rhs.u8Blue) && (lhs.u8Green == rhs.u8Green) && (lhs.u8Red == rhs.u8Red) && (lhs.u8Alpha == rhs.u8Alpha)) ? true : false; }

#endif // __RSPX_TYPES


////////////////////////////////////////////////////////////////////////////////
// Handy macros.
////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
#define TRUE	1
#endif

#ifndef FALSE
#define FALSE	0
#endif

#ifndef SUCCESS
#define SUCCESS	0
#endif

#ifndef FAILURE
#define FAILURE	-1
#endif

////////////////////////////////////////////////////////////////////////////////
// Usefull Templates
////////////////////////////////////////////////////////////////////////////////
#undef MIN
template <class T>
inline T MIN(T a,T b) { return (a < b) ? a : b; }

#undef MAX
template <class T>
inline T MAX(T a,T b) { return (a > b) ? a : b; }

#undef SWAP	// Swaps two identical typed variables
template <class T>
inline void SWAP(T &a,T &b) { T temp = a; a = b; b = temp; }

#undef SQR // squares a number
template <class T>
inline T SQR(T x) { return x * x; }

#undef ABS // returns the absolute value of a parameter
template <class T>
inline T ABS(T x) { return (x < 0) ? -x : x; }

template <class T> // returns the square of the absolute value
inline T ABS2(T x,T y) { return SQR(x)+SQR(y); }

template <class T> // returns the square of the absolute value
inline T ABS2(T x,T y,T z) { return SQR(x) + SQR(y) + SQR(z); }

#undef SGN // returns a binary sign (+1 or -1)
template <class T>
inline T SGN(T x) { return (x < 0) ? (T)-1 : (T)1; }

#undef SGN3 // returns a trinary sign (+1, 0, or -1)
template <class T>
inline T SGN3(T x) { return (x == 0) ? (T)0 : ((x < 0) ? (T)-1 : (T)1); }

template <class T> // attaches a sign to a value
inline T ADD_SGN(T sign,T val) { return (sign < 0) ? -val : val; }

template <class T> // symmetric mod wrt sign, good for DELTAS
// (This is the mathematically standard mod)
inline void DIV_MOD(T num,T den,T &div, T &mod) // does NOT check if (den == 0)
	{
	div = num / den;
	mod = num - div * den; 
	}

// Copy an array of objects (just like strcpy() but it works for any type).
// Object must have a default or overloaded operator= for assignment.
template<class T>
inline void rspObjCpy(T* pDst, const T* pSrc, size_t count)
	{
	if (count > 0)
		{
		while (count--)
			*pDst++ = *pSrc++;
		}
	}

// Compare an array of objects (just like strcmp() but it works for any type).
// Object must have a default or overloaded operator== to test for equality.
template<class T>
inline bool rspObjCmp(const T* p1, const T* p2, size_t count)
	{
	if (count > 0)
		{
		// Need to use '==' because that's what we're asking objects to overload!
		// (Hence the odd-looking notation of negating the result of the '=='.)
		while (count--)
			{
			if (!(*p1++ == *p2++))
				return false;
			}
		return true;
		}
	return true;
	}


inline char *ltoa(int32_t l, char *buf, int bufsize)
{
    snprintf(buf, bufsize, "%i", l);
    return(buf);
}

inline char *ltoa(uint32_t l, char *buf, int bufsize)
{
    snprintf(buf, bufsize, "%i", l);
    return(buf);
}

inline char *itoa(int l, char *buf, int bufsize)
{
    snprintf(buf, bufsize, "%d", l);
    return(buf);
}

#define strnicmp(x, y, z) strncasecmp(x, y, z)
#define _ltoa(x, y, z) ltoa(x, y, z)

#endif // UNIXSYSTEM_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
