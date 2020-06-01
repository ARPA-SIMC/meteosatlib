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

#include "dataset.h"
#include "gdaltranslate.h"
#include <gdal/vrtdataset.h>
#include <gdal/ogr_spatialref.h>
#include <msat/facts.h>
#include <stdint.h>

using namespace std;

namespace msat {
namespace dataset {

std::string spaceviewWKT(double sublon)
{
        // Also add GDAL projection (see the msg driver they have)
        // Taken from GDAL's msgdataset
        OGRSpatialReference osr;
        osr.SetGEOS(sublon, ORBIT_RADIUS_FOR_GDAL, 0, 0);
        //osr.SetWellKnownGeogCS("WGS84"); // Temporary line to satisfy ERDAS (otherwise the ellips is "unnamed"). Eventually this should become the custom a and b ellips (CGMS).

        // The following are 3 different try-outs for also setting the ellips a and b parameters.
        // We leave them out for now however because this does not work. In gdalwarp, when choosing some
        // specific target SRS, the result is an error message:
        // 
        // ERROR 1: geocentric transformation missing z or ellps
        // ERROR 1: GDALWarperOperation::ComputeSourceWindow() failed because
        // the pfnTransformer failed.
        // 
        // I can't explain the reason for the message at this time (could be a problem in the way the SRS is set here,
        // but also a bug in Proj.4 or GDAL.
        osr.SetGeogCS( NULL, NULL, NULL, 6378169, 295.488065897, NULL, 0, NULL, 0 );

        /*
           oSRS.SetGeogCS( "unnamed ellipse", "unknown", "unnamed", 6378169, 295.488065897, "Greenwich", 0.0);

           if( oSRS.importFromProj4("+proj=geos +h=35785831 +a=6378169 +b=6356583.8") == OGRERR_NONE )
           {
           oSRS.exportToWkt( &(poDS->pszProjection) );
           }
           */

        char* projstring;
        osr.exportToWkt(&projstring);
        string res = projstring;
        OGRFree(projstring);
        return res;
}

void decodeGeotransform(GDALDataset* ds, int& xs, int& ys, double& psx, double& psy)
{
	double gt[6];
	ds->GetGeoTransform(gt);
	
	psx = gt[1];
	psy = -gt[5];

	xs = lrint(-gt[0] / psx);
	ys = lrint(gt[3] / psy);
}

GDALDataset* recode(
		GDALDataset* ds,
		const msat::proj::ImageBox& cropArea,
		const std::string& fileName, GDALDriver* driver,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	int sx = cropArea.bottomRight.column - cropArea.topLeft.column;
	int sy = cropArea.bottomRight.line - cropArea.topLeft.line;
	return recode(ds,
			cropArea, sx, sy,
			fileName, driver,
			opt1, opt2, opt3);
}

GDALDataset* recode(
		GDALDataset* ds,
		int sx, int sy,
		const std::string& fileName, GDALDriver* driver,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	using namespace msat;
	proj::ImageBox all(proj::ImagePoint(0, 0), proj::ImagePoint(ds->GetRasterXSize(), ds->GetRasterYSize()));
	return recode(ds,
			all, sx, sy,
			fileName, driver,
			opt1, opt2, opt3);
}

GDALDataset* recode(
		GDALDataset* ds,
		const msat::proj::ImageBox& cropArea, int sx, int sy,
		const std::string& fileName, GDALDriver* driver,
		const std::string& opt1, const std::string& opt2, const std::string& opt3)
{
	GDALTranslate translate;

	translate.anSrcWin[0] = cropArea.topLeft.column;
	translate.anSrcWin[1] = cropArea.topLeft.line;
	translate.anSrcWin[2] = cropArea.bottomRight.column - cropArea.topLeft.column;
	translate.anSrcWin[3] = cropArea.bottomRight.line - cropArea.topLeft.line;

	GDALDataset* vds = translate.translate(ds);

	char** options = 0;
	if (!opt1.empty()) options = CSLAddString(options, opt1.c_str());
	if (!opt2.empty()) options = CSLAddString(options, opt2.c_str());
	if (!opt3.empty()) options = CSLAddString(options, opt3.c_str());

        GDALDataset* res = (GDALDataset*)GDALCreateCopy(driver, fileName.c_str(), (GDALDatasetH)vds,
                              TRUE, options, NULL, NULL );
                              //TRUE, options, pfnProgress, NULL );

        CSLDestroy(options);
	if (vds != ds)
                GDALClose( (GDALDatasetH) vds );

        return res;
}

//	int InvGeoTransform( double *gt_in, double *gt_out )
CPLErr invertGeoTransform(double* normal, double* inverted)
{
	// From Frank Warmerdam <warmerdam@pobox.com>
	// As posted at http://lists.maptools.org/pipermail/gdal-dev/2004-June/002852.html

	// We assume a 3rd row that is [1 0 0]

	// Compute determinate
	double det = normal[1] * normal[5] - normal[2] * normal[4];

	if (fabs(det) < 0.000000000000001)
	{
		CPLError(CE_Failure, CPLE_AppDefined, "geotransform matix is not invertible");
		return CE_Failure;
	}

	double inv_det = 1.0 / det;

	/* Compute adjoint, and divide by determinate */

	inverted[1] =  normal[5] * inv_det;
	inverted[4] = -normal[4] * inv_det;

	inverted[2] = -normal[2] * inv_det;
	inverted[5] =  normal[1] * inv_det;

	inverted[0] = ( normal[2] * normal[3] - normal[0] * normal[5]) * inv_det;
	inverted[3] = (-normal[1] * normal[3] + normal[0] * normal[4]) * inv_det;

	return CE_None;
}


GeoReferencer::GeoReferencer()
	: ds(0), proj(0), latlon(0), toLatLon(0), fromLatLon(0)
{
}

GeoReferencer::~GeoReferencer()
{
	if (proj) delete proj;
	if (latlon) delete latlon;
	if (toLatLon) delete toLatLon;
	if (fromLatLon) delete fromLatLon;
}

CPLErr GeoReferencer::init(GDALDataset* ds)
{
	this->ds = ds;

	if (ds->GetGeoTransform(geoTransform) != CE_None)
	{
		CPLError(CE_Failure, CPLE_AppDefined, "no geotransform found in input dataset");
		return CE_Failure;
	}

	CPLErr res = invertGeoTransform(geoTransform, invGeoTransform);
	if (res != CE_None) return res;

	const char* projname = ds->GetProjectionRef();
	if (!projname || !projname[0])
	{
		CPLError(CE_Failure, CPLE_AppDefined, "no projection name found in input dataset");
		return CE_Failure;
	}

	projection = projname;

	proj = new OGRSpatialReference(projection.c_str());
	latlon = proj->CloneGeogCS();
	toLatLon = OGRCreateCoordinateTransformation(proj, latlon);
	fromLatLon = OGRCreateCoordinateTransformation(latlon, proj);

	delete proj; proj = 0;
	delete latlon; latlon = 0;
	return CE_None;
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

CPLErr GeoReferencer::projectedToLatlon(double px, double py, double& lat, double& lon)
{
	if (!toLatLon->Transform(1, &px, &py))
	{
		CPLError(CE_Failure, CPLE_AppDefined, "points failed to transform to lat,lon");
		return CE_Failure;
	}

	lat = py;
	lon = px;
	return CE_None;
}

CPLErr GeoReferencer::latlonToProjected(double lat, double lon, double& px, double& py)
{
	if (!fromLatLon->Transform(1, &lon, &lat))
	{
		CPLError(CE_Failure, CPLE_AppDefined, "points failed to transform from lat,lon");
		return CE_Failure;
	}
	py = lat;
	px = lon;
	return CE_None;
}


ProxyDataset::ProxyDataset(GDALDataset& ds)
    : ds(ds)
{
    nRasterXSize = ds.GetRasterXSize();
    nRasterYSize = ds.GetRasterYSize();
}

const char* ProxyDataset::GetDescription() const { return ds.GetDescription(); }
char** ProxyDataset::GetMetadata(const char *pszDomain) { return ds.GetMetadata(pszDomain); }
const char* ProxyDataset::GetMetadataItem(const char *pszName, const char *pszDomain)
{
        return ds.GetMetadataItem(pszName, pszDomain);
}

#if GDAL_VERSION_MAJOR < 3
const char* ProxyDataset::GetProjectionRef(void) { return ds.GetProjectionRef(); }
#else
const char* ProxyDataset::_GetProjectionRef() override {
    return ds.GetProjectionRef();
}
const ProxyDataset::OGRSpatialReference* GetSpatialRef() const override {
    return GetSpatialRefFromOldGetProjectionRef();
}
#endif
CPLErr ProxyDataset::GetGeoTransform(double* d) { return ds.GetGeoTransform(d); }
GDALDriver* ProxyDataset::GetDriver(void) { return ds.GetDriver(); }
char** ProxyDataset::GetFileList(void) { return ds.GetFileList(); }
int ProxyDataset::GetGCPCount() { return ds.GetGCPCount(); }
const char* ProxyDataset::GetGCPProjection() { return ds.GetGCPProjection(); }
const GDAL_GCP* ProxyDataset::GetGCPs() { return ds.GetGCPs(); }

ProxyRasterBand::ProxyRasterBand(GDALDataset& ds, GDALRasterBand& rb, int idx)
    : rb(rb)
{
    poDS = &ds;
    nBand = idx;
    eDataType = rb.GetRasterDataType();

    rb.GetBlockSize(&nBlockXSize, &nBlockYSize);
}

const char* ProxyRasterBand::GetDescription() const { return rb.GetDescription(); }
char** ProxyRasterBand::GetMetadata(const char *pszDomain) { return rb.GetMetadata(pszDomain); }
const char* ProxyRasterBand::GetMetadataItem(const char *pszName, const char *pszDomain)
{
    return rb.GetMetadataItem(pszName, pszDomain);
}

char** ProxyRasterBand::GetCategoryNames() { return rb.GetCategoryNames(); }
double ProxyRasterBand::GetNoDataValue(int *pbSuccess) { return rb.GetNoDataValue(pbSuccess); }
double ProxyRasterBand::GetMinimum(int *pbSuccess) { return rb.GetMinimum(pbSuccess); }
double ProxyRasterBand::GetMaximum(int *pbSuccess) { return rb.GetMaximum(pbSuccess); }
double ProxyRasterBand::GetOffset(int *pbSuccess) { return rb.GetOffset(pbSuccess); }
double ProxyRasterBand::GetScale(int *pbSuccess) { return rb.GetScale(pbSuccess); }
const char* ProxyRasterBand::GetUnitType() { return rb.GetUnitType(); }
GDALColorInterp ProxyRasterBand::GetColorInterpretation() { return rb.GetColorInterpretation(); }
GDALColorTable* ProxyRasterBand::GetColorTable() { return rb.GetColorTable(); }
CPLErr ProxyRasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    return rb.ReadBlock(xblock, yblock, buf);
}

# if 0
CropDataset::CropDataset(GDALDataset* src, int x0, int y0, int width, int height)
        : ProxyDataset(src)
{
        nRasterXSize = width;
        nRasterYSize = height;
}
#endif

CalibratedDataset::CalibratedDataset(GDALDataset& ds)
    : ProxyDataset(ds)
{
    for (int i = 1; i <= ds.GetRasterCount(); ++i)
    {
        GDALRasterBand* rb = ds.GetRasterBand(i);
        // We can only calibrate if the source is an UInt16
        if (rb->GetRasterDataType() == GDT_UInt16)
            SetBand(i, new CalibratedRasterBand(*this, *rb, i));
        else
            SetBand(i, new ProxyRasterBand(*this, *rb, i));
    }
}

CalibratedRasterBand::CalibratedRasterBand(GDALDataset& ds, GDALRasterBand& rb, int idx)
    : ProxyRasterBand(ds, rb, idx)
{
    eDataType = GDT_Float32;
    ofs = rb.GetOffset();
    scale = rb.GetScale();
}

double CalibratedRasterBand::GetMinimum(int *pbSuccess)
{
    double res = rb.GetMinimum(pbSuccess);
    return res * scale + ofs;
}

double CalibratedRasterBand::GetMaximum(int *pbSuccess)
{
    double res = rb.GetMaximum(pbSuccess);
    return res * scale + ofs;
}

double CalibratedRasterBand::GetOffset(int *pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

double CalibratedRasterBand::GetScale(int *pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 1.0;
}

double CalibratedRasterBand::GetNoDataValue(int *pbSuccess)
{
    if (pbSuccess) *pbSuccess = TRUE;
    return 0.0;
}

CPLErr CalibratedRasterBand::IReadBlock(int xblock, int yblock, void *buf)
{
    // Read the raw data
    uint16_t raw[nBlockXSize * nBlockYSize];
    if (ProxyRasterBand::IReadBlock(xblock, yblock, raw) == CE_Failure)
        return CE_Failure;

    // Compute reflectances
    float* dest = (float*) buf;
    for (int i = 0; i < nBlockXSize * nBlockYSize; ++i)
    {
        // Calibrate from counts to radiance
        dest[i] = raw[i] * scale + ofs;
    }

    return CE_None;
}

}
}
