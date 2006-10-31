//-----------------------------------------------------------------------------
//
//  File        : Area.h
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
#include <Standard.h>
#include <Points.h>
#include <Projection.h>
#include <Proj_Geos.h>
#include <Proj_Polar.h>
#include <Proj_Mercator.h>

#ifndef __AREA_H__
#define __AREA_H__

#pragma interface

#define PNAMELEN     32
#define AREANAMELEN  64

typedef struct {
  char Name[AREANAMELEN]; // Conventional name of Area
  char PrjName[PNAMELEN]; // Projection name
  int nColumns;           // Width of area
  int nLines;             // Height of area
  double SampleX;         // Subsampling in x direction
  double SampleY;         // Subsampling in y direction
  long CFAC;              // Column scale factor
  long LFAC;              // Line scale factor
  long COFF;              // Column offset
  long LOFF;              // Line offset
  double Longitude;       // Satellite/Polar longitude
  double OrbitRadius;     // Satellite Orbit Radius
  BOOL ifNorth;           // TRUE if North Polar
  BOOL NorthSouth;        // TRUE if Scan is North/South
} AreaParameters;

typedef enum {
  PROJ_GEOS = 1,
  PROJ_POLAR,
  PROJ_MERCATOR,
  PROJ_LAMBERT
} ProjectionCode;

class Area {
  //
  // Area coordinate transformation
  //
  public:
    Area( AreaParameters *p );
    void Map_to_Image( MapPoint *M, ImagePoint *I );
    void Image_to_Map( ImagePoint *I, MapPoint *M );

  private:
    AreaParameters params;
    ProjectionCode pjcode;
    Projection *projection;
    ProjGeos pg;
    ProjPolar pp;
    ProjMercator pm;
    double xScale;
    double yScale;
    void Image_to_Projected( ImagePoint *I, ProjectedPoint *P );
    void Projected_to_Image( ProjectedPoint *P, ImagePoint *I );
};

#endif
