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
#ifndef PIXEL_H
#define PIXEL_H


class CPixel
	{
	public:
		CPixel()
			{ Init(); }
		CPixel(int16_t sBitsPerPixel)
			{ ASSERT(sBitsPerPixel % 8 == 0); m_sSize = sBitsPerPixel / 8; Init(); }

		void Init(void)
			{ for (int16_t i = 0; i < sizeof(m_au8); i++) m_au8[i] = 0; }

		uint8_t GetU8Val(void)
			{ return m_au8[0]; }
		uint16_t GetU16Val(void)
			{ return *((uint16_t*)m_au8); }
		uint32_t GetU24Val(void)
			{ return *((uint32_t*)m_au8); }
		uint32_t GetU32Val(void)
			{ return *((uint32_t*)m_au8); }

		void SetVal(uint8_t val)
			{ memcpy(m_au8, &val, sizeof(val)); }
		void SetVal(uint16_t val)
			{ memcpy(m_au8, &val, sizeof(val)); }
		void SetVal24(uint32_t val)
			{ memcpy(m_au8, &val, 3); }
		void SetVal(uint32_t val)
			{ memcpy(m_au8, &val, sizeof(val)); }

		void SetVal(uint8_t* puc)
			{ memcpy(m_au8, puc, m_sSize); }

		int operator <(CPixel &pixel)
			{ return (GetU32Val() < pixel.GetU32Val()); }

		int operator >(CPixel &pixel)
			{ return (GetU32Val() > pixel.GetU32Val()); }
		
		int operator ==(CPixel &pixel)
			{
			if (memcmp(m_au8, pixel.m_au8, MIN(m_sSize, pixel.m_sSize)) == 0)
				return TRUE;
			else
				return FALSE;
			}

	public:
		uint8_t		m_au8[sizeof(uint32_t)];
		int16_t	m_sSize;
	};

#endif	// PIXEL_H
///////////////////////////////////////////////////////////////////////////////
// EOF
///////////////////////////////////////////////////////////////////////////////
