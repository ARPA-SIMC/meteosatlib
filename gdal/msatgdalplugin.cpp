/*
 * msatgdalplugin - GDAL Plugin for meteosatlib
 *
 * Copyright (C) 2007  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Enrico Zini <enrico@enricozini.org>
 */

#include "safh5/safh5.h"
#include "xrit/xrit.h"
#include "netcdf/netcdf.h"
#include "netcdf/netcdf24.h"
#include "grib/grib.h"

extern "C" {
void GDALRegister_Meteosatlib(void)
{
	GDALRegister_MsatXRIT();
	GDALRegister_MsatSAFH5();
	GDALRegister_MsatNetCDF();
	GDALRegister_MsatNetCDF24();
	GDALRegister_MsatGRIB();
}
}

// vim:set ts=2 sw=2:
