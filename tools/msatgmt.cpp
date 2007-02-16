//---------------------------------------------------------------------------
//
//  File        :   msatgmt.cpp
//  Description :   Reproject on a regular grid and export to GMT NetCDF files
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  RCS ID      :   $Id: /local/meteosatlib/msat/ExportNetCDF.h 2114 2006-11-16T23:34:58.014010Z enrico  $
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
#include <msat/NetCDFUtils.h>

#include <stdexcept>
#include <netcdfcpp.h>
#include <cmath>

using namespace msat;
using namespace std;

#if 0
#include <sstream>
#include <stdexcept>
#include <memory>

#include <math.h>
#include <limits>

namespace msat {

template<typename NCObject, typename T>
static void ncfAddAttr(NCObject& ncf, const char* name, const T& val)
{
  if (!ncf.add_att(name, val))
  {
		std::stringstream msg;
    msg << "Adding '" << name << "' attribute " << val;
    throw std::runtime_error(msg.str());
  }
}

template<typename Sample>
static inline Sample getAttribute(const NcAtt& a) { throw std::runtime_error("requested to read attribute from unknown C++ type"); }
template<> static inline ncbyte getAttribute<ncbyte>(const NcAtt& a) { return a.as_ncbyte(0); }
template<> static inline char getAttribute<char>(const NcAtt& a) { return a.as_char(0); }
template<> static inline short getAttribute<short>(const NcAtt& a) { return a.as_short(0); }
template<> static inline int getAttribute<int>(const NcAtt& a) { return a.as_int(0); }
template<> static inline float getAttribute<float>(const NcAtt& a) { return a.as_float(0); }
template<> static inline double getAttribute<double>(const NcAtt& a) { return a.as_double(0); }

template<typename Sample>
static inline NcType getNcType() { throw std::runtime_error("requested NcType for unknown C++ type"); }
template<> static inline NcType getNcType<ncbyte>() { return ncByte; }
template<> static inline NcType getNcType<char>() { return ncChar; }
template<> static inline NcType getNcType<short>() { return ncShort; }
template<> static inline NcType getNcType<int>() { return ncInt; }
template<> static inline NcType getNcType<float>() { return ncFloat; }
template<> static inline NcType getNcType<double>() { return ncDouble; }

template<typename Sample>
static inline Sample getMissing()
{
	if (std::numeric_limits<Sample>::has_quiet_NaN)
		return std::numeric_limits<Sample>::quiet_NaN();
	else if (std::numeric_limits<Sample>::is_signed)
		return std::numeric_limits<Sample>::min();
	else
		return std::numeric_limits<Sample>::max();
}

struct NcEncoder
{
	virtual ~NcEncoder() {}
	virtual NcType getType() = 0;
	virtual void setData(NcVar& var, const Image& img) = 0;
};

template<typename Sample>
struct NcEncoderImpl : public NcEncoder
{
	virtual NcType getType() { return getNcType<Sample>(); }
	virtual void setData(NcVar& var, const Image& img)
	{
		Sample* pixels = new Sample[img.data->columns * img.data->lines];
		int missing = img.data->unscaledMissingValue();
		//Sample encodedMissing = getMissing<Sample>();
		ncfAddAttr(var, "missing_value", missing);

		for (size_t y = 0; y < img.data->lines; ++y)
			for (size_t x = 0; x < img.data->columns; ++x)
				pixels[y * img.data->columns + x] = img.data->scaledToInt(x, y);

		if (!var.put(pixels, 1, img.data->lines, img.data->columns))
			throw std::runtime_error("writing image values failed");

		delete[] pixels;
	}
};

static std::auto_ptr<NcEncoder> createEncoder(size_t bpp)
{
	if (bpp > 15)
		return std::auto_ptr<NcEncoder>(new NcEncoderImpl<int>);
	else if (bpp > 7)
		return std::auto_ptr<NcEncoder>(new NcEncoderImpl<short>);
	else
		return std::auto_ptr<NcEncoder>(new NcEncoderImpl<ncbyte>);
}

// Recompute the BPP for an image made of integer values, by looking at what is
// their maximum value.  Assume unsigned integers.
static void computeBPP(ImageData& img)
{
	if (img.scalesToInt)
	{
		int max = img.scaledToInt(0, 0);
		for (size_t y = 0; y < img.lines; ++y)
			for (size_t x = 0; x < img.columns; ++x)
			{
				int sample = img.scaledToInt(x, y);
				if (sample > max)
					max = sample;
			}
		img.bpp = (int)ceil(log2(max + 1));
	}
	// Else use the original BPPs
}

// Get the attribute if it exists, otherwise returns 0
static NcAtt* getAttrIfExists(const NcVar& var, const std::string& name)
{
	for (int i = 0; i < var.num_atts(); ++i)
	{
		NcAtt* cand = var.get_att(i);
		if (cand->name() == name)
			return cand;
	}
	return 0;
}

template<typename Sample>
void decodeMissing(const NcVar& var, ImageDataWithPixels<Sample>& img)
{
	NcAtt* attrMissing = getAttrIfExists(var, "missing_value");
	if (attrMissing != NULL)
		img.missing = getAttribute<Sample>(*attrMissing);
	else
		img.missing = getMissing<Sample>();
}

template<typename Sample>
static ImageData* acquireImage(const NcVar& var)
{
	if (var.num_dims() != 3)
	{
		std::stringstream msg;
		msg << "Number of dimensions for " << var.name() << " should be 3 but is " << var.num_dims() << " instead";
		throw std::runtime_error(msg.str());
	}

	int tsize = var.get_dim(0)->size();
	if (tsize != 1)
	{
		std::stringstream msg;
		msg << "Size of the time dimension for " << var.name() << " should be 1 but is " << tsize << " instead";
		throw std::runtime_error(msg.str());
	}

	std::auto_ptr< ImageDataWithPixels<Sample> > res(new ImageDataWithPixels<Sample>(var.get_dim(2)->size(), var.get_dim(1)->size()));
	decodeMissing(var, *res);

	if (!var.get(res->pixels, 1, res->lines, res->columns))
		throw std::runtime_error("reading image pixels failed");

	return res.release();
}

}
#endif

#if 0
//---------------------------------------------------------------------------
//
//  File        :   ExportNetCDF.cpp
//  Description :   Export data from an ImageData into a NetCDF file
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  Source      :   derived from SAFH5CT2NetCDF.cpp by Le Duc, as modified by
//                  Francesca Di Giuseppe and from XRIT2Grib.cpp by Graziano
//                  Giuliani (Lamma Regione Toscana)
//  RCS ID      :   $Id: /local/meteosatlib/msat/ExportNetCDF.cpp 2798 2007-02-16T10:13:53.314678Z enrico  $
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

#include <config.h>

#include <msat/ExportNetCDF.h>
#include <msat/proj/const.h>

#include <netcdfcpp.h>

#include <sstream>

#include "NetCDFUtils.h"

#define INSTITUTION "ARPA-SIM"
#define TYPE "Processed Products"

using namespace std;

namespace msat {
#endif

struct GMTExportOptions
{
	double latmin, latmax, lonmin, lonmax;
	double latstep, lonstep;

	GMTExportOptions():
		latmin(-90), latmax(90), lonmin(-180), lonmax(180),
		latstep(0.5), lonstep(0.5) {}

	int xsize() const { return (int)floor((lonmax - lonmin) / lonstep); }
	int ysize() const { return (int)floor((latmax - latmin) / latstep); }
	double lon(int x) const { return lonmin + lonstep*x; }
	double lat(int y) const { return latmin + latstep*y; }
};


//
// Creates NetCDF product
//
void ExportGMT(const GMTExportOptions& opts, const Image& img, const std::string& fileName)
{
  // Get the channel name
  string channelstring = Image::channelName(img.spacecraftIDToHRIT(img.spacecraft_id), img.channel_id);

  // Change sensitive characters into underscores
  for (string::iterator i = channelstring.begin();
	i != channelstring.end(); ++i)
    if (*i == ' ' || *i == '.' || *i == ',')
      *i = '_';

  // Build up output NetCDF file name and open it
  NcFile ncf(fileName.c_str(), NcFile::Replace);
  if (!ncf.is_valid())
    throw std::runtime_error("Creating a NcFile structure for file " + fileName);

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

	
	
	// Interpolate and fetch the value, makes it possible to then compute z_range
	// and also X and Y dimensions

	float data[opts.xsize() * opts.ysize()];
	float lats[opts.ysize()];
	float lons[opts.xsize()];
	for (int y = 0; y < opts.ysize(); ++y)
	{
		for (int x = 0; x < opts.xsize(); ++x)
		{
			int xpix, ypix;
			img.coordsToPixels(opts.lat(y), opts.lon(x), xpix, ypix);
			float datum = img.data->scaled(xpix, ypix);
			if (datum == img.data->missingValue)
				data[y * opts.xsize() + x] = FP_NAN;
			else
				data[y * opts.xsize() + x] = datum;
		}
		lats[y] = opts.lat(y);
	}
	for (int x = 0; x < opts.xsize(); ++x)
		lons[x] = opts.lon(x);

  NcDim *londim = ncf.add_dim("lon", opts.xsize());
  if (!londim->is_valid()) throw std::runtime_error("adding lon dimension failed");
  NcDim *latdim = ncf.add_dim("lat", opts.ysize());
  if (!latdim->is_valid()) throw std::runtime_error("adding lat dimension failed");

  NcVar *var = ncf.add_var("lon", ncFloat, londim);
  if (!var->is_valid()) throw std::runtime_error("adding lon variable failed");
	ncfAddAttr(*var, "long_name", "longitude");
	ncfAddAttr(*var, "units", "degrees_east");
	ncfAddAttr(*var, "modulo", 360.0);
  ncfAddAttr(*var, "topology", "circular");
  if (!var->put(lons, opts.xsize()))
		throw std::runtime_error("writing longitude values failed");

  var = ncf.add_var("lat", ncFloat, latdim);
  if (!var->is_valid()) throw std::runtime_error("adding lat variable failed");
	ncfAddAttr(*var, "long_name", "latitude");
	ncfAddAttr(*var, "units", "degrees_north");
  if (!var->put(lats, opts.ysize()))
		throw std::runtime_error("writing latitude values failed");

  var = ncf.add_var(channelstring.c_str(), ncFloat, latdim, londim);
  if (!var->is_valid()) throw std::runtime_error("adding " + channelstring + " variable failed");
	ncfAddAttr(*var, "axis", "YX");
	ncfAddAttr(*var, "long_name", channelstring.c_str());
  if (img.channel_id > 3 && img.channel_id < 12)
    ncfAddAttr(*var, "units", "K");
  else
    ncfAddAttr(*var, "units", "mW m^-2 sr^-1 (cm^-1)^-1");
	ncfAddAttr(*var, "add_offset", 0.0);
  ncfAddAttr(*var, "scale_factor", 1.0);
  ncfAddAttr(*var, "chnum", img.channel_id);
  if (!var->put(data, opts.ysize(), opts.xsize()))
		throw std::runtime_error("writing image values failed");





	/*
	float range[2];
  NcVar *var = ncf.add_var("x_range", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding x_range variable failed");
	var->add_att("units", "degrees");
	range[0] = opts.lonmin;
	range[1] = opts.lonmax;
  if (!var->put(range, 2))
		throw std::runtime_error("writing x_range values failed");

  var = ncf.add_var("y_range", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding y_range variable failed");
	var->add_att("units", "degrees");
	range[0] = opts.latmin;
	range[1] = opts.latmax;
  if (!var->put(range, 2))
		throw std::runtime_error("writing y_range values failed");

  var = ncf.add_var("spacing", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding spacing variable failed");
	//var->add_att("units", "degrees");
	range[0] = opts.lonstep;
	range[1] = opts.latstep;
  if (!var->put(range, 2))
		throw std::runtime_error("writing spacing values failed");

  var = ncf.add_var("dimension", ncFloat, sdim);
  if (!var->is_valid()) throw std::runtime_error("adding dimension variable failed");
	//var->add_att("units", "degrees");
	range[0] = opts.lonstep;
	range[1] = opts.latstep;
  if (!var->put(range, 2))
		throw std::runtime_error("writing dimension values failed");
	*/



#if 0
  // Add Global Attributes
  //cerr << "global." << endl;

  ncfAddAttr(ncf, "Satellite", Image::spacecraftName(Image::spacecraftIDToHRIT(img.spacecraft_id)).c_str());
  char reftime[64];
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
           img.year, img.month, img.day, img.hour, img.minute);
  ncfAddAttr(ncf, "Antenna", "Fixed");
  ncfAddAttr(ncf, "Receiver", "HIMET");
  ncfAddAttr(ncf, "Time", reftime);
  ncfAddAttr(ncf, "Area_Name", "SpaceView");
  char projname[16];
  sprintf(projname, "GEOS(%3.1f)", 0.0);
  //sprintf(projname, "GEOS(%3.1f)", pds.sublon);
  ncfAddAttr(ncf, "Projection", projname);
  ncfAddAttr(ncf, "Columns", 3712);
  //if (! ncf.add_att("Columns", pds.npix ) ) return false;
  ncfAddAttr(ncf, "Lines", 3712);
  //if (! ncf.add_att("Lines", pds.nlin ) ) return false;
  ncfAddAttr(ncf, "AreaStartPix", img.x0 );
  //if (! ncf.add_att("AreaStartPix", (int) m.grid.sp.X0 ) ) return false;
  ncfAddAttr(ncf, "AreaStartLin", img.y0 );
  //if (! ncf.add_att("AreaStartLin", (int) m.grid.sp.Y0 ) ) return false;
  ncfAddAttr(ncf, "SampleX", 1.0 );
  ncfAddAttr(ncf, "SampleY", 1.0 );
  ncfAddAttr(ncf, "Column_Scale_Factor", img.column_res * exp2(16));
  ncfAddAttr(ncf, "Line_Scale_Factor", img.line_res * exp2(16));
  ncfAddAttr(ncf, "Column_Offset", img.column_offset);
  ncfAddAttr(ncf, "Line_Offset", img.line_offset);
  ncfAddAttr(ncf, "Orbit_Radius", ORBIT_RADIUS);
  //if (! ncf.add_att("Orbit_Radius", pds.sh) ) return false;
  ncfAddAttr(ncf, "Longitude", 0);
  //if (! ncf.add_att("Longitude", pds.sublon) ) return false;
  ncfAddAttr(ncf, "NortPolar", 1);
  ncfAddAttr(ncf, "NorthSouth", img.column_res >= 0 ? 1 : 0);
  ncfAddAttr(ncf, "title", img.historyPlusEvent("Exported to NetCDF").c_str());
  ncfAddAttr(ncf, "Institution", INSTITUTION);
  ncfAddAttr(ncf, "Type", TYPE);
  ncfAddAttr(ncf, "Version", PACKAGE_VERSION);
  ncfAddAttr(ncf, "Conventions", "COARDS");
  ncfAddAttr(ncf, "history", "Created from SAF HDF5 data");

