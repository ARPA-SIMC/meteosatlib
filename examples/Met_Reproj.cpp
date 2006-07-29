//-----------------------------------------------------------------------------
//
//  File        : Reproj.cpp
//  Description : Reproject NetCDF exported Meteosat files on
//                regular latitude/longitude grid.
//  Project     : CETEMPS 2003
//  Author      : Graziano Giuliani (CETEMPS - University of L'Aquila
//  References  : LRIT/HRIT GLobal Specification par. 4.4 pag. 20-28
//                Doc. No. CGMS 03 Issue 2.6 12 August 1999
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
//-----------------------------------------------------------------------------

#include <config.h>

#include <iostream>

#include <Standard.h>
#include <Area.h>
#include <GRIB.h>

#include <netcdfcpp.h>
#include <unistd.h>

#include <cstdlib>
#include <cmath>
#include <ctime>

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#define FILL_VALUE         9.999E-20
#define GRIB_DECIMAL_SCALE 1
#define GRIB_CENTER        200
#define GRIB_SUBCENTER     0
#define GRIB_TABLE         1
#define GRIB_PROCESS       254

#define LOCALDEF3LEN       12
#define LOCALDEF3ID        3    // Satellite image data
#define LOCALDEF3CLASS     1    // Operations
#define LOCALDEF3TYPE      40   // Image Data
#define LOCALDEF3STREAM    55   // Meteosat 8 = 55
#define LOCALDEF3FUNC      1    // T in K - 145

extern long timezone;

static void printusage(char *pname);

