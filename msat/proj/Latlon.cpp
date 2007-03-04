//-----------------------------------------------------------------------------
//
//  File        : Latlon.h
//  Description : Mapping Algorithms interface - Latitude-Longitude grid
//  Author      : Enrico Zini (ARPA SIM Emilia Romagna)
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
#include <msat/proj/Latlon.h>
#include <cmath>

namespace msat {
namespace proj {

void Latlon::mapToProjected(const MapPoint& m, ProjectedPoint& p) const
{
  p.x = m.lon;
  p.y = m.lat;
}

void Latlon::projectedToMap(const ProjectedPoint& p, MapPoint& m) const
{
  m.lon = p.x;
  m.lat = p.y;
}

}
}

// vim:set ts=2 sw=2:
