#!/usr/bin/env python2
# listppal
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
# Test program to list the contents of a PPAL.
# See ppal.py for an English description of the structure.

import sys, struct

ppal = open(sys.argv[1])

if ppal.read(4) != "PPAL":
	sys.stderr.write("this is not a PPAL file\n")
	sys.exit(1)

print("this is a PPAL file")
count = struct.unpack("<I", ppal.read(4))[0]
print("there are {0} entries in this file".format(count))
entryStruct = struct.Struct("<BBBB")
for x in range(0, count):
	entry = entryStruct.unpack(ppal.read(4))
	print(entry)