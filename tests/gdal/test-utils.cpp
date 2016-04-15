/*
 * test-utils - Test utils for meteosatlib GDAL bindings
 *
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "test-utils.h"

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <msat/gdal/dataset.h>
/*
#include <msat/gdalutils.h>
#include <msat/GRIB.h>
#include <msat/XRIT.h>
#include <msat/NetCDF.h>
#include <msat/NetCDF24.h>
#include <msat/SAFH5.h>
*/
#include <stdexcept>

using namespace std;

namespace {

/*
 * Error handling
 */
class gdalexception : public std::exception
{
	std::string msg;

public:
	gdalexception() throw ();
	gdalexception(CPLErr eErrClass, int err_no, const char *msg) throw ();
	virtual ~gdalexception() throw () {}

	virtual const char* what() const throw() { return msg.c_str(); }
};

gdalexception::gdalexception() throw () : msg(CPLGetLastErrorMsg())
{
}

gdalexception::gdalexception(CPLErr eErrClass, int err_no, const char *msg) throw ()
{
	stringstream fullmsg;
	fullmsg << "GDAL ";
	switch (eErrClass)
	{
		case CE_None: fullmsg << "(no error)"; break;
		case CE_Debug: fullmsg << "debug"; break;
		case CE_Warning: fullmsg << "warning"; break;
		case CE_Failure: fullmsg << "failure"; break;
		case CE_Fatal: fullmsg << "fatal"; break;
		default: fullmsg << "(unknown)"; break;
	}
	fullmsg << " error " << err_no << ": " << msg;
	this->msg = fullmsg.str();
}


static void throwExceptionsOnGDALErrors(CPLErr eErrClass, int err_no, const char *msg)
{
		throw gdalexception(eErrClass, err_no, msg);
}

struct GDALInit
{
	GDALInit()
	{
		GDALAllRegister();
		CPLSetErrorHandler(throwExceptionsOnGDALErrors);
	}
};

GDALInit gdalInit;

}

