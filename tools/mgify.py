import RSPiX, sys, PIL.Image
if len(sys.argv) != 4:
	sys.stderr.write("usage: " + sys.argv[0] + " INFILE MAP OUTFILE\n")
	sys.exit(1)
pim = PIL.Image.open(sys.argv[1])
with open(sys.argv[2]) as fobj:
	pmap = eval(fobj.read())
rmg = RSPiX.RMultiGrid()
rmg.Alloc(pim.width, pim.height)
data = list(pim.getdata())
for y in range(0, pim.height):
	for x in range(0, pim.width):
		try:
			rmg.SetValueUncompressed(pmap[data[y * pim.width + x]], x, y)
		except KeyError:
			pass
rmg.Compress(32, 32)
rfl = RSPiX.RFile()
rfl.Open(RSPiX.fopen(sys.argv[3], "wb"), rfl.LittleEndian, rfl.Binary)
rmg.Save(rfl)
rfl.Close()