  // Dimensions
  //cerr << "dimensions." << endl;

  // Add Calibration values
  //cerr << "calibration. cs: " << channelstring << endl;

  NcVar *tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) throw std::runtime_error("adding time variable failed");
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime = (double) img.forecastSeconds2000();
  if (!tvar->put(&atime, 1)) throw std::runtime_error("setting time variable failed");

  NcVar *ivar = ncf.add_var(channelstring.c_str(), ncFloat, tdim, ldim, cdim);
  if (!ivar->is_valid()) throw std::runtime_error("adding " + channelstring + " variable failed");
	ncfAddAttr(*ivar, "add_offset", 0.0);
  ncfAddAttr(*ivar, "scale_factor", 1.0);
  ncfAddAttr(*ivar, "chnum", img.channel_id);

  float *pixels = img.data->allScaled();
  if (img.channel_id > 3 && img.channel_id < 12)
    ivar->add_att("units", "K");
  else
    ivar->add_att("units", "mW m^-2 sr^-1 (cm^-1)^-1");

  // Write output values
  //cerr << "output." << endl;
  if (!ivar->put(pixels, 1, img.data->lines, img.data->columns))
		throw std::runtime_error("writing image values failed");
#endif

  // Close NetCDF output
  (void) ncf.close( );

//  cout << "Wrote file " << NcName << "." << endl;
}
#if 0

}

