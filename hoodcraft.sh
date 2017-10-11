#!/bin/bash -ex
# hoodcraft
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
#
# This runs all of the hood making tools on a set of images to make the
# graphic files for a new hood. You should copy this file and the
# "tools" directory to the game install directory after running
# "make tools".

mkdir -p "res/hoods/$1"
cp "${@:2}" "res/hoods/$1"
tools/saktool e res/hoods/homer.sak homer
for img in emptybar emptybarselected fullbar fullbarselected topbar xraymask
do
	cp homer/hoods/homer/homer.$img.bmp "res/hoods/$1/$1.$img.bmp"
done
python2 tools/quantiser.py "res/hoods/$1/"*
for fname in "res/hoods/$1/"*
do
	python2 tools/ppalmap.py "$fname" tools/postal.ppal "$fname"
done
for fname in "res/hoods/$1/"*.png
do
	python2 tools/png2spry.py "$fname" "res/hoods/$1/$1.bmp" "${fname::-3}"say
done
python2 tools/paldump.py homer/hoods/homer/homer.bmp "homer.pal"
python2 tools/paldump.py "res/hoods/$1/$1.bmp" "$1.pal"
for al in ambient spot
do
	tools/alphagen homer.pal homer/hoods/homer/homer.$al.alpha "$1.pal" "res/hoods/$1/$1.$al.alpha"
done
tools/malphagen "res/hoods/$1/$1.bmp" "res/hoods/$1/$1.transparency.multialpha"
rm -r homer homer.pal "$1.pal" "res/hoods/$1/"*.png
