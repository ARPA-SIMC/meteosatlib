//-----------------------------------------------------------------------------
//
//  File        : Points.h
//  Description : Mapping Algorithms interface
//  Project     : CETEMPS 2003
//  Original author: Graziano Giuliani (CETEMPS - University of L'Aquila)
//  Modified by : Enrico Zini (ARPA SIM Emilia Romagna) 
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
#ifndef METEOSATLIB_PROJ_POINTS_H
#define METEOSATLIB_PROJ_POINTS_H

typedef struct {
  long column;
  long line;
  ImagePoint(const long& column, const long& line) : column(column), line(line) {}
} ImagePoint;

typedef struct {
  double x;
  double y;
  ProjectedPoint(const double& x, const double& y) : x(x), y(y) {}
} ProjectedPoint;

typedef struct {
  double lat;
  double lon;
  MapPoint(const double& lat, const double& lon) : lat(lat), lon(lon) {}
} MapPoint;

#endif
