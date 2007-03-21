//---------------------------------------------------------------------------
//
//  File        :   ExportGDAL.h
//  Description :   Export a msat::Image using GDAL
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
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

#ifndef MLIB_GDALEXPORT_H
#define MLIB_GDALEXPORT_H

#include <msat/Image.h>
#include <string>
#include <memory>

namespace msat {

/// Export data from an ImageData into a GRIB_FILE
//void ExportGDAL(const Image& img, GRIB_FILE& gf);

// Use a factory method, so that we don't have to include the GRIB headers here
std::auto_ptr<ImageConsumer> createGDALExporter();

}

#endif

// vim:set ts=2 sw=2:
