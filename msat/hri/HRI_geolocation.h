//-----------------------------------------------------------------------------
//
//  Copyright (C) 2004--2006  Graziano Giuliani <giuliani@lamma.rete.toscana.it>
//  Copyright (C) 2006--2012  ARPA-SIM <urpsim@smr.arpa.emr.it>
//
//  File        : HRI_geolocation.h
//  Description : Meteosat HRI format interface
//  Project     : Meteosatlub
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
#include <string>
#include <iostream>

typedef struct {
  long CFAC;
  long LFAC;
  long COFF;
  long LOFF;
} geolocation;

class HRI_geolocation {
  public:
    HRI_geolocation( ) { }
    HRI_geolocation( std::string format ) { set_format(format); }
    ~HRI_geolocation( ) { }
    void set_format( std::string format )
    {
      if (format == "AW"   || format == "AIW" ||
          format == "AIVH" || format == "AVH")
      {
	g.CFAC = -9102222;
	g.LFAC = -9102222;
	g.COFF = 1248;
	g.LOFF = 1249;
      }
      else if (format == "AV")
      {
	g.CFAC = -18204444;
	g.LFAC = -18204444;
	g.COFF = 2500;
	g.LOFF = 2500;
      }
      else if (format == "BW"  || format == "BIV" ||
	       format == "BIW" || format == "BIVH" )
      {
	g.CFAC = -18204444;
	g.LFAC = -18204444;
	g.COFF = 1248;
	g.LOFF = -1118;
      }
      else if (format == "XI" || format == "XVH" || format == "XW")
      {
	g.CFAC = -9102222;
	g.LFAC = -9102222;
	g.COFF = 1250;
	g.LOFF = 1250;
      }
      else
      {
	std::cerr << "Unknown or unsupported format in HRI_geolocation"
                  << std::endl;
	throw;
      }
    }
    geolocation &get_geolocation( ) { return g; }
  private:
    std::string format;
    geolocation g;
};
