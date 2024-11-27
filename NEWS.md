# User-visible news in Meteosatlib releases

## New in version 1.17

* Build with meson (#41)

## New in version 1.16

* Support for GDAL v3.6
* Support for gcc 13

## New in version 1.15

* Removed currently unused implementation of reflectance calculation for non-HRIT sources via vrt datasets (#29)

## New in version 1.14

* reimplemented msat-view using modern Gtk (#27)

## New in version 1.13

* multiple image rescale options in products script (#26)

## New in version 1.12

* fixed handling of MSG-4/Meteosat 11 (#24)
* updated scipy.misc imresize with pillow (#23)

## New in version 1.11

* Updated PublicDecompWT handling now that the code is free and published at
  https://gitlab.eumetsat.int/open-source/PublicDecompWT (#14)
   - Build with PublicDecompWT bundled as a git submodule
   - Build with system-installed PublicDecompWT using
     `./configure --with-system-pdwt`
* Fixed issues with new compilers (#10, #16)
* Fixed issues with gdal 3 (#19)
* Fixed issue with eccodes key that has been renamed (#20)

## New in version 1.10

* Added gdal 3 support

## New in version 1.9

* All scripts ported to python3
* Refactored IR 3.9 reflectance calculation via virtual dataset
* Refactored reflectance calculatuion for all datasets
* Support RSS data (both HRV and non-HRV)
* Various building improvements

## New in version 1.5

* Removed support for SAF HDF5 images: plain GDAL is enough to process them.
* Added support for WMO GRIB2 template 4.31 images. This is now the default for
  GRIB output.
* Unit tests do not need libtut to compile anymore.
* Added support for MSG2 Rapid Scan System (RSS)
* Reflectance calculations are enabled for all MeteosatLib dataset types,
  passing `MSAT_COMPUTE=reflectance` as an open option
  (like in `gdalwarp -oo MSAT_COMPUTE=reflectance`)
