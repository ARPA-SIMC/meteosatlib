/*
 * test-utils - Test utils for meteosatlib GDAL bindings
 *
 * Copyright (C) 2005--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
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
#ifndef GDAL_TEST_UTILS_H
#define GDAL_TEST_UTILS_H

#include "../test-utils.h"
#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include <gdal_priv.h>
#include <msat/facts.h>
#include <memory>
#include <cstdlib>

class OGRSpatialReference;
class OGRCoordinateTransformation;

#define FOR_DRIVER(name) do { \
       if (!hasDriver(name)) { \
               throw tut::ignored(std::string("GDAL driver ") + name + " not available"); \
       } \
} while (0)

namespace tut {

bool hasDriver(const std::string& name);

std::auto_ptr<GDALDataset> openro(const char* name);

static inline int32_t readInt32(GDALRasterBand* rb, int x, int y)
{
	int32_t res;
	rb->RasterIO(GF_Read, x, y, 1, 1, &res, 1, 1, GDT_Int32, 0, 0);
	return res;
}

static inline float readFloat32(GDALRasterBand* rb, int x, int y)
{
	float res;
	rb->RasterIO(GF_Read, x, y, 1, 1, &res, 1, 1, GDT_Float32, 0, 0);
	return res;
}

/*
struct TempRWDataset : public TempTestFile
{
	msat::Image* ds;
	TempRWDataset(const std::string& pathname, bool leave = false);
	~TempRWDataset();
};
*/

/*
// Instantiate this when you want gdal initialised
struct GDALInit
{
	GDALInit();
};
*/

std::auto_ptr<GDALDataset> recode(GDALDataset* ds, bool leaveFile,
	const char* driver,
	const std::string& opt1 = std::string(),
	const std::string& opt2 = std::string(),
	const std::string& opt3 = std::string());

std::auto_ptr<GDALDataset> recode(GDALDataset* ds, const msat::proj::ImageBox& cropArea, bool leaveFile,
	const char* driver,
	const std::string& opt1 = std::string(),
	const std::string& opt2 = std::string(),
	const std::string& opt3 = std::string());

class GeoReferencer
{
protected:
	GDALDataset* ds;
	std::string projection;
	double geoTransform[6];
	double invGeoTransform[6];
	OGRSpatialReference* proj;
	OGRSpatialReference* latlon;
	OGRCoordinateTransformation* toLatLon;
	OGRCoordinateTransformation* fromLatLon;

public:
	GeoReferencer(GDALDataset* ds);
	~GeoReferencer();

	void pixelToProjected(int x, int y, double& px, double& py) const;
	void projectedToPixel(double px, double py, int& x, int& y) const;

	void projectedToLatlon(double px, double py, double& lat, double& lon);
	void latlonToProjected(double lat, double lon, double& px, double& py);

	void pixelToLatlon(int x, int y, double& lat, double& lon)
	{
		double px, py;
		pixelToProjected(x, y, px, py);
		projectedToLatlon(px, py, lat, lon);
	}
	void latlonToPixel(double lat, double lon, int& x, int& y)
	{
		double px, py;
		latlonToProjected(lat, lon, px, py);
		projectedToPixel(px, py, x, y);
	}
};


#if 0
std::auto_ptr<msat::Image> recodeThroughGrib(msat::Image& img, bool leaveFile = false);
std::auto_ptr<msat::Image> recodeThroughNetCDF(msat::Image& img, bool leaveFile = false);
std::auto_ptr<msat::Image> recodeThroughNetCDF24(msat::Image& img, bool leaveFile = false);
#endif

struct ImportTest
{
	const char* driver;
	const char* testfile;

	ImportTest(const char* driver, const char* testfile)
		: driver(driver), testfile(testfile) {}

	virtual void checkFullImageData(GDALDataset* dataset) = 0;
	virtual void checkCroppedImageData(GDALDataset* dataset) = 0;

	std::auto_ptr<GDALDataset> openDS()
	{
		FOR_DRIVER(driver);
		return openro(testfile);
	}

	std::auto_ptr<GDALDataset> openPlain()
	{
		FOR_DRIVER(driver);
		std::auto_ptr<GDALDataset> dataset = openro(testfile);
		checkFullImageData(dataset.get());
		return dataset;
	}

	std::auto_ptr<GDALDataset> openRecoded(const char* other, bool leaveFile=false, const std::string& opt1 = std::string())
	{
		FOR_DRIVER(driver);
		FOR_DRIVER(other);
		std::auto_ptr<GDALDataset> dataset0 = openro(testfile);
		std::auto_ptr<GDALDataset> dataset1 = recode(dataset0.get(), leaveFile, other, opt1);
		checkFullImageData(dataset1.get());
		return dataset1;
	}

	std::auto_ptr<GDALDataset> openRecodedCropped(const msat::proj::ImageBox& cropArea, const char* other, bool leaveFile=false, const std::string& opt1 = std::string())
	{
		FOR_DRIVER(driver);
		FOR_DRIVER(other);
		std::auto_ptr<GDALDataset> dataset0 = openro(testfile);
		std::auto_ptr<GDALDataset> dataset1 = recode(dataset0.get(), cropArea, leaveFile, other, opt1);
		checkCroppedImageData(dataset1.get());
		return dataset1;
	}
};

}
#endif
