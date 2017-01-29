#!/bin/sh
# POSTAL Makeit
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
bindir="./bin"
bindir1="64"
bindir2="-release"
for i in "$@"
do
	if [ "$i" == "x86" ]
	then
		before="linux_x86=\"1\""
		bindir1="32"
	fi
	if [ "$i" == "debug" ]
	then
		after="debug"
		bindir2="-debug"
	fi
	if [ "$i" == "clean" ]
	then
		remove="yes"
	fi
done

if [ "$remove" == "yes" ]
then
	after="clean"
fi

bindir="$bindir$bindir1$bindir2"
eval "BINDIR=$bindir $before make -e $after"
