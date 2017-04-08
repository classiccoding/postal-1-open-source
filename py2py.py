# py2py
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

# The least Pythonic module in existence. This module is designed to
# help you get two instances of Python communicating.

import subprocess, sys

# Put escape characters into a string so it can be placed into a
# subscript.
def dblEscape(string):
	return string.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n")

# Invoke a brand new instance of Python to run a script passed as a
# string. Returns anything put on stdout.
def runSubscript(script):
	process = subprocess.Popen([sys.executable, "-c", script], stdout=subprocess.PIPE)
	(output, err) = process.communicate()
	exitCode = process.wait()
	return output