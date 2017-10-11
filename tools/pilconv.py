#!/usr/bin/env python2
# pilconv
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
# Saves argument 1 to argument 2 using PIL, converting it if you change
# the file extension. This only really exists so that I don't need to
# depend on ImageMagick as well as PIL.

import PIL.Image, sys
PIL.Image.open(sys.argv[1]).save(sys.argv[2])

