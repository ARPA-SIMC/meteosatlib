//---------------------------------------------------------------------------
//
//  File        :   ExportNetCDF.cpp
//  Description :   Export data from an ImageData into a NetCDF file
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  Source      :   derived from SAFH5CT2NetCDF.cpp by Le Duc, as modified by
//                  Francesca Di Giuseppe and from XRIT2Grib.cpp by Graziano
//                  Giuliani (Lamma Regione Toscana)
//  RCS ID      :   $Id: /local/meteosatlib/conv/ExportNetCDF.h 1746 2006-09-19T13:53:30.396225Z enrico  $
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//---------------------------------------------------------------------------

#ifndef MLIB_EXPORT_IMAGE_H
#define MLIB_EXPORT_IMAGE_H

#include <msat/Image.h>
#include <string>
#include <memory>

namespace msat {

/// Export data from an ImageData to an image file
void ExportImage(const Image& img, const std::string& fileName);

/// Display an image on screen
void DisplayImage(const Image& img);

// Use a factory method, so that we don't have to include the GRIB headers here
std::auto_ptr<ImageConsumer> createImageExporter(const std::string& format = std::string("jpg"));

// Use a factory method, so that we don't have to include the GRIB headers here
std::auto_ptr<ImageConsumer> createImageDisplayer();

}

#endif

// vim:set ts=2 sw=2:
