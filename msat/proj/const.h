#ifndef MSAT_PARAMETERS_H
#define MSAT_PARAMETERS_H

// Equatorial raius of the earth (Km.)
#define EARTH_RADIUS 6378.1370	// R

// Orbit_Radius (Km.)
#define ORBIT_RADIUS 42164.0	// orbit

// see the HRIT driver for gdal for how this is computed
#define ORBIT_RADIUS_FOR_GDAL 35785831

/// Camera height in units of earth radius
#define SEVIRI_CAMERA_H    (ORBIT_RADIUS / EARTH_RADIUS)

#define SEVIRI_ORIENTATION 180.0

#define METEOSAT_IMAGE_NCOLUMNS 3712
#define METEOSAT_IMAGE_NLINES 3712

#define METEOSAT_IMAGE_NCOLUMNS_HRV 11136
#define METEOSAT_IMAGE_NLINES_HRV 11136

const double METEOSAT_PIXELSIZE_X = 3000.40307617188;
const double METEOSAT_PIXELSIZE_Y = 3000.40307617188;
const double METEOSAT_PIXELSIZE_X_HRV = 1000.13433837891;
const double METEOSAT_PIXELSIZE_Y_HRV = 1000.13433837891;


#endif
