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

#define EXPERIMENTAL_REPROJECTION


namespace msat {
namespace proj {

struct ImagePoint
{
  long column;
  long line;
  ImagePoint() : column(0), line(0) {}
  ImagePoint(const long& column, const long& line) : column(column), line(line) {}
};

struct ProjectedPoint
{
  double x;
  double y;
  ProjectedPoint() : x(0), y(0) {}
  ProjectedPoint(const double& x, const double& y) : x(x), y(y) {}
};

struct MapPoint
{
  double lat;
  double lon;
  MapPoint() : lat(0), lon(0) {}
  MapPoint(const double& lat, const double& lon) : lat(lat), lon(lon) {}
};

#ifdef EXPERIMENTAL_REPROJECTION
template<typename Point>
class Box
{
protected:
  template<typename C>
  static C min(const C& a, const C& b) { return a <= b ? a : b; }
  template<typename C>
  static C max(const C& a, const C& b) { return a >= b ? a : b; }

public:
  Point topLeft;
  Point bottomRight;
  Box() {}
  Box(const Point& topLeft, const Point& bottomRight) : topLeft(topLeft), bottomRight(bottomRight) {}
};

struct ImageBox : public Box<ImagePoint>
{
  void boundingBox(int& x0, int& y0, int& w, int& h) const
  {
    x0 = min(this->topLeft.column, this->bottomRight.column);
    y0 = min(this->topLeft.line, this->bottomRight.line);
    int x1 = max(this->topLeft.column, this->bottomRight.column);
    int y1 = max(this->topLeft.line, this->bottomRight.line);
    w = x1 - x0;
    h = y1 - y0;
  }
  bool isNonZero() const
  {
    return this->topLeft.column && this->topLeft.line && this->bottomRight.column && this->bottomRight.line;
  }
  ImageBox() {}
  ImageBox(const ImagePoint& topLeft, const ImagePoint& bottomRight) : Box<ImagePoint>(topLeft, bottomRight) {}
};
struct ProjectedBox : public Box<ProjectedPoint>
{
  void boundingBox(double& x0, double& y0, double& w, double& h) const
  {
    x0 = min(this->topLeft.x, this->bottomRight.x);
    y0 = min(this->topLeft.y, this->bottomRight.y);
    double x1 = max(this->topLeft.x, this->bottomRight.x);
    double y1 = max(this->topLeft.y, this->bottomRight.y);
    w = x1 - x0;
    h = y1 - y0;
  }
  bool isNonZero() const
  {
    return this->topLeft.x != 0.0 && this->topLeft.y != 0.0 && this->bottomRight.x != 0.0 && this->bottomRight.y != 0.0;
  }
  ProjectedBox() {}
  ProjectedBox(const ProjectedPoint& topLeft, const ProjectedPoint& bottomRight) : Box<ProjectedPoint>(topLeft, bottomRight) {}
};
struct MapBox : public Box<MapPoint>
{
  void boundingBox(double& latmin, double& latmax, double& lonmin, double& lonmax) const
  {
    latmin = min(this->topLeft.lat, this->bottomRight.lat);
    latmax = max(this->topLeft.lat, this->bottomRight.lat);
    lonmin = min(this->topLeft.lon, this->bottomRight.lon);
    lonmax = max(this->topLeft.lon, this->bottomRight.lon);
  }
  bool isNonZero() const
  {
    return this->topLeft.lat != 0 || this->topLeft.lon != 0 || this->bottomRight.lat != 0 || this->bottomRight.lon != 0;
  }
  MapBox() {}
  MapBox(const MapPoint& topLeft, const MapPoint& bottomRight) : Box<MapPoint>(topLeft, bottomRight) {}
};
#endif

}
}

#endif
