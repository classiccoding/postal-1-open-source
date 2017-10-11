# pygimp
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

# Use GIMP from Python, not the other way around.

import doublepy, distutils.spawn

gimp_path = distutils.spawn.find_executable("gimp")

if not gimp_path:
	raise ImportError("GIMP could not be found.")

if not doublepy.have_rpyc:
	raise ImportError("RPyC is required by pygimp.")

conn = doublepy.runRPyC([gimp_path, "-i", "--batch-interpreter=python-fu-eval", "-b"])

# Copy remote GIMP module
for obj in dir(conn.modules.gimp):
	if not obj.startswith("__"):
		vars()[obj] = eval("conn.modules.gimp." + obj)