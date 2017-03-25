#!/usr/bin/env python2
# POSTAL Swiss Army Knife
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

# GUI for working with the proprietary file formats used in RSPiX.

import RSPiX, PIL.Image
import gtk, gobject, thread, threading, webkit, Queue, os, subprocess
import sys

dirPath = os.path.dirname(os.path.realpath(__file__))

# Backend functions.

# Convert Spry to Python list of Sprites.
# This empties the Spry.
def spryToList(mySpry):
	mySprites = []
	while mySpry.m_listSprites.GetHead():
		mySprites.append(RSPiX.ppRSprDereference(mySpry.m_listSprites.GetHeadData()))
		mySpry.m_listSprites.RemoveHead()
	return mySprites

# Convert Python list of Sprites to Spry.
def listToSpry(mySprites):
	mySpry = RSPiX.RSpry()
	for sprite in mySprites:
		mySpry.m_listSprites.InsertTail(sprite)
	return mySpry

# Convert RSPiX Image to Python list of RGBA tuples.
def rImageToList(myImage):
	# do magic
	pass

# Convert Python list of Sprites to one PIL Image.
def spritesToPImage(mySprites):
	myImages = []
	imWidth = 0
	imHeight = 0

	for sprite in mySprites:
		myImages.append([(sprite.m_sX, sprite.m_sY), (sprite.m_lWidth, sprite.m_lHeight), rImageToList(sprite.m_pImage)])
		right = sprite.m_sX + sprite.m_lWidth
		bottom = sprite.m_sY + sprite.m_lHeight
		if right > imWidth:
			imWidth = right
		if bottom > imHeight:
			imHeight = bottom

	finalImage = PIL.Image.new("RGBA", (imWidth, imHeight))

	for data in myImages:
		sprIm = PIL.Image.new("RGBA", data[1])
		sprIm.putdata(data[2])
		finalImage.paste(sprIm, data[0])
	
	return finalImage

# Interface classes and functions.
# The interface here is based on an excellent tutorial by David Baird.

def asyncGtkMessage(fun):
	def worker((function, args, kwargs)):
		apply(function, args, kwargs)
	
	def fun2(*args, **kwargs):
		gobject.idle_add(worker, (fun, args, kwargs))
	
	return fun2

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

def launchBrowser(uri, echo = True):
	window = gtk.Window()
	box = gtk.VBox(homogeneous = False, spacing = 0)
	myscroll = gtk.ScrolledWindow()
	browser = webkit.WebView()
	
	window.set_default_size(800, 600)
	window.set_title("POSTAL Swiss Army Knife")
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
	
	def webRecv():
		if messageQueue.empty():
			return None
		else:
			msg = messageQueue.get()
			if echo:
				print(">>>", msg)
			return msg
	
	def webSend(msg):
		if echo:
			print("<<<", msg)
		asyncGtkMessage(browser.execute_script)(msg)
	
	def navigate(page):
		browser.open("file://" + dirPath + "/psak_pages/" + page)
	
	navigate(uri)
	
	return browser, webRecv, webSend, navigate

def startGtkThread():
	gtk.gdk.threads_init()
	thread.start_new_thread(gtk.main, ())

def killGtkThread():
	asyncGtkMessage(gtk.main_quit)()

class Global(object):
	quit = False
	@classmethod
	def set_quit(cls, *args, **kwargs):
		cls.quit = True

# Now it's time for my own interface functions. Nothing below here is by
# David Baird. This is all my fault.

def runSubscript(script):
	process = subprocess.Popen([sys.executable, "-c", script], stdout=subprocess.PIPE)
	(output, err) = process.communicate()
	exitCode = process.wait()
	return output

alertFun = """
import gtk
dialog = gtk.MessageDialog(type = gtk.MESSAGE_ERROR, buttons = gtk.BUTTONS_OK)
dialog.set_markup("{0}")
dialog.run()
dialog.hide()
	"""

def alert(message):
	runSubscript(alertFun.format(message.replace("\\", "\\\\")))

fselectFun = """
import gtk, sys
fnfilter = gtk.FileFilter()
fnfilter.set_name("{2}")
fnfilter.add_pattern("{3}")
chooser = gtk.FileChooserDialog(title="{0}", action=gtk.FILE_CHOOSER_ACTION_{1}, buttons=(gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL, gtk.STOCK_{1}, gtk.RESPONSE_OK))
chooser.set_default_response(gtk.RESPONSE_OK)
chooser.add_filter(fnfilter)
response = chooser.run()
chooser.hide()
if response == gtk.RESPONSE_OK:
	result = chooser.get_filename()
else:
	result = "NONE"
chooser.destroy()
sys.stdout.write(result)
"""

def selectFile(title, action, name, pattern):
	fname = runSubscript(fselectFun.format(title, action, name, pattern))
	if fname == "NONE":
		return None
	else:
		return fname

# Start the program.
if __name__ == "__main__":
	startGtkThread()

	uri = "home.html"
	
	currobj = None
	
	browser, webRecv, webSend, navigate = syncGtkMessage(launchBrowser)(uri, echo = False)

	while not Global.quit:
		message = webRecv()
		if message == "spry":
			fname = selectFile("Select a Spry file", "OPEN", "Spry files", "*.say")
			if fname != None:
				currobj = RSPiX.RSpry()
				if currobj.Load(fname) == 0:
					syncGtkMessage(navigate)("spry.html")
					webSend("setsay('{0}', '{1}');".format(fname, str(currobj.m_listSprites.GetCount())))
				else:
					alert("Failed to load Spry from " + fname)
		elif message == "home":
			del currobj
			syncGtkMessage(navigate)(uri)
		elif message == "convpng":
			outname = selectFile("Save PNG file", "SAVE", "PNG files", "*.png")
			if outname != None:
				sprylist = spryToList(currobj)
				image = spritesToPImage(sprylist)
				image.save(outname)
				del image
				currobj = listToSpry(sprylist)
		elif message != None:
			alert("This feature has not yet been implemented.")
