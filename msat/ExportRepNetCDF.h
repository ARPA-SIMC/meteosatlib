//---------------------------------------------------------------------------
//
//  File        :   ExportRepNetCDF.h
//  Description :   Export data from an ImageData into a Met_Reproj-style NetCDF file
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

#ifndef MLIB_EXPORT_REP_NETCDF_H
#define MLIB_EXPORT_REP_NETCDF_H

#include <msat/Image.h>
#include <string>
#include <memory>

namespace msat {

/**
 * Export data from an ImageData to a Met_Reproj-style NetCDF file.
 *
 * Note that the Met_Reproj-style NetCDF format can only store regular
 * latitude-longitude grids.
 */
void ExportRepNetCDF(const Image& img, const std::string& fileName);

// Create a GDT NetCDF exporter
std::auto_ptr<ImageConsumer> createRepNetCDFExporter();

}

#endif

// vim:set ts=2 sw=2:
