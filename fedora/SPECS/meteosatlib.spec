# Note: define srcarchivename in Travis build only.
%{!?srcarchivename: %global srcarchivename %{name}-%{version}-%{release}}

Name:           meteosatlib
Version:        1.16
Release:        1
Summary:        Shared libraries to read Meteosat satellite images

Group:          Applications/Meteo
License:        GPL
URL:            https://github.com/ARPA-SIMC/meteosatlib
Source0:        https://github.com/arpa-simc/%{name}/archive/v%{version}-%{release}.tar.gz#/%{srcarchivename}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-buildroot

%if 0%{?rhel} == 7
%define python3_vers python36
# to have python 3.6 interpreter
BuildRequires: python3-rpm-macros >= 3-23
%else
%define python3_vers python3
%endif

BuildRequires:  libtool
BuildRequires:  gcc-c++
BuildRequires:  netcdf-cxx-devel
BuildRequires:  ImageMagick-c++-devel
BuildRequires:  gdal-devel >= 1.6
BuildRequires:  hdf5-devel > 1.8
BuildRequires:  dos2unix
BuildRequires:  help2man
BuildRequires:  PublicDecompWT-devel
BuildRequires:  eccodes-devel
BuildRequires: %{python3_vers}

Vendor:         Enrico Zini <enrico@enricozini.org> 
Packager:       Daniele Branchini <dbranchini@arpae.it>

%description

C++ source code to read Meteosat Images in various formats.
Supported: OpenMTP OpenMTP-IDS HRI HRIT/LRIT.

This library is used to read Meteosat files on a PDUS receiving
station. No decryption is performed.

%package gdal

Group: Applications/Meteo
Summary: GDAL drivers based on Meteosatlib

%description gdal
 GDAL plugins that allow to read and write images using Meteosatlib.
 
 It adds support for these new formats:
 
  - MsatXRIT (ro): Meteosat xRIT (if enabled in meteosatlib)
  - MsatSAFH5 (ro): SAF HDF5
  - MsatNetCDF (rw): Meteosatlib NetCDF
  - MsatNetCDF24 (rw): Meteosatlib NetCDF24
  - MsatGRIB (rw): Meteosatlib GRIB via grib_api or eccodes

%package devel

Group:		Libraries/Meteo
Summary:	Library of utility functions for working with Meteosat images

%description devel
Libraries to read Meteosat 7/8 images in various formats.
This is the library for C++ development.

%package tools

Group:		Applications/Meteo
Summary:  	Read, write, convert and display raster satellite images

%description tools
 This package provides two tools:
 
  - msat: a command line tool to crop, scale, convert and display satellite
    images
  - msat-view: an interactive tool to zoom into satellite images and
    georeference points and areas

%prep
%setup -q -n %{srcarchivename}

%build

%define gdal_pl_version %(gv=$(gdal-config --version); echo ${gv%.*})

## WARNING (GDAL < 1.8 PLUGINS PATH in RHEL/Fedora/CentOS)
## Gdal plugins path may vary accordingly to gdal versions and architecture:
## - plugins so lib are not versioned in gdal 1.6 and 1.7
## - in x86_64 gdal 1.7 looks for plugin in /usr/lib and not /usr/lib64
## (for further info:
##  http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=508340
##  and https://trac.osgeo.org/gdal/ticket/2983)
## so you might want to intercept the plugins and put them in the right place:
#mkdir -p $RPM_BUILD_ROOT/usr/lib
#mv $RPM_BUILD_ROOT/usr/lib64/gdalplugins/1.7 $RPM_BUILD_ROOT/usr/lib/gdalplugins
## ...and REMBEMBER to change "%files gdal" accordingly

autoreconf -ifv
%configure --with-system-pdwt
%make_build

%check
make check

%install
rm -rf $RPM_BUILD_ROOT
%make_install

%if 0%{?fedora} <36
# see: https://fedoraproject.org/wiki/Changes/RemoveLaFiles
find $RPM_BUILD_ROOT -name "*.la" -delete
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%post

%if 0%{?fedora} >= 28 || 0%{?rhel} >= 8
# see: https://bugzilla.redhat.com/show_bug.cgi?id=1676966
mkdir -p /usr/lib/gdalplugins
ln -s %{_libdir}/gdalplugins/%{gdal_pl_version}/ /usr/lib/gdalplugins/%{gdal_pl_version}
%endif


%files
%doc AUTHORS ChangeLog LICENSE INSTALL
%doc README.md
%{_bindir}/HRI2NetCDF
%{_bindir}/OpenMTP-IDS_debug
%{_bindir}/OpenMTP-IDS_sector
%{_bindir}/OpenMTP-IDS_to_pgm
%{_bindir}/OpenMTP-IDS_write
%{_bindir}/OpenMTP_info
%{_bindir}/OpenMTP_to_NetCDF
%{_bindir}/XRIT2Image
%{_bindir}/XRIT2NetCDF
%{_bindir}/db1_to_image
%{_bindir}/db1_to_netcdf
%{_bindir}/native2Image
%{_bindir}/nativedump
%{_bindir}/xritdump
%{_libdir}/libmsat.so*

%files devel
%{_includedir}/msat
%{_libdir}/libmsat.a
%{_libdir}/pkgconfig/libmsat.pc

%files gdal
%defattr(-,root,root,-)
%{_libdir}/gdalplugins/%{gdal_pl_version}/gdal_Meteosatlib.a
%{_libdir}/gdalplugins/%{gdal_pl_version}/gdal_Meteosatlib.so
%{_libdir}/gdalplugins/%{gdal_pl_version}/gdal_Meteosatlib.so.0
%{_libdir}/gdalplugins/%{gdal_pl_version}/gdal_Meteosatlib.so.0.0.0
%doc examples/products examples/add_palette
%doc examples/contrast-stretch examples/make-composite
%doc examples/bluered-palette.txt examples/ndvi

%files tools
%defattr(-,root,root,-)
%{_bindir}/msat
%{_bindir}/msat-view
%{_mandir}/man1/msat-view.1.gz
%{_mandir}/man1/msat.1.gz


%changelog
* Mon Sep 11 2023 Daniele Branchini <dbranchini@arpae.it> - 1.16-1
- Added support for gdal v3.6

* Wed Jun 29 2022 Daniele Branchini <dbranchini@arpae.it> - 1.15-1
- Removed currently unused implementation of reflectance calculation for non-HRIT sources via vrt datasets (#29)
- Removed .la files

* Tue May 24 2022 Daniele Branchini <dbranchini@arpae.it> - 1.14-3
- Bogus release to rebuild against new ImageMagick-c++ lib

* Tue Dec  7 2021 Daniele Branchini <dbranchini@arpae.it> - 1.14-2
- Bogus release to rebuild against new ImageMagick-c++ lib

* Wed Nov 18 2020 Daniele Branchini <dbranchini@arpae.it> - 1.14-1
- reimplemented msat-view using modern Gtk (#27)
- simplified manpage generation (#28)

* Tue Nov 10 2020 Daniele Branchini <dbranchini@arpae.it> - 1.13-1
- multiple image rescale options in products script (#26)

* Mon Nov  9 2020 Daniele Branchini <dbranchini@arpae.it> - 1.12-1
- fixed handling of MSG-4/Meteosat 11 (#24)
- updated scipy.misc imresize with pillow (#23)

* Fri Jun  5 2020 Daniele Branchini <dbranchini@arpae.it> - 1.11-2
- Bogus release to rebuild against official gdal package for epel8

* Fri May 29 2020 Daniele Branchini <dbranchini@arpae.it> - 1.11-1
- Updated with free version of PublicDecompWT (#14)
- Fixed issues with new compilers (#10, #16)
- Added make check
- Added check for python3 compilers (#17)
- Fixed issues with gdal 3 (#19)
- Fixed issue with eccodes key that has been renamed (#20)

* Fri May  8 2020 Daniele Branchini <dbranchini@arpae.it> - 1.10-1
- fixes for gdal 3

* Thu Jan 23 2020 Daniele Branchini  <dbranchini@arpae.it> - 1.9-2
- enabling CentOS 8 builds

* Thu Jun 13 2019 Daniele Branchini <dbranchini@arpae.it> - 1.9-1
- porting all scripts to python3

* Mon Mar 25 2019 Daniele Branchini <dbranchini@arpae.it> - 1.8-3
- patching compilations errors and gdal plugin wrong path on f28

* Wed Oct 5 2016 Daniele Branchini <dbranchini@arpae.it> - 1.8-1
- updated wobble
- implemented distributed IR 3.9 reflectivity calculation (#2)
- gdal plugins directory semi-auto-discover hack

* Fri Jul 22 2016 Daniele Branchini <dbranchini@arpae.it> - 1.7-1
- implemented reflectance calculations for all datasets

* Wed May 25 2016 Daniele Branchini <dbranchini@arpa.emr.it> - 1.6-1
- fixed RSS HRV channel import problem (#1)

* Thu Apr 28 2016 Daniele Branchini <dbranchini@arpa.emr.it> - 1.5-1
- grib implementation rewrite (github bug #2)
- port to GDAL 2.0 (#3)
- removed SAFH5 support (#4)

* Tue Apr 1 2014 Daniele Branchini <dbranchini@arpa.emr.it> - 1.4-1
- minor corrections on specfile for compliance with recent versions of rpm managers

* Wed Feb 27 2013 Daniele Branchini <dbranchini@arpa.emr.it> - 1.2-1
- Adding example python scripts, some corrections

* Fri Nov 30 2012 Enrico Zini <enrico@enricozini.org> - 0.9-1
- Merged .spec files, some corrections

* Tue Feb 16 2010 Paolo Patruno <patruno@arpa.emr.it> - 0.6-1
- Adapted to meteosatlib 0.6

* Wed Feb 28 2007 Paolo Patruno <ppatruno@arpa.emr.it> - 0.5-1
- hdf5 c++library is static, so we need a specific version in BuildRequires

* Tue Feb 13 2007 Paolo Patruno <ppatruno@arpa.emr.it> - 0.5-1
- Added grib and msat pachages and other change to switch to 0.5 release

* Mon Aug 08 2005  Graziano Giuliani  <graziano.giuliani@poste.it>  0.4.0
- Added ThornSDS DB1 file interface

* Fri Jul 01 2005  Deneys S. Maartens  <dsm@tlabs.ac.za>  0.3.6-1
- modified original meteosatlib.spec by Paolo Patruno
  <ppatruno@smr.arpa.emr.it>: added separate devel sections
