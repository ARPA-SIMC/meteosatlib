//-----------------------------------------------------------------------------
//
//  File        : Proj_Geos.h
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
#include <proj/Projection.h>

#ifndef METEOSATLIB_PROJ_GEOS_H
#define METEOSATLIB_PROJ_GEOS_H

// Normalised geostationary space view

#ifndef EARTH_REQU
#define EARTH_REQU     6378.1690       // Equatorial radius of earth
#endif

#ifndef EARTH_RPOL
#define EARTH_RPOL     6356.5838       // Polar radius of earth
#endif

#ifndef EARTH_E2
#define EARTH_E2       0.00675701      // e^2
#define EARTH_1E2      0.993243        // 1 - e^2
#endif

#ifndef EARTH_IE2
#define EARTH_IE2      1.006803        // 1.0 / (1.0 - e^2)
#endif

#ifndef EARTH_EDGE
#define EARTH_EDGE     1.396263        // 80 degrees as radians
#endif

#ifndef EARTH_ROTATION
#define EARTH_ROTATION 7.2722E-5       // Earth rotation radians / sec
#endif

namespace msat {
namespace proj {

class Geos : public Projection
{
public:
	double sublon;
	double orbitRadius;

	Geos();
	Geos(const double& sublon, const double& orbitRadius) : sublon(sublon), orbitRadius(orbitRadius) {}

  virtual void mapToProjected(const MapPoint& m, ProjectedPoint& p);
	virtual void projectedToMap(const ProjectedPoint& p, MapPoint& m);
	virtual std::string format();
};

}
}

// vim:set ts=2 sw=2:
#endif
