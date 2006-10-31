#ifndef MSAT_PARAMETERS_H
#define MSAT_PARAMETERS_H

//#define PI 3.14159265

// Equatorial raius of the earth (Km.)
#define EARTH_RADIUS 6378.1370	// R

// Orbit_Radius (Km.)
#define ORBIT_RADIUS 42164.0	// orbit

/// Camera height in units of earth radius
#define SEVIRI_CAMERA_H    (ORBIT_RADIUS / EARTH_RADIUS)

#define SEVIRI_ORIENTATION 180.0

#define METEOSAT_IMAGE_NCOLUMNS 3712
#define METEOSAT_IMAGE_NLINES 3712


#endif
