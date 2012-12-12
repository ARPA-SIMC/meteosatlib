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

#include "gdalutils.h"
#include <gdal/ogr_spatialref.h>
#include <msat/proj/const.h>
#include <msat/Image.h>
#include <exception>
#include <stdexcept>
#include <sstream>

using namespace std;

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

void initGDALIfNeeded()
{
	static bool gdalInitialised = false;

	if (gdalInitialised)
		return;
	GDALAllRegister();
	CPLSetErrorHandler(throwExceptionsOnGDALErrors);
	gdalInitialised = true;
}

int bppOfGDALDataType(GDALDataType type)
{
	switch (type)
	{
		case GDT_Byte: return 8;
		case GDT_UInt16: return 16;
		case GDT_Int16: return 16;
		case GDT_UInt32: return 32;
		case GDT_Int32: return 32;
		case GDT_Float32: return 32;
		case GDT_Float64: return 64;
		case GDT_CInt16: return 16;
		case GDT_CInt32: return 32;
		case GDT_CFloat32: return 32;
		case GDT_CFloat64: return 64;
		default:
						   stringstream str;
						   str << "Unhandled raster data type " << type;
						   throw std::runtime_error(str.str());
	}
}

//	int InvGeoTransform( double *gt_in, double *gt_out )
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

string getGeosWKT(double sublon)
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

void TypeChooser::init(GDALDriver& driver)
{
	string types = driver.GetMetadataItem(GDAL_DMD_CREATIONDATATYPES);
	size_t start = 0, pos = 0;
	while (pos != string::npos)
	{
		string type;
		pos = types.find(' ', start);
		if (pos == string::npos)
			type = types.substr(start);
		else
			type = types.substr(start, pos-start);

		if (type == "Byte")         insert(GDT_Byte);
		else if (type == "UInt16")  insert(GDT_UInt16);
		else if (type == "Int16")   insert(GDT_Int16);
		else if (type == "UInt32")  insert(GDT_UInt32);
		else if (type == "Int32")   insert(GDT_Int32);
		else if (type == "Float32") insert(GDT_Float32);
		else if (type == "Float64") insert(GDT_Float64);
		else
			throw std::runtime_error("Unrecognised output type: " + type);

		start = pos + 1;
	}
}

GDALDataType TypeChooser::choose(GDALRasterBand& rb)
{
	return choose(rb, rb.GetRasterDataType());
}

GDALDataType TypeChooser::choose(GDALRasterBand& rb, GDALDataType type)
{
	//fprintf(stderr, "Choose %d\n", (int)type);

	// If we have it, we use it
	if (find(type) != end()) return type;

	int bpp = msat::getBPP(rb);

	//fprintf(stderr, "Nope: escalate, bpp: %d\n", bpp);

	switch (type)
	{
		case GDT_Byte: {
			GDALDataType res1 = choose(rb, GDT_UInt16);
			GDALDataType res2 = choose(rb, GDT_Int16);
			if (res1 == GDT_Unknown) return res2;
			if (res2 == GDT_Unknown) return res1;
			if (res1 < res2) return res1;
			return res2;
		}
		case GDT_UInt16: {
			GDALDataType res[3];
			if (bpp < 16)
				res[0] = choose(rb, GDT_Int16);
			else
				res[0] = choose(rb, GDT_UInt32);
			res[1] = choose(rb, GDT_UInt32);
			res[2] = choose(rb, GDT_Int32);
			// Return the minimum
			GDALDataType min = res[0];
			for (int i = 1; i < 3; ++i)
				if (min == GDT_Unknown || res[i] < min)
					min = res[i];
			return min;
		}
		case GDT_Int16: return choose(rb, GDT_Int32);
		case GDT_UInt32: return choose(rb, GDT_Float32);
		case GDT_Int32: return choose(rb, GDT_Float32);
		case GDT_Float32: return choose(rb, GDT_Float64);
		case GDT_Float64:
		default:
			return GDT_Unknown;
	}
}


// vim:set ts=2 sw=2:
