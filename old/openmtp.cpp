#include <openmtp/OpenMTP.h>
#include <msat/Image.h>
#include "gdalutils.h"

#if 0
#include <string>

#include <sstream>
#include <iostream>
#include <stdexcept>

#include <math.h>

#include <gdal/ogr_spatialref.h>
#include "proj/const.h"
#include "proj/Geos.h"

#include "msat/Image.tcc"
#include "msat/Progress.h"
#endif

using namespace std;

namespace msat {
namespace openmtp {

class OpenMTPBand : public Band
{
protected:
	OpenMTP* omtp;

public:
#if 0
	/// Scaling factor of the values
	double scale;
	/// Offset of the values
	double offset;
#endif

	OpenMTPBand(GDALDataset* ds, int idx, OpenMTP* omtp) : omtp(omtp)
	{
		poDS = ds;
		nBand = idx;

		eDataType = GDT_Byte;
		nRasterXSize = nBlockXSize = omtp->npixels();
		nRasterYSize = nBlockYSize = omtp->nlines();
	}

	virtual const char* GetUnitType() { return omtp->get_chunit(); }

	virtual double GetOffset(int *pbSuccess=NULL)
	{
		return 0;
	}
	virtual double GetScale(int *pbSuccess=NULL)
	{
		return 1;
	}

	virtual std::string getName()
	{
  		return omtp->get_chname();
	}

	virtual int getOriginalBpp()
	{
		return omtp->bits_per_pixel();
	}

	/// Get the channel ID (from table TODO)
	/// http://www.ecmwf.int/publications/manuals/libraries/gribex/Proposed_Channel_ID_tables.doc
	virtual int getChannel()
	{
		string name = omtp->get_chname();
		if (name == "VIS") return 1;
		if (name == "IR") return 2;
		if (name == "WV") return 3;
	}

	virtual CPLErr IReadBlock(int xblock, int yblock, void *buf)
	{
		if (xblock != 0 || yblock != 0)
		{
			CPLError(CE_Failure, CPLE_AppDefined, "Invalid block number");
			return CE_Failure;
		}

		unsigned char* image = omtp->get_image();
		memcpy(buf, image, nBlockXSize * nBlockYSize);

		return CE_None;
	}
};


class Dataset : public Image
{
protected:
	// Cached version
	std::string projWkt;
	// Cached version
	double geoTransform[6];

	void fillProjectionCache()
	{
		projWkt = getGeosWKT(omtp->subsatellite_point());

		int cfac, lfac, coff, loff;

		if (omtp->is_A_format( ))
		{
			if (omtp->is_ir_data( ) || omtp->is_wv_data( ))
			{
				cfac = -9102222;
				lfac = -9102222;
				coff = 1248;
				loff = 1249;
				//scale = 1.0;
			}
			else if (omtp->is_vis_data( ))
			{
				cfac = -18204444;
				lfac = -18204444;
				coff = 2500;
				loff = 2500;
				//scale = 1.0;
			}
			else
				throw std::runtime_error("Unsupported AFormat image (?)");
		}
		else if (omtp->is_B_format( ))
		{
			cfac = -18204444;
			lfac = -18204444;
			coff = 1248;
			loff = -1118;
			//scale = 2.0;
		}
		else
			throw std::runtime_error("Currently supporting only A or B format images");

		const double psx = Band::pixelHSizeFromCFAC(cfac);
		const double psy = Band::pixelVSizeFromLFAC(lfac);

#if 0
		// SAF COFF and LOFF represent the distance in pixels from the top-left
		// cropped image point to the subsatellite point, increasing with increasing
		// latitudes and increasing longitudes
		const int column_offset = 1856;
		const int line_offset = 1856;
		const int x0 = 1856 - readIntAttribute(group, "COFF") + 1;
		const int y0 = 1856 - readIntAttribute(group, "LOFF") + 1;
#endif

#if 0
		// Compute geotransform matrix
		geoTransform[0] = -coff * psx;
		geoTransform[3] = loff * psy;
		geoTransform[1] = psx;
		geoTransform[5] = -psy;
		geoTransform[2] = 0.0;
		geoTransform[4] = 0.0;
#endif
		geoTransform[0] = -1;
		geoTransform[3] = 1;
		geoTransform[1] = 1;
		geoTransform[5] = -1;
		geoTransform[2] = 0.0;
		geoTransform[4] = 0.0;
	}

public:
	OpenMTP* omtp;
	string history;

	Dataset(OpenMTP* omtp) : omtp(omtp)
	{
		nRasterXSize = omtp->npixels();
		nRasterYSize = omtp->nlines();

		nBands = 1;
		SetBand(1, new OpenMTPBand(this, 1, omtp));
	}
	virtual ~Dataset()
	{
		if (omtp) delete omtp;
	}

	/// Get image time
	virtual void getTime(int& year, int& month, int& day, int& hour, int& minute, int& second)
	{
		struct tm& t = omtp->get_datetime();
		year = t.tm_year + 1900;
		month = t.tm_mon + 1;
		day = t.tm_mday;
		hour = t.tm_hour;
		minute = t.tm_min;
		second = t.tm_sec;
	}

	virtual const char* GetProjectionRef()
	{
		if (projWkt.empty())
			fillProjectionCache();
		return projWkt.c_str();
	}
	
	virtual CPLErr GetGeoTransform(double* padfTransform)
	{
		if (projWkt.empty())
			fillProjectionCache();
		memcpy(padfTransform, geoTransform, 6 * sizeof(double));
		return CE_None;
	}

	virtual int getSpacecraft()
	{
		string sname = omtp->get_satellite_name();
		// Remove spaces
		string name;
		for (string::const_iterator i = sname.begin();
				i != sname.end(); ++i)
			if (!isspace(*i))
				name += *i;
		return Image::spacecraftID(name);
;
	}

	virtual std::string getHistory()
	{
		return history;
	}

	virtual void addToHistory(const std::string& event)
	{
		history = historyPlusEvent(event);
	}

	static GDALDataset* Open(GDALOpenInfo *);
};


GDALDataset* Dataset::Open(GDALOpenInfo* info)
{
	// If it's not a real file, it's not OpenMTP
	if (info->fp == NULL)	
		return NULL;

	// Not enough header info
	if (info->nHeaderBytes < 256)
        return NULL;

	// Look for an openmtp signature in the header
	string header((const char*)info->pabyHeader, 0, info->nHeaderBytes);
	if (header.find("FormatID       OpenMTP") == string::npos)
		return NULL;

	string filename(info->pszFilename);
	auto_ptr<OpenMTP> omtp;
	omtp.reset(new OpenMTP);
	omtp->open(filename.c_str());

	if (!(omtp->is_A_format() || omtp->is_B_format()))
		throw std::runtime_error("Currently supporting only A or B format images");

	// Hand over ncf management to the dataset
	auto_ptr<Dataset> ds(new Dataset(omtp.release()));

    return ds.release();
}

}
}

extern "C" {
void GDALRegister_OpenMTP(void)
{
    if (GDALGetDriverByName("OpenMTP") == NULL)
    {
		auto_ptr<GDALDriver> driver(new GDALDriver());
        driver->SetDescription("OpenMTP");
        driver->SetMetadataItem(GDAL_DMD_LONGNAME, "OpenMTP old Meteosat image format");
        //driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
        //driver->SetMetadataItem(GDAL_DMD_EXTENSION, "mem");
        driver->pfnOpen = msat::openmtp::Dataset::Open;
        GetGDALDriverManager()->RegisterDriver(driver.release());
    }
}
}

#if 0
//
// Creates NetCDF product
//
bool NetCDFProduct( char *inpath )
{
  OpenMTP omtp;
  struct tm tmtime;
  char NcName[1024];
  char title[64];
  char reftime[64];
  int cfac, lfac, coff, loff;
  float scale;
  int wd, hg;
  int bpp;
  float *cal;
  NcVar *ivar;
  NcVar *tvar;
  NcDim *tdim;
  NcDim *ldim;
  NcDim *cdim;
  NcDim *caldim;

  omtp.open( inpath );

  tmtime = omtp.get_datetime( );

  // Build up output NetCDF file name and open it
  sprintf( NcName, "%s_%s_%s.nc", omtp.get_satellite_code( ),
           omtp.get_field_name( ), omtp.get_timestr( ));
  NcFile ncf ( NcName , NcFile::Replace );
  if (! ncf.is_valid()) return false;

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  if (! ncf.add_att("Satellite", omtp.get_satellite_name( ))) return false;
  if (! ncf.add_att("Antenna", "Meteosat Archive")) return false;
  if (! ncf.add_att("Receiver", "Meteosat Archive")) return false;
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
      tmtime.tm_year + 1900, tmtime.tm_mon + 1, tmtime.tm_mday,
      tmtime.tm_hour, tmtime.tm_min);
  if (! ncf.add_att("Time", reftime) ) return false;

  if (omtp.is_A_format( ))
  {
    if (! ncf.add_att("Area_Name", "AFormat" ) ) return false;
    if (omtp.is_ir_data( ) || omtp.is_wv_data( ))
    {
      cfac = -9102222;
      lfac = -9102222;
      coff = 1248;
      loff = 1249;
      scale = 1.0;
    }
    else if (omtp.is_vis_data( ))
    {
      cfac = -18204444;
      lfac = -18204444;
      coff = 2500;
      loff = 2500;
      scale = 1.0;
    }
    else
      throw "Unsupported AFormat image (?)\n";
  }
  else if (omtp.is_B_format( ))
  {
    if (! ncf.add_att("Area_Name", "BFormat" ) ) return false;
    cfac = -18204444;
    lfac = -18204444;
    coff = 1248;
    loff = -1118;
    scale = 2.0;
  }
  else
    throw "Currently supporting only A or B format images\n";

  if (! ncf.add_att("Projection", "GEOS(0.0)") ) return false;
  if (! ncf.add_att("Columns", omtp.npixels( ) * (int) scale) ) return false;
  if (! ncf.add_att("Lines", omtp.nlines( ) * (int) scale) ) return false;
  if (! ncf.add_att("SampleX", scale) ) return false;
  if (! ncf.add_att("SampleY", scale) ) return false;
  if (! ncf.add_att("Column_Scale_Factor", cfac) ) return false;
  if (! ncf.add_att("Line_Scale_Factor", lfac) ) return false;
  if (! ncf.add_att("Column_Offset", coff) ) return false;
  if (! ncf.add_att("Line_Offset", loff) ) return false;
  if (! ncf.add_att("Orbit_Radius", omtp.orbit_radius( )) ) return false;
  if (! ncf.add_att("Longitude", omtp.subsatellite_point( )) ) return false;
//  if (! ncf.add_att("NortPolar", omtp.is_NortPolar( )) ) return false;
//  if (! ncf.add_att("NorthSouth", omtp.is_NorthSouth( )) ) return false;
  if (! ncf.add_att("Institution", INSTITUTION) ) return false;
  if (! ncf.add_att("Conventions", "COARDS") ) return false;
  if (! ncf.add_att("history", "Created from OpenMTP format data") )
    return false;
  sprintf(title, "OpenMTP %s product for %04d-%02d-%02d %02d:%02d:00 UTC",
            omtp.get_field_name( ), tmtime.tm_year + 1900, tmtime.tm_mon + 1,
            tmtime.tm_mday, tmtime.tm_hour, tmtime.tm_min);
  if (! ncf.add_att("title", title) ) return false;

  // Dimensions
  wd = omtp.npixels( );
  hg = omtp.nlines( );
  bpp = (int) pow(2.0, omtp.bits_per_pixel( ));

  // Define dimensions
  tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) return false;
  ldim = ncf.add_dim("line", hg);
  if (!ldim->is_valid()) return false;
  cdim = ncf.add_dim("column", wd);
  if (!cdim->is_valid()) return false;
  caldim = ncf.add_dim("calibration", bpp);
  if (!caldim->is_valid()) return false;

  // Get calibration values
  cal = omtp.get_calibration( );

  // Add Calibration values
  NcVar *cvar = ncf.add_var("calibration", ncFloat, caldim);
  if (!cvar->is_valid()) return false;
  cvar->add_att("long_name", "Calibration coefficients");
  cvar->add_att("variable", omtp.get_chname( ));
  cvar->add_att("units", omtp.get_chunit( ));
  if (!cvar->put(cal, bpp)) return false;

  tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) return false;
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime;
  time_t ttime;
  extern long timezone;
  ttime = mktime(&tmtime);
  atime = ttime - 946684800 - timezone;
  if (!tvar->put(&atime, 1)) return false;

  // Define channel values variable
  ivar = ncf.add_var(omtp.get_chname( ), ncByte, tdim, ldim, cdim);
  if (!ivar->is_valid()) return false;
  if (!ivar->add_att("add_offset", 0.0)) return false;
  if (!ivar->add_att("scale_factor", 1.0)) return false;

  // Write output values
  if (!ivar->put(omtp.get_image( ), 1, hg, wd)) return false;

  // Close NetCDF output
  (void) ncf.close( );

  return( true );
}
#endif

// vim:set ts=2 sw=2:
