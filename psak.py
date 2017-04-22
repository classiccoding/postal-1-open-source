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

# Functions & GUI for working with the proprietary file formats used in RSPiX.

import RSPiX, PIL.Image, tempfile, os, array

try:
	import wkinter
	gui = True
except:
	gui = False

dirPath = os.path.dirname(os.path.realpath(__file__))

productName = "POSTAL Swiss Army Knife"

# Exception classes.

class RSPiXError(Exception):
	"""An RSPiX function failed."""

class NotPalettedException(Exception):
	"""splitPImage() got a non-paletted image."""

# Backend functions.

# Split a list into segments of equal size.
def splitList(l, n):
	return [l[i:i + n] for i in range(0, len(l), n)]

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

# Convert RSPiX Palette to Python list of R, G, B values.
# Not tuples! It alternates. This seems to be the format PIL wants.
def rPaletteToList(myPalette):
	myColours = []
	for colour in range(myPalette.m_sStartIndex, myPalette.m_sNumEntries):
		for func in [RSPiX.getRed, RSPiX.getGreen, RSPiX.getBlue]:
			myColours.append(func(myPalette, colour))
	return myColours

# Look up each item in a list on a map and replace it with the located
# value.
def mapData(data, datamap):
	return [datamap[x] for x in data]

# Use mapData() to map an image to a given palette.
def mapImage(data, curr, target):
	curr = splitList(curr, 3)
	target = splitList(target, 3)
	
	# Fix* Index 0 transparency magic
	# *results may vary
	curr[0] = target[0]
	
	palmap = [target.index(x) for x in curr]
	
	return mapData(data, palmap)
	

# Convert RSPiX Image to PIL Image.
def rImageToPImage(myImage, myPalette = None):
	# If there is no palette, load it from the Hood
	if myPalette and not myImage.m_pPalette:
		myImage.SetPalette(myPalette)
	
	# Convert to BMP
	myImage.Convert(myImage.BMP8)
	
	# Reserve 54 bytes for the BMP header
	size = 54
	
	# Get the size of the image data
	size += myImage.m_ulSize

	# Reserve space for the palette
	if myImage.m_pPalette:
		size += myImage.m_pPalette.m_ulSize

	# Save the Image as a DIB to an RFile in memory
	mem = RSPiX.allocateFile(size)
	myFile = RSPiX.RFile()
	myFile.Open(mem, size, myFile.LittleEndian)
	myImage.SaveDib(myFile)
	
	# Extract data from RFile into list
	data = []
	for x in range(0, size):
		data.append(RSPiX.getByte(mem, x))
	myFile.Close()
	
	# Copy list into Python memory file object
	myOutFile = tempfile.TemporaryFile()
	myOutFile.write(bytearray(data))
	
	# Load file into PIL Image
	return PIL.Image.open(myOutFile)

# Convert PIL Image to RSPiX Image.
def pImageToRImage(myImage):
	outIm = RSPiX.RImage()
	outIm.Init()
	
	# Create a Python memory file object
	outBmp = tempfile.TemporaryFile()
	
	# Save the image to it
	myImage.save(outBmp, "bmp")
	
	# Load BMP into RImage
	outBmp.seek(0)
	myData = outBmp.read()
	outBmp.close()
	mem = RSPiX.allocateFile(len(myData))
	myFile = RSPiX.RFile()
	myFile.Open(mem, len(myData), myFile.LittleEndian)
	myFile.Write(myData, len(myData))
	myFile.Seek(0, os.SEEK_SET)
	if outIm.Load(myFile) != 0:
		raise RSPiXError
	
	# Convert BMP to FSPR8 and remove palette information
	outIm.Convert(outIm.FSPR8)
	outIm.DestroyPalette()
	
	return outIm

# Split PIL Image into a list of chunks with size and coordinates.
# Effectively a wrapper for ameliorate's "am_chop".
def splitPImage(myImage):
	if myImage.mode != "P":
		raise NotPalettedException
	data = array.array("B", myImage.getdata()).tostring()
	box = RSPiX.am_chop(data, myImage.width, myImage.height)
	del data
	chunks = []
	
	while box:
		newim = PIL.Image.new(myImage.mode, (box.w, box.h))
		newim.palette = myImage.palette
		newim.putdata([ord(RSPiX.getBoxDataByte(box, x)) for x in range(0, box.w * box.h)])
		chunks.append( {"location": (box.x, box.y), "size": (box.w, box.h), "pimage": newim} )
		box = box.next
	
	return chunks

