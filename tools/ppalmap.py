#!/usr/bin/env python2
# ppalmap
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
# Add a PPAL into the palette of an image file.

import PIL.Image, sys, struct

# Split a list into segments of equal size.
def splitList(l, n):
	return [l[i:i + n] for i in range(0, len(l), n)]

if len(sys.argv) != 4:
	sys.stderr.write("Usage: " + sys.argv[0] + " INFILE PPAL OUTFILE\n")
	sys.exit(1)

image = PIL.Image.open(sys.argv[1])
ppal = open(sys.argv[2])

if image.mode != "P":
	sys.stderr.write(sys.argv[1] + " is not paletted\n")
	sys.exit(1)

if ppal.read(4) != "PPAL":
	sys.stderr.write(sys.argv[2] + " is not a PPAL file\n")
	sys.exit(1)

count = struct.unpack("<I", ppal.read(4))[0]
entryStruct = struct.Struct("<BBBB")
entries = []
for x in range(0, count):
	entries.append(list(entryStruct.unpack(ppal.read(4))))

freeZone = 256 - len(entries)

if freeZone == 0:
	sys.stderr.write("there is no free space in this PPAL\n")
	sys.exit(1)

print("retaining {0} entries".format(freeZone))

imagepal = splitList(image.getpalette(), 3)[:freeZone]

freeStart = 0

while freeStart == entries[freeStart][0]:
	freeStart += 1

if entries[freeStart][0] - entries[freeStart - 1][0] < freeZone:
	# I'm lazy.
	sys.stderr.write("this PPAL is fragmented, this is not currently supported\n")
	sys.exit(1)

entries = [x[1:] for x in entries] # pop index information

newpal = entries[:freeStart] + imagepal + entries[freeStart:]

newpal = sum(newpal, []) # flatten the list

t = image.info.get("transparency", None)
newdata = [0 if x == t else x + freeStart for x in image.getdata()]

image.putpalette(newpal)
image.putdata(newdata)

image.save(sys.argv[3])
