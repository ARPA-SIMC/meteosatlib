//-----------------------------------------------------------------------------
//
//  File        : HRI.cpp
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlib
//  Author      : Graziano Giuliani (Lamma Regione Toscana)
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
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <hri/HRI.h>

Hri::Hri( char *hri_filename, bool IS_TECNAVIA )
{
  readfrom( hri_filename, IS_TECNAVIA );
}

void Hri::readfrom( char *hri_filename, bool IS_TECNAVIA )
{
  char id;
  char interpretation_buffer[interpsize];

  std::ifstream hri(hri_filename, (ios_base::binary | ios_base::in));
  if (hri.fail())
  {
    std::cerr << "Cannot open input hri file " << hri_filename << std::endl;
    throw;
  }

  if (IS_TECNAVIA)
  {
    int offset = 512;
    mod_getbuff(hri, offset);
    id = *(mod_framebuff + 3 + offset);
    label.readfrom(mod_framebuff + 4 + offset);
    memset(seed, 0, 8);
    memcpy(seed, mod_framebuff + 28 + offset, 8);
    ident.readfrom(mod_framebuff + 36 + offset);
    memcpy(interpretation_buffer, mod_framebuff + 84 + offset, 1360);
    interp.readfrom(interpretation_buffer);
  }
  else
  {
    getbuff(hri);
    id = *(framebuff + 3);
    label.readfrom(framebuff + 4);
    memset(seed, 0, 8);
    memcpy(seed, framebuff + 28, 8);
    ident.readfrom(framebuff + 36);
    memset(interpretation_buffer, 0, 1360);
    memcpy(interpretation_buffer, framebuff + 84, 280);
    getbuff(hri);
    memcpy(interpretation_buffer + 280, framebuff + 4, 360);
    getbuff(hri);
    memcpy(interpretation_buffer + 640, framebuff + 4, 360);
    getbuff(hri);
    memcpy(interpretation_buffer + 1000, framebuff + 4, 360);
    interp.readfrom(interpretation_buffer);
  }

  if (id == HRI_A_FORMAT)
  {
    char keybuff[1440];

    if (IS_TECNAVIA)
    {
      int offset = 512;
      memcpy(keybuff, mod_framebuff + 84 + offset + 1360, 92);
      hri.read(mod_framebuff, 1536);
      if (hri.fail())
      {
        std::cerr << "Read failed." << std::endl;
        return;
      }
      memcpy(keybuff, mod_framebuff, 1440-92);
    }
    else
    {
      getbuff(hri);
      memcpy(keybuff, framebuff + 4, 360);
      getbuff(hri);
      memcpy(keybuff+360, framebuff + 4, 360);
      getbuff(hri);
      memcpy(keybuff+720, framebuff + 4, 360);
      getbuff(hri);
      memcpy(keybuff+1080, framebuff + 4, 360);
      keys.readfrom(keybuff);
    }
  }

  if (false)
  {
    if (label.is_A_format( ) || label.is_B_format( ))
      hri.seekg(122304, ios::beg);
    else if (label.is_X_format( ))
      hri.seekg(131040, ios::beg);
  }

  std::string format = label.format_code( );
  calibration_coefficients coeff;

  cout << "Format is : " << format << std::endl;

  if (format == "AV")
  {
    image[0].set_format_band(METEOSAT, A_FORMAT, VIS_BAND);
    nimages = 1;
    int mlines = image[0].nlines/2;
    int mpixels = image[0].npixels/2;
    for (int i = 0; i < mlines; i ++)
    {
      if (IS_TECNAVIA)
      {
        image[0].put_halfline(mod_get_dataline(hri, id), mpixels, i*2, true);
        image[0].put_halfline(mod_get_dataline(hri, id), mpixels, i*2, false);
        image[0].put_halfline(mod_get_dataline(hri, id), mpixels, i*2+1, true);
        image[0].put_halfline(mod_get_dataline(hri, id), mpixels, i*2+1, false);
      }
      else
      {
        image[0].put_halfline(get_dataline(hri, id), mpixels, i*2, true);
        image[0].put_halfline(get_dataline(hri, id), mpixels, i*2, false);
        image[0].put_halfline(get_dataline(hri, id), mpixels, i*2+1, true);
        image[0].put_halfline(get_dataline(hri, id), mpixels, i*2+1, false);
      }
    }
    image[0].set_calibration(&coeff);
  }
  else if (format == "AVH")
  {
    image[0].set_format_band(METEOSAT, A_FORMAT, VH_BAND);
    nimages = 1;
    if (IS_TECNAVIA)
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
    }
    else
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
    }
    coeff.mpef_absolute = interp.cal.calwv;
    coeff.space_count   = interp.cal.wvspc;
    image[0].set_calibration(&coeff);
  }
  else if (format == "AW")
  {
    image[0].set_format_band(METEOSAT, A_FORMAT, WV_BAND);
    nimages = 1;
    if (IS_TECNAVIA)
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
    }
    else
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
    }
    coeff.mpef_absolute = interp.cal.calwv;
    coeff.space_count   = interp.cal.wvspc;
    image[0].set_calibration(&coeff);
  }
  else if (format == "AIW")
  {
    image[0].set_format_band(METEOSAT, A_FORMAT, IR_BAND);
    image[1].set_format_band(METEOSAT, A_FORMAT, WV_BAND);
    nimages = 2;
    for (int i = 0; i < image[0].nlines; i ++)
    {
      if (IS_TECNAVIA)
      {
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_line(mod_get_dataline(hri, id), image[1].npixels, i); 
      }
      else
      {
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_line(get_dataline(hri, id), image[1].npixels, i); 
      }
    }
    coeff.mpef_absolute = interp.cal.calir;
    coeff.space_count   = interp.cal.irspc;
    image[0].set_calibration(&coeff);
    coeff.mpef_absolute = interp.cal.calwv;
    coeff.space_count   = interp.cal.wvspc;
    image[1].set_calibration(&coeff);
  }
  else if (format == "AIVH")
  {
    image[0].set_format_band(METEOSAT, A_FORMAT, IR_BAND);
    image[1].set_format_band(METEOSAT, A_FORMAT, VH_BAND);
    nimages = 2;
    for (int i = 0; i < image[0].nlines; i ++)
    {
      if (IS_TECNAVIA)
      {
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_line(mod_get_dataline(hri, id), image[1].npixels, i); 
      }
      else
      {
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_line(get_dataline(hri, id), image[1].npixels, i); 
      }
    }
    coeff.mpef_absolute = interp.cal.calir;
    coeff.space_count   = interp.cal.irspc;
    image[0].set_calibration(&coeff);
  }
  else if (format == "BW")
  {
    image[0].set_format_band(METEOSAT, B_FORMAT, WV_BAND);
    nimages = 1;
    if (IS_TECNAVIA)
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
    }
    else
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
    }
    coeff.mpef_absolute = interp.cal.calwv;
    coeff.space_count   = interp.cal.wvspc;
    image[0].set_calibration(&coeff);
  }
  else if (format == "BIV" || format == "BIVH")
  {
    image[0].set_format_band(METEOSAT, B_FORMAT, IR_BAND);
    image[1].set_format_band(METEOSAT, B_FORMAT, VIS_BAND);
    nimages = 2;
    int mlines = image[1].nlines/2;
    int mpixels = image[1].npixels/2;
    for (int i = 0; i < mlines; i ++)
    {
      if (IS_TECNAVIA)
      {
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_halfline(mod_get_dataline(hri, id), mpixels, i*2, true);
        image[1].put_halfline(mod_get_dataline(hri, id), mpixels, i*2, false);
        image[1].put_halfline(mod_get_dataline(hri, id), mpixels, i*2+1, true);
        image[1].put_halfline(mod_get_dataline(hri, id), mpixels, i*2+1, false);
      }
      else
      {
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_halfline(get_dataline(hri, id), mpixels, i*2, true);
        image[1].put_halfline(get_dataline(hri, id), mpixels, i*2, false);
        image[1].put_halfline(get_dataline(hri, id), mpixels, i*2+1, true);
        image[1].put_halfline(get_dataline(hri, id), mpixels, i*2+1, false);
      }
    }
    coeff.mpef_absolute = interp.cal.calir;
    coeff.space_count   = interp.cal.irspc;
    image[0].set_calibration(&coeff);
  }
  else if (format == "BIW")
  {
    image[0].set_format_band(METEOSAT, B_FORMAT, IR_BAND);
    image[1].set_format_band(METEOSAT, B_FORMAT, WV_BAND);
    nimages = 2;
    for (int i = 0; i < image[0].nlines; i ++)
    {
      if (IS_TECNAVIA)
      {
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
      }
      else
      {
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
        image[1].put_line(get_dataline(hri, id), image[0].npixels, i); 
      }
    }
    coeff.mpef_absolute = interp.cal.calir;
    coeff.space_count   = interp.cal.irspc;
    image[0].set_calibration(&coeff);
    coeff.mpef_absolute = interp.cal.calwv;
    coeff.space_count   = interp.cal.wvspc;
    image[1].set_calibration(&coeff);
  }
  else if (format == "XI")
  {
    HRI_image_satellite sat;
    if (ident.is_GOES_E( )) sat = GOES_E;
    else if (ident.is_GOES_W( )) sat = GOES_W;
    else if (ident.is_GMS( )) sat = GMS;
    else sat = INDOX;
    image[0].set_format_band(sat, X_FORMAT, IR_BAND);
    nimages = 1;
    if (IS_TECNAVIA)
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
    }
    else
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
    }
    coeff.mpef_absolute = interp.cal.calir;
    coeff.space_count   = interp.cal.irspc;
    image[0].set_calibration(&coeff);
  }
  else if (format == "XW")
  {
    HRI_image_satellite sat;
    if (ident.is_GOES_E( )) sat = GOES_E;
    else if (ident.is_GOES_W( )) sat = GOES_W;
    else if (ident.is_GMS( )) sat = GMS;
    else sat = INDOX;
    image[0].set_format_band(sat, X_FORMAT, WV_BAND);
    nimages = 1;
    if (IS_TECNAVIA)
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
    }
    else
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
    }
    image[0].set_calibration(&coeff);
  }
  else if (format == "XVH")
  {
    HRI_image_satellite sat;
    if (ident.is_GOES_E( )) sat = GOES_E;
    else if (ident.is_GOES_W( )) sat = GOES_W;
    else if (ident.is_GMS( )) sat = GMS;
    else sat = INDOX;
    image[0].set_format_band(sat, X_FORMAT, VH_BAND);
    nimages = 1;
    if (IS_TECNAVIA)
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(mod_get_dataline(hri, id), image[0].npixels, i); 
    }
    else
    {
      for (int i = 0; i < image[0].nlines; i ++)
        image[0].put_line(get_dataline(hri, id), image[0].npixels, i); 
    }
    coeff.mpef_absolute = interp.cal.calwv;
    coeff.space_count   = interp.cal.wvspc;
    image[0].set_calibration(&coeff);
  }
  else
  {
    std::cerr << "Invalid format or non disseminated image : "
              << format << std::endl;
    throw;
  }

  geo.set_format(format);
  hri.close();
}