# Convert one PIL Image to Python list of Sprites.
def pImageToSprites(myImage, palname):
	chunks = splitPImage(myImage)
	srcPalette = myImage.getpalette()
	destPalette = PIL.Image.open(palname).getpalette()
	for chunk in chunks:
		try:
			chunk["pimage"].putdata(mapImage(list(chunk["pimage"].getdata()), srcPalette, destPalette))
			chunk["rimage"] = pImageToRImage(chunk["pimage"])
		except SystemError:
			# FIXME: Wtf is actually happening here?
			# Workaround for now is just pass an empty one
			chunk["rimage"] = RSPiX.RImage()
		del chunk["pimage"]
	return (chunks, [RSPiX.RSprite(chunk["location"][0], chunk["location"][1], idx, 0, chunk["size"][0], chunk["size"][1], 0, chunk["rimage"]) for idx, chunk in enumerate(chunks)])

# Convert Python list of Sprites to one PIL Image.
def spritesToPImage(mySprites, hoodName):
	rHood = RSPiX.RImage()
	rHood.Load(hoodName)
	imWidth = rHood.m_sWidth
	imHeight = rHood.m_sHeight
	
	pHood = PIL.Image.open(hoodName)
	
	myImages = []

	for sprite in mySprites:
		myImages.append([(sprite.m_sX, sprite.m_sY),  rImageToPImage(sprite.m_pImage, rHood.m_pPalette)])
		right = sprite.m_sX + sprite.m_lWidth
		bottom = sprite.m_sY + sprite.m_lHeight
		if right > imWidth:
			imWidth = right
		if bottom > imHeight:
			imHeight = bottom

	finalImage = PIL.Image.new("P", (imWidth, imHeight))
	finalImage.palette = pHood.palette

	for data in myImages:
		finalImage.paste(data[1], data[0])
	
	return finalImage

# Convert a list of Sprites to PNG and save it.
def spritesToPng(mySprites, hoodName, outname):
	if not outname.endswith(".png"):
		outname += ".png"
	image = spritesToPImage(mySprites, hoodName)
	image.save(outname, transparency = 0, optimize = 1)
	del image

# Save a BMP file's contents to a PNG file with a passed palette.
def bmpToPng(fname, outname, palette):
	inim = PIL.Image.open(fname)
	outim = PIL.Image.new("P", inim.size)
	outim.putpalette(rPaletteToList(palette))
	outim.paste(inim, (0, 0))
	outim.save(outname, transparency = 0, optimize = 1)

# Convert a PNG file (or any image file, theoretically) to Spry and save it.
def pngToSpry(fname, palname, outname):
	inim = PIL.Image.open(fname)
	(chunks, spriteList) = pImageToSprites(inim, palname)
	outSpry = listToSpry(spriteList)
	
	# An irritating characteristic of RSPiX is file out, because it
	# automatically prepends the user's PrefPath to any paths it gets -
	# even absolute ones. The solution is to use fopen()... dumb.
	outFile = RSPiX.RFile()
	outFile.Open(RSPiX.fopen(outname, "wb"), outFile.LittleEndian, outFile.Binary)
	outSpry.Save(outFile)
	outFile.Close()
	spriteList = spryToList(outSpry) # Empty the Spry so it doesn't destroy the sprites

