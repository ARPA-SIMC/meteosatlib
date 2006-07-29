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
#include <Proj_Geos.h>
#include <string.h>
#include <cmath>

#define cosd(x) cos(DTR*(x))
#define sind(x) sin(DTR*(x))
#define tand(x) tan(DTR*(x))
#define atand(x) RTD*atan((x))
#define asind(x) RTD*asin((x))

ProjGeos::ProjGeos( ) { }

ProjGeos::ProjGeos( ProjectionParameters *p )
{
  set_parameters(p);
}

void ProjGeos::set_parameters( ProjectionParameters *p )
{
  memcpy(&params, p, sizeof(ProjectionParameters));
  return;
}

void ProjGeos::set_parameters( double Longitude, double OrbitRadius )
{
  params.Longitude = Longitude;
  params.OrbitRadius = OrbitRadius;
  return;
}

void ProjGeos::Map_to_Projected( MapPoint *M, ProjectedPoint *P )
{
  double c_lat;
  double r1,r2,r3,rn,rl;

  c_lat = atan( EARTH_1E2 * tand(M->latitude) );
  rl    = EARTH_RPOL / ( sqrt(1.0 - EARTH_E2 * pow(cos(c_lat), 2.0)) );

  r1 = params.OrbitRadius - \
        rl * cos(c_lat) * cosd((M->longitude - params.Longitude));
  r2 = -rl * cos(c_lat) * sind((M->longitude - params.Longitude));
  r3 = rl * sin(c_lat);
  rn = sqrt( r1*r1 + r2*r2 + r3*r3 );

  P->x = atand( (-r2 / r1) );
  P->y = asind( (-r3 / rn) );

  return;
}

void ProjGeos::Projected_to_Map( ProjectedPoint *P, MapPoint *M )
{
  double sd,sn;
  double s1,s2,s3,sxy;

  sd = sqrt(pow((params.OrbitRadius * cosd(P->x) * cosd(P->y)), 2.0) - \
       (pow(cosd(P->y), 2.0) + EARTH_IE2 * pow(sind(P->y), 2.0)) * \
       1737121856 ); 
  sn = (params.OrbitRadius * cosd(P->x) * cosd(P->y) - sd) / \
       (pow(cosd(P->y), 2.0) + EARTH_IE2 * pow(sind(P->y), 2.0));

  s1 = params.OrbitRadius - sn * cosd(P->x) * cosd(P->y);
  s2 = sn * sind(P->x) * cosd(P->y);
  s3 = -sn * sind(P->y);
  sxy = sqrt( s1*s1 + s2*s2 );

  M->longitude = atand((s2/s1))+params.Longitude;
  M->latitude  = atand((EARTH_IE2 * (s3 / sxy)));

  return;
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
