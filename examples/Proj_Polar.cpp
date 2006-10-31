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
#include <Proj_Polar.h>
#include <string.h>
#include <cmath>

double ProjPolar::signum(double x)
{
  if (x >= 0.0) return 1.0;
  else return -1.0;
}

ProjPolar::ProjPolar( ) { }

ProjPolar::ProjPolar( ProjectionParameters *p )
{
  set_parameters(p);
}

void ProjPolar::set_parameters( ProjectionParameters *p )
{
  memcpy(&params, p, sizeof(ProjectionParameters));
  if (p->ifNorth) sign = 1.0;
    else sign = -1.0;
  return;
}

void ProjPolar::set_parameters( double Longitude, BOOL ifNorth )
{
  params.Longitude = Longitude;
  params.ifNorth = ifNorth;
  if (ifNorth) sign = 1.0;
    else sign = -1.0;
  return;
}

void ProjPolar::Map_to_Projected( MapPoint *M, ProjectedPoint *P )
{
  double a;

  a = tan((DTR*(90.0-sign*M->latitude)) * 0.5);
  P->x = a * sin(DTR*(M->longitude-params.Longitude));
  P->y = a * cos(DTR*(M->longitude-params.Longitude));
  return;
}

void ProjPolar::Projected_to_Map( ProjectedPoint *P, MapPoint *M )
{
  M->longitude = RTD*atan2(P->x, P->y) + params.Longitude + \
                 sign * 90.0 * (1.0 - signum(P->y));
  M->latitude  = sign * (90.0 - 2.0 * \
                 RTD*atan(sqrt(pow(P->x, 2.0) + pow(P->y, 2.0))) );
  return;
}

#ifdef TESTME

#include <iostream>

int main(int argc, char *argv[])
{
  ProjectionParameters p;
  p.Longitude = 10.0;
  p.ifNorth = TRUE;

  MapPoint M;
  ProjectedPoint P;

  Projection *proj;
  ProjPolar pp(&p);

  proj = &pp;

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
