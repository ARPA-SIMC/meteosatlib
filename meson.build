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

# TODO: configure options that we might not need anymore
# dnl Compile time options.

msat_default_institution = 'ARPA-SIM'

enable_hri = true
# AC_ARG_ENABLE([hri],
#     [AS_HELP_STRING(
#         [--disable-hri],
#         [disable hri support])],
#     [],
#     [enable_hri="yes"])
#
enable_hrit = true
# AC_ARG_ENABLE([hrit],
#     [AS_HELP_STRING(
#         [--disable-hrit],
#         [disable hrit support])],
#     [],
#     [enable_hrit="yes"])
#
enable_thornsds_db1 = true
# AC_ARG_ENABLE([thornsds_db1],
#     [AS_HELP_STRING(
#         [--disable-thornsds_db1],
#         [disable thornsds_db1 support])],
#     [],
#     [enable_thornsds_db1="yes"])

enable_msg_native = true
# AC_ARG_ENABLE([msg-native],
#     [AS_HELP_STRING(
#         [--disable-msg-native],
#         [disable msg-native support])],
#     [],
#     [enable_msg_native="yes"])

enable_omtp_ids = true
# AC_ARG_ENABLE([omtp-ids],
#     [AS_HELP_STRING(
#         [--disable-omtp-ids],
#         [disable omtp-ids support])],
#     [],
#     [enable_omtp_ids="yes"])

enable_openmtp = true
# AC_ARG_ENABLE([openmtp],
#     [AS_HELP_STRING(
#         [--disable-openmtp],
#         [disable openmtp support])],
#     [],
#     [enable_openmtp="yes"])

# TODO
# AC_ARG_WITH([system-pdwt],
#     [AS_HELP_STRING([--with-system-pdwt],
# 	[use PublicDecomptWt installed in the system instead of bundled one])],
#     [],
#     [with_system_pdwt="no"])

# Dependencies

compiler = meson.get_compiler('cpp')

conf_data.set_quoted('MSAT_DEFAULT_INSTITUTION', msat_default_institution)
conf_data.set('HAVE_STDINT_H', compiler.has_header('stdint.h'))

# PublicDecompWT
#libpublicdecompwt_proj = subproject('decompress')
#libpublicdecompwt_dep = libpublicdecompwt_proj.get_variable('libpublicdecompwt__dep')
# TODO
# if test $with_system_pdwt != no; then
#     PKG_CHECK_MODULES(PDWT, [libpublicdecompwt], [
#         AC_SUBST(PDWT_CFLAGS)
#         AC_SUBST(PDWT_LIBS)
# 	have_pdwt=yes
#     ], [
#         have_pdwt=no
#         enable_hrit=no
#         AC_MSG_WARN("*** system version of PublicDecompWT not found. You can build without --with-system-pdwt")
#     ])
# 
#     AM_CONDITIONAL([BUNDLED_PDWT], [false])
# else
#     AM_CONDITIONAL([BUNDLED_PDWT], [true])
#     have_pdwt=bundled
# fi

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
endif

conf_data.set('HAVE_GDAL', gdal_dep.found())
conf_data.set('MSAT_HAVE_GDAL', gdal_dep.found())
# TODO
#     echo "GPD $GDAL_PLUGIN_DIRNAME"
#     if test -z "$GDAL_PLUGIN_DIRNAME"; then
#         GDAL_PLUGIN_DIRNAME=gdalplugins/$(gdal-config --version | sed -re ['s/([0-9]+\.[0-9]+).+/\1/'])
#     fi
#     echo "GPD $GDAL_PLUGIN_DIRNAME"
#     AC_MSG_NOTICE([GDAL plugin directory is $GDAL_PLUGIN_DIRNAME])
#     AC_SUBST(GDAL_PLUGIN_DIRNAME)

eccodes_dep = dependency('eccodes', required: false)
if not eccodes_dep.found()
    eccodes_dep = dependency('grib_api', required: false)
endif
conf_data.set('HAVE_GRIBAPI', eccodes_dep.found())


if enable_hrit
# if BUNDLED_PDWT
subdir('decompress')
# endif
endif

conf_data.set('HAVE_HRIT', enable_hrit)
conf_data.set('MSAT_HAVE_HRIT', enable_hrit)

# TODO: conditional stuff we might change to be true only if dependencies are found
# dnl Conditional stuff.
# 
# AM_CONDITIONAL([HRI], [test x"$enable_hri" = x"yes"])
# AM_CONDITIONAL([HRIT], [test x"$enable_hrit" = x"yes"])
# AM_CONDITIONAL([MSG_NATIVE], [test x"$enable_msg_native" = x"yes"])
# AM_CONDITIONAL([OMTP_IDS], [test x"$enable_omtp_ids" = x"yes"])
# AM_CONDITIONAL([OPENMTP], [test x"$enable_openmtp" = x"yes"])
# AM_CONDITIONAL([THORNSDS_DB1], [test x"$enable_thornsds_db1" = x"yes"])
# AM_CONDITIONAL([HAVE_NETCDF], [test x"$have_libnetcdf" = x"yes"])
# AM_CONDITIONAL([HAVE_MAGICK], [test x"$have_libmagick" = x"yes"])
# AM_CONDITIONAL([HAVE_GRIBAPI], [test x"$have_gribapi" = x"yes"])
# AM_CONDITIONAL([HAVE_GDAL], [test x"$have_gdal" = x"yes"])
# AM_CONDITIONAL([OLD_GRIB], [test x"$enable_grib" = x"yes"])
# 
# AC_SUBST(enable_hri)
# AC_SUBST(enable_hrit)
# AC_SUBST(enable_msg_native)
# AC_SUBST(enable_omtp_ids)
# AC_SUBST(enable_openmtp)
# AC_SUBST(enable_thornsds_db1)
# AC_SUBST(enable_grib)
# 
# dnl --------------------------------------------------------------------
# dnl Substitution variables for the .pc.in files
# 
# if test x"$have_libnetcdf" = x"yes"; then
# 	# FIXME: there seems to be no way to tell users of the .pc.in that they
# 	# need the netcdf libraries, as netcdf does not package .pc files.  Are
# 	# there better ways of handling this?
# 	PC_MSAT_LIBS="$PC_MSAT_LIBS -lnetcdf_c++ -lnetcdf"
# fi
# 
# if test x"$have_libmagick" = x"yes"; then
# 	PC_MSAT_REQS="$PC_MSAT_REQS Magick++"
# fi
# 
# AC_SUBST(PC_MSAT_REQS)
# AC_SUBST(PC_MSAT_LIBS)
# 
# dnl Default institution information required when generating NetCDF
# if test "x$MSAT_DEFAULT_INSTITUTION" == "x"; then
#     MSAT_DEFAULT_INSTITUTION="ARPA-SIM"
# fi
# AC_DEFINE_UNQUOTED([MSAT_DEFAULT_INSTITUTION], ["$MSAT_DEFAULT_INSTITUTION"], [Default institution name])
# AC_MSG_NOTICE([Default institution is $MSAT_DEFAULT_INSTITUTION])
# 
# AC_CHECK_PROG([have_help2man], [help2man], [yes], [no])
# if test "x$have_help2man" = xno
# then
#     AC_MSG_ERROR([help2man not found])
# fi
# 
# 
# dnl --------------------------------------------------------------------
# dnl Create output files.
# 
# AC_CONFIG_FILES([
#     Makefile
#     decompress/Makefile
#     msat/Makefile
#     gdal/Makefile
#     tools/Makefile
#     examples/Makefile
#     tests/Makefile
#     libmsat.pc
# ])
# AC_OUTPUT

# $PACKAGE_NAME-$PACKAGE_VERSION configuration:
message('Optional feature list:')
message('default institution:', msat_default_institution)
message('hri:', enable_hri)
message('hrit:', enable_hrit)
message('msg-native:', enable_msg_native)
message('omtp-ids:', enable_omtp_ids)
message('openmtp:', enable_openmtp)
message('thornsds_db1:', enable_thornsds_db1)
# AS_HELP_STRING([publicdecompwt:], [$have_pdwt])
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

# TODO
# SUBDIRS =
subdir('msat')
if gdal_dep.found()
  subdir('gdal')
endif
subdir('tools')
# TODO subdir('examples')
subdir('tests')

# pkgconfigdir = $(libdir)/pkgconfig
# pkgconfig_DATA = libmsat.pc

# TODO Generate pkg-config metadata
# pkg = import('pkgconfig')
# pkg.generate(libmsat,
#     description: 'Handle Meteosat images in various formats, directly or via GDAL',
#     filebase: 'libmsat',
#     requires: libmsat_deps,
# )
