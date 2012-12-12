#ifndef MSAT_GDAL_UTILS_H
#define MSAT_GDAL_UTILS_H

/*
 * gdalutils - Utility functions to work with GDAL
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

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <stdexcept>
#include <string>
#include <set>

class OGRSpatialReference;
class OGRCoordinateTransformation;

/*
 * Library init
 */
void initGDALIfNeeded();

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

inline void gdalChecked(CPLErr err)
{
	// initGDALIfNeeded sets gdal to automatically throw exceptions on errors,
	// so we don't need to do anything.
#if 0
	if (err)
		throw gdalexception();
#endif
}


/*
 * Templates to get GDALDataType values from C++ types
 */
template<typename Sample>
static inline GDALDataType gdalType() { throw std::runtime_error("requested GDALDataType for unknown C++ type"); }
template<> inline GDALDataType gdalType<signed char>() { return GDT_Byte; }
template<> inline GDALDataType gdalType<unsigned char>() { return GDT_Byte; }
template<> inline GDALDataType gdalType<uint16_t>() { return GDT_UInt16; }
template<> inline GDALDataType gdalType<int16_t>() { return GDT_Int16; }
template<> inline GDALDataType gdalType<uint32_t>() { return GDT_UInt32; }
template<> inline GDALDataType gdalType<int32_t>() { return GDT_Int32; }
template<> inline GDALDataType gdalType<float>() { return GDT_Float32; }
template<> inline GDALDataType gdalType<double>() { return GDT_Float64; }
template<typename T>
GDALDataType gdalType(const T& sample) { return gdalType<T>(); }

template<typename T>
struct GdalBuffer
{
	int sx, sy;
	T* data;

	GdalBuffer(GDALRasterBand& rb) : sx(rb.GetXSize()), sy(rb.GetYSize()), data(new T[sx*sy]) {}
	GdalBuffer(int sx, int sy) : sx(sx), sy(sy), data(new T[sx*sy]) {}
	~GdalBuffer() { delete[] data; }

	GDALDataType type() const { return gdalType<T>(); }

	void read(GDALRasterBand& rb, int x = 0, int y = 0)
	{
		gdalChecked(rb.RasterIO(GF_Read, x, y, sx, sy, data, sx, sy, type(), 0, 0));
	}
	void read(GDALRasterBand& rb, int x, int y, int width, int height)
	{
		gdalChecked(rb.RasterIO(GF_Read, x, y, width, height, data, sx, sy, type(), 0, 0));
	}
	T& get(int x, int y) const { return data[y*sx+x]; }
};

int bppOfGDALDataType(GDALDataType type);

/**
 * Invert a standard 3x2 "GeoTransform" style matrix with an
 * implicit [1 0 0] final row.
 */
void invertGeoTransform(double* normal, double* inverted);

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

std::string getGeosWKT(double sublon = 0.0);

struct TypeChooser : public std::set<GDALDataType>
{
	TypeChooser() {}
	TypeChooser(GDALDriver& driver) { init(driver); }
	~TypeChooser() {}

	void init(GDALDriver& driver);

	/// Given a data type, return the bigger type that can contain it.
	/// If Float64 is given as input, and it's not supported in the driver, an
	/// exception is raised
	GDALDataType choose(GDALRasterBand& rb);

protected:
	GDALDataType choose(GDALRasterBand& rb, GDALDataType type);
};

// vim:set ts=2 sw=2:
#endif
