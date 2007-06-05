Name:           meteosatlib
Version:        0.5.4
Release:        1
Summary:        Applications to manage Meteosat satellite images

Group:          Applications/Meteo
License:        GPL
URL:            http://meteosatlib.sf.net
Source0:        %{name}-%{version}.tar.gz
Source1:        PublicDecompWT.zip
NoSource:       1
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

BuildRequires:   netcdf-devel, hdf5-devel >= 1.6.5-5 , gdal-devel


%description
 Read, write, convert and display raster satellite images 
 (also in Meteosat8 HRIT format).
 Provide:
  msat as a simple commandline tool allowing to:
 .
  - Read images in GRIB version 1, SAF HDF5 and some NetCDF based formats (COARDS compliant).
  - Write images in GRIB version 1, JPG, PNG and some NetCDF based formats (COARDS compliant).
  - Crop and scale images
  - Convert between file formats 
  - Data can be reprojected in a regular latitude longitude grid of selected resolution
  - Precisely correlate every pixel with its geographical coordinates
  - Display images

 Library for working with georeferentiated raster images (shared library)
 The library allows to:
 .
  - Read images in GRIB, SAF HDF5 and some NetCDF based formats.
  - Write images in GRIB, JPG, PNG and some NetCDF based formats.


%package hri-devel

Group: Libraries/Meteo
Summary: Development for Meteosat HRI satellite images

%description hri-devel
Libraries to read Meteosat 7/8 images in various formats.

%package hrit-devel

License: Non Free (Copyright Eumetsat)
Group: Libraries/Meteo
Summary: Development for Meteosat HRI satellite images

%description hrit-devel
Libraries to read Meteosat 7/8 images in various formats.
For MSG data decompression, You need the EUMETSAT source code for
the decompression library for Wavelet, JPEG and T4 images.

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Please fill the form at

   http://www.eumetsat.de/en/dps/helpdesk/tools.html

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

%package msg-native-devel

Group: Libraries/Meteo
Summary: Development for Meteosat MSG satellite images

%description msg-native-devel
Libraries to read Meteosat 7/8 images in various formats.

%package omtp-ids-devel

Group: Libraries/Meteo
Summary: Development for Meteosat OpenMTP-IDS satellite images

%description omtp-ids-devel
Libraries to read Meteosat 7/8 images in various formats.

%package openmtp-devel

Group: Libraries/Meteo
Summary: Development for Meteosat OpenMTP-IDS satellite images

%description openmtp-devel
Libraries to read Meteosat 7/8 images in various formats.

%package thornsds_db1-devel

Group: Libraries/Meteo
Summary: Development for Meteosat ThornSDS DB1 PDUS images

%description thornsds_db1-devel
Libraries to read Meteosat 7/8 images in various formats.


%package msat-devel

Group: Libraries/Meteo
Summary: Library for working with georeferentiated raster images

%description msat-devel
Library for working with georeferentiated raster images
 The library allows to:
 .
  - Read images in GRIB, SAF HDF5 and some NetCDF based formats.
  - Write images in GRIB, JPG, PNG and some NetCDF based formats.
  - Precisely correlate every pixel with its geographical coordinates
  - Extract image subareas
  - Display images
  - Reproject images
 .
 This is the library for C++ development.
 .

%package grib-devel

Group: Libraries/Meteo
Summary: Library for reading and writing WMO GRIB image data

%description grib-devel
 Library for reading and writing WMO GRIB image data
 libgrib-dev provides functions for reading and writing gridded data in WMO
 GRIB format.
 .
 This is the library for C++ development.


%prep
%setup -q
test -f %{SOURCE1} \
    && ln -s %{SOURCE1} decompress/ \
    || echo " *** WARNING: source file not found: %{SOURCE1}"

%build
%configure
make %{?_smp_mflags}

%install
rm -rf $RPM_BUILD_ROOT
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%doc AUTHORS ChangeLog COPYING CREDITS INSTALL NEWS
%doc PublicDecompWT_LICENSE README TODO
%{_bindir}
%{_libdir}/libhri.so*
%{_libdir}/libhrit.so*
%{_libdir}/libnative.so*
%{_libdir}/libomtp-ids.so*
%{_libdir}/libopenmtp.so*
%{_libdir}/libthornsds_db1.so*
%{_libdir}/libmsat.so*
%{_libdir}/libgrib.so*
%{_mandir}/man1/msat.1.gz


%files hri-devel
%{_includedir}/hri
%{_libdir}/libhri.a
%{_libdir}/libhri.la

%files hrit-devel
%{_includedir}/hrit
%{_includedir}/PublicDecompWT
%{_libdir}/libCOMP.a
%{_libdir}/libDISE.a
%{_libdir}/libhrit.a
%{_libdir}/libhrit.la
%{_libdir}/libJPEG.a
%{_libdir}/libT4.a
%{_libdir}/libWT.a
/usr/lib/pkgconfig/libhrit.pc
/usr/share/aclocal/libhrit.m4


%files msg-native-devel
%{_includedir}/msg-native
%{_libdir}/libnative.a
%{_libdir}/libnative.la

%files omtp-ids-devel
%{_includedir}/omtp-ids
%{_libdir}/libomtp-ids.la
%{_libdir}/libomtp-ids.a

%files thornsds_db1-devel
%{_includedir}/thornsds_db1
%{_libdir}/libthornsds_db1.la
%{_libdir}/libthornsds_db1.a

%files openmtp-devel
%{_includedir}/openmtp
%{_libdir}/libopenmtp.la
%{_libdir}/libopenmtp.a


%files msat-devel
%{_includedir}/msat
%{_libdir}/libmsat.a
%{_libdir}/libmsat.la
/usr/lib/pkgconfig/libmsat.pc
/usr/share/aclocal/libmsat.m4

%files grib-devel
%{_includedir}/grib
%{_libdir}/libgrib.a
%{_libdir}/libgrib.la
/usr/lib/pkgconfig/libgrib.pc
/usr/share/aclocal/libgrib.m4



%changelog
* Wed Feb 28 2007 root <root@strip.metarpa> - 0.5-1
- hdf5 c++library is static, so we need a specific version in BuildRequires

* Tue Feb 13 2007 Paolo Patruno <ppatruno@arpa.emr.it> - 0.5-1
- added grib and msat pachages and other change to switch to 0.5 release

* Wed Aug 08 2005  Graziano Giuliani  <graziano.giuliani@poste.it>  0.4.0
- Added ThornSDS DB1 file interface

* Fri Jul 01 2005  Deneys S. Maartens  <dsm@tlabs.ac.za>  0.3.6-1
- modified original meteosatlib.spec by Paolo Patruno
  <ppatruno@smr.arpa.emr.it>: added sepparate devel sections

# -fin-
