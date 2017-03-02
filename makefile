
# !!! FIXME: Make this more robust. MUCH more robust.
# !!! FIXME: ...or at least comment the rest of these options...

BINDIR := ./bin
SRCDIR := .

ifeq ($(PANDORA),1)
  macosx := false
  CPUARCH := arm
  CC := g++
  LINKER := g++
  steamworks := false
  CFLAGS += -mcpu=cortex-a8 -mfpu=neon -mfloat-abi=softfp -ftree-vectorize -ffast-math -DPANDORA
  CLIENTEXE := $(BINDIR)/postal1-arm
else ifeq ($(ODROID),1)
  macosx := false
  CPUARCH := arm
  CC := g++
  LINKER := g++
  steamworks := false
  CFLAGS += -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -ftree-vectorize -ffast-math -DODROID
  CLIENTEXE := $(BINDIR)/postal1-arm
else ifeq ($(linux_x86),1)
  target := linux_x86
  CFLAGS += -m32
  CLIENTEXE := $(BINDIR)/postal1-x86
else ifeq ($(macosx_x86),1)
  target := macosx_x86
  steamworks := true
  CLIENTEXE := $(BINDIR)/postal1-x86
else
  target := linux_x86_64
  CLIENTEXE := $(BINDIR)/postal1-x86_64
endif

# ----------------------------------------------------- ... bleh.

ifeq ($(strip $(target)),linux_x86)
  macosx := false
  CPUARCH := x86
  CC := g++
  LINKER := g++ -m32
endif
ifeq ($(strip $(target)),linux_x86_64)
  macosx := false
  CPUARCH := x86_64
  CC := g++
  LINKER := g++
endif
ifeq ($(strip $(target)),macosx_x86)
  macosx := true
  CPUARCH := x86
  CC := g++
  LINKER := g++
endif

