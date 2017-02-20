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
// TriggerRgn.H
// Project: Nostril (aka Postal)
// 
// History:
//		05/01/97 JMI	Started.
//
//		05/01/97	JMI	Added Create(), Destroy(), SetMode(), and Macros and
//							Mode enum.
//
//		05/02/97	JMI	SetMode() was using the return type from RImage::Convert
//							wrong.
//							Also, added use of SetConvertFromFSPR1() which allows
//							us to control what colors the FSPR1 takes on when 
//							converted to another format.
//
//		05/09/97	JMI	Added u16InstanceId so the multigrid for pylon triggers
//							can contain a mapping from pylon ID to instance ID.
//							Load() and Save() read and write this value.
//
//////////////////////////////////////////////////////////////////////////////
//
// This structure defines a trigger region which consists, basically, of an
// image, the 'set' pixels of which represent the region, and a location for
// the image in the greater universe (a realm in this case).
//
//////////////////////////////////////////////////////////////////////////////

#ifndef TRIGGERRGN_H
#define TRIGGERRGN_H

//////////////////////////////////////////////////////////////////////////////
// C Headers -- Must be included before RSPiX.h b/c RSPiX utilizes SHMalloc.
//////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// RSPiX Headers.
///////////////////////////////////////////////////////////////////////////////

#include "RSPiX.h"

//////////////////////////////////////////////////////////////////////////////
// Postal headers.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Macros.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Protos.
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////
struct TriggerRgn
	{
	///////////////////////////////////////////////////////////////////////////
	// Typedefs/enums.
	///////////////////////////////////////////////////////////////////////////

	enum	// Macros.
		{
		MaxRgnWidth		= 300,
		MaxRgnHeight	= 300
		};

	typedef enum
		{
		Edit,		// Indicates edittable mode for modifying region.
		Storage	// Indicates storage mode for memory conservation.
		} Mode;

	///////////////////////////////////////////////////////////////////////////
	// Instantiable data.
	///////////////////////////////////////////////////////////////////////////
	int16_t		sX;
	int16_t		sY;
	RImage*	pimRgn;
	uint16_t		u16InstanceId;

	///////////////////////////////////////////////////////////////////////////
	// Constructor.
	///////////////////////////////////////////////////////////////////////////
	TriggerRgn()
		{
		pimRgn	= nullptr;
		}

	///////////////////////////////////////////////////////////////////////////
	// Destructor.
	///////////////////////////////////////////////////////////////////////////
	~TriggerRgn()
		{
		Destroy();
		}

	///////////////////////////////////////////////////////////////////////////
	// Destroy.
	///////////////////////////////////////////////////////////////////////////
	void Destroy(void)
		{
		delete pimRgn;
		pimRgn	= nullptr;
		}

	///////////////////////////////////////////////////////////////////////////
	// Create.
	///////////////////////////////////////////////////////////////////////////
	int16_t Create(		// Returns 0 on success.
		int16_t	sWidth,	// In:  Max width of region (width of image).
		int16_t	sHeight)	// In:  Max height of region (height of image).
		{
		int16_t sResult = SUCCESS;	// Assume success.

		Destroy();

		pimRgn	= new RImage;
		if (pimRgn != nullptr)
			{
			sResult	= pimRgn->CreateImage(	// Returns 0 if successful.
				sWidth,							// Width of new buffer.
				sHeight,							// Height of new buffer.
				RImage::BMP8);					// Type of new buffer.

			// If any errors occurred after allocation . . .
			if (sResult != SUCCESS)
				{
				Destroy();
				}
			}
		else
			{
			TRACE("Create(): Failed to allocate new RImage.\n");
			sResult = FAILURE;
			}

		return sResult;
		}

	///////////////////////////////////////////////////////////////////////////
	// Set current mode { Edit or Storage }.
	// Edit mode is the 8 bit mode used when editing the region.
	// Storage is whatever compressed mode is used for the sake of saving mem.
	///////////////////////////////////////////////////////////////////////////
	int16_t SetMode(
		Mode mode)	// In:  New mode { Edit, Storage }.
		{
		int16_t sResult = SUCCESS;	// Assume success.

		// If we have no image . . .
		if (pimRgn == nullptr)
			{
			sResult	= Create(MaxRgnWidth, MaxRgnHeight);
			}

		// If successful so far . . .
		if (sResult == SUCCESS)
			{
			switch (mode)
				{
				case Edit:
					// This allows us to choose the colors used when coming out
					// of the FSPR1 format.
					SetConvertFromFSPR1
						(
						250,	// u32ForeColor,				// Make it this color
						TRUE,	// sTransparent = TRUE,		// 1 or 2 color?
						0		//	u32BackColor = (uint32_t)0	// matters only if sTransparent = FALSE
						);

					if (pimRgn->Convert(RImage::BMP8) != RImage::BMP8)
						{
						sResult = FAILURE;
						}
					break;
				case Storage:
					if (pimRgn->Convert(RImage::FSPR1) != RImage::FSPR1)
						{
						sResult = FAILURE;
						}
					break;
				}
			}

		return sResult;
		}

	///////////////////////////////////////////////////////////////////////////
	// Load.
	///////////////////////////////////////////////////////////////////////////
	int16_t Load(			// Returns 0 on success.
		RFile* pfile)	// In:  File to load from.
		{
		int16_t sResult = SUCCESS;	// Assume success.

		Destroy();

		// Always a boolean indicating whether we exist . . .
		int16_t	sExist	= FALSE;	// Safety.
		if (pfile->Read(&sExist) == 1)
			{
			if (sExist != FALSE)
				{
				pimRgn	= new RImage;
				if (pimRgn != nullptr)
					{
					// Read position.
					pfile->Read(&sX);
					pfile->Read(&sY);
					// Read instance ID.
					pfile->Read(&u16InstanceId);
					// Load image.
					sResult	= pimRgn->Load(pfile);
					if (sResult == SUCCESS)
						{
						// Success.
						}
					else
						{
						TRACE("Load(): RImage::Load() failed.\n");
						delete pimRgn;
						pimRgn	= nullptr;
						}
					}
				else
					{
					TRACE("Load(): Failed to allocate new RImage.\n");
					sResult = FAILURE * 2;
					}
				}
			}
		else
			{
			TRACE("Load(): Failed to read existence flag.\n");
			sResult = FAILURE;
			}

		return sResult;
		}

	///////////////////////////////////////////////////////////////////////////
	// Save.
	///////////////////////////////////////////////////////////////////////////
	int16_t Save(			// Returns 0 on success.
		RFile* pfile)	// In:  File to save to.
		{
		int16_t sResult = SUCCESS;	// Assume success.

		// Always a boolean indicating whether we exist . . .
		int16_t	sExist	= (pimRgn != nullptr) ? TRUE : FALSE;
		if (pfile->Write(sExist) == 1)
			{
			if (pimRgn != nullptr)
				{
				// Write position.
				pfile->Write(sX);
				pfile->Write(sY);
				// Write instance ID.
				pfile->Write(&u16InstanceId);
				// Save image.
				sResult	= pimRgn->Save(pfile);
				if (sResult == SUCCESS)
					{
					// Success.
					}
				else
					{
					TRACE("Save(): RImage::Save() failed.\n");
					}
				}
			}
		else
			{
			TRACE("Save(): Failed to write existence flag.\n");
			sResult = FAILURE;
			}

		return sResult;
		}

	};

#endif	// TRIGGERRGN_H
//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
