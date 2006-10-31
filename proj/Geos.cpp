//-----------------------------------------------------------------------------
//
//  File        : Proj_Geos.cpp
//  Description : Mapping Algorithms interface - Geostationary Proj.
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
#include <proj/Geos.h>
#include <proj/Standard.h>
#include <string.h>
#include <cmath>
#include <sstream>

#define cosd(x) cos(DTR*(x))
#define sind(x) sin(DTR*(x))
#define tand(x) tan(DTR*(x))
#define atand(x) RTD*atan((x))
#define asind(x) RTD*asin((x))

namespace msat {
namespace proj {

Geos::Geos( ) { }

void Geos::mapToProjected(const MapPoint& m, ProjectedPoint& p)
{
  double c_lat;
  double r1,r2,r3,rn,rl;

  c_lat = atan( EARTH_1E2 * tand(m.lat) );
  rl    = EARTH_RPOL / ( sqrt(1.0 - EARTH_E2 * pow(cos(c_lat), 2.0)) );

  r1 = orbitRadius - \
        rl * cos(c_lat) * cosd((m.lon - sublon));
  r2 = -rl * cos(c_lat) * sind((m.lon - sublon));
  r3 = rl * sin(c_lat);
  rn = sqrt( r1*r1 + r2*r2 + r3*r3 );

  p.x = atand( (-r2 / r1) );
  p.y = asind( (-r3 / rn) );
}

void Geos::projectedToMap(const ProjectedPoint& p, MapPoint& m)
{
  double sd,sn;
  double s1,s2,s3,sxy;

  sd = sqrt(pow((orbitRadius * cosd(p.x) * cosd(p.y)), 2.0) - \
       (pow(cosd(p.y), 2.0) + EARTH_IE2 * pow(sind(p.y), 2.0)) * \
       1737121856 ); 
  sn = (orbitRadius * cosd(p.x) * cosd(p.y) - sd) / \
       (pow(cosd(p.y), 2.0) + EARTH_IE2 * pow(sind(p.y), 2.0));

  s1 = orbitRadius - sn * cosd(p.x) * cosd(p.y);
  s2 = sn * sind(p.x) * cosd(p.y);
  s3 = -sn * sind(p.y);
  sxy = sqrt( s1*s1 + s2*s2 );

  m.lon = atand((s2/s1))+sublon;
  m.lat  = atand((EARTH_IE2 * (s3 / sxy)));
}

std::string Geos::format()
{
	std::stringstream str;
	str << "GEOS(sublon: " << sublon << ", orbitRadius: " << orbitRadius << ")";
	return str.str();
}

}
}

#ifdef TESTME

#include <iostream>

int main(int argc, char *argv[])
{
  ProjectionParameters p;
  p.OrbitRadius = 42164.0;
  p.Longitude   = 0.0;

  Projection *proj;

  ProjGeos pg(&p);
  proj = &pg;

  MapPoint M;
  ProjectedPoint P;

  M.latitude  = 45.0;
  M.longitude = 13.0;

  proj->Map_to_Projected(&M, &P);

  std::cout << "Latitude  = " << M.latitude << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "X         = " << P.x << std::endl;
  std::cout << "Y         = " << P.y << std::endl;
  
  proj->Projected_to_Map(&P, &M);

  std::cout << "Latitude  = " << M.latitude << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "X         = " << P.x << std::endl;
  std::cout << "Y         = " << P.y << std::endl;
  
  return 0;
}

#endif
// vim:set ts=2 sw=2:
