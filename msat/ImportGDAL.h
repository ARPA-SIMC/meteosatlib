#ifndef MSATLIB_GDAL_IMPORT_H
#define MSATLIB_GDAL_IMPORT_H

//---------------------------------------------------------------------------
//
//  File        :   ImportGDAL.h
//  Description :   Import a msat::Image using GDAL
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

#include <msat/Image.h>
#include <memory>

namespace msat {

bool isGDAL(const std::string& filename);
std::auto_ptr<ImageImporter> createGDALImporter(const std::string& filename);

}

// vim:set sw=2:
#endif
