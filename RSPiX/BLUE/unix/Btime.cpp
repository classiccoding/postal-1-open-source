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
///////////////////////////////////////////////////////////////////////////////
//
//	btime.cpp
// 
// History:
//		06/03/04 RCG	Started.
//
//////////////////////////////////////////////////////////////////////////////
//
// Does all SDL specific time stuff.
//
//////////////////////////////////////////////////////////////////////////////

#include <SDL2/SDL.h>
#include <BLUE/unix/UnixBlue.h>

static microseconds_t MicrosecondsBase = 0;

//////////////////////////////////////////////////////////////////////////////
// Functions.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
// Initializes the time module.
//
//////////////////////////////////////////////////////////////////////////////
extern void Time_Init(void)
{
  MicrosecondsBase = rspGetAppMicroseconds();
}


//////////////////////////////////////////////////////////////////////////////
//
// Get the current Windows' time.
// Returns the time in a long.
//
//////////////////////////////////////////////////////////////////////////////
extern milliseconds_t rspGetMilliseconds(void)
{
  return SDL_GetTicks();
}

//////////////////////////////////////////////////////////////////////////////
//
// Get time since last rspGetMicroseconds(TRUE) call in microseconds.  May 
// not always be accurate to the nearest microsecond.  It is always on the
// Mac but possibly not in Windows; however, every machine tested produced
// good to excellent resolution.
// Returns the time in a long.
//
//////////////////////////////////////////////////////////////////////////////
extern microseconds_t rspGetMicroseconds(	// Returns microseconds between now and last
    int16_t sReset /*= FALSE*/)		// Set to TRUE to reset timer.  If you never reset the timer, it will wrap within just over 35 minutes.
{
  microseconds_t microsecs = rspGetAppMicroseconds();
  microseconds_t lTime = microsecs - MicrosecondsBase; // sorry, no time travel allowed

  // If reset requested . . .
  if (sReset != FALSE)
    MicrosecondsBase = microsecs;

  return lTime;
}

//////////////////////////////////////////////////////////////////////////////
//
// Get time since App started in microseconds.  This is safe for global use
// because it cannot be reset by anyone.  It requires 64-bit mathm however!
//
// May not always be accurate to the nearest microsecond.  It is always on the
// Mac but possibly not in Windows; however, every machine tested produced
// good to excellent resolution.
//
// Returns the time in an __int64.
//
//////////////////////////////////////////////////////////////////////////////
extern microseconds_t rspGetAppMicroseconds()
{
  return microseconds_t(SDL_GetTicks()) * 1000;
}

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
