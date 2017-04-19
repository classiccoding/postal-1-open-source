# doublepy
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

import subprocess, sys, distutils.spawn, atexit, socket, fcntl, os

# Use RPyC for advanced capabilities.
try:
	import rpyc
	have_rpyc = True
except:
	have_rpyc = False

if have_rpyc:
	# Find the RPyC Classic Server
	# FIXME: Add more names. This might be Arch-specific
	rpyc_location = None
	if sys.version_info[0] == 2:
		rpyc_location = distutils.spawn.find_executable("rpyc_classic2.py")
	if not rpyc_location:
		rpyc_location = distutils.spawn.find_executable("rpyc_classic.py")
	if not rpyc_location:
		have_rpyc = False
	rpyc_classic = "import sys\nsys.argv = ['{0}']\n".format(rpyc_location)
	rpyc_classic += open(rpyc_location).read()

# Put escape characters into a string so it can be placed into a
# subscript.
def dblEscape(string):
	return string.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n")

# Invoke a brand new instance of Python to run a script passed as a
# string. Returns anything put on stdout.
def runSubscript(script, interpreter = sys.executable):
	process = subprocess.Popen([interpreter, "-c", script], stdout=subprocess.PIPE)
	(output, err) = process.communicate()
	exitCode = process.wait()
	return output

# Return when a string is read from a pipe
def waitfor(pipe, string):
	fcntl.fcntl(pipe, fcntl.F_SETFL, fcntl.fcntl(pipe, fcntl.F_GETFL) | os.O_NONBLOCK)
	while True:
		try:
			out = pipe.readline().decode()
		except IOError:
			out = ""
		if string in out:
			break

if have_rpyc:
	processes = {}
	
	# Find an open port. Taken from StackOverflow. FIXME
	# This is bad. There should be a way to do this from within the
	# server application and then use the opened socket immediately.
	def getOpenPort():
		s = socket.socket()
		s.bind(("", 0))
		return s.getsockname()[1]
	
	# Run RPyC using a passed command. Returns a connection to the
	# server.
	def runRPyC(cmd):
		myPort = getOpenPort()
		cmd.append(rpyc_classic.replace("= self.port", "= {0}".format(myPort)))
		processes[myPort] = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		atexit.register(processes[myPort].terminate)
		waitfor(processes[myPort].stderr, "INFO:")
		return rpyc.classic.connect("localhost", port = myPort)