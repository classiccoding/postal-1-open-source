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

# Convert RSPiX Image to PIL Image.
def rImageToPImage(myImage):
	# Convert to BMP first.
	myImage.Convert(myImage.BMP8)
	
	# Reserve 54 bytes for the BMP header
	size = 54
	
	# Get the size of the image data.
	size += myImage.m_ulSize
	
	# Reserve space for the palette if it exists
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
def spritesToPImage(mySprites):
	myImages = []
	imWidth = 0
	imHeight = 0

	for sprite in mySprites:
		myImages.append([(sprite.m_sX, sprite.m_sY), rImageToPImage(sprite.m_pImage)])
		right = sprite.m_sX + sprite.m_lWidth
		bottom = sprite.m_sY + sprite.m_lHeight
		if right > imWidth:
			imWidth = right
		if bottom > imHeight:
			imHeight = bottom

	finalImage = PIL.Image.new("RGBA", (imWidth, imHeight))

	for data in myImages:
		finalImage.paste(data[1], data[0])
	
	return finalImage

# Start the program.
if __name__ == "__main__":
	wkinter.startGtkThread()

	uri = "home.html"
	
	currobj = None
	
	browser, webRecv, webSend, navigate = wkinter.syncGtkMessage(wkinter.launchBrowser)(productName, uri, echo = False, htmlLocation = dirPath + "/psak_pages")

	while not wkinter.Global.quit:
		message = webRecv()
		if message == "spry":
			fname = wkinter.selectFile("Select a Spry file", wkinter.OPEN, [["Spry files", ["*.say"]], ["All files", ["*"]]])
			if fname != None:
				currobj = RSPiX.RSpry()
				if currobj.Load(fname) == 0:
					wkinter.syncGtkMessage(navigate)("spry.html")
					while message != "ready":
						message = webRecv()
					webSend("setsay('{0}', '{1}');".format(fname, str(currobj.m_listSprites.GetCount())))
				else:
					wkinter.alert("Failed to load Spry from " + fname, title = productName)
		elif message == "home":
			del currobj
			wkinter.syncGtkMessage(navigate)(uri)
		elif message == "convpng":
			outname = wkinter.selectFile("Save PNG file", wkinter.SAVE, [["PNG files", ["*.png"]], ["All files", ["*"]]])
			if outname != None:
				if not outname.endswith(".png"):
					outname += ".png"
				sprylist = spryToList(currobj)
				image = spritesToPImage(sprylist)
				image.save(outname)
				del image
				currobj = listToSpry(sprylist)
		elif message != None:
			wkinter.alert("This feature has not yet been implemented.", title = productName)