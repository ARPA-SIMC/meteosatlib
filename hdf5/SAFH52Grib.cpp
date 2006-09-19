//---------------------------------------------------------------------------
//
//  File        :   H52Grib.cpp
//  Description :   Convert HDF5 format in Grib format
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

#include <conv/ImportSAFH5.h>
#include <conv/ExportGRIB.h>
#include <conv/ExportNetCDF.h>
#include "Utils.h"

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
using namespace msat;

static char rcs_id_string[] = "$Id$";

void do_help(const char* argv0, ostream& out)
{
  out << "Usage: " << argv0 << " [options] file" << endl << endl
      << "Convert the given SAF HDF5 file to one or more Grib messages" << endl << endl
      << "Options are:" << endl
      << "  --help   Print this help message" << endl
      << "  --view   View the contents of a file" << endl
      << "  --images Comma-separated list of images to convert (defaults to all)" << endl
      << "  --grib   Convert to GRIB (default unless invoked as SAFH52NetCDF)" << endl
      << "  --netcdf Convert to NetCDF (default when invoked as SAFH52NetCDF)" << endl;
}
void usage(char *pname)
{
  std::cout << pname << ": Convert SAF HDF5 files to Grib format."
	    << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << " filename.h5"
            << std::endl << std::endl
            << "Example: " << std::endl << "\t" << pname
            << " SAFNWC_MSG1_CT___04356_051_EUROPE______.h5"
            << std::endl;
  return;
}

void view(H5File& source);
void dump(H5File& file, const std::set<std::string>& selected);
void convertGrib(H5File& file, const std::set<std::string>& selected);
void convertNetCDF(H5File& file, const std::set<std::string>& selected);

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in NetCDF format */
/* ************************************************************************* */

int main( int argc, char* argv[] )
{
  enum { VIEW, DUMP, CONVERTGRIB, CONVERTNETCDF } action = CONVERTGRIB;
  set<string> sel_imgs;

  {
    string argv0(argv[0]);
    if (argv0.size() >= 12 && argv0.substr(argv0.size() - 12) == "SAFH52NetCDF")
      action = CONVERTNETCDF;
  }

  static struct option longopts[] = {
    	{ "help",	0, NULL, 'H' },
	{ "view",	0, NULL, 'V' },
	{ "images",	1, NULL, 'i' },
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
      case 'i': { // --images
	string args(optarg);
	size_t e;
	while ((e = args.find(',')) != string::npos)
	{
	  sel_imgs.insert(args.substr(0, e));
	  args = args.substr(e + 1);
	}
	sel_imgs.insert(args);
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

  if (! H5File::isHdf5(argv[optind]))
  {
    cerr << "File " << argv[optind] << " is not in HDF5 format." << endl;
    return 1;
  }

  if (!sel_imgs.empty())
  {
    cout << "Selected: ";
    for (set<string>::const_iterator i = sel_imgs.begin();
	i != sel_imgs.end(); ++i)
      cout << (i == sel_imgs.begin() ? "" : ", ") << *i;
    cout << endl;
  }

  auto_ptr<H5File> HDF5_source;
  try
  {
    Exception::dontPrint();
    HDF5_source.reset(new H5File(argv[optind], H5F_ACC_RDONLY));

    switch (action)
    {
      case VIEW:
        view(*HDF5_source);
        break;
      case DUMP:
        dump(*HDF5_source, sel_imgs);
        break;
      case CONVERTGRIB:
        convertGrib(*HDF5_source, sel_imgs);
        break;
      case CONVERTNETCDF:
        convertNetCDF(*HDF5_source, sel_imgs);
        break;
    }
  }
  catch (FileIException& error)
  {
    error.printError();
    cerr << "Detail: " << error.getDetailMsg() << endl;
    cerr << "Func: " << error.getFuncName() << endl;
    return 1;
  }
  catch (Exception& error)
  {
    error.printError();
    cerr << "Detail: " << error.getDetailMsg() << endl;
    cerr << "Func: " << error.getFuncName() << endl;

    return 1;
  }
  catch (std::exception& e)
  {
    cerr << e.what() << endl;
    return 1;
  }
  
  return(0);
}

vector<string> getImages(Group& group)
{
  vector<string> res;
  for (hsize_t i = 0; i < group.getNumObjs(); ++i)
  {
    string name = group.getObjnameByIdx(i);
    DataSet dataset = group.openDataSet(name);
    string c = readStringAttribute(dataset, "CLASS");
    if (c == "IMAGE")
      res.push_back(name);
  }
  return res;
}

void view(H5File& file)
{
  Group group = file.openGroup("/");
  vector<string> images = getImages(group);
  for (vector<string>::const_iterator i = images.begin();
      i != images.end(); ++i)
  {
    auto_ptr<Image> img = ImportSAFH5(group, *i);
		
		if (i == images.begin())
		{
			cout << img->name << " " << img->datetime() << endl;
			cout << " proj: " << img->projection << " ch.id: " << img->channel_id << " sp.id: " << img->spacecraft_id << endl;
			cout << " size: " << img->data->columns << "x" << img->data->lines << " factor: " << img->column_factor << "x" << img->line_factor
					 << " offset: " << img->column_offset << "x" << img->line_offset << endl;

			cout << " Images: " << endl;
		}

    cout << "  " << *i
    	 << "\t" << img->data->columns << "x" << img->data->lines << " " << img->data->bpp << "bpp"
	    " *" << img->data->slope << "+" << img->data->offset
			<< " PSIZE " << img->pixelSize()
			<< " DX " << img->seviriDX()
			<< " DXY " << img->seviriDY()
			<< " CHID " << img->channel_id
	 << endl;
  }
}

void dump(H5File& file, const std::set<std::string>& selected)
{
  Group group = file.openGroup("/");
  vector<string> images = getImages(group);
	bool first = true;
  for (vector<string>::const_iterator i = images.begin();
      i != images.end(); ++i)
  {
    if (!selected.empty() && selected.find(*i) == selected.end())
    	continue;
    auto_ptr<Image> img = ImportSAFH5(group, *i);
		if (first)
		{
			cout << img->name << " " << img->datetime() << endl;
			cout << " proj: " << img->projection << " ch.id: " << img->channel_id << " sp.id: " << img->spacecraft_id << endl;
			cout << " size: " << img->data->columns << "x" << img->data->lines << " factor: " << img->column_factor << "x" << img->line_factor
					 << " offset: " << img->column_offset << "x" << img->line_offset << endl;

			cout << " Images: " << endl;
			first = false;
		}

    cout << "  " << *i
    		 << "\t" << img->data->columns << "x" << img->data->lines << " " << img->data->bpp << "bpp"
						" *" << img->data->slope << "+" << img->data->offset << " decscale: " << img->data->decimalScale()
				 << endl;
    cout << "Coord\tUnscaled\tScaled" << endl;
    for (int l = 0; l < img->data->lines; ++l)
      for (int c = 0; c < img->data->lines; ++c)
				cout << c << "x" << l << '\t' << img->data->unscaled(c, l) << '\t' << img->data->scaled(c, l) << endl;
  }
}

void convertGrib(H5File& file, const std::set<std::string>& selected)
{
  Group group = file.openGroup("/");
  vector<string> images = getImages(group);

	char GribName[1024];
	GRIB_FILE gf;
	GribName[0] = 0;

	bool first = true;
	int count = 0;
  for (vector<string>::const_iterator i = images.begin();
      i != images.end(); ++i)
  {
    if (!selected.empty() && selected.find(*i) == selected.end())
			continue;
    auto_ptr<Image> img = ImportSAFH5(group, *i);
		if (first)
		{
			if (images.size() == 1)
			{
				// Build up output Grib file name and open it
				sprintf( GribName, "MSG_SAFNWC_%s_%4d%02d%02d_%02d%02d.grb", img->name.c_str(),
					img->year, img->month, img->day, img->hour, img->minute);
			} else {
				// Build up output Grib file name and open it
				sprintf( GribName, "MSG_SAFNWC_%s_%4d%02d%02d_%02d%02d.grb", img->name.c_str(),
					img->year, img->month, img->day, img->hour, img->minute);
			}
			int ret = gf.OpenWrite(GribName);
			if (ret != 0)
				throw std::runtime_error(string("error writing grib file ") + GribName);

			first = false;
		}

		cout << "Converting " << *i << "..." << endl;
		ExportGRIB(*img, gf);
		++count;
  }

  // Close Grib output
	if (GribName[0])
	{
		int ret = gf.Close( );
		if (ret != 0)
			throw std::runtime_error("closing grib file");
		cout << "Wrote " << count << " images in file " << GribName << endl;
	} else {
		cout << "No images found" << endl;
	}
}

void convertNetCDF(H5File& file, const std::set<std::string>& selected)
{
  Group group = file.openGroup("/");
  vector<string> images = getImages(group);

  for (vector<string>::const_iterator i = images.begin();
      i != images.end(); ++i)
  {
    if (!selected.empty() && selected.find(*i) == selected.end())
			continue;

		cout << "Converting " << *i << "..." << endl;
    auto_ptr<Image> img = ImportSAFH5(group, *i);

		// Get the channel name
		string channelstring = MSG_channel_name((t_enum_MSG_spacecraft)img->spacecraft_id, img->channel_id);
		// Change sensitive characters into underscores
		for (string::iterator i = channelstring.begin();
		i != channelstring.end(); ++i)
			if (*i == ' ' || *i == '.' || *i == ',')
				*i = '_';
		// Build up output NetCDF file name and open it
		char NcName[1024];
		sprintf(NcName, "%s_%4d%02d%02d_%02d%02d.nc", channelstring.c_str(),
						 img->year, img->month, img->day, img->hour, img->minute);

		ExportNetCDF(*img, NcName);
  }
}

// vim:set ts=2 sw=2:
