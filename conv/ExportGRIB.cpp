//---------------------------------------------------------------------------
//
//  File        :   GRIBExport.cpp
//  Description :   Write a GRIB using an ImageData structure
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  Source      :   derived from SAFH5CT2NetCDF.cpp by Le Duc, as modified by
//                  Francesca Di Giuseppe and from XRIT2Grib.cpp by Graziano
//                  Giuliani (Lamma Regione Toscana)
//  RCS ID      :   $Id$
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

#include "ExportGRIB.h"

#include <conv/Progress.h>
#include <proj/const.h>
#include <proj/Geos.h>
#include <grib/GRIB.h>
#include <cmath>
#include <stdexcept>

static char rcs_id_string[] = "$Id$";

#define FILL_VALUE         9.999E-20
//#define GRIB_CENTER        98	// ARPA SIM
#define GRIB_CENTER        200	// ARPA SIM
#define GRIB_SUBCENTER     0
#define GRIB_TABLE         1
#define GRIB_PROCESS       254

#define LOCALDEF3LEN       12
#define LOCALDEF3ID        3    // Satellite image data
#define LOCALDEF3CLASS     1    // Operations
#define LOCALDEF3TYPE      40   // Image Data
#define LOCALDEF3STREAM    55   // Meteosat 8 = 55
//#define LOCALDEF3FUNC      1    // T in K - 145
#define LOCALDEF3FUNC      0    // Pixel value

#define LOCALDEF24LEN      16
#define LOCALDEF24ID       24   // Satellite image simulation (?)
#define LOCALDEF24CLASS    1    // Operations
#define LOCALDEF24TYPE     40   // Image Data
#define LOCALDEF24STREAM   55   // Meteosat 8 = 55
#define LOCALDEF24SATID    55   // Meteosat 8 = 55
#define LOCALDEF24INSTR    207  // Seviri = 207
#define LOCALDEF24FUNC     1    // T in K - 145

// What local definition to use (0 for none, 3 or 24)
#define LOCALDEF 24

namespace msat {

//
// Export data from an ImageData into a GRIB_FILE
//
void ExportGRIB(const Image& img, GRIB_FILE& gf)
{
	ProgressTask p("Exporting GRIB");

  //
  // Extract data out of HDF5_source
  // 

#ifdef EXTENDED_PDS
  // Get image dataset
  float cal_offset = img.data->offset;
  float cal_slope = img.data->slope;

  //
  // Create the Grib file with the data we got
  //
  // Calibration offset
  pds.cal_offset = cal_offset;
  // Calibration slope
  pds.cal_slope = cal_slope;
#endif

  GRIB_MESSAGE m;

	// Access the fields of m directly, to avoid a copy at m.set_* time
  GRIB_GRID& grid = m.grid;
  GRIB_LEVEL& l = m.level;
  GRIB_TIME& t = m.gtime;
  GRIB_FIELD& f = m.field;

  t.set(img.year, img.month, img.day, img.hour, img.minute,
        GRIB_TIMEUNIT_MINUTE, GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1,
        0, 0, 0, 0);

  // Satellite identifier, satellite spectral band
  l.set(GRIB_LEVEL_SATELLITE_METEOSAT8, (img.channel_id >> 8) & 255, img.channel_id & 255);


  // Dimensions
  grid.set_size(img.data->columns, img.data->lines);

	if (proj::Geos* p = dynamic_cast<proj::Geos*>(img.proj.get()))
	{
		grid.set_earth_spheroid();
		grid.is_dirincgiven = true;

		// Earth Equatorial Radius is 6378.160 Km (IAU 1965)
		grid.set_spaceview(0.0, p->sublon, img.seviriDX(), img.seviriDY(),
										img.column_offset, img.line_offset,
										SEVIRI_ORIENTATION, SEVIRI_CAMERA_H * 1000,
										img.x0, img.y0);
	} else
		throw std::runtime_error("image has projection " + (img.proj.get() ? img.proj->format() : "(null)") + " instead of geostationary");

  // * Notes on encoding data

	// We cannot unscale the values, because GRIB cannot store slope and offset
	// so we need to feed the scaled values to GRIB, losing the original slope
	// and offset.
  // So we have to ignore scaleToInt.
	
	// The only thing we can and must compute is the decimal scale, since the
	// reference value is computed by the GRIB encoder automatically by taking
	// the smallest value to encode.

	// When the original value was an integer value, we can compute the logaritm
	// in base 10 of the scaling factor, add 1 if the scaling factor is not a
	// direct power of 10 and use the result as the count of decimal digits

	// When the original value was a float value, we can read the number of
	// significant digits from a table indexed by channels

  f.set_table(GRIB_CENTER, GRIB_SUBCENTER, GRIB_TABLE, GRIB_PROCESS);

  // Get the calibrated image
	p.activity("Computing calibrated image");
  float *fvals = img.data->allScaled();
  f.set_field_nocopy(GRIB_PARAMETER_IMG_D, fvals, img.data->lines * img.data->columns, img.data->missingValue, img.data->missingValue);
  //delete [ ] fvals;

  f.set_scale(img.decimalDigitsOfScaledValues());

#if LOCALDEF == 3
  unsigned char localdefinition3[LOCALDEF3LEN];
  localdefinition3[0] = LOCALDEF3ID;			// 41
  localdefinition3[1] = LOCALDEF3CLASS;			// 42
  localdefinition3[2] = LOCALDEF3TYPE;			// 43
  localdefinition3[3] = (LOCALDEF3STREAM >> 8) & 255;	// 44
  localdefinition3[4] = LOCALDEF3STREAM & 255;		// 45
  localdefinition3[5] = 0x30; // 0			// 46
  localdefinition3[6] = 0x30; // 0			// 47
  localdefinition3[7] = 0x30; // 0			// 48
  localdefinition3[8] = 0x31; // 1			// 49
  localdefinition3[9] = img.channel_id;			// 50
  localdefinition3[10] = LOCALDEF3FUNC;			// 51
  localdefinition3[11] = 0;				// 52
  f.add_local_def(LOCALDEF3LEN, localdefinition3);

#else
#if LOCALDEF == 24
  unsigned char localdefinition24[LOCALDEF24LEN];
  localdefinition24[0] = LOCALDEF24ID;
  localdefinition24[1] = LOCALDEF24CLASS;
  localdefinition24[2] = LOCALDEF24TYPE;
  localdefinition24[3] = (LOCALDEF24STREAM >> 8) & 255;
  localdefinition24[4] = LOCALDEF24STREAM & 255;
  // Passare come parametro opzionale alla convert
  localdefinition24[5] = 0x30; // 0
  localdefinition24[6] = 0x30; // 0
  localdefinition24[7] = 0x30; // 0
  localdefinition24[8] = 0x31; // 1
  localdefinition24[9] = (img.spacecraft_id >> 8) & 255;
  localdefinition24[10] = img.spacecraft_id & 255;
  localdefinition24[11] = (LOCALDEF24INSTR >> 8) & 255;
  localdefinition24[12] = LOCALDEF24INSTR & 255;
  localdefinition24[13] = (img.channel_id >> 8) & 255;
  localdefinition24[14] = img.channel_id & 255;
  localdefinition24[15] = LOCALDEF24FUNC;
  f.add_local_def(LOCALDEF24LEN, localdefinition24);
#endif
#endif

#if 0
	// Avoid copying all these members, since we set directly the ones in m
	p.activity("Putting all the components together");
  m.set_time(t);
  m.set_grid(grid);
  m.set_level(l);
  m.set_field(f);
#endif

  // Write output values
	p.activity("Writing message");
  gf.WriteMessage(m);
}


class GRIBExporter : public ImageConsumer
{
	GRIB_FILE gf;
	bool first;

public:
	GRIBExporter() : first(true) {}
	~GRIBExporter()
	{
		int ret = gf.Close( );
		if (ret != 0)
			throw std::runtime_error("closing grib file");
	}

	virtual void processImage(std::auto_ptr<Image> img)
	{
		if (first)
		{
			std::string gribName = img->defaultFilename() + ".grb";
			int ret = gf.OpenWrite(gribName);
			if (ret != 0)
				throw std::runtime_error(std::string("error writing grib file ") + gribName);
			first = false;
		}

		//cout << "Converting " << *i << "..." << endl;
		ExportGRIB(*img, gf);
  }
};

std::auto_ptr<ImageConsumer> createGribExporter()
{
	return std::auto_ptr<ImageConsumer>(new GRIBExporter());
}

}

// vim:set ts=2 sw=2:
