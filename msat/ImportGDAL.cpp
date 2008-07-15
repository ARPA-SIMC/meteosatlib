//---------------------------------------------------------------------------
//
//  File        :   ImportGDAL.cpp
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
#include "ImportGDAL.h"
#include <msat/Image.h>
#include <msat/Progress.h>

#include <gdal/gdal_priv.h>
#include <gdal/ogr_spatialref.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <limits>

#if 0
#include <cstdio>
#include <cmath>
#include <cstdlib>

#include "config.h"

#include <msat/ImportUtils.h>
#include "proj/const.h"
#include "proj/Geos.h"

// GRIB Interface

#include <grib/GRIB.h>

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <msat/Image.tcc>

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "HIMET"
#define TYPE "Obs file"
#define HIMET_VERSION 0.0

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

#endif

using namespace std;

static bool gdalInitialised = false;

static void throwExceptionsOnGDALErrors(CPLErr eErrClass, int err_no, const char *msg)
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
	throw std::runtime_error(fullmsg.str());
}
				  
static void initGDALIfNeeded()
{
	if (gdalInitialised)
		return;
	GDALAllRegister();
	CPLSetErrorHandler(throwExceptionsOnGDALErrors);
	gdalInitialised = true;
}

namespace msat {

namespace proj {
class OGR : public Projection
{
public:
	/// The custom coordinate system
	OGRSpatialReference sr;
	/// The normal lat-lon coordinate system
	OGRSpatialReference latlon;
	/// Transform FROM latlon
	OGRCoordinateTransformation* fromLatLon;
	/// Transform TO latlon
	OGRCoordinateTransformation* toLatLon;

	/// This creates an incompletely intialised class.
	/// The sr member must be initialised separately.
	OGR() : fromLatLon(0), toLatLon(0)
	{
		latlon.SetWellKnownGeogCS("WGS84");
	}

	~OGR()
	{
		if (fromLatLon) delete fromLatLon;
		if (toLatLon) delete toLatLon;
	}

	/// Called to finalise initialisation after sr has been properly initialised
	void doneWithInit()
	{
		fromLatLon = OGRCreateCoordinateTransformation(&latlon, &sr);
		if (!fromLatLon)
			throw std::runtime_error("cannot create conversion from latitude,longitude");
		toLatLon = OGRCreateCoordinateTransformation(&sr, &latlon);
		if (!toLatLon)
			throw std::runtime_error("cannot create conversion to latitude,longitude");
	}

	// ProjctedPoint gives the angle in degrees between the subsallite point, the
	// satellite and the given point on the surface
  virtual void mapToProjected(const MapPoint& m, ProjectedPoint& p) const
	{
		p.x = m.lon; p.y = m.lat;
		fromLatLon->Transform(1, &p.x, &p.y);
	}
	virtual void projectedToMap(const ProjectedPoint& p, MapPoint& m) const
	{
		m.lon = p.x; m.lat = p.y;
		toLatLon->Transform(1, &m.lon, &m.lat);
	}
	virtual std::string format() const
	{
		char* res;
		sr.exportToWkt(&res);
		string sres(res);
		OGRFree(res);
		return sres;
	}
	Projection* clone() const {
		proj::OGR* res = new proj::OGR();
		res->sr.CopyGeogCSFrom(&sr);
		return res;
	}
};
}

bool isGDAL(const std::string& filename)
{
	initGDALIfNeeded();
	CPLPushErrorHandler(CPLQuietErrorHandler);
	GDALDataset* poDataset = (GDALDataset*)GDALOpen(filename.c_str(), GA_ReadOnly);
	CPLPopErrorHandler();
	if (poDataset)
	{
		delete poDataset;
		return true;
	} else
		return false;
}

auto_ptr<Image> importGDALBand(GDALDataset* dataset, int idx)
{
	ProgressTask p("Reading GDAL raster band");

	printf("GDAL raster band %d\n", idx);
	GDALRasterBand* band = dataset->GetRasterBand(idx);

	int xsize = dataset->GetRasterXSize();
	int ysize = dataset->GetRasterYSize();

#if 0
	int nBlockXSize, nBlockYSize;
	poBand->GetBlockSize(&nBlockXSize, &nBlockYSize);
	printf("  Block=%dx%d Type=%s, ColorInterp=%s\n",
			nBlockXSize, nBlockYSize,
			GDALGetDataTypeName(poBand->GetRasterDataType()),
			GDALGetColorInterpretationName(
				poBand->GetColorInterpretation()) );

	int bGotMin, bGotMax;
	double adfMinMax[2];
	adfMinMax[0] = poBand->GetMinimum( &bGotMin );
	adfMinMax[1] = poBand->GetMaximum( &bGotMax );
	if (!(bGotMin && bGotMax))
		GDALComputeRasterMinMax((GDALRasterBandH)poBand, TRUE, adfMinMax);
	printf("  Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1] );

	if (poBand->GetOverviewCount() > 0)
		printf("  Band has %d overviews.\n", poBand->GetOverviewCount());

	if (poBand->GetColorTable() != NULL)
		printf("  Band has a color table with %d entries.\n", 
				poBand->GetColorTable()->GetColorEntryCount());
#endif

	// Read image data
	auto_ptr<ImageData> imageData;
	GDALDataType dataType = band->GetRasterDataType();
	switch (dataType)
	{
		case GDT_Byte: {
			// Eight bit unsigned integer
			ImageDataWithPixels<uint8_t>* d = new ImageDataWithPixels<uint8_t>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = true;
			d->missingValue = std::numeric_limits<uint8_t>::min();
			break;
		}
		case GDT_UInt16: {
			// Sixteen bit unsigned integer
			ImageDataWithPixels<uint16_t>* d = new ImageDataWithPixels<uint16_t>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = true;
			d->missingValue = std::numeric_limits<uint16_t>::min();
			break;
		}
		case GDT_Int16: {
			// Sixteen bit signed integer
			ImageDataWithPixels<int16_t>* d = new ImageDataWithPixels<int16_t>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = true;
			d->missingValue = std::numeric_limits<int16_t>::min();
			break;
		}
		case GDT_UInt32: {
			// Thirty two bit unsigned integer
			ImageDataWithPixels<uint32_t>* d = new ImageDataWithPixels<uint32_t>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = true;
			d->missingValue = std::numeric_limits<uint32_t>::min();
			break;
		}
		case GDT_Int32: {
			// Thirty two bit signed integer
			ImageDataWithPixels<int32_t>* d = new ImageDataWithPixels<int32_t>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = true;
			d->missingValue = std::numeric_limits<int32_t>::min();
			break;
		}
		case GDT_Float32: {
			// Thirty two bit floating point
			ImageDataWithPixels<float>* d = new ImageDataWithPixels<float>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = false;
			d->missingValue = std::numeric_limits<float>::max();
			break;
		}
		case GDT_Float64: {
			// Sixty four bit floating point
			ImageDataWithPixels<double>* d = new ImageDataWithPixels<double>(xsize, ysize);
			imageData.reset(d);
			band->RasterIO(GF_Read, 0, 0, xsize, ysize, d->pixels, xsize, ysize, dataType, 0, 0);
			d->scalesToInt = false;
			d->missingValue = std::numeric_limits<double>::max();
			break;
		}
		case GDT_CInt16: {
			// Complex Int16
			throw std::runtime_error("GDAL raster band has unsupported image type 'complex 16bit integer'");
		}
		case GDT_CInt32: {
			// Complex Int32
			throw std::runtime_error("GDAL raster band has unsupported image type 'complex 32bit integer'");
		}
		case GDT_CFloat32: {
			// Complex Float32
			throw std::runtime_error("GDAL raster band has unsupported image type 'complex 32bit float'");
		}
		case GDT_CFloat64: {
			// Complex Float64
			throw std::runtime_error("GDAL raster band has unsupported image type 'complex 64bit float'");
		}
		default:
			throw std::runtime_error("GDAL raster band has unknown data type");
	}

	// Missing values are not explicitly supported in GDAL, so we just use
	// something inferred from the data type
	//
	// res->missing = m.field.undef_high;
	// res->missingValue = res->missing;

	imageData->slope = band->GetScale();
	imageData->offset = band->GetOffset();

	// This is automatically computed by ImageDataWithPixels
	// res->bpp = m.field.numbits;

	auto_ptr< Image > img(new Image());
	img->setData(imageData.release());

	// GDAL does not seem to support timestamping of images
	img->year = 0;
	img->month = 0;
	img->day = 0;
	img->hour = 0;
	img->minute = 0;

	// Set the projection
	std::auto_ptr<proj::OGR> prj(new proj::OGR);
	const char* pname = dataset->GetProjectionRef();
	// importFromWtk changes the pointer, but not its contents, so it should be safe to cast here
	prj->sr.importFromWkt(const_cast<char**>(&pname));
	prj->doneWithInit();
	img->proj = prj;

	// Set the geographical transformation of the projected data

	double geoTransform[6];
	if (dataset->GetGeoTransform(geoTransform) != CE_None)
		cerr << "Warning: couldn't read the geographical transformation from GDAL file" << endl;

	img->x0 = (int)round(geoTransform[0] / geoTransform[1]);
	img->y0 = (int)round(geoTransform[3] / geoTransform[5]);
	img->column_res = 1/geoTransform[1];
	img->line_res = 1/geoTransform[5];
	img->column_offset = 0;
	img->line_offset = 0;

	img->unit = band->GetUnitType();

	// GDAL does not seem to support satellite-specific metadata
	img->spacecraft_id = 0;
	img->channel_id = 0;

	/* TODO
  img->defaultFilename = util::satelliteSingleImageFilename(*img);
  img->shortName = util::satelliteSingleImageShortName(*img);
	*/

  return img;
}


class GDALImageImporter : public ImageImporter
{
	std::string filename;

public:
	GDALImageImporter(const std::string& filename)
		: filename(filename) {}

	virtual void read(ImageConsumer& output)
	{
		initGDALIfNeeded();

		GDALDataset* poDataset = (GDALDataset*)GDALOpen(filename.c_str(), GA_ReadOnly);
		if( poDataset == NULL )
			// GDAL should already have called our error handler, which throws a
			// runtime exception
			return;

		/*
		double adfGeoTransform[6];

		printf("GDAL Import driver is %s/%s\n",
				poDataset->GetDriver()->GetDescription(), 
				poDataset->GetDriver()->GetMetadataItem(GDAL_DMD_LONGNAME));

		printf("GDAL Import size is %dx%dx%d\n", 
				poDataset->GetRasterXSize(), poDataset->GetRasterYSize(),
				poDataset->GetRasterCount());

		if (poDataset->GetProjectionRef() != NULL)
			printf("GDAL Import projection is `%s'\n", poDataset->GetProjectionRef());

		if (poDataset->GetGeoTransform(adfGeoTransform) == CE_None)
		{
			printf("GDAL Import origin is (%.6f,%.6f)\n",
					adfGeoTransform[0], adfGeoTransform[3]);

			printf("GDAL Import pixel size is (%.6f,%.6f)\n",
					adfGeoTransform[1], adfGeoTransform[5]);

			printf("GDAL Import full geotransform:");
			for (int i = 0; i < 6; ++i)
				printf(" %.6f", adfGeoTransform[i]);
			printf("\n");
		}

		printf("GDAL Import number of raster bands is `%d'\n", poDataset->GetRasterCount());
		*/

		int count = 0;

		for (int i = 1; i <= poDataset->GetRasterCount(); ++i)
		{
			auto_ptr<Image> img = importGDALBand(poDataset, i);
			img->setQualityFromPathname(filename);
			// Use defaultFilename to avoid exposing the original image file name,
			// which could cause unexpected leakage of private information
			img->addToHistory("Imported from GDAL " + img->defaultFilename);
			cropIfNeeded(*img);
			output.processImage(img);
			++count;
		}

		delete poDataset;

		if (count == 0)
			throw std::runtime_error("cannot read any GDAL raster bands from file " + filename);
	}
};

std::auto_ptr<ImageImporter> createGDALImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new GDALImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
