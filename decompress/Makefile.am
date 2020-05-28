# Process this file with automake to produce Makefile.in.
#
# Copyright (C) 2020  Enrico Zini <enrico@enricozini.org>
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

PDWT_SOURCES = \
	PublicDecompWT/DISE/CxRITFile.cpp \
	PublicDecompWT/DISE/CxRITFileHeaderRecords.cpp \
	PublicDecompWT/DISE/CxRITAnnotation.cpp \
	PublicDecompWT/DISE/ErrorHandling.cpp \
	PublicDecompWT/DISE/TimeSpan.cpp \
	PublicDecompWT/DISE/UTCTime.cpp \
	PublicDecompWT/COMP/Src/CBuffer.cpp \
	PublicDecompWT/COMP/Src/CImage.cpp \
	PublicDecompWT/COMP/Src/CQualityMeasures.cpp \
	PublicDecompWT/COMP/T4/Src/CBitBuffer.cpp \
	PublicDecompWT/COMP/T4/Src/CompressT4.cpp \
	PublicDecompWT/COMP/T4/Src/T4Coder.cpp \
	PublicDecompWT/COMP/T4/Src/T4Codes.cpp \
	PublicDecompWT/COMP/T4/Src/T4Decoder.cpp \
	PublicDecompWT/COMP/JPEG/Src/CHOptim.cpp \
	PublicDecompWT/COMP/JPEG/Src/CHcodec.cpp \
	PublicDecompWT/COMP/JPEG/Src/CHufftables.cpp \
	PublicDecompWT/COMP/JPEG/Src/CJBlock.cpp \
	PublicDecompWT/COMP/JPEG/Src/CJPEGDecoder.cpp \
	PublicDecompWT/COMP/JPEG/Src/CJPEGLossLessCoder.cpp \
	PublicDecompWT/COMP/JPEG/Src/CJPEGLossyCoder.cpp \
	PublicDecompWT/COMP/JPEG/Src/CQuantizationTable.cpp \
	PublicDecompWT/COMP/JPEG/Src/CompressJPEG.cpp \
	PublicDecompWT/COMP/WT/Src/CACCoder.cpp \
	PublicDecompWT/COMP/WT/Src/CACDecoder.cpp \
	PublicDecompWT/COMP/WT/Src/CACModel.cpp \
	PublicDecompWT/COMP/WT/Src/CVLCCoder.cpp \
	PublicDecompWT/COMP/WT/Src/CVLCDecoder.cpp \
	PublicDecompWT/COMP/WT/Src/CWBlock.cpp \
	PublicDecompWT/COMP/WT/Src/CWTCoder.cpp \
	PublicDecompWT/COMP/WT/Src/CWTDecoder.cpp \
	PublicDecompWT/COMP/WT/Src/CompressWT.cpp

noinst_LTLIBRARIES = libpublicdecompwt.la

libpublicdecompwt_la_SOURCES = $(PDWT_SOURCES)
libpublicdecompwt_la_CPPFLAGS = \
	-Wno-long-long -pedantic \
	-I$(srcdir)/PublicDecompWT/DISE/ \
	-I$(srcdir)/PublicDecompWT/COMP/Inc/ \
	-I$(srcdir)/PublicDecompWT/COMP/Src/Inc/ \
	-I$(srcdir)/PublicDecompWT/COMP/T4/Inc/ \
	-I$(srcdir)/PublicDecompWT/COMP/JPEG/Inc/ \
	-I$(srcdir)/PublicDecompWT/COMP/WT/Inc/