CLEANUP := $(wildcard *.exe) $(wildcard *.obj) \
          $(wildcard $(BINDIR)/*.exe) $(wildcard $(BINDIR)/*.obj) \
          $(wildcard *~) $(wildcard *.err) \
          $(wildcard .\#*) core

SRCS := \
	AnimThing.cpp \
	ball.cpp \
	barrel.cpp \
	bouy.cpp \
	character.cpp \
	chunk.cpp \
	deathWad.cpp \
	demon.cpp \
	dispenser.cpp \
	doofus.cpp \
	dude.cpp \
	explode.cpp \
	fire.cpp \
	fireball.cpp \
	firebomb.cpp \
	flag.cpp \
	flagbase.cpp \
	grenade.cpp \
	heatseeker.cpp \
	hood.cpp \
	item3d.cpp \
	mine.cpp \
	napalm.cpp \
	navnet.cpp \
	ostrich.cpp \
	person.cpp \
	PowerUp.cpp \
	pylon.cpp \
	rocket.cpp \
	sentry.cpp \
	SndRelay.cpp \
	SoundThing.cpp \
	thing.cpp \
	Thing3d.cpp \
	trigger.cpp \
	warp.cpp \
	weapon.cpp \
	alphablitforpostal.cpp \
	Anim3D.cpp \
	BufQ.cpp \
	bulletFest.cpp \
	camera.cpp \
	crawler.cpp \
	cutscene.cpp \
	encrypt.cpp \
	gameedit.cpp \
	GameSettings.cpp \
	grip.cpp \
	IdBank.cpp \
	InputSettings.cpp \
	InputSettingsDlg.cpp \
	keys.cpp \
	Log.cpp \
	logtab.cpp \
	MemFileFest.cpp \
	MenuSettings.cpp \
	MenuTrans.cpp \
	net.cpp \
	NetBrowse.cpp \
	NetClient.cpp \
	NetDlg.cpp \
	netmsgr.cpp \
	NetServer.cpp \
	organ.cpp \
	Personatorium.cpp \
	ProtoBSDIP.cpp \
	realm.cpp \
	scene.cpp \
	score.cpp \
	settings.cpp \
	smash.cpp \
	socket.cpp \
	StockPile.cpp \
	TexEdit.cpp \
	toolbar.cpp \
	TriggerRegions.cpp \
	update.cpp \
	yatime.cpp \
	aivars.cpp \
	band.cpp \
	credits.cpp \
	game.cpp \
	input.cpp \
	localize.cpp \
	main.cpp \
	menus.cpp \
	play.cpp \
	SampleMaster.cpp \
	title.cpp \
	RSPiX/BLUE/sdl2/Bdebug.cpp \
	RSPiX/BLUE/sdl2/Bjoy.cpp \
	RSPiX/BLUE/sdl2/Bkey.cpp \
	RSPiX/BLUE/sdl2/Bmain.cpp \
	RSPiX/BLUE/sdl2/Bmouse.cpp \
	RSPiX/BLUE/sdl2/Btime.cpp \
	RSPiX/BLUE/sdl2/Bdisp.cpp \
	RSPiX/BLUE/sdl2/Bsound.cpp \
	RSPiX/GREEN/Hot/hot.cpp \
	RSPiX/GREEN/Image/Image.cpp \
	RSPiX/GREEN/Image/ImageCon.cpp \
	RSPiX/GREEN/Image/ImageFile.cpp \
	RSPiX/GREEN/InputEvent/InputEvent.cpp \
	RSPiX/GREEN/Mix/mix.cpp \
	RSPiX/GREEN/Mix/MixBuf.cpp \
	RSPiX/GREEN/Image/Pal.cpp \
	RSPiX/GREEN/Image/PalFile.cpp \
	RSPiX/GREEN/Sample/sample.cpp \
	RSPiX/GREEN/Snd/snd.cpp \
	RSPiX/GREEN/SndFx/SndFx.cpp \
	RSPiX/GREEN/3D/pipeline.cpp \
	RSPiX/GREEN/3D/render.cpp \
	RSPiX/GREEN/3D/types3d.cpp \
	RSPiX/GREEN/3D/zbuffer.cpp \
	RSPiX/GREEN/Blit/AlphaBlit.cpp \
	RSPiX/GREEN/Blit/Blit.cpp \
	RSPiX/GREEN/Blit/BlitInit.cpp \
	RSPiX/GREEN/Blit/BlitT.cpp \
	RSPiX/GREEN/Blit/Cfnt.cpp \
	RSPiX/GREEN/Blit/Fspr1.cpp \
	RSPiX/GREEN/Blit/Fspr8.cpp \
	RSPiX/GREEN/Blit/line.cpp \
	RSPiX/GREEN/Blit/mono.cpp \
	RSPiX/GREEN/Blit/Rotate96.cpp \
	RSPiX/GREEN/Blit/RPrint.cpp \
	RSPiX/GREEN/Blit/ScaleFlat.cpp \
	RSPiX/ORANGE/GameLib/AnimSprite.cpp \
	RSPiX/ORANGE/Attribute/attribute.cpp \
	RSPiX/ORANGE/GUI/btn.cpp \
	RSPiX/ORANGE/Channel/channel.cpp \
	RSPiX/ORANGE/color/colormatch.cpp \
	RSPiX/ORANGE/DirtRect/DirtRect.cpp \
	RSPiX/ORANGE/color/dithermatch.cpp \
	RSPiX/ORANGE/GUI/dlg.cpp \
	RSPiX/ORANGE/GUI/edit.cpp \
	RSPiX/ORANGE/File/file.cpp \
	RSPiX/ORANGE/QuickMath/FixedPoint.cpp \
	RSPiX/ORANGE/GUI/guiItem.cpp \
	RSPiX/ORANGE/IFF/iff.cpp \
	RSPiX/ORANGE/ImageTools/lasso.cpp \
	RSPiX/ORANGE/Laymage/laymage.cpp \
	RSPiX/ORANGE/GUI/ListBox.cpp \
	RSPiX/ORANGE/GUI/ListContents.cpp \
	RSPiX/ORANGE/Meter/meter.cpp \
	RSPiX/ORANGE/MsgBox/MsgBox.cpp \
	RSPiX/ORANGE/GUI/MultiBtn.cpp \
	RSPiX/ORANGE/MultiGrid/MultiGrid.cpp \
	RSPiX/ORANGE/MultiGrid/MultiGridIndirect.cpp \
	RSPiX/ORANGE/GUI/ProcessGui.cpp \
	RSPiX/ORANGE/Debug/profile.cpp \
	RSPiX/ORANGE/GUI/PushBtn.cpp \
	RSPiX/ORANGE/QuickMath/QuickMath.cpp \
	RSPiX/ORANGE/GameLib/Region.cpp \
	RSPiX/ORANGE/RString/rstring.cpp \
	RSPiX/ORANGE/GUI/scrollbar.cpp \
	RSPiX/ORANGE/GameLib/Shapes.cpp \
	RSPiX/ORANGE/Parse/SimpleBatch.cpp \
	RSPiX/ORANGE/GameLib/Sprite.cpp \
	RSPiX/ORANGE/str/str.cpp \
	RSPiX/ORANGE/GUI/txt.cpp \
	RSPiX/CYAN/sdl2/uDialog.cpp \
	RSPiX/CYAN/sdl2/uColors.cpp \
	RSPiX/CYAN/sdl2/uPath.cpp \
	WishPiX/Menu/menu.cpp \
	WishPiX/Prefs/prefline.cpp \
	WishPiX/Prefs/prefs.cpp \
	WishPiX/ResourceManager/resmgr.cpp \
	WishPiX/Spry/spry.cpp

    # wtf is THIS?!

OBJS0 := $(SRCS:.s=.o)
OBJS1 := $(OBJS0:.c=.o)
OBJS2 := $(OBJS1:.cpp=.o)
OBJS3 := $(OBJS2:.nasm=.o)
OBJS4 := $(OBJS3:.s=.o)
OBJS := $(foreach f,$(OBJS4),$(BINDIR)/$(f))
SRCS := $(foreach f,$(SRCS),$(SRCDIR)/$(f))

CFLAGS += -w

ifeq ($(strip $(macosx)),true)
  CFLAGS += -DPLATFORM_MACOSX
endif

# defines the game needs...
CFLAGS += -DLOCALE=US -DTARGET=POSTAL_2015

# includes ...
CFLAGS += -I$(SRCDIR)/sys/include
CFLAGS += -I$(SRCDIR)/RSPiX
CFLAGS += -I$(SRCDIR)/WishPiX

ifeq ($(strip $(expiring_beta)),true)
  CFLAGS += -DBETAEXPIRE=$(shell date +%s)
endif

ifeq ($(strip $(macosx)),true)
  CFLAGS += -arch i386 -mmacosx-version-min=10.5
  LDFLAGS += -arch i386 -mmacosx-version-min=10.5
  LDFLAGS += -framework CoreFoundation -framework Cocoa
  LIBS += sys/lib/macosx/libSDL2-2.0.0.dylib
  STEAMLDFLAGS += steamworks/sdk/redistributable_bin/osx32/libsteam_api.dylib
else
  ifeq ($(CPUARCH),arm)
    LIBS += -lSDL2
  else
	ifeq ($(CPUARCH),x86_64)
	  LIBS += -lSDL2
	else
	  LIBS += sys/lib/linux-x86/libSDL2-2.0.so.0
	  LDFLAGS += -Wl,-rpath,\$$ORIGIN
	  STEAMLDFLAGS += steamworks/sdk/redistributable_bin/linux32/libsteam_api.so
	endif
 endif
endif

ifeq ($(strip $(steamworks)),true)
  CFLAGS += -DSTEAM_CONNECTED -Isteamworks/sdk/public
  LDFLAGS += $(STEAMLDFLAGS)
endif

CXXFLAGS = $(CFLAGS) -std=c++11

.PHONY: all bindir


all: debugoff $(CLIENTEXE)


debug: debugon $(CLIENTEXE)

debugon:
	$(eval CFLAGS += -DDEBUG -D_DEBUG -O0 -g)

debugoff:
	$(eval OPTFLAG = -O3)
	$(eval CFLAGS += -DNDEBUG -D_NDEBUG -DRELEASE -O2)

$(BINDIR)/%.o: $(SRCDIR)/%.s
	$(CC) $(CFLAGS) -DELF -x assembler-with-cpp -o $@ -c $<

$(BINDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(BINDIR)/%.o: $(SRCDIR)/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/%.a: $(SRCDIR)/%.a
	cp $< $@
	ranlib $@

$(CLIENTEXE): $(BINDIR) $(OBJS) $(LIBS)
	$(LINKER) -o $(CLIENTEXE) $(OBJS) $(LDFLAGS) $(LIBS)

$(BINDIR) :
	$(MAKE) bindir

bindir :
	mkdir -p $(BINDIR)
	mkdir -p $(BINDIR)/RSPiX/BLUE/sdl2
	mkdir -p $(BINDIR)/RSPiX/GREEN/Hot
	mkdir -p $(BINDIR)/RSPiX/GREEN/Image
	mkdir -p $(BINDIR)/RSPiX/GREEN/InputEvent
	mkdir -p $(BINDIR)/RSPiX/GREEN/Mix
	mkdir -p $(BINDIR)/RSPiX/GREEN/Sample
	mkdir -p $(BINDIR)/RSPiX/GREEN/Snd
	mkdir -p $(BINDIR)/RSPiX/GREEN/SndFx
	mkdir -p $(BINDIR)/RSPiX/GREEN/3D
	mkdir -p $(BINDIR)/RSPiX/GREEN/Blit
	mkdir -p $(BINDIR)/RSPiX/ORANGE/GameLib
	mkdir -p $(BINDIR)/RSPiX/ORANGE/Attribute
	mkdir -p $(BINDIR)/RSPiX/ORANGE/GUI
	mkdir -p $(BINDIR)/RSPiX/ORANGE/Channel
	mkdir -p $(BINDIR)/RSPiX/ORANGE/color
	mkdir -p $(BINDIR)/RSPiX/ORANGE/DirtRect
	mkdir -p $(BINDIR)/RSPiX/ORANGE/File
	mkdir -p $(BINDIR)/RSPiX/ORANGE/QuickMath
	mkdir -p $(BINDIR)/RSPiX/ORANGE/IFF
	mkdir -p $(BINDIR)/RSPiX/ORANGE/ImageTools
	mkdir -p $(BINDIR)/RSPiX/ORANGE/Laymage
	mkdir -p $(BINDIR)/RSPiX/ORANGE/Meter
	mkdir -p $(BINDIR)/RSPiX/ORANGE/MsgBox
	mkdir -p $(BINDIR)/RSPiX/ORANGE/MultiGrid
	mkdir -p $(BINDIR)/RSPiX/ORANGE/Debug
	mkdir -p $(BINDIR)/RSPiX/ORANGE/RString
	mkdir -p $(BINDIR)/RSPiX/ORANGE/Parse
	mkdir -p $(BINDIR)/RSPiX/ORANGE/str
	mkdir -p $(BINDIR)/RSPiX/CYAN/sdl2
	mkdir -p $(BINDIR)/WishPiX/Menu
	mkdir -p $(BINDIR)/WishPiX/Prefs
	mkdir -p $(BINDIR)/WishPiX/ResourceManager
	mkdir -p $(BINDIR)/WishPiX/Spry
	mkdir -p $(BINDIR)/libs

distclean: clean

clean:
	rm -f $(CLEANUP)
	rm -rf $(BINDIR)
	#rm -f $(SRCDIR)/parser/y.tab.c
	#rm -f $(SRCDIR)/parser/lex.yy.c

# end of Makefile ...
