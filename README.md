Meteosatlib
===============================================================

Introduction
------------

This is a set of libraries, tools and GDAL plugins to read and write Meteosat
images in several formats.

There are low-level functions to work with: HRIT/LRIT, HRI, OpenMTP, and
OpenMTP-IDS.

The GDAL driver can work with xRIT, two NetCDF dialects, GRIB.

Use at Your own risk and for any purposes.

See AUTHORS file for contact information.

See https://github.com/ARPA-SIMC/meteosatlib/wiki for more examples and
troubleshooting.

Installation instructions
-------------------------

**IMPORTANT** about HRIT support:

  This library is used to read Meteosat files on a PDUS receiving station. No
  decryption is performed. Files are expected to be already decrypted by using
  the EUMETSAT key unit, or to be transitted already in clear form.
  
  For MSG data decompression, You need the EUMETSAT source code for the
  decompression library for Wavelet, JPEG and T4 images.
  
  To get it, please fill the form at http://oiswww.eumetsat.int/WEBOPS-cgi/wavelet/register
  to obtain the source code, and put the resulting PublicDecompWT.zip file
  under the directory "decompress".

Besides the optional PublicDecompWT.zip requisite described above, meteosatlib
is a standard autotools package. See the INSTALL file for detailed
instructions, which generally can be summarised as tue usual:

    ./configure
    make
    make check # optional
    make install


Package contents
----------------

decompress/  put PublicDecompWT.zip here. The directory contains scripts and
             patches to convince it to build.

msat/        the low-level meteosatlib library. It contains functions to
             directly access all supported formats.

gdal/        the GDAL plugin sources, which implement GDAL drivers using the
             low-level meteosatlib library.

tests/       unit tests.

tools/       command line tools using the library.

examples/    example code using the low-level libraries and the GDAL plugin.


License
-------

Meteosatlib is licensed under the terms of the GNU General Public License version
2.  Please see the file LICENSE for details.
