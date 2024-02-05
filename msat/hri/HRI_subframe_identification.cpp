//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPAE-SIMC <urpsim@arpae.it>
//
//  File        : HRI_subframe_identification.cpp
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Authors     : Graziano Giuliani (Lamma Regione Toscana)
//              : Enrico Zini <enrico@enricozini.com>
//  References  : Meteosat High Resolution Image Dissemination
//                Doc. No. EUM TD 02 Revision 4 April 1998
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
//-----------------------------------------------------------------------------
#include <msat/hri/HRI_subframe_identification.h>
#include <cstring>
#include <ctime>
#include <cstdio>

HRI_subframe_identification::HRI_subframe_identification( char *hsi )
{
  readfrom(hsi);
}

void HRI_subframe_identification::readfrom( char *hsi )
{
  struct tm *tm1time;
  time_t jtime;

  memcpy(satellite_indicator, hsi, 2);
  meteosat_id = -1;
  if (satellite_indicator[0] == 212)
  {
    satellite   = "METEOSAT";
    meteosat_id = 1;
    if (satellite_indicator[1] > 240)
    {
      char id[8];
      meteosat_id = satellite_indicator[1] - 240;
      sprintf(id, "-%d", meteosat_id);
      satellite += id;
    }
  }
  else if (satellite_indicator[0] == 0)
  {
    if (satellite_indicator[1] == 0) satellite = "GOES-E";
    else if (satellite_indicator[1] == 1) satellite = "GOES-W";
    else if (satellite_indicator[1] == 2) satellite = "GMS";
    else if (satellite_indicator[1] == 3) satellite = "GOMS";
    else
    {
      std::cerr << "Unknown satellite id : " << std::hex
           << satellite_indicator[0] << satellite_indicator[1]
           << std::dec << std::endl;
      throw;
    }
  }
  else
  {
    std::cerr << "Unknown satellite id : " << std::hex
         << satellite_indicator[0] << satellite_indicator[1]
         << std::dec << std::endl;
    throw;
  }

  year = (int) conv.i2_from_buff((const unsigned char *) hsi+2);
  julian_day = conv.i2_from_buff((const unsigned char *) hsi+4);
  memset(&tmtime, 0, sizeof(struct tm));
  tmtime.tm_year = year-1900;
  tmtime.tm_mday = 1;
  tmtime.tm_hour = 12;
  jtime = mktime(&tmtime) + (julian_day - 1) * 24 * 60 * 60;
  tm1time = gmtime(&jtime);
  month = tm1time->tm_mon + 1;
  day   = tm1time->tm_mday;

  nominal_time = conv.i2_from_buff((const unsigned char *) hsi+6);
  hour = nominal_time / 100;
  minute = nominal_time - hour*100;

  memset(&tmtime, 0, sizeof(struct tm));
  tmtime.tm_year = year-1900;
  tmtime.tm_mon  = month-1;
  tmtime.tm_mday = day;
  tmtime.tm_hour = hour;
  tmtime.tm_min  = minute;

  datetime = asctime(&tmtime);

  column_offset = conv.i2_from_buff((const unsigned char *) hsi+20);
  line_offset   = conv.i2_from_buff((const unsigned char *) hsi+22);
  longitude     = (double) conv.i2_from_buff((const unsigned char *) hsi+24);

  return;
}

bool HRI_subframe_identification::is_Meteosat( )
{
  if (satellite_indicator[0] > 0) return true;
  return false;
}

bool HRI_subframe_identification::is_GOES_E( )
{
  if (satellite_indicator[0] == 0 && satellite_indicator[1] == 0)
    return true;
  return false;
}

bool HRI_subframe_identification::is_GOES_W( )
{
  if (satellite_indicator[0] == 0 && satellite_indicator[1] == 1)
    return true;
  return false;
}

bool HRI_subframe_identification::is_GMS( )
{
  if (satellite_indicator[0] == 0 && satellite_indicator[1] == 2)
    return true;
  return false;
}

bool HRI_subframe_identification::is_GOMS( )
{
  if (satellite_indicator[0] == 0 && satellite_indicator[1] == 3)
    return true;
  return false;
}
