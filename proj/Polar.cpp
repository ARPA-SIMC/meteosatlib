//-----------------------------------------------------------------------------
//
//  File        : Proj_Polar.cpp
//  Description : Mapping Algorithms interface - Polar Projection
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
#include <proj/Polar.h>
#include <string.h>
#include <cmath>
#include <sstream>

#ifndef DTR
#define DTR 0.017453292
#endif

#ifndef RTD
#define RTD 57.29577951
#endif


namespace msat {
namespace proj {

static double signum(double x)
{
  if (x >= 0.0) return 1.0;
  else return -1.0;
}

#if 0
void ProjPolar::set_parameters( ProjectionParameters *p )
{
  memcpy(&params, p, sizeof(ProjectionParameters));
  if (p->ifNorth) sign = 1.0;
    else sign = 0.0;
  return;
}

void ProjPolar::set_parameters( double Longitude, BOOL ifNorth )
{
  params.Longitude = Longitude;
  params.ifNorth = ifNorth;
  if (ifNorth) sign = 1.0;
    else sign = 0.0;
  return;
}
#endif

void Polar::mapToProjected(const MapPoint& m, ProjectedPoint& p) const
{
  double sign = north ? 1.0 : 0.0;

  double a = tan((DTR*(90.0 - sign * m.lat)) * 0.5);
  p.x = a * sin(DTR*(m.lon - longitude));
  p.y = a * cos(DTR*(m.lon - longitude));
}

void Polar::projectedToMap(const ProjectedPoint& p, MapPoint& m) const
{
  double sign = north ? 1.0 : 0.0;

  m.lon = RTD*atan2(p.x, p.y) + longitude + \
          sign * 90.0 * (1.0 - signum(p.y));
  m.lat = sign * (90.0 - 2.0 * \
          RTD*atan(sqrt(pow(p.x, 2.0) + pow(p.y, 2.0))) );
}

std::string Polar::format() const
{
  std::stringstream str;
  str << "Polar(longitude: " << longitude << ", north: " << north << ")";
  return str.str();
}

}
}

// vim:set ts=2 sw=2:
