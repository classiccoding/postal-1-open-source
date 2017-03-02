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
#include <cmath>
#include "QuickMath.h"
#include "FixedPoint.h"

double SINQ[csNumRotSteps], COSQ[csNumRotSteps];
float fSINQ[csNumRotSteps], fCOSQ[csNumRotSteps];
int16_t ATANQ[60];
int16_t	SQRTQ[MAX_FAST_SQRT];

void InitTrig()
{
  int16_t i;
  double rad;

  const double cdStepsToRad = rspPI * 2.0 / double(csNumRotSteps);

  for (i = 0; i < csNumRotSteps; ++i)
  {
    rad = (double)i * cdStepsToRad;
    SINQ[i] = (double)sin(rad);
    COSQ[i] = (double)cos(rad);
    fSINQ[i] = (float) SINQ[i];
    fCOSQ[i] = (float) COSQ[i];
  }

  // Set up Arctan 45 degrees:
  for (i = 0; i < 58; ++i)
  {
    ATANQ[i] = int16_t(0.5 + atan(rspDegToRad * i) * rspRadToDeg);
  }

  for(int32_t l = 0; l < MAX_FAST_SQRT; ++l)
  {
    SQRTQ[l] = int16_t(sqrt(double(l)));
  }

  TRACE("QTRIG initialized!\n");
  RQuickTrigFP	dummy;
}
