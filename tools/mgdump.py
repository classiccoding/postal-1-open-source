import RSPiX, sys, PIL.Image
if len(sys.argv) != 3:
	sys.stderr.write("usage: " + sys.argv[0] + " INFILE OUTFILE\n")
	sys.exit(1)
rfl = RSPiX.RFile()
rfl.Open(RSPiX.fopen(sys.argv[1], "rb"), rfl.LittleEndian, rfl.Binary)
rmg = RSPiX.RMultiGrid()
rmg.Load(rfl)
rfl.Close()
pim = PIL.Image.new("RGB", (rmg.m_sWidth, rmg.m_sHeight))
data = []
for y in range(0, rmg.m_sHeight):
	for x in range(0, rmg.m_sWidth):
		pix = rmg.GetVal(x, y)
		data.append(((pix & 0xFF00) >> 8, pix & 0x00FF, 0))
pim.putdata(data)
pim.save(sys.argv[2])
