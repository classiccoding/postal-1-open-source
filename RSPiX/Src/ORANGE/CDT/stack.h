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
// stack.h
// 
// History:
//		08/14/95 JMI	Started.
//
//		07/08/96	JMI	No longer automatically makes user type a pointer.
//
//		07/08/96	JMI	Converted to new CList that does not convert your 
//							template type into a pointer.
//
//		07/09/96	JMI	Reconverted to newest CList which does convert
//							your template type into a pointer.  Now it CList
//							is based on CBList which does not do this conversion.
//
//		07/31/96	JMI	ReReconverted to CBList which does NOT convert
//							your template type into a pointer.
//
//		08/01/96 MJR	Bug fixed in Pop() - returned incorrect error code.
//
//		10/30/96	JMI	Changed:
//							Old label:		New label:
//							=========		=========
//							CBList			RBList
//							CStack			RStack
//
//////////////////////////////////////////////////////////////////////////////
//
// This module provides dynamic stack services.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef H_STACK
#define H_STACK

#include <BLUE/System.h>

#include "listbase.h"

//////////////////////////////////////////////////////////////////////////////
// Typedefs.
//////////////////////////////////////////////////////////////////////////////

template <class T> class RStack
	{
	protected:	// Internal types.
		typedef T STACKDATA, *PSTACKDATA; 

	public:	// Construction/Destruction.
		// Default constructor.
		RStack()		{ m_sNumItems = 0; }
		// Destructor.
		~RStack()	{ }

//////////////////////////////////////////////////////////////////////////////

	public:	// Methods.
		// Push an item onto the stack.
		// Returns 0 on success.
		int16_t Push(STACKDATA sd)
			{
			int16_t sResult	= m_list.InsertHead(sd);
			if (sResult == SUCCESS)
				{
				m_sNumItems++;
				}
			return sResult;
			}

		// Pop an item off the stack.
      // Returns item on success; nullptr otherwise.
		int16_t Pop(PSTACKDATA psd)
			{
			int16_t sResult	= m_list.GetHead(psd);
			if (sResult == SUCCESS)
				{
				m_sNumItems--;
				m_list.Remove();
				}
			return sResult;
			}

//////////////////////////////////////////////////////////////////////////////

	public:	// Querries.
		// Returns the number of items currently in stack.
		int16_t GetNumItems(void)
			{ return m_sNumItems; }

		// Returns the top item.
		int16_t GetTop(PSTACKDATA psd)
			{
			return m_list.GetHead(psd);
			}

//////////////////////////////////////////////////////////////////////////////

	protected:	// Internal functions.

//////////////////////////////////////////////////////////////////////////////

	public:	// Member variables.

	protected:	// Protected member variables.
		RBList <T>	m_list;	// The dynamic stack.  Head is top.
		int16_t			m_sNumItems;	// Number of items in stack.
	};



#endif // H_STACK

//////////////////////////////////////////////////////////////////////////////
// EOF
//////////////////////////////////////////////////////////////////////////////
