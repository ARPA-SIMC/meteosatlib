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

#include <config.h>

#include <msat/ImportGRIB.h>
#include <msat/ImportSAFH5.h>
#include <msat/ImportNetCDF.h>
#include <msat/ImportNetCDF24.h>
#include <msat/ImportXRIT.h>
#include <msat/ExportGRIB.h>
#include <msat/ExportNetCDF.h>
#include <msat/ExportNetCDF24.h>
#include <msat/ExportGDTNetCDF.h>
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

void do_help(const char* argv0, ostream& out)
{
  out << "Usage: " << "msat" << " [options] file(s)..." << endl
			<< "msat can read and write various formats of satellite images or other" << endl
			<< "georeferentiated raster data." << endl
			<< "Images can be displayed, cropped and converted from a format to another." << endl
			<< endl
			<< "Options:" << endl
      << "  --help           Print detailed usage information." << endl
      << "  --version        Print the program version and exit." << endl
      << "  -q, --quiet      Work silently." << endl
      << "  --view           View the contents of a file." << endl
      << "  --dump           View the contents of a file, including the pixel data." << endl
      << "  --grib           Convert to GRIB." << endl
#ifdef HAVE_HDF5
      << "  --netcdf         Convert to NetCDF." << endl
      << "  --netcdf24       Convert to NetCDF24." << endl
      << "  --gdt            Convert to NetCDF using GDT conventions." << endl
#endif
#ifdef HAVE_MAGICKPP
      << "  --jpg            Convert to JPEG." << endl
      << "  --png            Convert to PNG." << endl
      << "  --display        Display the image on a X11 window." << endl
#endif
      << "  --area='x,dx,y,dy'  Crop the source image(s) to the given area." << endl
      << "  --Area='latmin,latmax,lonmin,lonmax'  Crop the source image(s) to the given coordinates." << endl
      << "  --around='lat,lon,lath,lonw'  Create an image centered at the given location and with the given width and height." << endl
			<< "  --resize='columns,lines'  Scale the output image to the given size." << endl
			<< "  -R,--reproject='proj:parms'  Reproject the image data (see below for the available projections)." << endl
		  << endl
      << "Formats supported are:" << endl
			<< endl
#ifdef HAVE_NETCDF
      << " NetCDF     Import/Export" << endl
      << " NetCDF24   Import/Export" << endl
      << " GDT/NetCDF Export only" << endl
#endif
      << " Grib       Import/Export" << endl
#ifdef HAVE_HDF5
      << " SAFH5      Import only" << endl
#endif
#ifdef HAVE_HRIT
      << " XRIT       Import only" << endl
#endif
			<< endl
			<< "Available projections are:" << endl
			<< endl
			<< " mercator                          Mercator" << endl
			<< " polar:central meridian:N or S     Polar" << endl
			<< " geos[:subsat lon[:orbit radius]]  Geostationary view" << endl
			<< " latlon                            Regular grid" << endl
			<< endl
			<< "Examples:" << endl
			<< endl
			<< " $ msat --display --Area=30,60,-10,40 file.grb" << endl
			<< " $ msat --jpg file.grb" << endl
			<< " $ msat --grib dir/H:MSG1:HRV:200611130800" << endl
			<< endl
			<< "Report bugs to " << PACKAGE_BUGREPORT << endl;
			;
}
void usage(char *pname)
{
  std::cout << pname << ": Convert meteosat image files from and to various formats."
	    << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << "<output option> file(s)..."
            << std::endl << std::endl
            << "Examples: " << std::endl << "\t" << pname
            << "  " << pname << " --grib Data0.nc SAFNWC_MSG1_CT___04356_051_EUROPE______.h5 hritdir/H:MSG1:HRV:200605031200"
            << std::endl;
  return;
}

enum Action { VIEW, DUMP, GRIB
#ifdef HAVE_NETCDF
	, NETCDF, NETCDF24, GDT
#endif
#ifdef HAVE_MAGICKPP
	, JPG, PNG, DISPLAY
#endif
};

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

/*
 * Created an exported based on the given action
 */
