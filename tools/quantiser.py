#!/usr/bin/env python2
# quantiser
# Copyright 2017 Declan Hoare
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of version 2 of the GNU General Public License as published by
# the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
# Indexes a collection of images to the same 96-colour palette.

import sys, distutils.spawn, subprocess

script = """
argv = {0}

image = pdb.gimp_file_load(argv[1], argv[1])
for fname in argv[2:]:
	image.add_layer(pdb.gimp_file_load_layer(image, fname))

pdb.gimp_image_convert_indexed(image, 0, 0, 96, False, False, "")

for fname, layer in zip(reversed(argv[1:]), image.layers):
	image_copy = image.duplicate()
	image_copy.resize(layer.width, layer.height)
	pdb.gimp_file_save(image_copy, layer, fname, fname)

pdb.gimp_quit(True)
""".format(sys.argv)

gimp_path = distutils.spawn.find_executable("gimp")
if not gimp_path:
	sys.stderr.write("Could not find GIMP.\n")
	sys.exit(1)

proc = subprocess.Popen([gimp_path, "-i", "--batch-interpreter=python-fu-eval", "-b", script], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
proc.wait()
sys.exit(proc.returncode)
