//-----------------------------------------------------------------------------
//
//  File        : Proj_Mercator.h
//  Description : Mapping Algorithms interface - Mercator Projecton
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

#ifndef __PROJ_MERCATOR_H__
#define __PROJ_MERCATOR_H__

class ProjMercator : public Projection {
  public:
    ProjMercator();
    ProjectionParameters params;
    virtual void Map_to_Projected( MapPoint *M, ProjectedPoint *P );
    virtual void Projected_to_Map( ProjectedPoint *P, MapPoint *M );
};

#endif
