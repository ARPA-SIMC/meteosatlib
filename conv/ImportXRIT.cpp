//---------------------------------------------------------------------------
//
//  File        :   ImportXRIT.cpp
//  Description :   Import MSG HRIT format
//  Project     :   Lamma 2004
//  Author      :   Graziano Giuliani (Lamma Regione Toscana)
//                  modified by Enrico Zini (ARPA Emilia Romagna)
//  Source      :   n/a
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

#include <ImportXRIT.h>
#include <stdexcept>
#include <fstream>
#include <hrit/MSG_HRIT.h>
#include <glob.h>

using namespace std;

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

// Pad the string 'base' with trailing underscores to ensure it's at least
// final_len characters long
static std::string underscoreit(const std::string& base, int final_len)
{
	string res = base;
	res.resize(final_len, '_');
	return res;
}

void XRITImportOptions::ensureComplete() const
{
	if (directory.empty())
		throw std::runtime_error("source directory is missing");
	if (resolution.empty())
		throw std::runtime_error("resolution is missing");
	if (productid1.empty())
		throw std::runtime_error("first product ID is missing");
	if (productid2.empty())
		throw std::runtime_error("second product ID is missing");
	if (timing.empty())
		throw std::runtime_error("timing is missing");
}

std::string XRITImportOptions::prologueFile() const
{
  std::string filename = directory
		       + PATH_SEPARATOR
					 + resolution
		       + "-???-??????-"
					 + underscoreit(productid1, 12) + "-"
					 + underscoreit("_", 9) + "-"
					 + "PRO______-"
					 + timing
					 + "-__";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str(), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

  if (globbuf.gl_pathc > 1)
    throw std::runtime_error("Non univoque prologue file.... Do not trust calibration.");

	string res(globbuf.gl_pathv[1]);
  globfree(&globbuf);
  return res;
}

std::vector<std::string> XRITImportOptions::segmentFiles() const
{
  string filename = directory
					 + PATH_SEPARATOR
					 + resolution
           + "-???-??????-"
           + underscoreit(productid1, 12) + "-"
           + underscoreit(productid2, 9) + "-"
           + "0?????___" + "-"
           + timing + "-" + "?_";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str( ), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

	std::vector<std::string> res;
	for (int i = 0; i < globbuf.gl_pathc; ++i)
		res.push_back(globbuf.gl_pathv[i+1]);
  globfree(&globbuf);
	return res;
}

std::auto_ptr<ImageImporter> createXRITImporter(const XRITImportOptions& opts)
{
	opts.ensureComplete();

  std::ifstream hrit(opts.prologueFile().c_str(), (std::ios::binary | std::ios::in));
  if (hrit.fail())
		throw std::runtime_error("Cannot open input hrit file " + opts.prologueFile());

  MSG_header PRO_head;
  PRO_head.read_from(hrit);

  MSG_data PRO_data;
  PRO_data.read_from(hrit, PRO_head);

  hrit.close();
  //std::cout << PRO_head;

	std::vector<std::string> segments = opts.segmentFiles();

  MSG_header header[segments.size()];
  MSG_data msgdat[segments.size()];

  for (int i = 0; i < segments.size(); ++i)
  {
    std::ifstream hrit(segments[i].c_str(), (std::ios::binary | std::ios::in));
    if (hrit.fail())
			throw std::runtime_error("Cannot open input hrit file " + segments[i]);
    header[i].read_from(hrit);
    msgdat[i].read_from(hrit, header[i]);
    hrit.close( );
    // std::cout << header[i];
  }

  if (header[0].segment_id->data_field_format == MSG_NO_FORMAT)
		throw std::runtime_error("Product dumped in binary format.");

  int totalsegs = header[0].segment_id->planned_end_segment_sequence_number;
  int segsindexes[totalsegs]; 

  for (int i = 0; i < totalsegs; i ++)
    segsindexes[i] = -1;

  for (int i = 0; i < segments.size(); i ++)
    segsindexes[header[i].segment_id->sequence_number-1] = i;

	// Perform image rotation and cropping on the uncalibrated data
	ImageDataWithPixels<MSG_SAMPLE> work_img;

	// Number of lines per segment
	int seglines = header[0].image_structure->number_of_lines;
  work_img.columns = header[0].image_structure->number_of_columns;
  work_img.lines = seglines * totalsegs;

  size_t npixperseg = work_img.columns * seglines;
  int SP_X0 = work_img.columns/2;
  int SP_Y0 = work_img.lines/2;

  // Assemble all the segments together
	size_t total_size = work_img.columns * work_img.lines;
  work_img.pixels = new MSG_SAMPLE[total_size];
  bzero(work_img.pixels, total_size * sizeof(MSG_SAMPLE));
  size_t pos = 0;
  for (int i = 0; i < totalsegs; i ++)
  {
    if (segsindexes[i] >= 0)
			memcpy(work_img.pixels+pos, msgdat[segsindexes[i]].image->data,
					npixperseg*sizeof(MSG_SAMPLE));
		pos += npixperseg;
  }

	// TODO: it's more efficient to slice before rotating, but the slicing area must be rotated

  // Rotate the image 180deg
	work_img.rotate180();

  // Slice the subarea if needed
  if (opts.subarea)
		work_img.crop(opts.AreaPixStart, opts.AreaLinStart, opts.AreaNpix, opts.AreaNlin);







	// Final, calibrated image
	auto_ptr< ImageDataWithPixels<float> > img(new ImageDataWithPixels<float>);

  // Get calibration values
  float *cal = PRO_data.prologue->radiometric_proc.get_calibration(
								header[0].segment_id->spectral_channel_id,
								header[0].image_structure->number_of_bits_per_pixel);
	float base;
	switch (header[0].segment_id->spectral_channel_id)
	{
  	case MSG_SEVIRI_1_5_VIS_0_6:
  	case MSG_SEVIRI_1_5_VIS_0_8:
  	case MSG_SEVIRI_1_5_HRV:
			base = 0.0;
			break;
		default:
			base = 145.0;
			break;
	}
	// Perform calibration and copy the data to the result image
	int size = work_img.columns * work_img.lines;
  img->pixels = new float[size];
  for (int i = 0; i < (int)size; ++i)
    if (work_img.pixels[i] > 0)
			img->pixels[i] = cal[work_img.pixels[i]] - base;
    else
			img->pixels[i] = 0.0;
	img->columns = work_img.columns;
	img->lines = work_img.lines;
  delete [ ] cal;


	// TODO
#if 0
  char *channelstring = strdup(MSG_channel_name((t_enum_MSG_spacecraft) pds.spc,
                               pds.chn).c_str( ));
  char *channel = chname(channelstring, strlen(channelstring) + 1);

  // Build up output Grib file name and open it
  sprintf( GribName, "%s_%4d%02d%02d_%02d%02d.grb", channel,
           tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
	   tmtime->tm_hour, tmtime->tm_min );
#endif
	img->name = "" /* TODO */;

#if 0
  MSG_data_level_15_header *p = PRO_data->prologue;
  pds.cal_offset = p->radiometric_proc.ImageCalibration[pds.chn-1].Cal_Offset;
  pds.cal_slope = p->radiometric_proc.ImageCalibration[pds.chn-1].Cal_Slope;
#endif
  img->slope = 1 /* TODO */;
  img->offset = 0 /* TODO */;

	// Image time
  struct tm *tmtime = PRO_data.prologue->image_acquisition.PlannedAquisitionTime.TrueRepeatCycleStart.get_timestruct( );
  img->year = tmtime->tm_year+1900;
	img->month = tmtime->tm_mon+1;
	img->day = tmtime->tm_mday;
	img->hour = tmtime->tm_hour;
	img->minute = tmtime->tm_min;



#if 0
  g.set_size(AreaNpix, AreaNlin);
  g.set_earth_spheroid( );
  g.is_dirincgiven = true;
  // Earth Equatorial Radius is 6378.160 Km (IAU 1965)
  g.set_spaceview(0.0, pds.sublon, SEVIRI_DX, SEVIRI_DY,
                  SP_X0, SP_Y0, SEVIRI_ORIENTATION,
                  SEVIRI_CAMERA_H, AreaPixStart+1, AreaLinStart+1);
#endif
  img->projection = "" /* TODO */;


  img->channel_id = (unsigned char ) header[0].segment_id->spectral_channel_id;
  img->spacecraft_id = header[0].segment_id->spacecraft_id;
  img->column_factor = header[0].image_navigation->column_scaling_factor;
  img->line_factor = header[0].image_navigation->line_scaling_factor;
  img->column_offset = header[0].image_navigation->column_offset;
  img->line_offset = header[0].image_navigation->line_offset;


  // FIXME: and this? pds.sublon = header[0].image_navigation->subsatellite_longitude;
  // FIXME: and this? pds.sh = header[0].image_navigation->satellite_h;

	/* FIXME: and this?
  int fakechan = 0;
  float abase = 145.0;
  if (pds.chn == MSG_SEVIRI_1_5_VIS_0_6) { fakechan = 10; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_VIS_0_8) { fakechan = 11; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_HRV)     { fakechan = 12; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_IR_1_6)   fakechan = 0;
  if (pds.chn == MSG_SEVIRI_1_5_IR_3_9)   fakechan = 1;
  if (pds.chn == MSG_SEVIRI_1_5_IR_8_7)   fakechan = 2;
  if (pds.chn == MSG_SEVIRI_1_5_IR_9_7)   fakechan = 3;
  if (pds.chn == MSG_SEVIRI_1_5_IR_10_8)  fakechan = 4;
  if (pds.chn == MSG_SEVIRI_1_5_IR_12_0)  fakechan = 5;
  if (pds.chn == MSG_SEVIRI_1_5_IR_13_4)  fakechan = 6;
  if (pds.chn == MSG_SEVIRI_1_5_WV_6_2)   fakechan = 20;
  if (pds.chn == MSG_SEVIRI_1_5_WV_7_3)   fakechan = 21;
  fakechan = pds.chn;
	*/
  return img;
}

#if 0

#include <config.h>

#include <cstdio>
#include <cmath>
#include <cstdlib>

// Grib Library

#include <GRIB.h>

#define FILL_VALUE         9.999E-20
#define GRIB_DECIMAL_SCALE 1
#define GRIB_CENTER        201 // HIMET
#define GRIB_SUBCENTER     0
#define GRIB_TABLE         1
#define GRIB_PROCESS       254

#define LOCALDEF3LEN       12
#define LOCALDEF3ID        3    // Satellite image data
#define LOCALDEF3CLASS     1    // Operations
#define LOCALDEF3TYPE      40   // Image Data
#define LOCALDEF3STREAM    55   // Meteosat 8 = 55
#define LOCALDEF3FUNC      1    // T in K - 145

#define LOCALDEF24LEN      16
#define LOCALDEF24ID       24   // Satellite image simulation (?)
#define LOCALDEF24CLASS    1    // Operations
#define LOCALDEF24TYPE     40   // Image Data
#define LOCALDEF24STREAM   55   // Meteosat 8 = 55
#define LOCALDEF24SATID    55   // Meteosat 8 = 55
#define LOCALDEF24INSTR    207  // Seviri = 207
#define LOCALDEF24FUNC     1    // T in K - 145

#define SEVIRI_SCANSTEP    (0.00025153/3.0) // scan step angle (rd) for 3 lines
#define SEVIRI_ORIENTATION 180.0
#define SEVIRI_CAMERA_H    6610.6839590101218
#define SEVIRI_DX          2.0*asin(1.0/SEVIRI_CAMERA_H)/SEVIRI_SCANSTEP
#define SEVIRI_DY          SEVIRI_DX

// HRI format interface

#include <hrit/MSG_HRIT.h>

#include <getopt.h>

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

void usage(char *pname);
std::string underscoreit(std::string base, int final_len);
char *chname(char *chdesc, int len);
bool GribProduct(MSG_header *PRO_head, MSG_data* PRO_data,
                   int totalsegs, int *segsindexes,
		   MSG_header *header, MSG_data *msgdat);

bool is_subarea = false;
int AreaLinStart = 1, AreaNlin, AreaPixStart = 1, AreaNpix;

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in Grib format */
/* ************************************************************************* */

int main( int argc, char* argv[] )
{
  char *directory = NULL;
  char *resolution = NULL;
  char *productid1 = NULL;
  char *productid2 = NULL;
  char *timing = NULL;
  char *areastring = NULL;

  char *pname = strdup(argv[0]);

  while (1) {
    int option_index = 0;
    int c;

    static struct option long_options[] = {
      {"directory", 1, 0, 'd'},
      {"resolution", 1, 0, 'r'},
      {"productid1", 1, 0, 's'},
      {"productid2", 1, 0, 'c'},
      {"time", 1, 0, 't'},
      {"subarea", 1, 0, 'a'},
      {"version", 0, 0, 'V'},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "d:r:s:c:t:a:Vh?",
                     long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'd':
       directory = strdup(optarg);
       break;
      case 'r':
       resolution = strdup(optarg);
       break;
      case 's':
       productid1 = strdup(optarg);
       break;
      case 'c':
       productid2 = strdup(optarg);
       break;
      case 't':
       timing = strdup(optarg);
       break;
      case 'a':
       is_subarea = true;
       areastring = strdup(optarg);
       sscanf(areastring, "%d,%d,%d,%d",
              &AreaLinStart, &AreaNlin, &AreaPixStart, &AreaNpix);
       AreaPixStart --;
       AreaLinStart --;
       break;
      case 'V':
       std::cout << pname << " " << PACKAGE_STRING << std::endl;
       return 0;
      case '?':
      case 'h':
       usage(pname);
       return(0);
       break;
      default:
       std::cerr << "?? getopt returned character code"
                 << std::oct << c << "??" << std::endl;
       usage(pname);
       return(1);
    }
  }

  if (resolution == NULL || productid1 == NULL ||
      productid2 == NULL || timing == NULL)
  {
    usage(pname);
    return(1);
  }

  std::string filename;

  MSG_header PRO_head;
  MSG_data PRO_data;

  if (directory) filename = directory;
  else filename = ".";

  filename = filename + PATH_SEPARATOR + resolution;
  filename = filename + "-???-??????-";
  filename = filename + underscoreit(productid1, 12) + "-";
  filename = filename + underscoreit("_", 9) + "-";
  filename = filename + "PRO______-";
  filename = filename + timing + "-__";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str( ), GLOB_DOOFFS, NULL, &globbuf)) != 0)
  {
    std::cerr << "No such file(s)." << std::endl;
    return 1;
  }

  if (globbuf.gl_pathc > 1)
  {
    std::cerr << "Non univoque prologue file.... Do not trust calibration."
              << std::endl;
    return 1;
  }

  std::ifstream hrit(globbuf.gl_pathv[1], (std::ios::binary | std::ios::in));
  if (hrit.fail())
  {
    std::cerr << "Cannot open input hrit file "
              << globbuf.gl_pathv[1] << std::endl;
      return 1;
  }
  PRO_head.read_from(hrit);
  PRO_data.read_from(hrit, PRO_head);
  hrit.close( );
  std::cout << PRO_head;

  globfree(&globbuf);

  if (directory) filename = directory;
  else filename = ".";

  filename = filename + PATH_SEPARATOR + resolution;
  filename = filename + "-???-??????-";
  filename = filename + underscoreit(productid1, 12) + "-";
  filename = filename + underscoreit(productid2, 9) + "-";
  filename = filename + "0?????___" + "-";
  filename = filename + timing + "-" + "?_";

  globbuf.gl_offs = 1;

  if ((glob(filename.c_str( ), GLOB_DOOFFS, NULL, &globbuf)) != 0)
  {
    std::cerr << "No such file(s)." << std::endl;
    return 1;
  }

  int nsegments = globbuf.gl_pathc;

  MSG_header *header;
  MSG_data *msgdat;

  header = new MSG_header[nsegments];
  msgdat = new MSG_data[nsegments];

  for (int i = 0; i < nsegments; i ++)
  {
    std::ifstream hrit(globbuf.gl_pathv[i+1],
                       (std::ios::binary | std::ios::in));
    if (hrit.fail())
    {
      std::cerr << "Cannot open input hrit file "
                << globbuf.gl_pathv[i+1] << std::endl;
      return 1;
    }
    header[i].read_from(hrit);
    msgdat[i].read_from(hrit, header[i]);
    hrit.close( );
    // std::cout << header[i];
  }

  globfree(&globbuf);

  if (header[0].segment_id->data_field_format == MSG_NO_FORMAT)
  {
    std::cout << "Product dumped in binary format." << std::endl;
    return 0;
  }

  int totalsegs = header[0].segment_id->planned_end_segment_sequence_number;
  int *segsindexes = new int[totalsegs]; 

  for (int i = 0; i < totalsegs; i ++)
    segsindexes[i] = -1;

  for (int i = 0; i < nsegments; i ++)
    segsindexes[header[i].segment_id->sequence_number-1] = i;

  std::cout << "Writing Grib output..." << std::endl;
  if(!GribProduct(&PRO_head, &PRO_data,
                    totalsegs, segsindexes, header, msgdat))
    throw "Created Grib product NOT OK";

  delete [ ] header;
  delete [ ] msgdat;
  delete [ ] segsindexes;

  return(0);
}

void usage(char *pname)
{
  std::cout << pname << ": Convert HRIT/LRIT files to Grib format."
	    << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << " [-d directory] -r resol -s prodid1 -c prodid2 -t time"
            << " [-a linestart,nlin,pixstart,npix]" << std::endl << std::endl
            << "Example: " << std::endl << "\t" << pname
            << " -d data/HRIT -r H -s MSG1 -c HRV -t 200402101315"
            << std::endl;
  return;
}

char *chname(char *chdesc, int len)
{
  char *name = new char[len];
  for (int i = 0; i < len; i ++)
    if (chdesc[i] == ' ' || chdesc[i] == '.') name[i] = '_';
    else name[i] = chdesc[i];
  return name;
}

//
// Creates Grib product
//
bool GribProduct(MSG_header *PRO_head, MSG_data* PRO_data,
                   int totalsegs, int *segsindexes,
		   MSG_header *header, MSG_data *msgdat)
{
  struct tm *tmtime;
  char GribName[1024];
  int bpp;
  int ncal;
  float *cal;
  GRIB_MSG_PDS pds;

  // std::cout << PRO_data->prologue->sat_status;
  // std::cout << PRO_data->prologue->image_acquisition;
  // std::cout << PRO_data->prologue->celestial_events;
  // std::cout << PRO_data->prologue->image_description;
  // std::cout << PRO_data->prologue->radiometric_proc;
  // std::cout << PRO_data->prologue->geometric_proc;

  int npix = header[0].image_structure->number_of_columns;
  int nlin = header[0].image_structure->number_of_lines;
  size_t npixperseg = npix*nlin;
  nlin *= totalsegs;
  int SP_X0 = npix/2;
  int SP_Y0 = nlin/2;

  size_t total_size = totalsegs*npixperseg;
  MSG_SAMPLE *pixels = new MSG_SAMPLE[total_size];
  MSG_SAMPLE *rotated = new MSG_SAMPLE[total_size];
  memset(pixels, 0, total_size*sizeof(MSG_SAMPLE));
  size_t pos = 0;
  for (int i = 0; i < totalsegs; i ++)
  {
    if (segsindexes[i] >= 0)
      memcpy(pixels+pos, msgdat[segsindexes[i]].image->data,
             npixperseg*sizeof(MSG_SAMPLE));
    pos += npixperseg;
  }

  // Rotate 180deg the image!!!
  for (int i = 0; i < nlin; i ++)
    for (int j = 0; j < npix; j ++)
      rotated[i*npix+j] = pixels[((nlin-1)-i)*npix+(npix-1-j)];

  delete [ ] pixels;
  pixels = rotated;

  // Manage subarea
  if (is_subarea)
  {
    if (AreaLinStart < 0                             ||
        AreaLinStart > nlin - AreaNlin ||
        AreaNlin > nlin - AreaLinStart)
    {
      std::cerr << "Wrong Subarea in lines...." << std::endl;
      throw;
    }
    if (AreaPixStart < 0               ||
        AreaPixStart > npix - AreaNpix ||
        AreaNpix > npix - AreaPixStart)
    {
      std::cerr << "Wrong Subarea in Pixels...." << std::endl;
      throw;
    }
    size_t newsize = AreaNpix * AreaNlin;
    MSG_SAMPLE *newpix = new MSG_SAMPLE[newsize];
    memset(newpix, 0, newsize*sizeof(MSG_SAMPLE));
    for (int i = 0; i < AreaNlin; i ++)
      memcpy(newpix + i * AreaNpix,
             pixels + (AreaLinStart + i) * npix + AreaPixStart,
             AreaNpix * sizeof(MSG_SAMPLE));
    delete [ ] pixels;
    pixels = newpix;
    total_size = newsize;
  }
  else
  {
    AreaNpix = npix;
    AreaNlin = nlin;
  }

  tmtime = PRO_data->prologue->image_acquisition.PlannedAquisitionTime.TrueRepeatCycleStart.get_timestruct( );

  // Fill pds extra section

  pds.spc = (unsigned short) header[0].segment_id->spacecraft_id;
  pds.chn = (unsigned char ) header[0].segment_id->spectral_channel_id;
  pds.sublon = header[0].image_navigation->subsatellite_longitude;
  pds.npix = npix;
  pds.nlin = nlin;
  pds.cfac = header[0].image_navigation->column_scaling_factor;
  pds.lfac = header[0].image_navigation->line_scaling_factor;
  pds.coff = header[0].image_navigation->column_offset;
  pds.loff = header[0].image_navigation->line_offset;
  pds.sh = header[0].image_navigation->satellite_h;
  MSG_data_level_15_header *p = PRO_data->prologue;
  pds.cal_offset = p->radiometric_proc.ImageCalibration[pds.chn-1].Cal_Offset;
  pds.cal_slope = p->radiometric_proc.ImageCalibration[pds.chn-1].Cal_Slope;

  char *channelstring = strdup(MSG_channel_name((t_enum_MSG_spacecraft) pds.spc,
                               pds.chn).c_str( ));
  char *channel = chname(channelstring, strlen(channelstring) + 1);

  // Build up output Grib file name and open it
  sprintf( GribName, "%s_%4d%02d%02d_%02d%02d.grb", channel,
           tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
	   tmtime->tm_hour, tmtime->tm_min );

  GRIB_FILE gf;
  int ret = gf.OpenWrite(GribName);
  if (ret != 0) return false;

  GRIB_MESSAGE m;
  GRIB_GRID g;
  GRIB_LEVEL l;
  GRIB_TIME t;
  GRIB_FIELD f;

  int fakechan = 0;
  float abase = 145.0;
  if (pds.chn == MSG_SEVIRI_1_5_VIS_0_6) { fakechan = 10; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_VIS_0_8) { fakechan = 11; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_HRV)     { fakechan = 12; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_IR_1_6)   fakechan = 0;
  if (pds.chn == MSG_SEVIRI_1_5_IR_3_9)   fakechan = 1;
  if (pds.chn == MSG_SEVIRI_1_5_IR_8_7)   fakechan = 2;
  if (pds.chn == MSG_SEVIRI_1_5_IR_9_7)   fakechan = 3;
  if (pds.chn == MSG_SEVIRI_1_5_IR_10_8)  fakechan = 4;
  if (pds.chn == MSG_SEVIRI_1_5_IR_12_0)  fakechan = 5;
  if (pds.chn == MSG_SEVIRI_1_5_IR_13_4)  fakechan = 6;
  if (pds.chn == MSG_SEVIRI_1_5_WV_6_2)   fakechan = 20;
  if (pds.chn == MSG_SEVIRI_1_5_WV_7_3)   fakechan = 21;
  fakechan = pds.chn;

  t.set(tmtime->tm_year+1900, tmtime->tm_mon+1,
        tmtime->tm_mday, tmtime->tm_hour, tmtime->tm_min,
        GRIB_TIMEUNIT_MINUTE, GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1,
        0, 0, 0, 0);

  // l.set(GRIB_LEVEL_SATELLITE_METEOSAT8, pds.chn, LOCALDEF24FUNC);
 
  l.set(GRIB_LEVEL_SATELLITE_METEOSAT8, fakechan, LOCALDEF3FUNC);

  // Dimensions
  bpp = header[0].image_structure->number_of_bits_per_pixel;
  ncal = (int) pow(2.0, bpp);

  g.set_size(AreaNpix, AreaNlin);
  g.set_earth_spheroid( );
  g.is_dirincgiven = true;
  // Earth Equatorial Radius is 6378.160 Km (IAU 1965)
  g.set_spaceview(0.0, pds.sublon, SEVIRI_DX, SEVIRI_DY,
                  SP_X0, SP_Y0, SEVIRI_ORIENTATION,
                  SEVIRI_CAMERA_H, AreaPixStart+1, AreaLinStart+1);

  // Get calibration values
  cal = PRO_data->prologue->radiometric_proc.get_calibration((int) pds.chn,
                  bpp);

  float *fvals = new float[total_size];
  for (int i = 0; i < (int) total_size; i ++)
  {
    if (pixels[i] > 0) fvals[i] = cal[pixels[i]] - abase;
    else fvals[i] = 0.0;
  }

  f.set_table(GRIB_CENTER, GRIB_SUBCENTER, GRIB_TABLE, GRIB_PROCESS);
  f.set_field(GRIB_PARAMETER_IMG_D, fvals, total_size, FILL_VALUE, FILL_VALUE);
  f.set_scale(GRIB_DECIMAL_SCALE);

  unsigned char localdefinition3[LOCALDEF3LEN];

  localdefinition3[0] = LOCALDEF3ID;
  localdefinition3[1] = LOCALDEF3CLASS;
  localdefinition3[2] = LOCALDEF3TYPE;
  localdefinition3[3] = (LOCALDEF3STREAM >> 8) & 255;
  localdefinition3[4] = LOCALDEF3STREAM & 255;
  localdefinition3[5] = 0x30; // 0
  localdefinition3[6] = 0x30; // 0
  localdefinition3[7] = 0x30; // 0
  localdefinition3[8] = 0x31; // 1
  localdefinition3[9] = fakechan;
  localdefinition3[10] = LOCALDEF3FUNC;
  localdefinition3[11] = 0;

  // unsigned char localdefinition24[LOCALDEF24LEN];

  // localdefinition24[0] = LOCALDEF24ID;
  // localdefinition24[1] = LOCALDEF24CLASS;
  // localdefinition24[2] = LOCALDEF24TYPE;
  // localdefinition24[3] = (LOCALDEF24STREAM >> 8) & 255;
  // localdefinition24[4] = LOCALDEF24STREAM & 255;
  // localdefinition24[5] = 0x30; // 0
  // localdefinition24[6] = 0x30; // 0
  // localdefinition24[7] = 0x30; // 0
  // localdefinition24[8] = 0x31; // 1
  // localdefinition24[9] = (LOCALDEF24SATID >> 8) & 255;
  // localdefinition24[10] = LOCALDEF24SATID & 255;
  // localdefinition24[11] = (LOCALDEF24INSTR >> 8) & 255;
  // localdefinition24[12] = LOCALDEF24INSTR & 255;
  // localdefinition24[13] = (pds.chn >> 8) & 255;
  // localdefinition24[14] = pds.chn & 255;
  // localdefinition24[15] = LOCALDEF24FUNC;

  std::cout << "Adding local use PDS of " << sizeof(GRIB_MSG_PDS) << " bytes"
            << std::endl;
  size_t add_total = LOCALDEF3LEN + sizeof(GRIB_MSG_PDS);
  unsigned char *pdsadd = new unsigned char[add_total];
  memcpy(pdsadd, localdefinition3, LOCALDEF3LEN);
  memcpy(pdsadd+LOCALDEF3LEN, &pds, sizeof(GRIB_MSG_PDS));
  f.add_local_def(add_total, pdsadd);

  // Write output values
  m.set_time(t);
  m.set_grid(g);
  m.set_level(l);
  m.set_field(f);

  gf.WriteMessage(m);

  // Close Grib output
  ret = gf.Close( );
  if (ret != 0) return -1;

  delete [ ] pixels;
  delete [ ] cal;
  delete [ ] fvals;

  return( true );
}
#endif

//---------------------------------------------------------------------------
// vim:set ts=2 sw=2:
