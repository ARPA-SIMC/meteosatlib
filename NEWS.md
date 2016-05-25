# User-visible news in Meteosatlib releases

## New in version 1.5

* Removed support for SAF HDF5 images: plain GDAL is enough to process them.
* Added support for WMO GRIB2 template 4.31 images. This is now the default for
  GRIB output.
* Unit tests do not need libtut to compile anymore.
* Added support for MSG2 Rapid Scan System (RSS)
* Reflectance calculations are enabled for all MeteosatLib dataset types,
  passing `MSAT_COMPUTE=reflectance` as an open option
  (like in `gdalwarp -oo MSAT_COMPUTE=reflectance`)
