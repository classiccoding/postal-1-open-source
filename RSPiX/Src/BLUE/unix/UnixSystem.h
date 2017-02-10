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

#include <climits>
#include <cstdint>

#include <map>
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// For compilers that support partial paths in #include's, define this.  For
// those that don't, don't define it.
////////////////////////////////////////////////////////////////////////////////
#define PATHS_IN_INCLUDES

////////////////////////////////////////////////////////////////////////////////
// Handy defines.
////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
# define TRUE     1
#endif

#ifndef FALSE
# define FALSE    0
#endif

#if !defined(STRICT) && !defined(PENDANT)
#ifndef SUCCESS
# define SUCCESS  0
#endif

#ifndef FAILURE
# define FAILURE -1
#endif

#else

#undef SUCCESS
#undef FAILURE

enum errcode : error_t
{
  FAILURE = -1,
  SUCCESS,
};

constexpr errcode operator * (errcode c, error_t mult)
  { return errcode(error_t(c) * mult); }

static_assert(errcode::SUCCESS == 0, "enumeration error");

struct errcode_t
{
  error_t code;
#if defined(PENDANT)
  constexpr errcode_t& operator =(errcode c)
  {
    code = c;
    return *this;
  }

  constexpr errcode_t& operator =(errcode_t c)
  {
    code = c.code;
    return *this;
  }
#else
  template<typename T>
  constexpr errcode_t& operator =(T c)
  {
    static_assert(sizeof(T) <= sizeof(errcode_t), "not large enough");
    code = c;
    return *this;
  }
#endif
  template<typename T>
  constexpr operator T(void)
  {
    static_assert(sizeof(T) >= sizeof(errcode_t), "data would truncate");
    return reinterpret_cast<T>(code);
  }
};
#endif

////////////////////////////////////////////////////////////////////////////////
// Standard types, type limits, standard functions and endian detection
////////////////////////////////////////////////////////////////////////////////

#include <BLUE/portable_endian.h>

#if defined(__WINDOWS__)
# pragma message("I find your lack of POSIX disturbing. ;)")
# include <BaseTsd.h>
typedef SSIZE_T ssize_t;

# if !defined(snprintf)
#  define snprintf _snprintf
# endif
# if !defined(vsnprintf)
#  define vsnprintf _vsnprintf
# endif
# if !defined(strcasecmp)
#  define strcasecmp _stricmp
# endif

# define NOTE(x) __pragma(message("NOTE: " x))

#include <BLUE/stdint_msvc.h>
#else
# include <sys/types.h>
#define DO_PRAGMA(x) _Pragma (#x)
# if !defined(__GNUC__) || __GNUC_PREREQ(4,4)
#  define NOTE(x) DO_PRAGMA(message("NOTE: " x))
# else
#  define NOTE(x) DO_PRAGMA(warning("NOTE: " x)
# endif
#endif

static_assert(sizeof(uintptr_t) == sizeof(void*), "your compiler is broken!");

#if CHAR_BIT != 8
# error "unsupported char size"
#endif

/* Minimum of unsigned integral types.  */
# define UINT8_MIN 0
# define UINT16_MIN 0
# define UINT32_MIN 0
# define UINT64_MIN 0

// 128-bit got a little trickier...
#if BYTE_ORDER == LITTLE_ENDIAN
typedef struct {	uint64_t	lo;
                  int64_t	hi;} int128_t;
typedef struct {	uint64_t	lo;
                  uint64_t	hi;} uint128_t;
#elif BYTE_ORDER == BIG_ENDIAN
typedef struct {	int64_t	hi;
                  uint64_t	lo;} int128_t;
typedef struct {	uint64_t	hi;
                  uint64_t	lo;} uint128_t;
#elif BYTE_ORDER == PDP_ENDIAN
# error Middle-endian is not implemented.
#elif defined(BYTE_ORDER)
# error Unknown endian format detected!
#else
# error The endianness of your machine could not be detected!
#endif

////////////////////////////////////////////////////////////////////////////////
// Macros to avoid warnings
////////////////////////////////////////////////////////////////////////////////

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

struct c_string
{
  const char* data;

  template<typename T>
  constexpr c_string& operator =(T other)
  {
    static_assert(sizeof(c_string) != sizeof(T), "did you mean to use nullptr?");
    data = other;
    return *this;
  }

  constexpr operator const char*(void)
  {
    return data;
  }
};

////////////////////////////////////////////////////////////////////////////////
// Pixel types
////////////////////////////////////////////////////////////////////////////////

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

constexpr bool operator==(const RPixel24& lhs, const RPixel24& rhs)
  { return (lhs.u8Blue == rhs.u8Blue) && (lhs.u8Green == rhs.u8Green) && (lhs.u8Red == rhs.u8Red); }
constexpr bool operator==(const RPixel32& lhs, const RPixel32& rhs)
  { return (lhs.u8Blue == rhs.u8Blue) && (lhs.u8Green == rhs.u8Green) && (lhs.u8Red == rhs.u8Red) && (lhs.u8Alpha == rhs.u8Alpha); }


////////////////////////////////////////////////////////////////////////////////
// Usefull Templates
////////////////////////////////////////////////////////////////////////////////
#undef MIN
template <class T>
constexpr T MIN(T a,T b) { return (a < b) ? a : b; }

#undef MAX
template <class T>
constexpr T MAX(T a,T b) { return (a > b) ? a : b; }

#undef SWAP	// Swaps two identical typed variables
template <class T>
constexpr void SWAP(T &a,T &b) { T temp = a; a = b; b = temp; }

#undef SQR // squares a number
template <class T>
constexpr T SQR(T x) { return x * x; }

#undef ABS // returns the absolute value of a parameter
template <class T>
constexpr T ABS(T x) { return (x < 0) ? -x : x; }

template <class T> // returns the square of the absolute value
constexpr T ABS2(T x,T y) { return SQR(x)+SQR(y); }

template <class T> // returns the square of the absolute value
constexpr T ABS2(T x,T y,T z) { return SQR(x) + SQR(y) + SQR(z); }

#undef SGN // returns a binary sign (+1 or -1)
template <class T>
constexpr T SGN(T x) { return (x < 0) ? (T)-1 : (T)1; }

#undef SGN3 // returns a trinary sign (+1, 0, or -1)
template <class T>
constexpr T SGN3(T x) { return (x == 0) ? (T)0 : ((x < 0) ? (T)-1 : (T)1); }

template <class T> // attaches a sign to a value
constexpr T ADD_SGN(T sign,T val) { return (sign < 0) ? -val : val; }

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
