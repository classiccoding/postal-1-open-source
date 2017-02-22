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
//////////////////////////////////////////////////////////////////////////////
//
// shapes.h
//
// 
// History:
//		12/08/95	BRH	Started.
//
//		11/01/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CPt				RPt
//							C2DPoint			R2DPoint
//							C3DPoint			R3DPoint
//							CRay				RRay
//							C2DRay			R2DRay
//							C3DRay			R3dRay
//							CRectangle		RRectangle
//							CCube				RCube
//							CCircle			RCircle
//							CSphere			RSphere
//
//		02/17/97	JMI	Removed ~RSphere() proto (had no body).
//
//		02/18/97	JMI	Added empty bodies to ~RRectangle(), ~RCube(), and 
//							~RCircle().
//							Also, added R2DLine and R3DLine.
//
//////////////////////////////////////////////////////////////////////////////
//
//	These shape classes were initially created to be used with the
// regions and in the games.  For most of the shapes there are 2D and 3D
// versions.  The one you choose would be based on the type of game being
// created.  
//
//////////////////////////////////////////////////////////////////////////////

#ifndef SHAPES_H
#define SHAPES_H

#include <cstdint>

struct RPt
{
  int32_t X;
  int32_t Y;

  RPt(int32_t lX, int32_t lY) : X(lX), Y(lY) { }
  RPt(void) : RPt(0, 0) { }
};

typedef RPt R2DPoint;

struct R3DPoint : R2DPoint
{
  int32_t Z;

  R3DPoint(void) : Z(0) { }
  R3DPoint(int32_t lX, int32_t lY, int32_t lZ)
    : R2DPoint (lX, lY), Z(lZ) { }
};


struct RRay
{
  int32_t X;        // X originating position
  int32_t Y;        // Y originating position
  float fXVect;     // X element of unit vector
  float fYVect;     // Y element of unit vector

  RRay(int32_t lX, int32_t lY, float fXunit, float fYunit)
    : X(lX), Y(lY), fXVect(fXunit), fYVect(fYunit) { }
  RRay(void) : RRay(0, 0, 0.0, 0.0) { }
};


struct R2DRay : RRay
{
  R2DRay(void) : RRay() { }
  R2DRay(int32_t lX, int32_t lY, float fXunit, float fYunit)
    : RRay(lX, lY, fXunit, fYunit) { }
  R2DRay(int32_t lX1, int32_t lY1, int32_t lX2, int32_t lY2);
};

struct R3DRay : R2DRay
{
  int32_t Z;        // Z originating position
  float fZVect;     // Z element of unit vector

  R3DRay(void) : Z(0), fZVect(0.0) { }
  R3DRay(int32_t lX, int32_t lY, int32_t lZ, float fXunit, float fYunit, float fZunit)
    : R2DRay(lX, lY, fXunit, fYunit), Z(lZ), fZVect(fZunit) { }

  R3DRay(int32_t lX1, int32_t lY1, int32_t lZ1, int32_t lX2, int32_t lY2, int32_t lZ2);
};

struct R2DLine
{
  int32_t X1;       // X originating position
  int32_t Y1;       // Y originating position
  int32_t X2;       // X ending position
  int32_t Y2;       // Y ending position

  R2DLine(void) : X1(0), Y1(0), X2(0), Y2(0) { }
};

struct R3DLine : R2DLine
{
  int32_t Z1;       // Z originating position
  int32_t Z2;       // Z ending position

  R3DLine(void) : Z1(0), Z2(0) { }
};

struct RRectangle
{
  int32_t lLeft;    // Left side
  int32_t lRight;   // Right side
  int32_t lTop;     // Top side
  int32_t lBottom;  // Bottom side

  RRectangle(int32_t lL, int32_t lR, int32_t lT, int32_t lB)
    : lLeft(lL), lRight(lR), lTop(lT), lBottom(lB) { }

  RRectangle(void) : RRectangle(0, 0, 0, 0) { }
};

struct RCube : RRectangle
{
  int32_t lFront;   // Front side
  int32_t lBack;    // Back side

  RCube(void) : lFront(0), lBack(0) { }
  RCube(int32_t lL, int32_t lR, int32_t lT, int32_t lB, int32_t lF, int32_t lBk)
    : RRectangle(lL, lR, lT, lB), lFront(lF), lBack(lBk) { }
};

struct RCircle
{
  int32_t X;			// X coordinate of center point
  int32_t Y;			// Y coordinate of center point
  int32_t lRadius;	// Radius

  RCircle(int32_t lX, int32_t lY, int32_t lR)
    : X(lX), Y(lY), lRadius(lR) { }
  RCircle(void) : RCircle(0, 0, 0) { }
};

struct RSphere : RCircle
{
  int32_t Z;			// Z coordinate of center point

  RSphere(void) : Z(0) { }
  RSphere(int32_t lX, int32_t lY,  int32_t lZ, int32_t lR)
    : RCircle(lX, lY, lR), Z(lZ) { }
};


#endif //SHAPES_H

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
