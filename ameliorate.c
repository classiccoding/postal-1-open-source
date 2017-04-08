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
	
	Supplemental C library for PSAK. Hopefully gives a speed boost to
	functions that are too slow in pure Python.
*/

#include <stdlib.h>
#include <stdbool.h>
#include "ameliorate.h"

#define GET_PIXEL(x, y) ((y) * width + x)
#define DEF_CHUNK_SIZE 64

box* am_chop(char* indata, long width, long height)
{
	box* start = NULL;					/* Return value */
	box* box1 = NULL;					/* Storage for finished box */
	box* box2 = NULL;					/* Storage for current box */
	char* map = malloc(width * height);	/* Mask of already split data */
	memset(map, 0, width * height);
	
	for (long y = 0; y < height; y++)
	{
		for (long x = 0; x < width; x++)
		{
			if (indata[GET_PIXEL(x, y)] && !map[GET_PIXEL(x, y)])
			{
				/* Initialise a box. */
				box2 = malloc(sizeof(box));
				box2->next = NULL;
				box2->w = box2->h = DEF_CHUNK_SIZE;
				box2->x = x;
				box2->y = y;
				
				/* Shrink the box against the edges of the image. */
				if (box2->w > width - x) box2->w = width - x;
				if (box2->h > height - y) box2->h = height - y;
				
				/* Shrink the box against existing boxes. */
				for (long myy = y + box2->h - 1; myy >= y; myy--)
				{
					for (long myx = x; myx < x + box2->w; myx++)
					{
						if (map[GET_PIXEL(myx, myy)])
						{
							box2->h--;
							break;
						}
					}
				}
				
				for (long myx = x + box2->w - 1; myx >= x; myx--)
				{
					for (long myy = y; myy < y + box2->h; myy++)
					{
						if (map[GET_PIXEL(myx, myy)])
						{
							box2->w--;
							break;
						}
					}
				}
				
				/* Shrink the box against transparency. */
				bool edge;
				
				for (long myy = y + box2->h - 1; myy >= y; myy--)
				{
					edge = true;
					for (long myx = x; myx < x + box2->w; myx++)
					{
						if (indata[GET_PIXEL(myx, myy)])
						{
							edge = false;
							break;
						}
					}
					if (edge) box2->h--; else break;
				}
				
				for (long myx = x + box2->w - 1; myx >= x; myx--)
				{
					edge = true;
					for (long myy = y; myy < y + box2->h; myy++)
					{
						if (indata[GET_PIXEL(myx, myy)])
						{
							edge = false;
							break;
						}
					}
					if (edge) box2->w--; else break;
				}
				
				/* Expand the box, searching backwards. */
				for (long myy = y; myy >= 0; myy--)
				{
					edge = true;
					for (long myx = x; myx < x + box2->w; myx++)
					{
						if (indata[GET_PIXEL(myx, myy)])
						{
							edge = false;
							break;
						}
					}
					if (edge) break;
					box2->h++;
					box2->y--;
				}
				
				for (long myx = x; myx >= 0; myx--)
				{
					edge = true;
					for (long myy = box2->y; myy < box2->y + box2->h; myy++)
					{
						if (indata[GET_PIXEL(myx, myy)])
						{
							edge = false;
							break;
						}
					}
					if (edge) break;
					box2->w++;
					box2->x--;
				}
				
				/* Copy the data into the box and edit the map. */
				box2->data = malloc(box2->w * box2->h);
				for (long myy = 0; myy < box2->h; myy++)
				{
					memcpy(box2->data + myy * box2->w, indata + GET_PIXEL(box2->x, box2->y + myy), box2->w);
					memset(map + GET_PIXEL(box2->x, box2->y + myy), 1, box2->w);
				}
				
				/* Move the boxes along. */
				if (box1 == NULL) start = box1 = box2;
				else
				{
					box1->next = box2;
					box1 = box2;
				}
			}
		}
	}
	
	free(map);
	return start;
}