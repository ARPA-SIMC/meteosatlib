#! /bin/sh -ex
# $Id$
# Copyright (C) 2005 Deneys S. Maartens <dsm@tlabs.ac.za>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
#
# Use this script to generate the configure script required to configure
# and build this package.
#
# This script needs to be called from the top level of the source tree,
# where configure.ac can be found, for example:
#
#   ./config/autogen.sh
#
# You need to have automake, autoconf and (possibly) libtool installed.

# find config directory
cfgaux=`dirname $0`
test $cfgaux = . && cfgaux=`pwd`
cfgaux=`basename $cfgaux`

aclocal -I $cfgaux
libtoolize --force
autoheader
automake --add-missing
autoconf

# -fin-
