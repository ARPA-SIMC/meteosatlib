#ifndef MSAT_GDALDRIVER_DATASET_H
#define MSAT_GDALDRIVER_DATASET_H

/*
 * Copyright (C) 2010  ARPAE-SIMC <urpsim@arpae.it>
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

#include <msat/gdal/clean_gdal_priv.h>
#include <msat/gdal/points.h>

struct OGRSpatialReference;
struct OGRCoordinateTransformation;

namespace msat {
namespace dataset {

/// Get the WKT description for the Spaceview projection
std::string spaceviewWKT(double sublon = 0.0);

/// Set the given spatial reference to Spaceview
void set_spaceview(OGRSpatialReference& osr, double sublon=0.0);

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
    const char* GetDescription() const override;
    char** GetMetadata(const char *pszDomain="") override;
    const char* GetMetadataItem(const char *pszName, const char *pszDomain="") override;

    // GDALDataset
#if GDAL_VERSION_MAJOR < 3
    virtual const char* GetProjectionRef(void) override;
#else
    const OGRSpatialReference* GetSpatialRef() const override;
#endif
    CPLErr GetGeoTransform(double*) override;
    GDALDriver* GetDriver(void) override;
    char** GetFileList(void) override;
    int GetGCPCount() override;
    virtual const char* GetGCPProjection();
    const GDAL_GCP* GetGCPs() override;
};

class ProxyRasterBand : public GDALRasterBand
{
protected:
    GDALRasterBand& rb;

public:
    ProxyRasterBand(GDALDataset& ds, GDALRasterBand& rb, int idx);

    // GDALMajorObject
    const char* GetDescription() const override;
    char** GetMetadata(const char *pszDomain="") override;
    const char* GetMetadataItem(const char *pszName, const char *pszDomain="") override;

    // GDALRasterBand
    char** GetCategoryNames() override;
    double GetNoDataValue(int *pbSuccess=NULL) override;
    double GetMinimum(int *pbSuccess=NULL) override;
    double GetMaximum(int *pbSuccess=NULL) override;
    double GetOffset(int *pbSuccess=NULL) override;
    double GetScale(int *pbSuccess=NULL) override;
    const char* GetUnitType() override;
    GDALColorInterp GetColorInterpretation() override;
    GDALColorTable* GetColorTable() override;
    CPLErr IReadBlock(int xblock, int yblock, void *buf) override;
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

    double GetMinimum(int *pbSuccess=NULL) override;
    double GetMaximum(int *pbSuccess=NULL) override;
    double GetOffset(int *pbSuccess=NULL) override;
    double GetScale(int *pbSuccess=NULL) override;
    double GetNoDataValue(int *pbSuccess=NULL) override;

    CPLErr IReadBlock(int xblock, int yblock, void *buf) override;
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