char * Hri::get_format( )
{
  return (char *) label.format_code( ).c_str( );
}

struct tm *Hri::get_datetime( )
{
  return &(ident.tmtime);
}

char * Hri::get_satellite_name( )
{
  return (char *) ident.satellite.c_str( );
}

float Hri::get_satellite_longitude( )
{
  return ident.longitude;
}

geolocation *Hri::get_geolocation( )
{
  return &(geo.get_geolocation( ));
}

float Hri::get_orbit_radius( )
{
  return interp.imag.satdis;
}

bool Hri::get_northpolar()
{
  return true;
}

bool Hri::get_northsouth()
{
  return true;
}

char *Hri::get_area_name( )
{
  static char areanames[3][16] = { "AFormat", "BFormat", "XFormat" };
  if (label.is_A_format( )) return areanames[0];
  else if (label.is_B_format( )) return areanames[1];
  return areanames[2];
}

unsigned char * Hri::get_dataline( ifstream &hri, int format )
{
  static unsigned char linebuff[2500];

  if (format == HRI_A_FORMAT)
  {
    getbuff(hri);
    memcpy(linebuff, framebuff+68, 296);
    getbuff(hri);
    memcpy(linebuff+296, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+656, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+1016, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+1376, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+1736, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+2096, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+2456, framebuff+4, 44);
  }
  else if (format == HRI_BX_FORMAT)
  {
    getbuff(hri);
    memcpy(linebuff, framebuff+36, 328);
    getbuff(hri);
    memcpy(linebuff+328, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+688, framebuff+4, 360);
    getbuff(hri);
    memcpy(linebuff+1048, framebuff+4, 202);
  }
  else
  {
    std::cerr << "Invalid HRI format : " << format << std::endl;
    throw;
  }
  return linebuff;
}