std::auto_ptr<ImageConsumer> getExporter(Action action)
{
	switch (action)
	{
		case VIEW: return createImageDumper(false);
		case DUMP: return createImageDumper(true);
		case GRIB: return createGribExporter();
#ifdef HAVE_NETCDF
		case NETCDF: return createNetCDFExporter();
		case NETCDF24: return createNetCDF24Exporter();
		case GDT: return createGDTNetCDFExporter();
#endif
#ifdef HAVE_MAGICKPP
		case JPG: return createImageExporter("jpg");
		case PNG: return createImageExporter("png");
		case DISPLAY: return createImageDisplayer();
#endif
	}
	return auto_ptr<ImageConsumer>(0);
}

#include <msat/proj/Latlon.h>
#include <msat/proj/Mercator.h>
#include <msat/proj/Polar.h>
#include <msat/proj/Geos.h>
#include <msat/proj/const.h>
struct Reprojector : public ImageConsumer
{
	size_t width;
	size_t height;
	proj::MapBox& box;
	proj::Projection& projection;
	ImageConsumer& next;

	Reprojector(size_t width, size_t height,
							proj::MapBox& box, proj::Projection& projection,
							ImageConsumer& next) :
		width(width), height(height), box(box), projection(projection), next(next) {}

	virtual void processImage(std::auto_ptr<Image> image)
	{
		//proj::MapBox box(proj::MapPoint(60,-10), proj::MapPoint(10, 50));
		//std::auto_ptr<proj::Projection> pr(new proj::Mercator);
		//proj::MapBox box(proj::MapPoint(70,-40), proj::MapPoint(10, 40));
		//std::auto_ptr<proj::Projection> pr(new proj::Polar(20.0, true));
		//std::auto_ptr<proj::Projection> pr(new proj::Geos(0, ORBIT_RADIUS));

		std::auto_ptr<proj::Projection> pr(projection.clone());
		std::auto_ptr<Image> projected = image->reproject(width, height, pr, box);
		next.processImage(projected);
	}
};

struct Resampler : public ImageConsumer
{
	int width;
	int height;
	ImageConsumer& next;

	Resampler(size_t width, size_t height, ImageConsumer& next) :
		width(width), height(height), next(next) {}

	virtual void processImage(std::auto_ptr<Image> image)
	{
		std::auto_ptr<Image> rescaled = image->rescaled(width, height);
		next.processImage(rescaled);
	}
};

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in NetCDF format */
/* ************************************************************************* */

int main( int argc, char* argv[] )
{
	// Defaults to view
  Action action = VIEW;
	// Crop rectangle in pixels
	proj::ImageBox imgArea;
	// Crop rectangle in lat-lon coordinates
	proj::MapBox geoArea;
	// Rescaling new image width and height
	size_t newWidth = 0, newHeight = 0;
	// Target projection of a reprojection
	std::auto_ptr<proj::Projection> projection;
	// Should we be verbose?
	bool quiet = false;

  static struct option longopts[] = {
    { "help",	0, NULL, 'H' },
    { "version",	0, NULL, 'v' },
    { "quiet", 0, NULL, 'q' },
		{ "view",	0, NULL, 'V' },
		{ "dump",	0, NULL, 'D' },
		{ "grib",	0, NULL, 'G' },
#ifdef HAVE_NETCDF
		{ "netcdf",	0, NULL, 'N' },
		{ "netcdf24",	0, NULL, '2' },
		{ "gdt",	0, NULL, 'g' },
#endif
#ifdef HAVE_MAGICKPP
		{ "jpg",	0, NULL, 'j' },
		{ "png",	0, NULL, 'p' },
		{ "display",	0, NULL, 'd' },
#endif
		{ "area", 1, 0, 'a' },
		{ "Area", 1, 0, 'A' },
		{ "around", 1, 0, 'C' },
		{ "resize", 1, 0, 'r' },
		{ "reproject", 1, 0, 'R' },
		{ 0, 0, 0, 0 },
  };

  bool done = false;
  while (!done) {
    int c = getopt_long(argc, argv, "qR:", longopts, (int*)0);
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
      case 'V': // --view
				action = VIEW;
				break;
      case 'D': // --dump
				action = DUMP;
				break;
      case 'G': // --grib
				action = GRIB;
				break;
#ifdef HAVE_NETCDF
      case 'N': // --netcdf
				action = NETCDF;
				break;
      case '2': // --netcdf24
				action = NETCDF24;
				break;
      case 'g': // --netcdf24
				action = GDT;
				break;
#endif
#ifdef HAVE_MAGICKPP
			case 'j': // --jpg
				action = JPG;
				break;
			case 'p': // --png
				action = PNG;
				break;
			case 'd': // --display
				action = DISPLAY;
				break;
#endif
			case 'a': {
				using namespace proj;
				int ax, ay, aw, ah;
				if (sscanf(optarg, "%d,%d,%d,%d", &ax,&aw,&ay,&ah) != 4)
				{
					cerr << "Area value should be in the format x,dx,y,dy" << endl;
					do_help(argv[0], cerr);
					return 1;
				}
				imgArea = ImageBox(ImagePoint(ax, ay), ImagePoint(ax+aw, ay+ah));
				break;
			}
			case 'A':
				if (sscanf(optarg, "%lf,%lf,%lf,%lf",
							&geoArea.bottomRight.lat,
							&geoArea.topLeft.lat,
							&geoArea.topLeft.lon,
							&geoArea.bottomRight.lon) != 4)
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
				geoArea.bottomRight.lat = lat - h/2;
				geoArea.topLeft.lat = lat + h/2;
				geoArea.topLeft.lon = lon - w/2;
				geoArea.bottomRight.lon = lon + w/2;
				break;
			}
			case 'r': {
				if (sscanf(optarg, "%zd,%zd", &newWidth,&newHeight) != 2)
				{
					cerr << "size value should be in the format width,height" << endl;
					do_help(argv[0], cerr);
					return 1;
				}
				break;
			}
			case 'R': {
				// Reproject.  The reprojection specification is in optarg.

				// Tokenize optarg
				vector<string> args;
				size_t start = 0, tend = 0, end = strlen(optarg);
				while (start < end)
				{
					tend = start + strcspn(optarg+start, ":");
					if (tend != start)
						args.push_back(string(optarg, start, tend-start));
					start = tend + strspn(optarg+tend, ":");
				}
				if (args.empty())
					throw std::runtime_error("no projection name given to -R/--reproject");

				// Get the projection name from the first element and create the
				// reprojecting consumer
				if (args[0] == "mercator")
				{
					projection.reset(new proj::Mercator);
				} else if (args[0] == "latlon") {
					projection.reset(new proj::Latlon);
				} else if (args[0] == "polar") {
					if (args.size() != 3)
						throw std::runtime_error("polar projections is specified as polar:central meridian:N or S");
					char ns = tolower(args[2][0]);
					if (ns != 'n' && ns != 's')
						throw std::runtime_error(string("the second projection parameter should be N or S, not ") + ns + " (polar projections is specified as polar:central meridian:N or S)");
					double lon = strtod(args[1].c_str(), NULL);
					bool north = ns == 'n';
					projection.reset(new proj::Polar(lon, north));
				} else if (args[0] == "geos") {
					double sublon = 0;
					double orbitradius = ORBIT_RADIUS;
					if (args.size() > 1)
						sublon = strtod(args[1].c_str(), NULL);
					if (args.size() > 2)
						orbitradius = strtod(args[2].c_str(), NULL);
					projection.reset(new proj::Geos(sublon, orbitradius));
				} else
					throw std::runtime_error(args[0] + " is an unsupported (or misspelled) projection");
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

	// Extra consistency checks
	if (projection.get() != 0 && (newWidth == 0 || newHeight == 0))
		throw std::runtime_error("when reprojecting, you need to also specify width and height with --resize");
	if (projection.get() != 0 && !geoArea.isNonZero())
		throw std::runtime_error("when reprojecting, you need to also specify the target area with --Area");

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
			importer->cropImgArea = imgArea;
			if (projection.get() == 0)
				importer->cropGeoArea = geoArea;
			std::auto_ptr<ImageConsumer> consumer = getExporter(action);
			if (newWidth != 0 && newHeight != 0)
			{
				if (projection.get() != 0)
				{
					Reprojector reproj(newWidth, newHeight, geoArea, *projection, *consumer);
					importer->read(reproj);
				} else {
					Resampler resampler(newWidth, newHeight, *consumer);
					importer->read(resampler);
				}
			} else
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
