//---------------------------------------------------------------------------
//
//  File        :   interpolate.cpp
//  Description :   Interpolate image values according to a given set of (lat,lon) coordinates
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  RCS ID      :   $Id: /local/meteosatlib/tools/msatconv.cpp 1879 2006-10-11T15:01:33.022993Z enrico  $
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

#include <msat/ImportGRIB.h>
#include <msat/ImportSAFH5.h>
#include <msat/ImportNetCDF.h>
#include <msat/ImportNetCDF24.h>
#include <msat/ImportXRIT.h>

#include <set>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>

#include <getopt.h>

using namespace std;
using namespace msat;

void do_help(const char* argv0, ostream& out)
{
  out << "Usage: " << argv0 << " [options] file(s)..." << endl << endl
      << "Convert meteosat image files from and to various formats." << endl << endl
      << "Formats supported are:" << endl
      << "  NetCDF   Import" << endl
      << "  NetCDF24 Import" << endl
      << "  Grib     Import" << endl
      << "  SAFH5    Import" << endl
      << "  XRIT     Import" << endl
      << "Options are:" << endl
      << "  --help           Print this help message" << endl
      << "  --area='x,y,w,h' Crop the source image(s) to the given area" << endl;
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
	int ax = 0, ay = 0, aw = 0, ah = 0;

  static struct option longopts[] = {
    { "help",	0, NULL, 'H' },
		{ "area", 1, 0, 'a' },
  };

  bool done = false;
  while (!done) {
    int c = getopt_long(argc, argv, "i:", longopts, (int*)0);
    switch (c) {
      case 'H':	// --help
				do_help(argv[0], cout);
				return 0;
			case 'a':
				if (sscanf(optarg, "%d,%d,%d,%d", &ax,&ay,&aw,&ah) != 4)
				{
					cerr << "Area value should be in the format x,y,width,height" << endl;
					do_help(argv[0], cerr);
					return 1;
				}
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

	// Read the images
	ImageVector imgs;
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
			importer->cropImgArea = proj::ImageBox(proj::ImagePoint(ax, ay), proj::ImagePoint(ax+aw, ay+ah));
			importer->read(imgs);
		}
  }
  catch (std::exception& e)
  {
    cerr << e.what() << endl;
    return 1;
  }

	/*
	for (ImageVector::const_iterator i = imgs.begin();
			i != imgs.end(); ++i)
		cerr << "Read " << (*i)->data->columns << 'x' << (*i)->data->lines << " image." << endl;
	*/

	while (!feof(stdin))
	{
		double lat, lon;
		char buf[100];
		fgets(buf, 99, stdin);
		sscanf(buf, "%lf,%lf", &lat, &lon);
		fprintf(stdout, "%f,%f", lat, lon);
		for (ImageVector::const_iterator i = imgs.begin();
					i != imgs.end(); ++i)
		{
			int x, y;
			(*i)->coordsToPixels(lat, lon, x, y);
			//fprintf(stderr, "  (%f,%f) -> (%d,%d)\n", lat, lon, x, y);
			if (x < 0 || (unsigned)x >= (*i)->data->columns || y < 0 || (unsigned)y >= (*i)->data->lines)
				fprintf(stdout, ",");
			else
				fprintf(stdout, ",%f", (*i)->data->scaled(x, y));
		}
		fprintf(stdout, "\n");
	}
  
  return(0);
}

// vim:set ts=2 sw=2:
