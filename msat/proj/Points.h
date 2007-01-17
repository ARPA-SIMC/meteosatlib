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

namespace msat {
namespace proj {

struct ImagePoint
{
  long column;
  long line;
  ImagePoint() {}
  ImagePoint(const long& column, const long& line) : column(column), line(line) {}
};

struct ProjectedPoint
{
  double x;
  double y;
  ProjectedPoint() {}
  ProjectedPoint(const double& x, const double& y) : x(x), y(y) {}
};

struct MapPoint
{
  double lat;
  double lon;
  MapPoint() {}
  MapPoint(const double& lat, const double& lon) : lat(lat), lon(lon) {}
};

template<typename Point>
struct Box
{
  Point topLeft;
  Point topRight;
  Point bottomLeft;
  Point bottomRight;
  Box() {}
  Box(const Point& topLeft, const Point& topRight, const Point& bottomLeft, const Point& bottomRight) :
	  topLeft(topLeft), topRight(topRight) {}
	  bottomLeft(bottomLeft), bottomRight(bottomRight) {}
};

typedef Box<ImagePoint> ImageBox;
struct ProjectedBox : public Box<ProjectedPoint>
{
  void boundingBox(double& x0, double& y0, double& w, double& h)
  {
    x0 = this->topleft.x;
    if (this->topright.x < x0) x0 = this->topright.x;
    if (this->bottomleft.x < x0) x0 = this->bottomleft.x;
    if (this->bottomright.x < x0) x0 = this->bottomright.x;
    y0 = this->topleft.y;
    if (this->topright.y < y0) y0 = this->topright.y;
    if (this->bottomleft.y < y0) y0 = this->bottomleft.y;
    if (this->bottomright.y < y0) y0 = this->bottomright.y;
    int x1;
    int y1;
    x1 = this->topLeft.x;
    if (this->topRight.x > x1) x1 = this->topRight.x;
    if (this->bottomLeft.x > x1) x1 = this->bottomLeft.x;
    if (this->bottomRight.x > x1) x1 = this->bottomRight.x;
    y1 = this->topleft.y;
    if (this->topright.y > y1) y1 = this->topright.y;
    if (this->bottomleft.y > y1) y1 = this->bottomleft.y;
    if (this->bottomright.y > y1) y1 = this->bottomright.y;
    w = x1 - x0;
    h = y1 - y0;
  }
};
typedef Box<MapPoint> MapBox;

}
}

#endif