// vim:set ts=2 sw=2:
//---------------------------------------------------------------------------
//
//  File        :   NetCDF2Grib.cpp
//  Description :   Convert NetCDF format in Grib format
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  Source      :   derived from SAFH5CT2NetCDF.cpp by Le Duc, as modified by
//                  Francesca Di Giuseppe and from XRIT2Grib.cpp by Graziano
//                  Giuliani (Lamma Regione Toscana)
//  RCS ID      :   $Id: /local/meteosatlib/tools/msat.cpp 2615 2007-01-17T18:43:23.235896Z enrico  $
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
#endif

#include <config.h>

#include <msat/ImportGRIB.h>
#include <msat/ImportSAFH5.h>
#include <msat/ImportNetCDF.h>
#include <msat/ImportNetCDF24.h>
#include <msat/ImportXRIT.h>
#include <msat/ExportGRIB.h>
#include <msat/ExportNetCDF.h>
#include <msat/ExportNetCDF24.h>
#include <msat/Progress.h>

#ifdef HAVE_MAGICKPP
#include <msat/ExportImage.h>
#endif

#include <set>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <getopt.h>

using namespace std;
using namespace msat;

class GMTExporter : public ImageConsumer
{
	GMTExportOptions opts;
public:
	GMTExporter(const GMTExportOptions& opts) : opts(opts) {}
	virtual void processImage(auto_ptr<Image> img)
	{
		ExportGMT(opts, *img, img->defaultFilename() + ".grd");
	}
};

