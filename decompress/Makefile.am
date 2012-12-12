# Process this file with automake to produce Makefile.in.
#
# Copyright (C) 2005, 2006  Deneys S. Maartens  <dsm@tlabs.ac.za>
# Copyright (C) 2006--2012  Enrico Zini <enrico@enricozini.org>
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

OUR_SOURCES = \
    LICENSE \
    patch_EUMETSAT_2.02 \
    patch_EUMETSAT_2.04 \
    patch_EUMETSAT_2.06 \
    PublicDecompWT.sh

# Also create symbolic links if an out-of-source build
all: \
    LICENSE \
    patch_EUMETSAT_2.02 \
    patch_EUMETSAT_2.04 \
    patch_EUMETSAT_2.06 \
    PublicDecompWT.sh
	if test $(builddir) != $(srcdir) ; then \
	    for file in $(OUR_SOURCES); do \
	        rm -f $$file; \
	        $(LN_S) $(srcdir)/$$file . ; \
	    done; \
	fi
	test -e PublicDecompWT.zip || $(LN_S) $(srcdir)/PublicDecompWT.zip .
	./PublicDecompWT.sh

dist_noinst_DATA = $(OUR_SOURCES)

distclean-local:
	rm -rf COMP DISE
	rm -f pdwt.patched
