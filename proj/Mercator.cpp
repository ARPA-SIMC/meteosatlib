//-----------------------------------------------------------------------------
//
//  File        : Proj_Mercator.cpp
//  Description : Mapping Algorithms interface - Mercator Projection
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
//
//-----------------------------------------------------------------------------
#include <Proj_Mercator.h>
#include <cmath>

ProjMercator::ProjMercator( )
{
  // empty
}

void ProjMercator::Map_to_Projected( MapPoint *M, ProjectedPoint *P )
{
  P->x = M->longitude / 180.0;
  P->y = M_1_PI * log(tan(DTR*(90.0-M->latitude)*0.5));
  return;
}

void ProjMercator::Projected_to_Map( ProjectedPoint *P, MapPoint *M )
{
  M->longitude = 180.0 * P->x;
  M->latitude  = 90.0 - 2.0 * RTD*atan(exp(P->y * M_PI));
  return;
}

#ifdef TESTME

#include <iostream>

int main(int argc, char *argv[])
{
  MapPoint M;
  ProjectedPoint P;

  Projection *proj;

  ProjMercator pm;
  proj = &pm;

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
