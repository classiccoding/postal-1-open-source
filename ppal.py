#!/usr/bin/env python2
# ppal
# Copyright 2017 Declan Hoare
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# 
# Scans a list of paletted image files and generates a PPAL (Partial
# PALette) file containing the common palette entries.
#
# PPAL files consist of a "PPAL" magic number header followed by a U32
# of the amount of entries in the file. (This should never exceed 256,
# but it is 32-bit so that the header will take up 8 bytes.) 
# Each entry consists of a U8 containing the index number of the colour
# in the full palette, followed by three more U8s for the rgb values. 
# The file is little endian.

import sys, PIL.Image, struct

# Split a list into tuples of equal size.
def splitList(l, n):
	return [tuple(l[i:i + n]) for i in range(0, len(l), n)]

if len(sys.argv) < 3:
	sys.stderr.write("Usage: " + sys.argv[0] + " IMAGE1 IMAGE2 [IMAGE3]...\n")
	sys.stderr.write("Note: This program prints binary data to stdout. Redirect it!\n")
	sys.exit(1)

firstPal = splitList(PIL.Image.open(sys.argv[1]).getpalette(), 3)
commonMap = [True] * 256
entries = []

for arg in sys.argv[2:]:
	thisPal = splitList(PIL.Image.open(arg).getpalette(), 3)
	for idx, val in enumerate(thisPal):
		if firstPal[idx] != val:
			commonMap[idx] = False

for idx, val in enumerate(commonMap):
	if val:
		entries.append((idx,) + firstPal[idx])

entryStruct = struct.Struct("<BBBB")

output = "PPAL"
output += struct.pack("<I", len(entries))
for (i, r, g, b) in entries:
	output += entryStruct.pack(i, r, g, b)

sys.stdout.write(output)