namespace tut {
/*
GDALInit::GDALInit()
{
	static bool initialised = false;
	if (!initialised)
	{
		GDALRegister_MsatGRIB();
		GDALRegister_XRIT();
		GDALRegister_MsatNetCDF();
		GDALRegister_MsatNetCDF24();
		GDALRegister_SAFHDF5();
		initGDALIfNeeded();
		initialised = true;
	}
}
*/

bool hasDriver(const std::string& name)
{
	GDALDriverManager* dm = GetGDALDriverManager();
	return dm->GetDriverByName(name.c_str()) != 0;
}

unique_ptr<GDALDataset> openro(const char* name)
{
    return unique_ptr<GDALDataset>((GDALDataset*)GDALOpen(name, GA_ReadOnly));
}

/*
TempRWDataset::TempRWDataset(const std::string& pathname, bool leave)
	: TempTestFile(leave), ds(0)
{
	// copy
	if (::system(("cp " + pathname + " " + name()).c_str()) != 0)
		throw std::runtime_error("cannot copy " + pathname + " to " + name());
	
	// open
	ds = dynamic_cast<msat::Image*>((GDALDataset*)GDALOpen(name().c_str(), GA_Update));
}

TempRWDataset::~TempRWDataset()
{
	if (ds) delete ds;
}
*/

std::unique_ptr<GDALDataset> recode(GDALDataset* ds, bool leaveFile,
        const char* driverName, const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
    GDALDriver* driver = (GDALDriver*)GDALGetDriverByName(driverName);
    TempTestFile tf(leaveFile);
    unique_ptr<GDALDataset> dataset((GDALDataset*)GDALCreateCopy(driver, tf.name().c_str(), ds,
                TRUE, NULL,
                GDALDummyProgress, NULL));
    return dataset;
}

std::unique_ptr<GDALDataset> recode(GDALDataset* ds, const msat::proj::ImageBox& cropArea, bool leaveFile,
        const char* driverName, const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
    TempTestFile file(leaveFile);

    GDALDriver* driver = GetGDALDriverManager()->GetDriverByName(driverName);
    ensure(driver != 0);

    msat::dataset::recode(ds, cropArea, file.name(), driver, opt1, opt2, opt3);

    return openro(file.name().c_str());
}

#if 0
std::unique_ptr<msat::Image> recodeThroughGrib(msat::Image& img, bool leaveFile)
{
    using namespace msat;

    TempTestFile file(leaveFile);

    // Write the grib
    GRIB_FILE gf;
    if (gf.OpenWrite(file.name()) != 0)
        return std::unique_ptr<Image>();
    ExportGRIB(img, gf);
    if (gf.Close() != 0)
        return std::unique_ptr<Image>();

    // Reread the grib
    ImageVector imgs(*createGribImporter(file.name()));
    if (imgs.empty())
        return std::unique_ptr<Image>();
    return imgs.shift();
}

std::unique_ptr<msat::Image> recodeThroughNetCDF(msat::Image& img, bool leaveFile)
{
    using namespace msat;

    TempTestFile file(leaveFile);

    // Write the NetCDF24
    ExportNetCDF(img, file.name());

    // Reread the NetCDF24
    std::unique_ptr<ImageImporter> imp(createNetCDFImporter(file.name()));
    ImageVector imgs;
    imp->read(imgs);
    if (imgs.empty())
        return std::unique_ptr<Image>();
    return imgs.shift();
}

std::unique_ptr<msat::Image> recodeThroughNetCDF24(msat::Image& img, bool leaveFile)
{
    using namespace msat;

    TempTestFile file(leaveFile);

    // Write the NetCDF24
    ExportNetCDF24(img, file.name());

    // Reread the NetCDF24
    std::unique_ptr<ImageImporter> imp(createNetCDF24Importer(file.name()));
    ImageVector imgs;
    imp->read(imgs);
    if (imgs.empty())
        return std::unique_ptr<Image>();
    return imgs.shift();
}
#endif

/**
 * Invert a standard 3x2 "GeoTransform" style matrix with an
 * implicit [1 0 0] final row.
 */
void invertGeoTransform(double* normal, double* inverted);

void invertGeoTransform(double* normal, double* inverted)
{
	// From Frank Warmerdam <warmerdam@pobox.com>
	// As posted at http://lists.maptools.org/pipermail/gdal-dev/2004-June/002852.html

	// We assume a 3rd row that is [1 0 0]

	// Compute determinate
	double det = normal[1] * normal[5] - normal[2] * normal[4];

	if (fabs(det) < 0.000000000000001)
	{
		stringstream str;
		str << "geotransform matrix "
			<< normal[0] << ", " << normal[1] << ", " << normal[2] << "; "
			<< normal[3] << ", " << normal[4] << ", " << normal[5]
			<< " is not invertible";
		throw std::runtime_error(str.str());
	}

	double inv_det = 1.0 / det;

	/* Compute adjoint, and divide by determinate */

	inverted[1] =  normal[5] * inv_det;
	inverted[4] = -normal[4] * inv_det;

	inverted[2] = -normal[2] * inv_det;
	inverted[5] =  normal[1] * inv_det;

	inverted[0] = ( normal[2] * normal[3] - normal[0] * normal[5]) * inv_det;
	inverted[3] = (-normal[1] * normal[3] + normal[0] * normal[4]) * inv_det;
}

GeoReferencer::GeoReferencer(GDALDataset* ds)
	: ds(ds), proj(0), latlon(0), toLatLon(0), fromLatLon(0)
{
	if (ds->GetGeoTransform(geoTransform) != CE_None)
		throw std::runtime_error("no geotransform found in input dataset");

	invertGeoTransform(geoTransform, invGeoTransform);

	const char* projname = ds->GetProjectionRef();
	if (!projname || !projname[0])
		throw std::runtime_error("no projection name found in input dataset");

	projection = projname;

	proj = new OGRSpatialReference(projection.c_str());
	latlon = proj->CloneGeogCS();
	toLatLon = OGRCreateCoordinateTransformation(proj, latlon);
	fromLatLon = OGRCreateCoordinateTransformation(latlon, proj);

	delete proj; proj = 0;
	delete latlon; latlon = 0;
}

GeoReferencer::~GeoReferencer()
{
	if (proj) delete proj;
	if (latlon) delete latlon;
	if (toLatLon) delete toLatLon;
	if (fromLatLon) delete fromLatLon;
}

void GeoReferencer::pixelToProjected(int x, int y, double& px, double& py) const
{
	px = geoTransform[0] + geoTransform[1] * x
		+ geoTransform[2] * y;
	py = geoTransform[3] + geoTransform[4] * x
		+ geoTransform[5] * y;
}

void GeoReferencer::projectedToPixel(double px, double py, int& x, int& y) const
{
	x = lrint(invGeoTransform[0] + px * invGeoTransform[1] + py * invGeoTransform[2]);
	y = lrint(invGeoTransform[3] + px * invGeoTransform[4] + py * invGeoTransform[5]);
}

void GeoReferencer::projectedToLatlon(double px, double py, double& lat, double& lon)
{
	if (!toLatLon->Transform(1, &px, &py))
		throw std::runtime_error("points failed to transform to lat,lon");
	lat = py;
	lon = px;
}

void GeoReferencer::latlonToProjected(double lat, double lon, double& px, double& py)
{
	if (!fromLatLon->Transform(1, &lon, &lat))
		throw std::runtime_error("points failed to transform from lat,lon");
	py = lat;
	px = lon;
}


}
