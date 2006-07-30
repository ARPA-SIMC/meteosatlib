//---------------------------------------------------------------------------
//
//  File        :   NetCDF2Grib.cpp
//  Description :   Convert NetCDF format in Grib format
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

//#include "../config.h"

#include <conv/ImportNetCDF.h>
#include <conv/ExportGRIB.h>
#include <conv/ExportNetCDF.h>

#include <set>
#include <string>
#include <vector>
#include <stdexcept>

// HDF5 format interface
#include <H5Cpp.h>

// Grib Library
#include <grib/GRIB.h>

// For MSG_channel_name
#include <hrit/MSG_HRIT.h>

#include <getopt.h>

using namespace H5;

static char rcs_id_string[] = "$Id$";

void do_help(const char* argv0, ostream& out)
{
  out << "Usage: " << argv0 << " [options] file" << endl << endl
      << "Convert the given NetCDF file to a Grib message" << endl << endl
      << "Options are:" << endl
      << "  --help   Print this help message" << endl
      << "  --view   View the contents of a file" << endl
      << "  --grib   Convert to GRIB (default unless invoked as NetCDF2GRIB)" << endl
      << "  --netcdf Convert to NetCDF (default when invoked as NetCDF2NetCDF)" << endl;
}
void usage(char *pname)
{
  std::cout << pname << ": Convert NetCDF files to Grib format."
	    << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << " filename.nc"
            << std::endl << std::endl
            << "Example: " << std::endl << "\t" << pname
//            << " SAFNWC_MSG1_CT___04356_051_EUROPE______.h5"
            << std::endl;
  return;
}

void view(const ImageData& img);
void dump(const ImageData& img);
void convertGrib(const ImageData& img);
void convertNetCDF(const ImageData& img);

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in NetCDF format */
/* ************************************************************************* */

int main( int argc, char* argv[] )
{
  enum { VIEW, DUMP, CONVERTGRIB, CONVERTNETCDF } action = CONVERTGRIB;

  {
    string argv0(argv[0]);
    if (argv0.size() >= 12 && argv0.substr(argv0.size() - 12) == "SAFH52NetCDF")
      action = CONVERTNETCDF;
  }

  static struct option longopts[] = {
    	{ "help",	0, NULL, 'H' },
	{ "view",	0, NULL, 'V' },
	{ "dump",	0, NULL, 'D' },
	{ "grib",	0, NULL, 'G' },
	{ "netcdf",	0, NULL, 'N' },
  };

  bool done = false;
  while (!done) {
    int c = getopt_long(argc, argv, "i:", longopts, (int*)0);
    switch (c) {
      case 'H':	// --help
				do_help(argv[0], cout);
				return 0;
      case 'V': // --view
				action = VIEW;
				break;
      case 'D': // --dump
				action = DUMP;
				break;
      case 'G': // --grib
				action = CONVERTGRIB;
				break;
      case 'N': // --netcdf
				action = CONVERTNETCDF;
				break;
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

  try
  {
		string name = argv[optind];
		std::auto_ptr<ImageData> image = ImportNetCDF(name);

    switch (action)
    {
      case VIEW:
        view(*image);
        break;
      case DUMP:
        dump(*image);
        break;
      case CONVERTGRIB:
        convertGrib(*image);
        break;
      case CONVERTNETCDF:
        convertNetCDF(*image);
        break;
    }
  }
  catch (std::exception& e)
  {
    cerr << e.what() << endl;
    return 1;
  }
  
  return(0);
}

void view(const ImageData& img)
{
	cout << img.name << " " << img.datetime() << endl;
	cout << " proj: " << img.projection << " ch.id: " << img.channel_id << " sp.id: " << img.spacecraft_id << endl;
	cout << " size: " << img.columns << "x" << img.lines << " factor: " << img.column_factor << "x" << img.line_factor
			 << " offset: " << img.column_offset << "x" << img.line_offset << endl;

	cout << " Images: " << endl;

	cout << "  " //<< *i
		 << "\t" << img.columns << "x" << img.lines << " " << img.bpp << "bpp"
		" *" << img.slope << "+" << img.offset
		<< " PSIZE " << img.pixelSize()
		<< " DX " << img.seviriDX()
		<< " DXY " << img.seviriDY()
		<< " CHID " << img.channel_id
 << endl;
}

void dump(const ImageData& img)
{
	cout << img.name << " " << img.datetime() << endl;
	cout << " proj: " << img.projection << " ch.id: " << img.channel_id << " sp.id: " << img.spacecraft_id << endl;
	cout << " size: " << img.columns << "x" << img.lines << " factor: " << img.column_factor << "x" << img.line_factor
			 << " offset: " << img.column_offset << "x" << img.line_offset << endl;

	cout << " Images: " << endl;

	cout << "  " //<< *i
			 << "\t" << img.columns << "x" << img.lines << " " << img.bpp << "bpp"
					" *" << img.slope << "+" << img.offset << " decscale: " << img.decimalScale()
			 << endl;
	cout << "Coord\tUnscaled\tScaled" << endl;
	for (int l = 0; l < img.lines; ++l)
		for (int c = 0; c < img.lines; ++c)
			cout << c << "x" << l << '\t' << img.unscaled(c, l) << '\t' << img.scaled(c, l) << endl;
}

void convertGrib(const ImageData& img)
{
	char GribName[1024];
	GRIB_FILE gf;
	GribName[0] = 0;

	bool first = true;

	// Build up output Grib file name and open it
	sprintf( GribName, "MSG_SAFNWC_%s_%4d%02d%02d_%02d%02d.grb", img.name.c_str(),
		img.year, img.month, img.day, img.hour, img.minute);
	int ret = gf.OpenWrite(GribName);
	if (ret != 0)
		throw std::runtime_error(string("error writing grib file ") + GribName);

//	cout << "Converting " << *i << "..." << endl;
	ExportGRIB(img, gf);

  // Close Grib output
	if (GribName[0])
	{
		int ret = gf.Close( );
		if (ret != 0)
			throw std::runtime_error("closing grib file");
		cout << "Wrote image in file " << GribName << endl;
	} else {
		cout << "No images found" << endl;
	}
}

void convertNetCDF(const ImageData& img)
{
	// Get the channel name
	string channelstring = MSG_channel_name((t_enum_MSG_spacecraft)img.spacecraft_id, img.channel_id);
	// Change sensitive characters into underscores
	for (string::iterator i = channelstring.begin();
	i != channelstring.end(); ++i)
		if (*i == ' ' || *i == '.' || *i == ',')
			*i = '_';
	// Build up output NetCDF file name and open it
	char NcName[1024];
	sprintf(NcName, "%s_%4d%02d%02d_%02d%02d.nc", channelstring.c_str(),
					 img.year, img.month, img.day, img.hour, img.minute);

	ExportNetCDF(img, NcName);
}

// vim:set ts=2 sw=2:
