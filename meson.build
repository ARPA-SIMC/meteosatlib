project('meteosatlib', ['cpp', 'c'], version: '1.36', license : 'GPL-2.0-or-later', default_options: ['warning_level=3', 'cpp_std=c++17'])
libmsat_so_version = '1.0.0'

# TODO: use warning_level=everything from meson 1.0

cpp = meson.get_compiler('cpp')
c = meson.get_compiler('c')

warning_control = [
  # Turn some warning classes to errors
  '-Werror=format',
  # '-Werror=suggest-override',
  '-Werror=deprecated-copy-dtor',
  '-Werror=missing-declarations',
  '-Werror=overloaded-virtual',
  # '-Werror=cast-qual',
  '-Werror=duplicated-branches',
  '-Werror=logical-op',
  '-Werror=catch-value',
  '-Werror=conditionally-supported',
  '-Werror=noexcept',
  '-Werror=c++23-extensions',
  '-Werror=dangling-else',
  # '-Werror=suggest-attribute=format',
  # '-Werror=deprecated-declarations',

  '-Wno-padded',
  '-Wno-abi-tag',
  '-Wno-unused-macros',
  '-Wno-missing-include-dirs',
  '-Wno-multiple-inheritance',
  '-Wno-sign-promo',
  '-Wswitch',
  '-Wno-switch-enum',
  '-Wno-effc++',

  # TODO: remove the following ones over time
  '-Wno-shadow',
  '-Wno-zero-as-null-pointer-constant',
  '-Wno-mismatched-tags',
  '-Wno-unused-const-variable',
  '-Wno-redundant-tags',
  '-Wno-useless-cast',
  '-Wno-switch-default',
  '-Wno-old-style-cast',
  '-Wno-unused-parameter',

  # These ones can be activated from time to time
  '-Wno-float-equal',
  '-Wno-suggest-attribute=noreturn',
  '-Wno-format-truncation',
  '-Wno-arith-conversion',
  '-Wno-conversion',
]

add_project_arguments(
  cpp.get_supported_arguments(warning_control),
  language : 'cpp')

conf_data = configuration_data()
conf_data.set_quoted('PACKAGE_VERSION', meson.project_version())
conf_data.set_quoted('PACKAGE_STRING', meson.project_name() + ' ' + meson.project_version())

toplevel_inc = include_directories('.')

default_institution = get_option('institution')
enable_hri = get_option('hri')
enable_hrit = get_option('hrit')
enable_thornsds_db1 = get_option('thornsds_db1')
enable_msg_native = get_option('msg_native')
enable_omtp_ids = get_option('omtp_ids')
enable_openmtp = get_option('openmtp')

# Dependencies

compiler = meson.get_compiler('cpp')

conf_data.set_quoted('MSAT_DEFAULT_INSTITUTION', default_institution)
conf_data.set('HAVE_STDINT_H', compiler.has_header('stdint.h'))

# PublicDecompWT
if enable_hrit
  if get_option('system_pdwt')
    publicdecompwt_dep = dependency('libpublicdecompwt')
    publicdecompwt_type = 'system'
  else
    subdir('decompress')
    publicdecompwt_type = 'bundled'
  endif
else
  publicdecompwt_type = 'none'
endif

magickpp_dep = dependency('Magick++', required: false)
conf_data.set('HAVE_MAGICKPP', magickpp_dep.found())
conf_data.set('MSAT_HAVE_MAGICKPP', magickpp_dep.found())

netcdf_dep = dependency('netcdf', language: 'cpp', required: false)
conf_data.set('HAVE_NETCDF', netcdf_dep.found())
conf_data.set('MSAT_HAVE_NETCDF', netcdf_dep.found())
if not netcdf_dep.found()
  enable_hri = false
  enable_hrit = false
  enable_thornsds_db1 = false
endif

# Legacy NetCDF C++ library
netcdfpp_h = cpp.check_header('netcdfcpp.h', required: false)
libnetcdfpp = cpp.find_library('netcdf_c++', required: false)

gdal_dep = dependency('', required: false)
gdal_config = find_program('gdal-config', required: false)
if gdal_config.found()
  # gdal_includes = run_command(gdal_config, '--includes').stdout().split()
    gdal_cflags = run_command(gdal_config, '--cflags').stdout().split()
    gdal_libs = run_command(gdal_config, '--libs').stdout().split()
    gdal_version = run_command(gdal_config, '--version').stdout().strip()
    gdal_dep = declare_dependency(
      # include_directories: gdal_includes,
        compile_args: gdal_cflags,
        link_args: gdal_libs,
        version: gdal_version,
    )

    # Build the name of the GDAL plugin directory
    gdal_version_parts = gdal_version.split('.')
    gdal_plugins_dir_name_parts = []
    foreach i: range(2)
      gdal_plugins_dir_name_parts += gdal_version_parts[i]
    endforeach
    gdal_plugins_dir = get_option('libdir') / 'gdalplugins' / '.'.join(gdal_plugins_dir_name_parts)
endif

conf_data.set('HAVE_GDAL', gdal_dep.found())
conf_data.set('MSAT_HAVE_GDAL', gdal_dep.found())

eccodes_dep = dependency('eccodes', required: false)
if not eccodes_dep.found()
    eccodes_dep = dependency('grib_api', required: false)
endif
conf_data.set('HAVE_GRIBAPI', eccodes_dep.found())


conf_data.set('HAVE_HRIT', enable_hrit)
conf_data.set('MSAT_HAVE_HRIT', enable_hrit)

help2man = find_program('help2man', required: false)

# $PACKAGE_NAME-$PACKAGE_VERSION configuration:
message('Optional feature list:')
message('default institution:', default_institution)
message('publicdecompwt library:', publicdecompwt_type)
message('hri:', enable_hri)
message('hrit:', enable_hrit)
message('msg-native:', enable_msg_native)
message('omtp-ids:', enable_omtp_ids)
message('openmtp:', enable_openmtp)
message('thornsds_db1:', enable_thornsds_db1)
# AS_HELP_STRING([magick++:], [$have_libmagick])
# AS_HELP_STRING([GDAL:], [$have_gdal])
# AS_HELP_STRING([grib_api:], [$have_gribapi])


# Generate config.h
configure_file(output: 'config.h', configuration: conf_data)

# Generate the builddir's version of run-local
run_local_cfg = configure_file(output: 'run-local', input: 'run-local.in', configuration: {
    'top_srcdir': meson.source_root(),
    'top_builddir': meson.build_root(),
})
# Just using the configure_file object in a custom_target command gives:
#   'AttributeError: 'File' object has no attribute 'replace'
# Using find_program on the resulting file works around that
# See https://github.com/mesonbuild/meson/issues/8039
run_local = find_program(run_local_cfg)

subdir('msat')
if gdal_dep.found()
  subdir('gdal')
endif
subdir('tools')
subdir('tests')

# Generate pkg-config metadata
# if test x"$have_libnetcdf" = x"yes"; then
# 	# FIXME: there seems to be no way to tell users of the .pc.in that they
# 	# need the netcdf libraries, as netcdf does not package .pc files.  Are
# 	# there better ways of handling this?
# 	PC_MSAT_LIBS="$PC_MSAT_LIBS -lnetcdf_c++ -lnetcdf"
# fi
# if test x"$have_libmagick" = x"yes"; then
# 	PC_MSAT_REQS="$PC_MSAT_REQS Magick++"
# fi
pkg = import('pkgconfig')
pkg.generate(libmsat,
    name: 'libmsat',
    description: 'Handle Meteosat images in various formats, directly or via GDAL',
    filebase: 'libmsat',
)