unsigned char * Hri::mod_get_dataline( ifstream &hri, int format )
{
  static unsigned char linebuff[2500];

  if (format == HRI_A_FORMAT)
  {
    mod_getbuff(hri, 0);
    memcpy(linebuff, mod_framebuff+68+320, 1660);
    hri.read(mod_framebuff, 1024);
    if (hri.fail())
    {
      std::cerr << "Read failed." << std::endl;
      throw;
    }
    memcpy(linebuff+1660, mod_framebuff, 840);
  }
  else if (format == HRI_BX_FORMAT)
  {
    getbuff(hri);
    memcpy(linebuff, framebuff+36+160, 168);
    hri.read(mod_framebuff, 1172);
    if (hri.fail())
    {
      std::cerr << "Read failed." << std::endl;
      throw;
    }
    memcpy(linebuff+168, mod_framebuff, 1082);
  }
  else
  {
    std::cerr << "Invalid HRI format : " << format << std::endl;
    throw;
  }
  return linebuff;
}

void Hri::mod_getbuff( ifstream &hri, int offset )
{
  unsigned short chk_sync[2];

  memset(mod_framebuff, 0, mod_framesize);

  hri.read(mod_framebuff, mod_framesize);
  if (hri.fail())
  {
    std::cerr << "Read failed." << std::endl;
    return;
  }

  memcpy(chk_sync, mod_framebuff+offset, 4);

  if (chk_sync[0] != 0x0C05 && (chk_sync[1] & 255) != 0xDF)
  {
    std::cerr << "Sync error in input hri file." << std::endl;
    std::cerr << "Position is : " << std::hex
         << hri.tellg( ) << " - " << mod_framesize << std::endl;
    throw;
  }
  return;
}

void Hri::getbuff( ifstream &hri )
{
  unsigned short chk_sync[2];

  memset(framebuff, 0, framesize);

  hri.read(framebuff, framesize);
  if (hri.fail())
  {
    std::cerr << "Read failed." << std::endl;
    return;
  }
  memcpy(chk_sync, framebuff, 4);
  if (chk_sync[0] != 0x0C05 && (chk_sync[1] & 255) != 0xDF)
  {
    std::cerr << "Sync error in input hri file." << std::endl;
    std::cerr << "Position is : " << std::hex
         << hri.tellg( ) << " - " << framesize << std::endl;
    throw;
  }
  return;
}

#ifdef TESTME

int main(int argc, char *argv[])
{
  Hri test;

  if (argc < 2)
  {
    std::cerr << "Not enough arguments" << std::endl;
    std::cerr << "Usage : " << argv[0] << " hrifile" << std::endl;
    return -1;
  }

  try
  {
    test.readfrom(argv[1], false);
  }
  catch( ... )
  {
    return -1;
  }
  cout << test;

  return 0;
}

#endif
