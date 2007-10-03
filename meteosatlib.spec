Name:           meteosatlib
Version:        0.5.5
Release:        1
Summary:        Applications to manage Meteosat satellite images

Group:          Applications/Meteo
License:        GPL
URL:            http://meteosatlib.sf.net
Source0:        %{name}-%{version}.tar.gz
Source1:        PublicDecompWT.zip
NoSource:       1
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

Vendor:         Graziano Giuliani <giuliani@lamma.rete.toscana.it>
Packager:       Deneys S. Maartens <dsm@tlabs.ac.za>

%description
Programs to convert Meteosat8 HRIT format images to WMO GRIB version 1
format, NetCDF COARDS compliant format, image format (any ImageMagick
supported format).

Converted NetCDF images can be reprojected in a regular latitude
longitude grid of selected resolution.

A program to convert GRIB images back to NetCDF images for reprojection
is also given.

%package hri-devel
Requires: meteosatlib = %{version}

Group: Libraries/Meteo
Summary: Development for Meteosat HRI satellite images

%description hri-devel
Libraries to read Meteosat 7/8 images in various formats.

#%package hrit-devel
#Requires: meteosatlib = %{version}
#
#License: Non Free (Copyright Eumetsat)
#Group: Libraries/Meteo
#Summary: Development for Meteosat HRI satellite images
#
#%description hrit-devel
#Libraries to read Meteosat 7/8 images in various formats.
#For MSG data decompression, You need the EUMETSAT source code for
#the decompression library for Wavelet, JPEG and T4 images.
#
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#
#Please fill the form at
#
#   http://www.eumetsat.de/en/dps/helpdesk/tools.html
#
#!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#%package msg-native-devel
#Requires: meteosatlib = %{version}
#
#Group: Libraries/Meteo
#Summary: Development for Meteosat MSG satellite images
#
#%description msg-native-devel
#Libraries to read Meteosat 7/8 images in various formats.

%package omtp-ids-devel
Requires: meteosatlib = %{version}

Group: Libraries/Meteo
Summary: Development for Meteosat OpenMTP-IDS satellite images

%description omtp-ids-devel
Libraries to read Meteosat 7/8 images in various formats.

%package openmtp-devel
Requires: meteosatlib = %{version}

Group: Libraries/Meteo
Summary: Development for Meteosat OpenMTP-IDS satellite images

%description openmtp-devel
Libraries to read Meteosat 7/8 images in various formats.

%package thornsds_db1-devel
Requires: meteosatlib = %{version}

Group: Libraries/Meteo
Summary: Development for Meteosat ThornSDS DB1 PDUS images

%description thornsds_db1-devel
Libraries to read Meteosat 7/8 images in various formats.

%prep
%setup -q
test -f %{SOURCE1} \
    && ln -s %{SOURCE1} decompress/ \
    || echo " *** WARNING: source file not found: %{SOURCE1}"

%build
configure_disable=
#configure_disable="$configure_disable --disable-hri"
configure_disable="$configure_disable --disable-hrit"
configure_disable="$configure_disable --disable-msg-native"
#configure_disable="$configure_disable --disable-omtp-ids"
#configure_disable="$configure_disable --disable-openmtp"
#configure_disable="$configure_disable --disable-thornsds_db1"
#configure_disable="$configure_disable --disable-hdf5"

%configure $configure_disable
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
#%{_libdir}/libhrit.so*
#%{_libdir}/libnative.so*
%{_libdir}/libomtp-ids.so*
%{_libdir}/libopenmtp.so*
%{_libdir}/libthornsds_db1.so*

%files hri-devel
%{_includedir}/hri
%{_libdir}/libhri.a
%{_libdir}/libhri.la

#%files hrit-devel
#%{_includedir}/hrit
#%{_includedir}/PublicDecompWT
#%{_libdir}/libCOMP.a
#%{_libdir}/libDISE.a
#%{_libdir}/libhrit.a
#%{_libdir}/libhrit.la
#%{_libdir}/libJPEG.a
#%{_libdir}/libT4.a
#%{_libdir}/libWT.a

#%files msg-native-devel
#%{_includedir}/msg-native
#%{_libdir}/libnative.a
#%{_libdir}/libnative.la

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

%changelog
* Tue Aug 15 2006  Deneys S. Maartens  <dsm@tlabs.ac.za>  0.4.2
- Added logic to add conditional disable-* flags to %configure

* Wed Aug 08 2005  Graziano Giuliani  <graziano.giuliani@poste.it>  0.4.0
- Added ThornSDS DB1 file interface

* Fri Jul 01 2005  Deneys S. Maartens  <dsm@tlabs.ac.za>  0.3.6-1
- modified original meteosatlib.spec by Paolo Patruno
  <ppatruno@smr.arpa.emr.it>: added sepparate devel sections

# -fin-
