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
#include <proj/Mercator.h>
#include <cmath>

namespace msat {
namespace proj {

void Mercator::mapToProjected(const MapPoint& m, ProjectedPoint& p) const
{
  p.x = m.lon / 180.0;
  p.y = M_1_PI * log(tan((M_PI/360)*(90.0-m.lat)));
}

void Mercator::projectedToMap(const ProjectedPoint& p, MapPoint& m) const
{
  m.lon = 180.0 * p.x;
  m.lat  = 90.0 - (360/M_PI) * atan(exp(p.y * M_PI));
}

}
}

// vim:set ts=2 sw=2:
