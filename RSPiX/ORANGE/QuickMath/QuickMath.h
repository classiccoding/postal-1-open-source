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
// This uses a class to pre-initialize the size/cosine tables.
// It is currently set to use only integer degrees as input.
#ifndef QUICK_MATH_H
#define QUICK_MATH_H

#include <BLUE/System.h>
#include <cmath>

#define rspPI         3.14159265359
#define rspRadToDeg   57.29577951308
#define rspDegToRad   0.01745329251994
#define rspSQRT2      1.414213562373
//#define rspSQRT3      1.732050807569

#define MAX_FAST_SQRT 0x20000 // Will take a 256KB of memory
/*****************************************************************
Quickmath contains several extansions, but FixedPoint.cpp is
required because it is called to initialize from the normal
quickmath initialization.  QuickMath is based upon the concept
of integer degrees, from 0-359 inclusive.  

  Hungarian notation: 
    & = implicit pass by reference, do NOT use a pointer
        & is used in almost all cases to prevent VC from creating
        a local stack frame which would slow things down by 20 times.
    T = a single templated type
    -64 = enhanced 64-bit math mode...

*****************************************************************
inline void rspMod360(&sDeg) // USE BEFORE ALL CALLS IF IN QUESTION!
inline void rspMod(&sValue,&sRange) 

inline void rspDivMod(T num,T den,T &div, T &mod) // symmetric (methematical)
inline void rspDivModA(T num,T den,T &div, T &mod) // asymmetric

short rspATan(sDeltaY,sDeltaX);
inline short rspDegDelta(sDegSrc,sDegDst)
short rspATan(dVal);
inline double rspSin(sDeg)
inline double rspCos(sDeg)
inline float rspfSin(sDeg)
inline float rspfCos(sDeg)

inline short rspSqrt(long lVal)

*****************************************************************

****************************************************************/
const int16_t csNumRotSteps = 360;
extern double SINQ[csNumRotSteps],COSQ[csNumRotSteps];
extern float fSINQ[csNumRotSteps],fCOSQ[csNumRotSteps];
extern int16_t ATANQ[60];
extern int16_t SQRTQ[MAX_FAST_SQRT];
/****************************************************************/
inline int16_t rspSqrt(int32_t lVal)
{
  if (lVal < MAX_FAST_SQRT)
    return SQRTQ[lVal];
  return int16_t(sqrt(double(lVal)));
}

inline double rspSin(int16_t sDeg) 
{
  ASSERT((sDeg >= 0) || (sDeg < 360));

  return SINQ[sDeg];
}

inline double rspCos(int16_t sDeg) 
{
  ASSERT((sDeg >= 0) || (sDeg < 360));

  return COSQ[sDeg];
}

inline float rspfSin(int16_t sDeg) 
{
  ASSERT((sDeg >= 0) || (sDeg < 360));

  return fSINQ[sDeg];
}

inline float rspfCos(int16_t sDeg) 
{
  ASSERT((sDeg >= 0) || (sDeg < 360));

  return fCOSQ[sDeg];
}

template<typename T>
int16_t rspATan(T sDeltaY,T sDeltaX)
{
  // absolute versions
  T sDelX = ABS(sDeltaX);
  T sDelY = ABS(sDeltaY);
  int16_t sDeg;

  if (sDelY <= sDelX)
  {
    if (sDelX)
      sDeg = ATANQ[int32_t(0.5 + (rspRadToDeg * sDelY) / sDelX)];
    else
      sDeg = 90;
  }
  else
  {
    if (sDelY)
      sDeg = 90 - ATANQ[int32_t(0.5 + (rspRadToDeg * sDelX) / sDelY)];
    else
      sDeg = 90;
  }

  // Keep in bounds
  if (sDeltaX < 0)
    sDeg = 180 - sDeg;
  if (sDeltaY < 0)
    sDeg = 360 - sDeg;
  if (sDeg == 360)
    sDeg = 0;

  return sDeg;
}

template<typename T>
int16_t rspATan(T dVal)
{
  // absolute versions
  T dAbsVal = ABS(dVal);
  int16_t sDeg;

  if (dAbsVal <= 1.0)
    sDeg = ATANQ[int32_t(0.5 + rspRadToDeg * dAbsVal)];
  else
    sDeg = 90 - ATANQ[int32_t(0.5 + rspRadToDeg  / dAbsVal)];

  // Keep in bounds
  if (dVal < 0.0)
    sDeg = 360 - sDeg;
  if (sDeg == 360)
    sDeg = 0;

  return sDeg;
}

inline double rspDegDelta(double sDegSrc, double sDegDst)
{
  double sDel = sDegDst - sDegSrc;

  if (sDel >= 0.0) // positive turn
  {
    if (sDel > 180.0)
      sDel -= 360.0; // neg turn
  }
  else if (sDel < -180.0) // negative turn
    sDel += 360.0; // pos turn

  return sDel;
}

// Use individual whiles if you know what to expect
template<typename T>
inline T rspMod360(T sDeg)
{
  while (sDeg < 0)
    sDeg += T(360);
  while (sDeg > 359)
    sDeg -= T(360);
  return sDeg;
}
#ifdef UNUSED_FUNCTIONS
inline int16_t rspMod(int16_t sVal, const int16_t sRange)
{
  while (sVal < 0)
    sVal += sRange;
  while (sVal >= sRange)
    sVal -= sRange;
  return sVal;
}

inline void rspMod360(int16_t* sDeg)
{
  *sDeg = rspMod360(*sDeg);
}

inline void rspMod(int16_t* sVal, const int16_t sRange)
{
  *sVal = rspMod(*sVal,sRange);
}
#endif

extern void InitTrig();

// Auto Initialize hook!
class RQuickTrig
{
public:
  RQuickTrig() { InitTrig(); }
  ~RQuickTrig() {  }
};

template <typename T> // aymmetric mod wrt sign, good for deltas
inline void rspDivMod(T num, T den, T& div, T& mod) // does NOT check if (den == 0)
{  // Algorithm not verified for negative denominator!!!!
  div = num / den;
  mod = num - div * den;
}

template <typename N, typename T> // asymmetric mod wrt sign, good for POSITION
inline void rspDivModA(N num, T den, T& div, T& mod) // does NOT check if (den == 0)
{  // Algorithm not verified for negative denominator!!!!
  div = num / den;
  mod = num - div * den;
#if 1
  if (mod && div < 0)
  {
    --div;
    mod += den;
  }
#else
  if (mod)
  {
    if (div < 0)
    {
      --div;
      mod += den;
    }
  }
  else if (!div)
  {
    if (mod < 0) // _always_ false
    {
      --div; // unreachable!
      mod += den;
    }
  }
#endif
}

//-------------------------------
#endif
