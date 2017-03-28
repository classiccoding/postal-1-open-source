# wkinter
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

# An extended version of David Baird's threaded GTK/Webkit interface
# code. Features some horrifying extensions by me for subwindows.

import gtk, gobject, sys, subprocess, thread, threading, webkit, Queue

# DAVID BAIRD'S ORIGINAL FUNCTIONS (Although, modified a bit by me)

# Run a function on the GTK thread.
def asyncGtkMessage(fun):
	def worker((function, args, kwargs)):
		apply(function, args, kwargs)
	
	def fun2(*args, **kwargs):
		gobject.idle_add(worker, (fun, args, kwargs))
	
	return fun2

# Run a function on the GTK thread, wait for it to finish, and pass
# along its return value.
def syncGtkMessage(fun):
	def worker((R, condition, function, args, kwargs)):
		R.result = apply(function, args, kwargs)
		condition.acquire()
		condition.notify()
		condition.release()
	
	def fun2(*args, **kwargs):
		condition = threading.Condition()
		condition.acquire()
		class R:
			pass
		gobject.idle_add(worker, (R, condition, fun, args, kwargs))
		condition.wait()
		condition.release()
		return R.result
	
	return fun2

# Start and insert a scrollable WebView. Returns some functions for
# interacting with it.
def launchBrowser(title, uri, echo = True, htmlLocation = "."):
	window = gtk.Window()
	box = gtk.VBox(homogeneous = False, spacing = 0)
	myscroll = gtk.ScrolledWindow()
	browser = webkit.WebView()
	
	window.set_default_size(800, 600)
	window.set_title(title)
	window.connect("destroy", Global.set_quit)
	
	window.add(box)
	box.pack_start(myscroll, expand = True, fill = True, padding = 0)
	myscroll.add(browser)
	
	window.show_all()
	
	messageQueue = Queue.Queue()
	
	def titleChanged(widget, frame, title):
		if title != "null":
			messageQueue.put(title)
	
	browser.connect("title-changed", titleChanged)
	
	settings = browser.get_settings()
	settings.set_property("enable-default-context-menu", False)
	
	# Push new titles onto the Python Queue.
	def webRecv():
		if messageQueue.empty():
			return None
		else:
			msg = messageQueue.get()
			if echo:
				print(">>>", msg)
			return msg
	
	# Run a JS function in the WebView.
	def webSend(msg):
		if echo:
			print("<<<", msg)
		asyncGtkMessage(browser.execute_script)(msg)
	
	# Go to another page.
	def navigate(page):
		browser.open("file://" + htmlLocation + "/" + page)
	
	navigate(uri)
	
	return browser, webRecv, webSend, navigate

# Guess what these two functions do! You get three tries.
def startGtkThread():
	gtk.gdk.threads_init()
	thread.start_new_thread(gtk.main, ())

def killGtkThread():
	asyncGtkMessage(gtk.main_quit)()

# Used for getting flags across threads. Which currently just means
# knowing when to quit.
class Global(object):
	quit = False
	@classmethod
	def set_quit(cls, *args, **kwargs):
		cls.quit = True

# MY NEW FUNCTIONS

# Put escape characters into a string so it can be placed into a
# subscript. I hope these are the only characters where this is
# important.
def dblEscape(string):
	return string.replace("\\", "\\\\").replace("\"", "\\\"").replace("\n", "\\n")

# Invoke a brand new instance of Python to run a script passed as a
# string. Returns anything put on stdout. I told you it was horrifying.
def runSubscript(script):
	process = subprocess.Popen([sys.executable, "-c", script], stdout=subprocess.PIPE)
	(output, err) = process.communicate()
	exitCode = process.wait()
	return output

# Pop up a GTK alert.
alertFun = """
import gtk
dialog = gtk.MessageDialog(type = {1}, buttons = gtk.BUTTONS_OK)
dialog.set_markup("{0}")
dialog.set_title("{2}")
dialog.run()
dialog.hide()
	"""

# Possible alert types (icons).
INFO = int(gtk.MESSAGE_INFO)
WARNING = int(gtk.MESSAGE_WARNING)
QUESTION = int(gtk.MESSAGE_QUESTION)
ERROR = int(gtk.MESSAGE_ERROR)
OTHER = int(gtk.MESSAGE_OTHER)

def alert(message, icon = ERROR, title = ""):
	runSubscript(alertFun.format(dblEscape(message), icon, dblEscape(title)))

# Pop up a GTK file selector dialog. Returns the file chosen, or None if
# the user cancelled.
fselectFun = """
import gtk, sys
chooser = gtk.FileChooserDialog(title="{0}", action=gtk.FILE_CHOOSER_ACTION_{1}, buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_{1}, gtk.RESPONSE_OK))
chooser.set_default_response(gtk.RESPONSE_OK)
for myfilter in {2}:
	fnfilter = gtk.FileFilter()
	fnfilter.set_name(myfilter[0])
	for pattern in myfilter[1]:
		fnfilter.add_pattern(pattern)
	chooser.add_filter(fnfilter)
response = chooser.run()
chooser.hide()
result = chooser.get_filename()
chooser.destroy()
if result:
	sys.stdout.write(result)
"""

# Possible file chooser types. You could just pass a string by itself,
# but I think they look nicer as symbols.
OPEN = "OPEN"
SAVE = "SAVE"

def selectFile(title, action, filters):
	fname = runSubscript(fselectFun.format(dblEscape(title), action, filters))
	return None if fname == "" else fname # Python ternary is stupid