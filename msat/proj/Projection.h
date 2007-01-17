//-----------------------------------------------------------------------------
//
//  File        : Projection.h
//  Description : Mapping Algorithms interface - Projection ADC
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

#include <msat/proj/Points.h> 
#include <string>

#ifndef METEOSATLIB_PROJ_PROJECTION_H
#define METEOSATLIB_PROJ_PROJECTION_H

#if 0
typedef struct {
  double Longitude;
  double OrbitRadius;
  BOOL   ifNorth;
} ProjectionParameters;
#endif

namespace msat {
namespace proj {

struct Projection
{
    virtual ~Projection() { }
    virtual void mapToProjected(const MapPoint& m, ProjectedPoint& p) const = 0;
#ifdef EXPERIMENTAL_REPROJECTION
    virtual void mapToProjected(const MapBox& m, ProjectedBox& p) const
    {
	    mapToProjected(m.topLeft, p.topLeft);
	    mapToProjected(m.topRight, p.topRight);
	    mapToProjected(m.bottomLeft, p.bottomLeft);
	    mapToProjected(m.bottomRight, p.bottomRight);
    }
#endif
    virtual void projectedToMap(const ProjectedPoint& p, MapPoint& m) const = 0;
#ifdef EXPERIMENTAL_REPROJECTION
    virtual void projectedToMap(const ProjectedBox& p, MapBox& m) const
    {
	    projectedToMap(p.topLeft, m.topLeft);
	    projectedToMap(p.topRight, m.topRight);
	    projectedToMap(p.bottomLeft, m.bottomLeft);
	    projectedToMap(p.bottomRight, m.bottomRight);
    }
#endif
    virtual std::string format() const = 0;
};

}
}

#endif
