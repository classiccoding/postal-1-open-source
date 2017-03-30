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

import RSPiX, PIL.Image, tempfile, wkinter, os

dirPath = os.path.dirname(os.path.realpath(__file__))

productName = "POSTAL Swiss Army Knife"

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

# Convert RSPiX Palette to Python list of R, G, B values.
# Not tuples! It alternates. This seems to be the format PIL wants.
def paletteToList(myPalette):
	myColours = []
	for colour in range(myPalette.m_sStartIndex, myPalette.m_sNumEntries):
		for func in [RSPiX.getRed, RSPiX.getGreen, RSPiX.getBlue]:
			myColours.append(func(myPalette, colour))
	return myColours

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

# Convert Python list of Sprites to one PIL Image.
def spritesToPImage(mySprites, myPalette, imWidth = 0, imHeight = 0):
	myImages = []

	for sprite in mySprites:
		myImages.append([(sprite.m_sX, sprite.m_sY), rImageToPImage(sprite.m_pImage, myPalette)])
		right = sprite.m_sX + sprite.m_lWidth
		bottom = sprite.m_sY + sprite.m_lHeight
		if right > imWidth:
			imWidth = right
		if bottom > imHeight:
			imHeight = bottom

	finalImage = PIL.Image.new("P", (imWidth, imHeight))
	finalImage.putpalette(paletteToList(myPalette))

	for data in myImages:
		finalImage.paste(data[1], data[0])
	
	return finalImage

# Convert a list of Sprites to PNG and saves it.
def spryToPng(mySprites, palette, outname, imWidth, imHeight):
	if not outname.endswith(".png"):
		outname += ".png"
	image = spritesToPImage(mySprites, palette, imWidth, imHeight)
	image.save(outname, transparency = 0, optimize = 1)
	del image

# Save a BMP file's contents to a PNG file with a passed palette.
def bmpToPng(fname, outname, palette):
	inim = PIL.Image.open(fname)
	outim = PIL.Image.new("P", inim.size)
	outim.putpalette(paletteToList(palette))
	outim.paste(inim, (0, 0))
	outim.save(outname, transparency = 0, optimize = 1)

# Start the program.
if __name__ == "__main__":
	wkinter.startGtkThread()

	uri = "home.html"
	
	(currobj, suppobj) = (None, None)
	
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
					suppobj = RSPiX.RImage()
					if suppobj.Load(basename) == 0:
						currobj = RSPiX.RSpry()
						if currobj.Load(fname) == 0:
							wkinter.syncGtkMessage(navigate)("spry.html")
							while message != "ready":
								message = webRecv()
							webSend("setsay('{0}', '{1}');".format(fname, str(currobj.m_listSprites.GetCount())))
						else:
							del suppobj
							wkinter.alert("Failed to load Spry from " + fname, title = productName)
					else:
						wkinter.alert("Failed to load RImage from " + basename, title = productName)
		elif message == "home":
			(currobj, suppobj) = (None, None)
			wkinter.syncGtkMessage(navigate)(uri)
		elif message == "convpng":
			outname = wkinter.selectFile("Save PNG file", wkinter.SAVE, [["PNG files", ["*.png"]], ["All files", ["*"]]])
			if outname != None:
				sprylist = spryToList(currobj)
				spryToPng(sprylist, suppobj.m_pPalette, outname)
				currobj = listToSpry(sprylist)
		elif message == "batchpng":
			fname = wkinter.selectFile("Select a directory", wkinter.DIR, [["Directory", ["*"]]])
			if fname != None:
				for subdir, dirs, files in os.walk(fname):
					for name in files:
						filepath = subdir + os.sep + name
						if filepath.endswith(".say"):
							basename = filepath[:-7] + ".bmp"
							outname = filepath[:-4] + ".png"
							tmpim = RSPiX.RImage()
							if tmpim.Load(basename) == 0:
								tmpspry = RSPiX.RSpry()
								tmpspry.Load(filepath)
								sprylist = spryToList(tmpspry)
								spryToPng(sprylist, tmpim.m_pPalette, outname, tmpim.m_sWidth, tmpim.m_sHeight)
							else:
								wkinter.alert("Failed to load base image for " + name + " - Skipping.", title = productName, icon = wkinter.WARNING)
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