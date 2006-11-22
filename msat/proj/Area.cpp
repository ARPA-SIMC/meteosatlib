//-----------------------------------------------------------------------------
//
//  File        : Area.cpp
//  Description : Mapping Algorithms interface
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

#include <msat/proj/Area.h>
#include <string.h>
#include <ctype.h>
#include <cmath>
#include <iostream>

#pragma implementation

Area::Area( AreaParameters *p )
{
  char tmp[PNAMELEN];
  int i;
  
  memcpy(&params, p, sizeof(AreaParameters));
  for (i = 0; i < (int) strlen(p->PrjName); i ++)
  {
    tmp[i] = toupper(p->PrjName[i]);
  }
  if (strstr(tmp, "GEOS") != NULL)
    pjcode = PROJ_GEOS;
  else if (strstr(tmp, "POLAR") != NULL)
    pjcode = PROJ_POLAR;
  else if (strstr(tmp, "MERCATOR") != NULL)
    pjcode = PROJ_MERCATOR;
  else
  {
    std::cerr << "Unsupported projection string." << std::endl;
    throw "Area definition invalid.";
  }

  xScale = pow(2.0, -16.0) * p->CFAC;
  yScale = pow(2.0, -16.0) * p->LFAC;

  switch (pjcode)
  {
    case PROJ_GEOS:
      pg.set_parameters(params.Longitude, params.OrbitRadius);
      projection = &pg;
      break;
    case PROJ_POLAR:
      pp.set_parameters(params.Longitude, params.ifNorth);
      projection = &pp;
      break;
    case PROJ_MERCATOR:
      projection = &pm;
      break;
    default:
      std::cerr << "Unsupported projection string." << std::endl;
      throw "Area definition invalid.";
  }
}

void Area::Image_to_Projected( ImagePoint *I, ProjectedPoint *P )
{
  double column, line;

  if (params.NorthSouth)
  {
    column = (double) (I->column) * params.SampleX;
    line   = (double) (I->line)   * params.SampleY;
  }
  else
  {
    column = (double)params.nColumns-((double)(I->column)*params.SampleX)+1;
    line   = (double)params.nLines  -((double)(I->line)  *params.SampleY)+1;
  }
  P->x = (column - (double) params.COFF) / xScale;
  P->y = (line   - (double) params.LOFF) / yScale;

  return;
}

void Area::Projected_to_Image( ProjectedPoint *P, ImagePoint *I )
{
  double column, line;

  column = (double) params.COFF + P->x * xScale - 1.0;
  line   = (double) params.LOFF + P->y * yScale - 1.0;

  if (params.NorthSouth)
  {
    I->column = (long) (rint(column) / params.SampleX);
    I->line   = (long) (rint(line)   / params.SampleY);
  }
  else
  {
    I->column = (long)rint(((double)params.nColumns-column-1)/params.SampleX);
    I->line   = (long)rint(((double)params.nLines  -line  -1)/params.SampleY);
  }
  return;
}

void Area::Map_to_Image( MapPoint *M, ImagePoint *I )
{
  ProjectedPoint p;

  projection->Map_to_Projected(M, &p);
  Projected_to_Image(&p, I);
  return;
}

void Area::Image_to_Map( ImagePoint *I, MapPoint *M )
{
  ProjectedPoint p;

  Image_to_Projected(I, &p);
  projection->Projected_to_Map(&p, M);
  return;
}

#ifdef TESTME

int main(int argc, char *argv[])
{
  AreaParameters p;

  MapPoint M;
  ImagePoint I;

  p.nColumns    = 2500;
  p.nLines      = 1250;
  p.SampleX     = 2.0;
  p.SampleY     = 2.0;
  strcpy(p.PrjName, "GEOS(0.0)");
  p.CFAC        = -18204444;
  p.LFAC        = -18204444;
  p.COFF        = 1248;
  p.LOFF        = -1118;
  p.Longitude   = 0.0;
  p.OrbitRadius = 42164.0;
  p.ifNorth     = TRUE;
  p.NorthSouth  = FALSE;

  Area a(&p);

  M.latitude  = 43.0;
  M.longitude = 12.0;

  a.Map_to_Image(&M, &I);

  std::cout << "Latitude  = " << M.latitude  << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "Column    = " << I.column    << std::endl;
  std::cout << "Line      = " << I.line      << std::endl;

  a.Image_to_Map(&I, &M);

  std::cout << "Latitude  = " << M.latitude  << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "Column    = " << I.column    << std::endl;
  std::cout << "Line      = " << I.line      << std::endl;

  p.nColumns    = 960;
  p.nLines      = 960;
  strcpy(p.PrjName, "POLAR(N,10)");
  p.CFAC        = 201326592;
  p.LFAC        = 201326592;
  p.COFF        = -670;
  p.LOFF        = -1103;
  p.SampleX     = 1.0;
  p.SampleY     = 1.0;
  p.Longitude   = 10.0;
  p.OrbitRadius = 42164.0;
  p.ifNorth     = TRUE;
  p.NorthSouth  = TRUE;

  Area b(&p);

  M.latitude  = 43.0;
  M.longitude = 12.0;

  b.Map_to_Image(&M, &I);

  std::cout << "Latitude  = " << M.latitude  << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "Column    = " << I.column    << std::endl;
  std::cout << "Line      = " << I.line      << std::endl;

  b.Image_to_Map(&I, &M);

  std::cout << "Latitude  = " << M.latitude  << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "Column    = " << I.column    << std::endl;
  std::cout << "Line      = " << I.line      << std::endl;

  p.nColumns    = 801;
  p.nLines      = 1058;
  strcpy(p.PrjName, "MERCATOR");
  p.CFAC        = 471859200;
  p.LFAC        = 471859200;
  p.COFF        = -200;
  p.LOFF        = 2316;
  p.Longitude   = 0.0;
  p.OrbitRadius = 42164.0;
  p.ifNorth     = TRUE;
  p.NorthSouth  = FALSE;

  Area c(&p);

  M.latitude  = 43.0;
  M.longitude = 12.0;

  c.Map_to_Image(&M, &I);

  std::cout << "Latitude  = " << M.latitude  << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "Column    = " << I.column    << std::endl;
  std::cout << "Line      = " << I.line      << std::endl;

  c.Image_to_Map(&I, &M);

  std::cout << "Latitude  = " << M.latitude  << std::endl;
  std::cout << "Longitude = " << M.longitude << std::endl;
  std::cout << "Column    = " << I.column    << std::endl;
  std::cout << "Line      = " << I.line      << std::endl;

  return 0;
}

#endif
