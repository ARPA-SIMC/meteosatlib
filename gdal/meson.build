msatdrv_sources = [
  'utils.cpp',
  'reflectance/pixeltolatlon.cpp',
  'reflectance/base.cpp',
  'reflectance/reflectance.cpp',
  'reflectance/cos_sol_za.cpp',
  'reflectance/sat_za.cpp',
  'reflectance/jday.cpp',
]

msatdrv_deps = [gdal_dep]
msatdrv_link_with = [msat_base, libmsat]

# dist_noinst_HEADERS = \
#     utils.h \
#     reflectance/pixeltolatlon.h \
#     reflectance/base.h \
#     reflectance/reflectance.h \
#     reflectance/cos_sol_za.h \
#     reflectance/sat_za.h \
#     reflectance/jday.h

if eccodes_dep.found()
# dist_noinst_HEADERS += \
#     grib/grib.h \
#     grib/utils.h
  msatdrv_sources += ['grib/grib.cpp']
  msatdrv_deps += [eccodes_dep]
endif

if netcdfpp_h and libnetcdfpp.found()
# dist_noinst_HEADERS += \
#     netcdf/utils.h \
#     netcdf/netcdf.h \
#     netcdf/netcdf24.h
  msatdrv_sources += [
    'netcdf/utils.cpp',
    'netcdf/netcdf.cpp',
    'netcdf/netcdf24.cpp',
  ]
  msatdrv_deps += [libnetcdfpp, netcdf_dep]
endif

if enable_hrit
# dist_noinst_HEADERS += \
#     xrit/xrit.h \
#     xrit/dataset.h \
#     xrit/rasterband.h
  msatdrv_sources += [
    'xrit/xrit.cpp',
    'xrit/dataset.cpp',
    'xrit/rasterband.cpp',
  ]
  msatdrv_link_with += [msat_hrit]
endif

libmsatdrv = static_library(
  'msatdrv', msatdrv_sources,
  include_directories: toplevel_inc,
  pic: true,
  dependencies: msatdrv_deps,
  link_with: msatdrv_link_with,
)

gdalplugin = shared_module(
  'gdal_Meteosatlib', ['msatgdalplugin.cpp'],
  name_prefix: '',
  link_whole: [libmsatdrv],
  install: true,
  install_dir: gdal_plugins_dir)
