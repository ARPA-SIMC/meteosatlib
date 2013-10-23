/*
 * Copyright (C) 2010--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include "xrit.h"
#include "dataset.h"

#include <string>
#include <memory>
#include <cctype>

using namespace std;

namespace msat {
namespace xrit {

GDALDataset* XRITOpen(GDALOpenInfo* info)
{
	// Se if it looks like a XRIT filename
	if (!msat::xrit::isValid(info->pszFilename))
		return NULL;

    // Check if the file name has been tweaked with the request of some
    // alternate dataset for this channel, and cleanup the file name
    string fname = info->pszFilename;
    size_t pos = fname.rfind(':');
    XRITDataset::Effect effect = XRITDataset::PP_NONE;
    if (pos != string::npos && pos > 0 && islower(fname[pos-1]))
    {
        switch (fname[pos-1])
        {
            case 'r': effect = XRITDataset::PP_REFLECTANCE; break;
            case 'a': effect = XRITDataset::PP_SZA; break;
        }
        // Remove the suffix
        fname.erase(pos-1, 1);
    }

    // Create the dataset
    std::auto_ptr<XRITDataset> ds(new XRITDataset(fname, effect));

	// Initialise the dataset
	if (!ds->init()) return NULL;

	return ds.release();
}

}
}

extern "C" {

void GDALRegister_MsatXRIT()
{
	if (! GDAL_CHECK_VERSION("MsatXRIT"))
		return;

	if (GDALGetDriverByName("MsatXRIT") == NULL)
	{
		auto_ptr<GDALDriver> driver(new GDALDriver());
		driver->SetDescription("MsatXRIT");
		driver->SetMetadataItem(GDAL_DMD_LONGNAME, "Meteosat xRIT (via Meteosatlib)");
		//driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
		//driver->SetMetadataItem(GDAL_DMD_EXTENSION, "mem");
		driver->pfnOpen = msat::xrit::XRITOpen;
		GetGDALDriverManager()->RegisterDriver(driver.release());
	}
}

}