std::auto_ptr<ImageConsumer> createGMTExporter(const GMTExportOptions& opts)
{
	return std::auto_ptr<ImageConsumer>(new GMTExporter(opts));
}


void do_help(const char* argv0, ostream& out)
{
  out << "Usage: " << "msatgmt" << " [options] file(s)..." << endl
			<< "msatgmt can read and write various formats of satellite images or other" << endl
			<< "georeferentiated raster data, reproject them on a regular grid and export" << endl
			<< "them to NetCDF files that are readable by GMT tools." << endl
			<< endl
			<< "Options:" << endl
      << "  --help           Print detailed usage information." << endl
      << "  --version        Print the program version and exit." << endl
      << "  -q, --quiet      Work silently." << endl
      << "  --Area='latmin,latmax,lonmin,lonmax'  Crop the source image(s) to the given coordinates." << endl
      << "  --around='lat,lon,lath,lonw'  Create an image centered at the given location and with the given width and height." << endl
			<< "  --step='latstep,lonstep'  Specify the latitude and longitude step of the grid." << endl
		  << endl
      << "Formats supported are:"
			<< endl
#ifdef HAVE_NETCDF
      << " NetCDF    Import" << endl
      << " NetCDF24  Import" << endl
#endif
      << " Grib      Import" << endl
#ifdef HAVE_HDF5
      << " SAFH5     Import" << endl
#endif
#ifdef HAVE_HRIT
      << " XRIT      Import" << endl
#endif
			<< endl
			<< "Examples:" << endl
			<< endl
			<< " $ msatgmt --step=0.5,0.5 --Area=30,60,-10,40 file.grb" << endl
			<< " $ msatgmt file.grb" << endl
			<< " $ msatgmt --step=1,1 dir/H:MSG1:HRV:200611130800" << endl
			<< endl
			<< "Report bugs to " << PACKAGE_BUGREPORT << endl;
			;
}

/*
 * Create an importer for the given file, auto-detecting the file type.
 * 
 * If no supported file type could be detected, returns an empty auto_ptr.
 */
std::auto_ptr<ImageImporter> getImporter(const std::string& filename)
{
	if (isGrib(filename))
		return createGribImporter(filename);
#ifdef HAVE_NETCDF
	if (isNetCDF(filename))
		return createNetCDFImporter(filename);
	if (isNetCDF24(filename))
		return createNetCDF24Importer(filename);
#endif
#ifdef HAVE_HDF5
	if (isSAFH5(filename))
		return createSAFH5Importer(filename);
#endif
#ifdef HAVE_HRIT
	if (isXRIT(filename))
		return createXRITImporter(filename);
#endif
	return std::auto_ptr<ImageImporter>();
}

int main( int argc, char* argv[] )
{
	// Defaults to view
	GMTExportOptions gmtopts;
	bool quiet = false;

  static struct option longopts[] = {
    { "help",	0, NULL, 'H' },
    { "version",	0, NULL, 'v' },
    { "quiet", 0, NULL, 'q' },
		{ "Area", 1, 0, 'A' },
		{ "around", 1, 0, 'C' },
		{ "step", 1, 0, 's' },
		{ 0, 0, 0, 0 },
  };

  bool done = false;
  while (!done) {
    int c = getopt_long(argc, argv, "q", longopts, (int*)0);
    switch (c) {
      case 'H':	// --help
				do_help(argv[0], cout);
				return 0;
      case 'v':	// --version
				cout << "msat version " PACKAGE_VERSION << endl;
				return 0;
			case 'q': // -q,--quiet
				quiet = true;
				break;
			case 'A':
				if (sscanf(optarg, "%lf,%lf,%lf,%lf",
							&gmtopts.latmin,
							&gmtopts.latmax,
							&gmtopts.lonmin,
							&gmtopts.lonmax) != 4)
				{
					cerr << "Area value should be in the format latmin,latmax,lonmin,lonmax" << endl;
					do_help(argv[0], cerr);
					return 1;
				}
				break;
			case 'C': {
				double lat, lon, h, w;
				if (sscanf(optarg, "%lf,%lf,%lf,%lf", &lat,&lon,&h,&w) != 4)
				{
					cerr << "around value should be in the format lat,lon,lath,lonw" << endl;
					do_help(argv[0], cerr);
					return 1;
				}
				gmtopts.latmin = lat - h/2;
				gmtopts.latmax = lat + h/2;
				gmtopts.lonmin = lon - w/2;
				gmtopts.lonmax = lon + w/2;
				break;
			}
			case 's': {
				if (sscanf(optarg, "%lf,%lf", &gmtopts.latstep,&gmtopts.lonstep) != 2)
				{
					cerr << "step value should be in the format latstep,lonstep" << endl;
					do_help(argv[0], cerr);
					return 1;
				}
				break;
			}
      case -1:
				done = true;
				break;
      default:
				cerr << "Error parsing commandline." << endl;
				do_help(argv[0], cerr);
				return 1;
    }
  }

  if (optind == argc)
  {
    do_help(argv[0], cerr);
    return 1;
  }

	if (!quiet)
		Progress::get().setHandler(new StreamProgressHandler(cerr));

  try
  {
		for (int i = optind; i < argc; ++i)
		{
			std::auto_ptr<ImageImporter> importer = getImporter(argv[i]);
			if (!importer.get())
			{
				cerr << "No importer found for " << argv[i] << ": ignoring." << endl;
				continue;
			}
			//importer->cropImgArea = imgArea;
			//importer->cropGeoArea = geoArea;

			std::auto_ptr<ImageConsumer> consumer = createGMTExporter(gmtopts);
			importer->read(*consumer);
		}
  }
  catch (std::exception& e)
  {
    cerr << e.what() << endl;
    return 1;
  }
  
  return(0);
}

// vim:set ts=2 sw=2:
