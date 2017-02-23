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

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <climits>
#include <cstdint>

#include <map>

#include <CompileOptions.h>

////////////////////////////////////////////////////////////////////////////////
// Handy defines.
////////////////////////////////////////////////////////////////////////////////
#ifndef TRUE
# define TRUE     1
#endif

#ifndef FALSE
# define FALSE    0
#endif

#if 1
//!defined(STRICT) && !defined(PENDANT)

# ifndef SUCCESS
#  define SUCCESS  0
# endif

# ifndef FAILURE
#  define FAILURE -1
# endif

typedef const char* c_string;
typedef int errcode_t;

#else

#undef SUCCESS
#undef FAILURE

enum errcode : int
{
  FAILURE = -1,
  SUCCESS,
};

constexpr errcode operator * (errcode c, int mult)
  { return errcode(reinterpret_cast<int>(c) * mult); }

struct errcode_t
{
  int code;
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

#endif

////////////////////////////////////////////////////////////////////////////////
// types, type limits, standard functions and endian detection
////////////////////////////////////////////////////////////////////////////////

#include <BLUE/portable_endian.h>

#if defined(WIN32)
# pragma message("I find your lack of POSIX disturbing. ;)")
# include <BaseTsd.h>
typedef SSIZE_T ssize_t;
# if !defined(_MSC_VER) || _MSC_VER < 1900
#  define constexpr inline
#  if !defined(snprintf)
#   define snprintf _snprintf
#  endif
#  if !defined(vsnprintf)
#   define vsnprintf _vsnprintf
#  endif
#  if !defined(mkdir)
#   define mkdir _mkdir
#  endif
#  if !defined(PATH_MAX)
#   define PATH_MAX _MAX_PATH
#  endif
# endif
# if !defined(strcasecmp)
#  define strcasecmp _stricmp
# endif
# define NOTE(x) __pragma(message("NOTE: " x))
# include <BLUE/stdint_msvc.h>
# define SYSTEM_PATH_SEPARATOR	'\\'
#else
# include <sys/types.h>
# if defined __GNUC__ && defined __GNUC_MINOR__
#  define __GNUC_PREREQ(maj, min) ((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
# else
#  define __GNUC_PREREQ(maj, min) 0
# endif
# define DO_PRAGMA(x) _Pragma (#x)
# if !defined(__GNUC__) || __GNUC_PREREQ(4,4)
#  define NOTE(x) DO_PRAGMA(message("NOTE: " x))
# else
#  define NOTE(x) DO_PRAGMA(warning("NOTE: " x)
# endif
# if defined(__DOS__)
#  if defined(__STRICT_ANSI__) && defined(__DJGPP__)
#   error You need to disable C++ standards complaince for DJGPP
#  endif
#  include <limits.h> // somehow not referenced by climits
#  define SYSTEM_PATH_SEPARATOR	'\\'
# else
#  define SYSTEM_PATH_SEPARATOR	'/'
# endif
#endif

// non standard defines
#if !defined(F_OK)
# define F_OK 00
#endif

#if !defined(R_OK)
# define R_OK 04
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



typedef uint32_t milliseconds_t;
typedef uint64_t microseconds_t;

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
constexpr T ABS2(T x,T y) { return SQR(x) + SQR(y); }

template <class T> // returns the square of the absolute value
constexpr T ABS2(T x,T y,T z) { return SQR(x) + SQR(y) + SQR(z); }

#undef SGN // returns a binary sign (+1 or -1)
template <class T>
constexpr T SGN(T x) { return (x < 0) ? (T)-1 : (T)1; }

#undef SGN3 // returns a trinary sign (+1, 0, or -1)
template <class T>
constexpr T SGN3(T x) { return (x == 0) ? (T)0 : ((x < 0) ? (T)-1 : (T)1); }


////////////////////////////////////////////////////////////////////////////////
// Debug API
//
// Define the TRACE, STRACE, and ASSERT macros.  If _DEBUG or TRACENASSERT are
// defined, then these macros are usefull debugging aids.  Otherwise, it is
// assumed that the program is being compiled in "release" mode, and all three
// of the macros are changed such that no code nor data results from their
// use, thereby eliminating all traces of them from the program.
//
// TRACE is like printf, but sends the output to the debug window.  Note that
// it slips in the file and line number information before printing whatever
// the user requested.
//
// STRACE is like TRACE, except that it doesn't display the file and line
// number information.
//
// ASSERT is used to assert that an expression is true.  If it is, in fact,
// true, then ASSERT does nothing.  If not, then it emits a signal and halts
// execution.
//
////////////////////////////////////////////////////////////////////////////////

extern void rspTrace(const char* szFrmt, ...);

#if defined(_DEBUG) || defined(TRACENASSERT)
// TRACE macro, the preferred method of sending output to debug window
# define STRACE(...)  rspTrace(__VA_ARGS__)
# define TRACE(...)   STRACE("%s(%d):", __FILE__, __LINE__),STRACE(__VA_ARGS__)
# include <cassert>
# define ASSERT(...)  assert(__VA_ARGS__)
#else
# define STRACE(...)  rspTrace(__VA_ARGS__)
# define TRACE(...)   STRACE(__VA_ARGS__)
# define ASSERT(...)
#endif

#endif // UNIXSYSTEM_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