# Start the program.
if __name__ == "__main__" and gui:
	wkinter.startGtkThread()

	uri = "home.html"
	
	currobj = None
	
	browser, webRecv, webSend, navigate = wkinter.syncGtkMessage(wkinter.launchBrowser)(productName, uri, echo = False, htmlLocation = dirPath + "/psak_pages")

	while not wkinter.Global.quit:
		message = webRecv()
		if message == "spry":
			fname = wkinter.selectFile("Select a Spry file", wkinter.OPEN, [["Spry files", ["*.say"]], ["All files", ["*"]]])
			if fname != None:
				basename = fname[:-7] + ".bmp"
				if not os.path.isfile(basename):
					wkinter.alert("PSAK also requires access to the hood's base image for palette information. The file could not be found. Please locate it manually.", icon = wkinter.INFO, title = productName)
					basename = wkinter.selectFile("Select the hood base BMP", wkinter.OPEN, [["BMP files", ["*.bmp"]], ["All files", ["*"]]])
				if basename != None:
					currobj = RSPiX.RSpry()
					if currobj.Load(fname) == 0:
						wkinter.syncGtkMessage(navigate)("spry.html")
						while message != "ready":
							message = webRecv()
						webSend("setsay('{0}', '{1}');".format(fname, str(currobj.m_listSprites.GetCount())))
					else:
						currobj = None
						wkinter.alert("Failed to load Spry from " + fname, title = productName)
		elif message == "home":
			currobj = None
			wkinter.syncGtkMessage(navigate)(uri)
		elif message == "convpng":
			outname = wkinter.selectFile("Save PNG file", wkinter.SAVE, [["PNG files", ["*.png"]], ["All files", ["*"]]])
			if outname != None:
				sprylist = spryToList(currobj)
				spritesToPng(sprylist, basename, outname)
				currobj = listToSpry(sprylist)
		elif message == "convsay":
			fname = wkinter.selectFile("Select a PNG file", wkinter.OPEN, [["PNG files", ["*.png"]], ["All files", ["*"]]])
			if fname != None:
				outname = wkinter.selectFile("Save Spry file", wkinter.SAVE, [["Spry files", ["*.say"]], ["All files", ["*"]]])
				if outname != None:
					basename = fname[:-7] + ".bmp"
					if not os.path.isfile(basename):
						wkinter.alert("PSAK also requires access to the hood's base image for palette information. The file could not be found. Please locate it manually.", icon = wkinter.INFO, title = productName)
						basename = wkinter.selectFile("Select the hood base BMP", wkinter.OPEN, [["BMP files", ["*.bmp"]], ["All files", ["*"]]])
					if basename != None:
						pngToSpry(fname, basename, outname)
		elif message == "batchpng":
			fname = wkinter.selectFile("Select a directory", wkinter.DIR, [["Directory", ["*"]]])
			if fname != None:
				for subdir, dirs, files in os.walk(fname):
					for name in files:
						filepath = subdir + os.sep + name
						if filepath.endswith(".say"):
							basename = filepath[:-7] + ".bmp"
							outname = filepath[:-4] + ".png"
							tmpspry = RSPiX.RSpry()
							tmpspry.Load(filepath)
							sprylist = spryToList(tmpspry)
							spritesToPng(sprylist, basename, outname)
		elif message == "batchsay":
			dirname = wkinter.selectFile("Select a directory", wkinter.DIR, [["Directory", ["*"]]])
			if dirname != None:
				interest = []
				for subdir, dirs, files in os.walk(dirname):
					for fname in files:
						filepath = subdir + os.sep + fname
						if filepath.endswith(".png"):
							basename = filepath[:-7] + ".bmp"
							if os.path.isfile(basename):
								outname = filepath[:-4] + ".say"
								interest.append((filepath, basename, outname))
							else:
								wkinter.alert("Failed to load base image for " + fname + " - Skipping.", title = productName, icon = wkinter.WARNING)
				if len(interest) > 0:
					wkinter.syncGtkMessage(navigate)("progress.html")
					while message != "ready":
						message = webRecv()
					for idx, (fname, basename, outname) in enumerate(interest):
						webSend("document.getElementById('fname').innerHTML = 'Converting {0}';".format(fname))
						webSend("setprog({0}, {1});".format(idx, len(interest)))
						pngToSpry(fname, basename, outname)
					wkinter.syncGtkMessage(navigate)(uri)
		elif message == "correct":
			fname = wkinter.selectFile("Select a BMP", wkinter.OPEN, [["BMP files", ["*.bmp"]], ["All files", ["*"]]])
			if fname != None:
				outname = wkinter.selectFile("Save PNG file", wkinter.SAVE, [["PNG files", ["*.png"]], ["All files", ["*"]]])
				if outname != None:
					rspim = RSPiX.RImage()
					if rspim.Load(fname) == 0:
						bmpToPng(fname, outname, rspim.m_pPalette)
						del rspim
					else:
						wkinter.alert("Failed to load BMP from " + fname, title = productName)
		elif message != None:
			wkinter.alert("This feature has not yet been implemented.", title = productName)