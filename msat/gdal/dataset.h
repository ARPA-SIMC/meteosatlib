#ifndef MSAT_GDALDRIVER_DATASET_H
#define MSAT_GDALDRIVER_DATASET_H

/*
 * Copyright (C) 2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include <gdal/gdal_priv.h>
#include "points.h"

struct OGRSpatialReference;
struct OGRCoordinateTransformation;

namespace msat {
namespace dataset {

/// Get the WKT description for the Spaceview projection
std::string spaceviewWKT(double sublon = 0.0);

/// Decode a geotransformation matrix to the image offset and scale 
void decodeGeotransform(GDALDataset* ds, int& xs, int& ys, double& psx, double& psy);

/// Recode the raster band 'ridx' of the dataset, using the given crop area and driver
GDALDataset* recode(GDALDataset* ds,
	    const msat::proj::ImageBox& cropArea,
	    const std::string& fileName, GDALDriver* driver,
	    const std::string& opt1 = std::string(),
	    const std::string& opt2 = std::string(),
	    const std::string& opt3 = std::string());

/// Recode the raster band 'ridx' of the dataset, using the given target size and driver
GDALDataset* recode(GDALDataset* ds,
	    int sx, int sy,
	    const std::string& fileName, GDALDriver* driver,
	    const std::string& opt1 = std::string(),
	    const std::string& opt2 = std::string(),
	    const std::string& opt3 = std::string());

/// Recode the raster band 'ridx' of the dataset, using the given crop area, target size and driver
GDALDataset* recode(GDALDataset* ds,
	    const msat::proj::ImageBox& cropArea, int sx, int sy,
	    const std::string& fileName, GDALDriver* driver,
	    const std::string& opt1 = std::string(),
	    const std::string& opt2 = std::string(),
	    const std::string& opt3 = std::string());

CPLErr invertGeoTransform(double* normal, double* inverted);

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
	GeoReferencer();
	~GeoReferencer();

	CPLErr init(GDALDataset* ds);

	void pixelToProjected(int x, int y, double& px, double& py) const;
	void projectedToPixel(double px, double py, int& x, int& y) const;

	CPLErr projectedToLatlon(double px, double py, double& lat, double& lon);
	CPLErr latlonToProjected(double lat, double lon, double& px, double& py);

	CPLErr pixelToLatlon(int x, int y, double& lat, double& lon)
	{
		double px, py;
		pixelToProjected(x, y, px, py);
		return projectedToLatlon(px, py, lat, lon);
	}
	CPLErr latlonToPixel(double lat, double lon, int& x, int& y)
	{
		double px, py;
		CPLErr res = latlonToProjected(lat, lon, px, py);
		if (res != CE_None) return res;
		projectedToPixel(px, py, x, y);
		return CE_None;
	}
};

/**
 * Proxy all virtual methods to another dataset.
 *
 * This is used as a base class for datasets that would reimplement some
 * methonds to change something but not everything.
 *
 * This currently only works with read methods.
 */
class ProxyDataset : public GDALDataset
{
protected:
    GDALDataset& ds;

public:
    ProxyDataset(GDALDataset& ds);

    // GDALMajorObject
    virtual const char* GetDescription() const;
    virtual char** GetMetadata(const char *pszDomain="");
    virtual const char* GetMetadataItem(const char *pszName, const char *pszDomain="");

    // GDALDataset
    virtual const char* GetProjectionRef(void);
    virtual CPLErr GetGeoTransform(double*);
    virtual GDALDriver* GetDriver(void);
    virtual char** GetFileList(void);
    virtual int GetGCPCount();
    virtual const char* GetGCPProjection();
    virtual const GDAL_GCP* GetGCPs();
};

class ProxyRasterBand : public GDALRasterBand
{
protected:
    GDALRasterBand& rb;

public:
    ProxyRasterBand(GDALDataset& ds, GDALRasterBand& rb, int idx);

    // GDALMajorObject
    virtual const char* GetDescription() const;
    virtual char** GetMetadata(const char *pszDomain="");
    virtual const char* GetMetadataItem(const char *pszName, const char *pszDomain="");

    // GDALRasterBand
    virtual char** GetCategoryNames();
    virtual double GetNoDataValue(int *pbSuccess=NULL);
    virtual double GetMinimum(int *pbSuccess=NULL);
    virtual double GetMaximum(int *pbSuccess=NULL);
    virtual double GetOffset(int *pbSuccess=NULL);
    virtual double GetScale(int *pbSuccess=NULL);
    virtual const char* GetUnitType();
    virtual GDALColorInterp GetColorInterpretation();
    virtual GDALColorTable* GetColorTable();
    virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);
};

struct CalibratedDataset : public ProxyDataset
{
    CalibratedDataset(GDALDataset& ds);
};

struct CalibratedRasterBand : public ProxyRasterBand
{
    double ofs;
    double scale;

    CalibratedRasterBand(GDALDataset& ds, GDALRasterBand& rb, int idx);

    double GetMinimum(int *pbSuccess=NULL);
    double GetMaximum(int *pbSuccess=NULL);
    double GetOffset(int *pbSuccess=NULL);
    double GetScale(int *pbSuccess=NULL);
    double GetNoDataValue(int *pbSuccess=NULL);

    virtual CPLErr IReadBlock(int xblock, int yblock, void *buf);
};

#if 0
class CropDataset : public ProxyDataset
{
public:
        CropDataset(GDALDataset* src, int x0, int y0, int width, int height);
};
#endif

}
}

// vim:set sw=2:
#endif
