//---------------------------------------------------------------------------
//
//  File        :   XRIT2NetCDF.cpp
//  Description :   Export MSG HRIT format in NetCDF format
//  Project     :   Lamma 2004
//  Author      :   Graziano Giuliani (Lamma Regione Toscana)
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
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "config.h"

// Unidata NetCDF

#include <netcdfcpp.h>

// HRI format interface

#include <msat/hrit/MSG_HRIT.h>

#include <getopt.h>
#include <glob.h>

#define TITLE "Observation File from MSG-SEVIRI"
#define INSTITUTION "HIMET"
#define TYPE "Obs file"
#define HIMET_VERSION 0.0

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

void usage(char *pname);
std::string underscoreit(std::string base, int final_len);
char *chname(char *chdesc, int len);
bool NetCDFProduct(MSG_header *PRO_head, MSG_data* PRO_data,
                   int totalsegs, int *segsindexes,
		   MSG_header *header, MSG_data *msgdat);

bool is_subarea = false;
int AreaLinStart = 1, AreaNlin, AreaPixStart = 1, AreaNpix;

/* ************************************************************************* */
/* Reads Data Images, performs calibration and store output in NetCDF format */
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
      {"area", 1, 0, 'a'},
      {"time", 1, 0, 't'},
      {"version", 0, 0, 'V'},
      {"help", 0, 0, 'h'},
      {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "d:r:s:c:a:t:Vh?",
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

  std::cout << "Writing NetCDF output..." << std::endl;
  if(!NetCDFProduct(&PRO_head, &PRO_data,
                    totalsegs, segsindexes, header, msgdat))
    throw "Created NetCDF product NOT OK";

  delete [ ] header;
  delete [ ] msgdat;
  delete [ ] segsindexes;

  return(0);
}

void usage(char *pname)
{
  std::cout << pname << ": Convert HRIT/LRIT files to NetCDF format."
	    << std::endl;
  std::cout << std::endl << "Usage:" << std::endl << "\t"
            << pname << " [-d directory] [-a linestart,nlin,pixstart,npix] "
            << "-r resol -s prodid1 -c prodid2 -t time"
            << std::endl << std::endl
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

std::string underscoreit(std::string base, int final_len)
{
  char *tmp;
  int mlen;

  tmp = (char *) malloc((final_len+1) * sizeof(char));
  tmp[final_len] = 0;
  memset(tmp, 0x5F, final_len);
  mlen = strlen(base.c_str( ));
  if (mlen > final_len) mlen = final_len;
  memcpy(tmp, base.c_str( ), mlen);
  std::string retval = tmp;
  free(tmp);
  return retval;
}

//
// Creates NetCDF product
//
bool NetCDFProduct(MSG_header *PRO_head, MSG_data* PRO_data,
                   int totalsegs, int *segsindexes,
		   MSG_header *header, MSG_data *msgdat)
{
  struct tm *tmtime;
  char NcName[1024];
  char reftime[64];
  char projname[16];
  int wd, hg;
  int bpp;
  int ncal;
  float *cal;
  NcVar *ivar;
  NcVar *tvar;
  NcDim *tdim;
  NcDim *ldim;
  NcDim *cdim;
  NcDim *caldim;

  int npix = header[0].image_structure->number_of_columns;
  int nlin = header[0].image_structure->number_of_lines;
  size_t npixperseg = npix*nlin;

  size_t total_size = totalsegs*npixperseg;
  MSG_SAMPLE *pixels = new MSG_SAMPLE[total_size];
  memset(pixels, 0, total_size*sizeof(MSG_SAMPLE));
  size_t pos = 0;
  for (int i = 0; i < totalsegs; i ++)
  {
    if (segsindexes[i] >= 0)
      memcpy(pixels+pos, msgdat[segsindexes[i]].image->data,
             npixperseg*sizeof(MSG_SAMPLE));
    pos += npixperseg;
  }

  nlin = nlin*totalsegs;

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
  t_enum_MSG_spacecraft spc = header[0].segment_id->spacecraft_id;
  uint_1 chn = header[0].segment_id->spectral_channel_id;
  float sublon = header[0].image_navigation->subsatellite_longitude;
  int cfac = header[0].image_navigation->column_scaling_factor;
  int lfac = header[0].image_navigation->line_scaling_factor;
  int coff = header[0].image_navigation->column_offset;
  int loff = header[0].image_navigation->line_offset;
  float sh = header[0].image_navigation->satellite_h;

  char *channelstring = strdup(MSG_channel_name(spc, chn).c_str( ));
  char *channel = chname(channelstring, strlen(channelstring) + 1);

  // Build up output NetCDF file name and open it
  sprintf( NcName, "%s_%4d%02d%02d_%02d%02d.nc", channel,
           tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
	   tmtime->tm_hour, tmtime->tm_min );
  NcFile ncf ( NcName , NcFile::Replace );
  if (! ncf.is_valid()) return false;

  // Fill arrays on creation
  ncf.set_fill(NcFile::Fill);

  // Add Global Attributes
  if (! ncf.add_att("Satellite", MSG_spacecraft_name(spc).c_str()))
	            return false;
  sprintf(reftime, "%04d-%02d-%02d %02d:%02d:00 UTC",
      tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
      tmtime->tm_hour, tmtime->tm_min);
  if (! ncf.add_att("Antenna", "Fixed") ) return false;
  if (! ncf.add_att("Receiver", "HIMET") ) return false;
  if (! ncf.add_att("Time", reftime) ) return false;
  if (! ncf.add_att("Area_Name", "SpaceView" ) ) return false;
  sprintf(projname, "GEOS(%3.1f)", sublon);
  if (! ncf.add_att("Projection", projname) ) return false;
  if (! ncf.add_att("Columns", AreaNpix ) ) return false;
  if (! ncf.add_att("Lines", AreaNlin ) ) return false;
  if (! ncf.add_att("SampleX", 1.0 ) ) return false;
  if (! ncf.add_att("SampleY", 1.0 ) ) return false;
  if (! ncf.add_att("AreaStartPix", AreaPixStart ) ) return false;
  if (! ncf.add_att("AreaStartLin", AreaLinStart ) ) return false;
  if (! ncf.add_att("Column_Scale_Factor", cfac) ) return false;
  if (! ncf.add_att("Line_Scale_Factor", lfac) ) return false;
  if (! ncf.add_att("Column_Offset", coff) ) return false;
  if (! ncf.add_att("Line_Offset", loff) ) return false;
  if (! ncf.add_att("Orbit_Radius", sh) ) return false;
  if (! ncf.add_att("Longitude", sublon) ) return false;
  if (! ncf.add_att("NortPolar", 1) ) return false;
  if (! ncf.add_att("NorthSouth", 1) ) return false;
  if (! ncf.add_att("title", TITLE) ) return false;
  if (! ncf.add_att("Institution", INSTITUTION) ) return false;
  if (! ncf.add_att("Type", TYPE) ) return false;
  if (! ncf.add_att("Version", HIMET_VERSION) ) return false;
  if (! ncf.add_att("Conventions", "COARDS") ) return false;
  if (! ncf.add_att("history", "Created from raw data") ) return false;

  // Dimensions
  wd = AreaNpix;
  hg = AreaNlin;
  bpp = header[0].image_structure->number_of_bits_per_pixel;
  ncal = (int) pow(2.0, bpp);

  tdim = ncf.add_dim("time");
  if (!tdim->is_valid()) return false;
  ldim = ncf.add_dim("line", hg);
  if (!ldim->is_valid()) return false;
  cdim = ncf.add_dim("column", wd);
  if (!cdim->is_valid()) return false;
  caldim = ncf.add_dim("calibration", ncal);
  if (!caldim->is_valid()) return false;

  // Get calibration values
  cal = PRO_data->prologue->radiometric_proc.get_calibration((int) chn, bpp);

  // Add Calibration values
  NcVar *cvar = ncf.add_var("calibration", ncFloat, caldim);
  if (!cvar->is_valid()) return false;
  cvar->add_att("long_name", "Calibration coefficients");
  cvar->add_att("variable", channel);
  if (chn > 3 && chn < 12)
    cvar->add_att("units", "K");
  else
    cvar->add_att("units", "mW m^-2 sr^-1 (cm^-1)^-1");
  if (!cvar->put(cal, ncal)) return false;

  tvar = ncf.add_var("time", ncDouble, tdim);
  if (!tvar->is_valid()) return false;
  tvar->add_att("long_name", "Time");
  tvar->add_att("units", "seconds since 2000-01-01 00:00:00 UTC");
  double atime;
  time_t ttime;
  extern long timezone;
  ttime = mktime(tmtime);
  atime = ttime - 946684800 - timezone;
  if (!tvar->put(&atime, 1)) return false;

  ivar = ncf.add_var(channel, ncShort, tdim, ldim, cdim);
  if (!ivar->is_valid()) return false;
  if (!ivar->add_att("add_offset", 0.0)) return false;
  if (!ivar->add_att("scale_factor", 1.0)) return false;
  if (!ivar->add_att("chnum", chn)) return false;

  // Write output values
  if (!ivar->put((const short int *) pixels, 1, hg, wd)) return false;

  // Close NetCDF output
  (void) ncf.close( );

  delete [ ] pixels;
  delete [ ] cal;

  return( true );
}

//---------------------------------------------------------------------------