int main(int argc, char *argv[])
{
  int i, j;
  int optch;
  static char optstring[] = "o:gvhV";
  char outname[MAX_PATH] = "";
  char inpname[MAX_PATH] = "";
  char pname[MAX_PATH] = "";
  BOOL verbose = FALSE;
  BOOL singlepoint = FALSE;
  BOOL grib = FALSE;
  double minlat, maxlat, reslat;
  double minlon, maxlon, reslon;
  AreaParameters p;
  int AreaStartPix = 0;
  int AreaStartLin = 0;
  float afill = 0.0;

  strncpy(pname, argv[0], MAX_PATH);

  while ((optch = getopt(argc, argv, optstring)) != -1)
  {
    switch (optch)
    {
      case 'o':
	strncpy(outname, optarg, MAX_PATH);
	break;
      case 'v':
	verbose = TRUE;
	break;
      case 'g':
	grib = TRUE;
	break;
      case 'h':
	printusage(pname);
	return (0);
	break;
      case 'V':
	std::cout << pname << " " << PACKAGE_STRING << std::endl;
	return (0);
	break;
      default:
	std::cerr << "Undefined option." << std::endl;
	printusage(pname);
	return -1;
	break;
    }
  }

  if (argc < 8)
  {
    std::cerr << "Not enough arguments." << std::endl;
    printusage(argv[0]);
    return -1;
  }

  strncpy(inpname, argv[optind], MAX_PATH);

  NcFile ncf(inpname, NcFile::ReadOnly);
  if (!ncf.is_valid())
  {
    std::cerr << "Cannot Open NetCDF input Satellite file " 
              << inpname << std::endl;
    return -1;
  }

  if (outname[0] == 0)
  {
    strncpy(outname, inpname, MAX_PATH);
    char *pnt = strrchr(outname, '.');
    if (! grib) sprintf(pnt, "_rep.nc");
    else sprintf(pnt, "_rep.grb");
  }

  if (verbose)
  {
    std::cout << "Input file is  : " << inpname << std::endl;
    std::cout << "Output file is : " << outname << std::endl;
  }

  char *ep = NULL;
  minlat = strtod(argv[++optind], &ep);
  if (ep == argv[optind] || *ep != 0)
  {
    printusage(pname);
    return -1;
  }
  maxlat = strtod(argv[++optind], &ep);
  if (ep == argv[optind] || *ep != 0)
  {
    printusage(pname);
    return -1;
  }
  reslat = strtod(argv[++optind], &ep);
  if (ep == argv[optind] || *ep != 0)
  {
    printusage(pname);
    return -1;
  }
  minlon = strtod(argv[++optind], &ep);
  if (ep == argv[optind] || *ep != 0)
  {
    printusage(pname);
    return -1;
  }
  maxlon = strtod(argv[++optind], &ep);
  if (ep == argv[optind] || *ep != 0)
  {
    printusage(pname);
    return -1;
  }
  reslon = strtod(argv[++optind], &ep);
  if (ep == argv[optind] || *ep != 0)
  {
    printusage(pname);
    return -1;
  }

  if (minlat > maxlat || minlon > maxlon)
  {
    std::cerr << "MIN > MAX !!!" << std::endl;
    printusage(pname);
    return -1;
  }

  if (reslat > (maxlat-minlat) || reslon > (maxlon-minlon))
  {
    std::cerr << "Resolution greater than range?" << std::endl;
    printusage(pname);
    return -1;
  }
 
  if (reslon < 0.0 || reslat < 0.0)
  {
    std::cerr << "Resolution is negative?" << std::endl;
    printusage(pname);
    return -1;
  }

  if (reslon == 0.0 && reslat == 0.0)
  {
    singlepoint = TRUE;
  }

  if (verbose)
  {
    if (! singlepoint)
    {
      std::cout << "Latitude in  (" << minlat << "," << maxlat << ")"
                << " resolution " << reslat << std::endl;
      std::cout << "Longitude in (" << minlon << "," << maxlon << ")"
                << " resolution " << reslon << std::endl;
    }
    else
    {
      std::cout << "Output in point (" << minlat << "," << minlon << ")"
	        << std::endl;
    }
  }

  strncpy(p.Name, ncf.get_att("Area_Name")->as_string(0), AREANAMELEN);
  strncpy(p.PrjName, ncf.get_att("Projection")->as_string(0), PNAMELEN);
  p.nColumns = ncf.get_att("Columns")->as_int(0);
  p.nLines = ncf.get_att("Lines")->as_int(0);
  p.SampleX = ncf.get_att("SampleX")->as_double(0);
  p.SampleY = ncf.get_att("SampleY")->as_double(0);
  AreaStartPix = ncf.get_att("AreaStartPix")->as_long(0);
  AreaStartLin = ncf.get_att("AreaStartLin")->as_long(0);
  p.CFAC = ncf.get_att("Column_Scale_Factor")->as_long(0);
  p.LFAC = ncf.get_att("Line_Scale_Factor")->as_long(0);
  p.COFF = ncf.get_att("Column_Offset")->as_long(0);
  p.LOFF = ncf.get_att("Line_Offset")->as_long(0);
  p.Longitude = ncf.get_att("Longitude")->as_double(0);
  p.OrbitRadius = ncf.get_att("Orbit_Radius")->as_double(0);
  p.ifNorth = ncf.get_att("NortPolar")->as_ncbyte(0);
  p.NorthSouth = ncf.get_att("NorthSouth")->as_ncbyte(0);

  if (verbose)
  {
    std::cout << "Area Name    : " << p.Name        << std::endl;
    std::cout << "Projection   : " << p.PrjName     << std::endl;
    std::cout << "nColumns     : " << p.nColumns    << std::endl;
    std::cout << "nLines       : " << p.nLines      << std::endl;
    std::cout << "SampleX      : " << p.SampleX     << std::endl;
    std::cout << "SampleY      : " << p.SampleY     << std::endl;
    std::cout << "CFAC         : " << p.CFAC        << std::endl;
    std::cout << "LFAC         : " << p.LFAC        << std::endl;
    std::cout << "COFF         : " << p.COFF        << std::endl;
    std::cout << "LOFF         : " << p.LOFF        << std::endl;
    std::cout << "Longitude    : " << p.Longitude   << std::endl;
    std::cout << "Orbit radius : " << p.OrbitRadius << std::endl;
    if (p.ifNorth)
    {
      std::cout << "North polar  : TRUE" << std::endl;
    }
    else
    {
      std::cout << "North polar  : FALSE" << std::endl;
    }
    if (p.NorthSouth)
    {
      std::cout << "North/South  : TRUE" << std::endl;
    }
    else
    {
      std::cout << "North/South  : FALSE" << std::endl;
    }
  }

  bool not_calibrated = true;
  long ncal = 0;
  int varpos;
  if (ncf.num_dims( ) > 3)
  {
    varpos = 2;
    ncal = ncf.get_dim("calibration")->size();
  }
  else
  {
    std::cout << "Coming from GRIB, calibrated variable..." << std::endl;
    not_calibrated = false;
    varpos = 1;
  }

  Area a(&p);

  if (singlepoint && ! not_calibrated)
  {
    MapPoint M;
    ImagePoint I;
    M.latitude = minlat;
    M.longitude = minlon;
    a.Map_to_Image(&M, &I);
    long ncol = ncf.get_dim("column")->size();
    long nlin = ncf.get_dim("line")->size();
    if (I.line   < 0 || I.line   > nlin ||
	I.column < 0 || I.column > ncol)
    {
      std::cout << "Point out of image" << std::endl;
      if (verbose) std::cout << "Done." << std::endl;
      ncf.close();
      return 0;
    }
    int value = 127+ncf.get_var(varpos)->as_int(I.line*ncol+I.column);
    float calibrated = ncf.get_var("calibration")->as_int(value);
    if (verbose)
      std::cout << "Calibrated Value : " << calibrated
	        << ncf.get_var("calibration")->get_att("units")->as_string(0)
		<< std::endl;
    else
      std::cout << calibrated 
	        << ncf.get_var("calibration")->get_att("units")->as_string(0)
		<< std::endl;
    if (verbose) std::cout << "Done." << std::endl;
    ncf.close();
    return 0;
  }

  double deltalat = maxlat-minlat;
  double deltalon = maxlon-minlon;
  long nlat = (long) rint(deltalat / reslat) + 1;
  long nlon = (long) rint(deltalon / reslon) + 1;
  reslon = deltalon/(double) (nlon - 1);
  reslat = deltalat/(double) (nlat - 1);
  long ncol = ncf.get_dim("column")->size();
  long nlin = ncf.get_dim("line")->size();
  if (verbose)
  {
    std::cout << "Satellite    : "
              << ncf.get_att("Satellite")->as_string(0) << std::endl;
    std::cout << "Antenna      : "
              << ncf.get_att("Antenna")->as_string(0) << std::endl;
    std::cout << "Receiver     : "
              << ncf.get_att("Receiver")->as_string(0) << std::endl;
    std::cout << "Time         : "
              << ncf.get_att("Time")->as_string(0) << std::endl;
    std::cout << "Institution  : "
              << ncf.get_att("Institution")->as_string(0) << std::endl;
    std::cout << "Conventions  : "
              << ncf.get_att("Conventions")->as_string(0) << std::endl;
    std::cout << "Title        : "
              << ncf.get_att("title")->as_string(0) << std::endl;
    std::cout << "History      : "
              << ncf.get_att("history")->as_string(0) << std::endl;
  }

  float *latitude = new float[nlat];
  float *longitude = new float[nlon];
  float *vals = new float[nlat*nlon];
  short *ibyte = 0;
  float *fvals = 0;
  float *calibration = 0;

  if (not_calibrated)
  {
    ibyte = new short[ncol*nlin];
    calibration = new float[ncal];
    ncf.get_var("calibration")->get(calibration, ncal);
    ncf.get_var(varpos)->get(ibyte, 1, nlin, ncol);
  }
  else
  {
    fvals = new float[ncol*nlin];
    ncf.get_var(varpos)->get(fvals, 1, nlin, ncol);
  }

  for (i = 0; i < nlon; i ++)
  {
    longitude[i] = (float) minlon + (reslon * (float) i);
    if (fabs(longitude[i]) < 1E-10) longitude[i] = 0.0;
  }
  for (i = 0; i < nlat; i ++)
  {
    latitude[i] = (float) minlat + (reslat * (float) i);
    if (fabs(latitude[i]) < 1E-10) latitude[i] = 0.0;
  }

  if (verbose)
  {
    std::cout << "Defined latitude,longitude grid of (" << nlat
              << "," << nlon << ") points" << std::endl;
  }
    
  double xtime;
  ncf.get_var("time")->get(&xtime, 1);
  
  const char *vname = ncf.get_var(varpos)->name();
  const char *lname;
  if (not_calibrated)
    lname = ncf.get_var("calibration")->get_att("variable")->as_string(0);
  else
    lname = ncf.get_var(varpos)->name();

  MapPoint M;
  ImagePoint I;

  if (verbose)
    std::cout << "Interpolating...";

  for (i = 0; i < nlat; i ++)
  {
    for (j = 0; j < nlon; j ++)
    {
      M.latitude = latitude[i];
      M.longitude = longitude[j];
      a.Map_to_Image(&M, &I);
      I.line = I.line - AreaStartLin + 1;
      I.column = I.column - AreaStartPix + 1;
      if (verbose)
        std::cout << "Point line,column: " << I.line
                  << "," << I.column << std::endl;
      if (I.line   < 0 || I.line   >= nlin ||
          I.column < 0 || I.column >= ncol)
      {
        vals[i*nlon+j] = afill;
      }
      else
      {
        if (not_calibrated)
        {
           int value = (int) ibyte[I.line*ncol+I.column];
           if (value == 0) vals[i*nlon+j] = afill;
           else vals[i*nlon+j] = calibration[value];
        }
        else
        {
           vals[i*nlon+j] = fvals[I.line*ncol+I.column];
        }
      }
    }
  }

  if (!grib)
  {
    NcFile nco(outname, NcFile::Replace);
    if (!ncf.is_valid())
    {
      std::cerr << "Cannot Open NetCDF output file "<< outname << std::endl;
      return -1;
    }

    nco.add_att("Satellite", ncf.get_att("Satellite")->as_string(0));
    nco.add_att("Antenna", ncf.get_att("Antenna")->as_string(0));
    nco.add_att("Receiver", ncf.get_att("Receiver")->as_string(0));
    nco.add_att("Time", ncf.get_att("Time")->as_string(0));
    nco.add_att("Institution", ncf.get_att("Institution")->as_string(0));
    nco.add_att("Conventions", ncf.get_att("Conventions")->as_string(0));
    nco.add_att("history", "Reprojected and calibrated from raw data");
    nco.add_att("title", ncf.get_att("title")->as_string(0));

    NcDim *dtime = nco.add_dim("time");
    NcDim *dlon = nco.add_dim("longitude", nlon);
    NcDim *dlat = nco.add_dim("latitude", nlat);

    NcVar *vtime = nco.add_var("time", ncDouble, dtime);
    NcVar *lon = nco.add_var("longitude", ncFloat, dlon);
    NcVar *lat = nco.add_var("latitude", ncFloat, dlat);
    vtime->add_att("units",
                    ncf.get_var("time")->get_att("units")->as_string(0));
    lat->add_att("units", "degrees_north");
    lon->add_att("units", "degrees_east");

    lon->put(longitude, nlon);
    lat->put(latitude, nlat);
    vtime->put(&xtime, 1);

    NcVar *val = nco.add_var(vname, ncFloat, dtime, dlat, dlon);
    val->add_att("long_name", lname);
    if (not_calibrated)
      val->add_att("units",
                 ncf.get_var("calibration")->get_att("units")->as_string(0));
    else
      val->add_att("units",
                 ncf.get_var(varpos)->get_att("units")->as_string(0));
    val->add_att("_FillValue", afill);

    val->put(vals, 1, nlat, nlon);

    nco.close();
  }
  else
  {
    GRIB_FILE gf;
    int ret = gf.OpenWrite(outname);
    if (ret != 0) return -1;

    GRIB_MESSAGE m;
    GRIB_GRID g;
    GRIB_LEVEL l;
    GRIB_TIME t;
    GRIB_FIELD f;

    time_t time_tval = (time_t) xtime + 946684800 + timezone;
    struct tm *realtime = gmtime(&time_tval);
  
    t.set(realtime->tm_year+1900, realtime->tm_mon+1,
          realtime->tm_mday, realtime->tm_hour, realtime->tm_min,
          GRIB_TIMEUNIT_MINUTE, GRIB_TIMERANGE_FORECAST_AT_REFTIME_PLUS_P1,
          0, 0, 0, 0);

    g.set_size(nlon, nlat);
    g.is_y_negative = true;
    int chn = ncf.get_var(varpos)->get_att("chnum")->as_long(0);;
    g.set_regular_latlon(minlat, maxlat, minlon, maxlon, reslat+0.0001,
                         reslon+0.0001);

    l.set(GRIB_LEVEL_SATELLITE_METEOSAT8, chn, LOCALDEF3FUNC);

    f.set_table(GRIB_CENTER, GRIB_SUBCENTER, GRIB_TABLE, GRIB_PROCESS);
    f.set_field(GRIB_PARAMETER_IMG_D, vals, nlat*nlon,
                FILL_VALUE, FILL_VALUE);
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
    localdefinition3[9] = chn;
    localdefinition3[10] = LOCALDEF3FUNC;
    localdefinition3[11] = 0;

    f.add_local_def(LOCALDEF3LEN, localdefinition3);

    // Write output values
    m.set_time(t);
    m.set_grid(g);
    m.set_level(l);
    m.set_field(f);
    
    gf.WriteMessage(m);
    
    //  Close Grib output
    ret = gf.Close( );
    if (ret != 0) return -1;
  }

  delete [ ] latitude;
  delete [ ] longitude;
  if (not_calibrated)
  {
    delete [ ] calibration;
    delete [ ] ibyte;
  }
  else
    delete [ ] fvals;
  delete [ ] vals;

  ncf.close();

  if (verbose)
  {
    std::cout << std::endl;
    std::cout << "Done." << std::endl;
  }
  return 0;
}

void printusage(char *pname)
{
  std::cerr << "Usage " << pname
            << " [options] satfile.nc" 
            << " minlat maxlat reslat minlon maxlon reslon"
            << std::endl
            << "where option can be:" << std::endl
	    << "\t\t-o output_name : name output file (default satfile_rep.nc)"
	    << std::endl << "\t\t-g             : output in grib format"
	    << std::endl << "\t\t-v             : verbose output" << std::endl
	    << "\t\t-h             : print this help" << std::endl;
  return;
}
