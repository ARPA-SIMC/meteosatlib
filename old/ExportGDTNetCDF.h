//---------------------------------------------------------------------------
//
//  File        :   ExportGDTNetCDF.h
//  Description :   Export data from an ImageData into a NetCDF file usign GDT conventions
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  GDT link    :   http://www-pcmdi.llnl.gov/drach/GDT_convention.html
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

#ifndef MLIB_EXPORT_GDT_NETCDF_H
#define MLIB_EXPORT_GDT_NETCDF_H

#include <msat/Image.h>
#include <string>
#include <memory>

namespace msat {

/*
struct GDTExportOptions
{
	double latmin, latmax, lonmin, lonmax;
	double latstep, lonstep;

	GDTExportOptions():
		latmin(-90), latmax(90), lonmin(-180), lonmax(180),
		latstep(0.5), lonstep(0.5) {}

	int xsize() const { return (int)floor((lonmax - lonmin) / lonstep); }
	int ysize() const { return (int)floor((latmax - latmin) / latstep); }
	double lon(int x) const { return lonmin + lonstep*x; }
	double lat(int y) const { return latmin + latstep*y; }
};
*/

/**
 * Export data from an ImageData to a GDT NetCDF file.
 *
 * Note that the GDT NetCDF format can only store regular latitude-longitude
 * grids.
 */
void ExportGDTNetCDF(/*const GDTExportOptions& opts,*/ const Image& img, const std::string& fileName);

// Create a GDT NetCDF exporter
std::auto_ptr<ImageConsumer> createGDTNetCDFExporter(/*const GDTExportOptions& opts*/);

}

#endif

// vim:set ts=2 sw=2:
