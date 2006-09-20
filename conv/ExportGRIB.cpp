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

#include <grib/GRIB.h>
#include <cmath>
#include <stdexcept>
#include "parameters.h"

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

//#define EXTENDED_PDS

namespace msat {

//
// Export data from an ImageData into a GRIB_FILE
//
void ExportGRIB(const Image& img, GRIB_FILE& gf)
{
  //
  // Extract data out of HDF5_source
  // 

  // Get image dataset
  float cal_offset = img.data->offset;
  float cal_slope = img.data->slope;

	/*
  cout << "Scaling factor: " << cal_offset << endl;
  cout << "Offset: " << cal_slope << endl;
  cout << "nlines: " << img.lines << endl;
  cout << "ncols: " << img.columns << endl;
  cout << "Read " << img.columns << "x" << img.lines << " image"
          " (" << img.columns * img.lines << " " << img.bpp << "bit samples)" << endl;
	*/

  //
  // Create the Grib file with the data we got
  //
#ifdef EXTENDED_PDS
  GRIB_MSG_PDS pds;

  // Fill pds extra section
  pds.spc = (unsigned short) img.spacecraft_id;
  // Spectral channel id
  // Mettere la versione rimappata (fakechan)
  // TODO fdg: cosa va nel chn 'ristretto'?
  pds.chn = (unsigned char) img.channel_id;
  // Subsatellite longitude
  pds.sublon = 0;
  pds.npix = METEOSAT_IMAGE_NCOLUMNS;
  pds.nlin = METEOSAT_IMAGE_NLINES;
  pds.cfac = img.column_factor;
  pds.lfac = img.line_factor;
  pds.coff = img.column_offset;
  pds.loff = img.line_offset;
  // Satellite height (hopefully constant for Meteosat)
  pds.sh = SEVIRI_CAMERA_H * 1000000;
  // Calibration offset
  pds.cal_offset = cal_offset;
  // Calibration slope
  pds.cal_slope = cal_slope;
#endif

  GRIB_MESSAGE m;
  GRIB_GRID grid;
  GRIB_LEVEL l;
  GRIB_TIME t;
  GRIB_FIELD f;

  t.set(img.year, img.month, img.day, img.hour, img.minute,
        GRIB_TIMEUNIT_MINUTE, GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1,
        0, 0, 0, 0);

  // Satellite identifier, satellite spectral band, ?
#if LOCALDEF == 3
  l.set(GRIB_LEVEL_SATELLITE_METEOSAT8, img.channel_id, LOCALDEF3FUNC);
#else
  l.set(GRIB_LEVEL_SATELLITE_METEOSAT8, (img.channel_id >> 8) & 255, img.channel_id & 255);
#endif


  // Dimensions
  grid.set_size(img.data->columns, img.data->lines);
  grid.set_earth_spheroid();
  grid.is_dirincgiven = true;

/*
 *  Data represent type = space/ortho  (Table 6)        90
 *  Number of points along X axis.                    1300
 *  Number of points along Y axis.                     700
 *  Latitude of sub-satellite point.                     0
 *  Longitude of sub-satellite point.                    0
 *  Diameter of the earth in x direction.             3623
 *  Diameter of the earth in y direction.             3623
 *  X coordinate of sub-satellite point.              1856
 *  Y coordinate of sub-satellite point.              1856
 *  Scanning mode flags (Code Table 8)            00000000
 *  Number of vertical coordinate parameters.            0
 *  Orientation of the grid.                        180000
 *  Altitude of the camera.                        6610710
 *  Y coordinate of origin of sector image.           1500
 *  X coordinate of origin of sector image.            200
 *  Earth flag                                          64
 *  Components flag                                      0
 */

  // Earth Equatorial Radius is 6378.160 Km (IAU 1965)
  grid.set_spaceview(0.0, 0.0,
									img.seviriDX() / 1000, img.seviriDY() / 1000,
                  METEOSAT_IMAGE_NCOLUMNS/2, METEOSAT_IMAGE_NLINES/2,
									SEVIRI_ORIENTATION, SEVIRI_CAMERA_H * 1000,
									METEOSAT_IMAGE_NCOLUMNS/2-img.column_offset + 1, METEOSAT_IMAGE_NLINES/2-img.line_offset + 1);

  // Get the calibrated image
  float *fvals = img.data->allScaled();

  f.set_table(GRIB_CENTER, GRIB_SUBCENTER, GRIB_TABLE, GRIB_PROCESS);
  f.set_field(GRIB_PARAMETER_IMG_D, fvals, img.data->lines * img.data->columns, FILL_VALUE, FILL_VALUE);
  f.set_scale(img.data->decimalScale());

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

//  std::cout << "Adding local use PDS of " << sizeof(GRIB_MSG_PDS) << " bytes"
//            << std::endl;
#ifdef EXTENDED_PDS
  size_t add_total = LOCALDEF3LEN + sizeof(GRIB_MSG_PDS);
  unsigned char *pdsadd = new unsigned char[add_total];
  memcpy(pdsadd, localdefinition3, LOCALDEF3LEN);
  memcpy(pdsadd+LOCALDEF3LEN, &pds, sizeof(GRIB_MSG_PDS));
  f.add_local_def(add_total, pdsadd);
	delete[] pdsadd;
#else
  f.add_local_def(LOCALDEF3LEN, localdefinition3);
#endif

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
  localdefinition24[9] = (LOCALDEF24SATID >> 8) & 255;
  localdefinition24[10] = LOCALDEF24SATID & 255;
  localdefinition24[11] = (LOCALDEF24INSTR >> 8) & 255;
  localdefinition24[12] = LOCALDEF24INSTR & 255;
  localdefinition24[13] = (img.channel_id >> 8) & 255;
  localdefinition24[14] = img.channel_id & 255;
  localdefinition24[15] = LOCALDEF24FUNC;

//  std::cout << "Adding local use PDS of " << sizeof(GRIB_MSG_PDS) << " bytes"
//            << std::endl;
#ifdef EXTENDED_PDS
  size_t add_total = LOCALDEF24LEN + sizeof(GRIB_MSG_PDS);
  unsigned char *pdsadd = new unsigned char[add_total];
  memcpy(pdsadd, localdefinition24, LOCALDEF24LEN);
  memcpy(pdsadd+LOCALDEF24LEN, &pds, sizeof(GRIB_MSG_PDS));
  f.add_local_def(add_total, pdsadd);
	delete[] pdsadd;
#else
  f.add_local_def(LOCALDEF24LEN, localdefinition24);
#endif

#endif
#endif

  // Write output values
  m.set_time(t);
  m.set_grid(grid);
  m.set_level(l);
  m.set_field(f);

  gf.WriteMessage(m);

  delete [ ] fvals;
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

	virtual void processImage(const Image& img)
	{
		if (first)
		{
			char GribName[1024];
			GribName[0] = 0;

			// Build up output Grib file name and open it
			sprintf( GribName, "MSG_SAFNWC_%s_%4d%02d%02d_%02d%02d.grb",
					img.name.c_str(),
					img.year, img.month, img.day, img.hour, img.minute);

			int ret = gf.OpenWrite(GribName);
			if (ret != 0)
				throw std::runtime_error(std::string("error writing grib file ") + GribName);
			first = false;
		}

		//cout << "Converting " << *i << "..." << endl;
		ExportGRIB(img, gf);
  }
};

std::auto_ptr<ImageConsumer> createGribExporter()
{
	return std::auto_ptr<ImageConsumer>(new GRIBExporter());
}

}

// vim:set ts=2 sw=2:
