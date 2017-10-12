/*
	ameliorate
	Copyright 2017 Declan Hoare

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License along
	with this program; if not, write to the Free Software Foundation, Inc.,
	51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
	
	Header file for ameliorate.c
*/

#ifndef AMELIORATE_H
#define AMELIORATE_H

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct box_s
{
	long x, y, w, h;
	char* data;
	struct box_s* next;
} box;

extern box* am_chop(char* indata, long width, long height);

#ifdef __cplusplus
}
#endif

#endif