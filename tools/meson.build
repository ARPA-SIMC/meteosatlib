config = configure_file(output: 'config.h', configuration: conf_data)

msat_sources = ['msat.cpp']
msat_deps = []
msat_link_with = [msat_base, libmsat]

if magickpp_dep.found()
  msat_sources += ['image.cpp']
  msat_deps += [magickpp_dep]
endif

if gdal_dep.found()
  msat_deps += [gdal_dep]
endif

msat = executable('msat', [config] + msat_sources,
    include_directories: toplevel_inc,
    dependencies: msat_deps,
    link_with: [msat_base, libmsat],
    install: true)

msat_view = install_data('msat-view', install_dir: get_option('bindir'))


# HRI
if enable_hri and libnetcdfpp.found()
# bin_PROGRAMS += hri/HRI2NetCDF
  hri2netcdf = executable('HRI2NetCDF', [config, 'hri/HRI2NetCDF.cpp'],
    include_directories: toplevel_inc,
    dependencies: [netcdf_dep, libnetcdfpp],
    link_with: [msat_base, libmsat],
    install: true)
endif

# HRIT
if enable_hrit
  # bin_PROGRAMS += hrit/xritdump
  xritdump = executable('xritdump', [config, 'hrit/xritdump.cpp'],
    include_directories: toplevel_inc,
    link_with: [msat_base, libmsat, msat_hrit])

  if netcdf_dep.found() and libnetcdfpp.found() 
    # bin_PROGRAMS += hrit/XRIT2NetCDF
    xrit2netcdf = executable('XRIT2NetCDF', [config, 'hrit/XRIT2NetCDF.cpp'],
      include_directories: toplevel_inc,
      dependencies: [netcdf_dep, libnetcdfpp],
      link_with: [msat_base, libmsat, msat_hrit],
      install: true)
  endif

  if magickpp_dep.found()
    # bin_PROGRAMS += hrit/XRIT2Image
    xrit2image = executable('XRIT2Image', [config, 'hrit/XRIT2Image.cpp'],
      include_directories: toplevel_inc,
      dependencies: [magickpp_dep],
      link_with: [msat_base, libmsat, msat_hrit],
      install: true)
  endif
endif

# MSG_NATIVE
if enable_msg_native
  if enable_hrit
    # bin_PROGRAMS += msg-native/nativedump
    nativedump = executable('nativedump', [config, 'msg-native/nativedump.cpp'],
      include_directories: toplevel_inc,
      link_with: [msat_base, libmsat])

    if magickpp_dep.found()
      # bin_PROGRAMS += msg-native/native2Image
      native2image = executable('native2Image', [config, 'msg-native/native2Image.cpp'],
        include_directories: toplevel_inc,
        dependencies: [magickpp_dep],
        link_with: [msat_base, libmsat],
      install: true)
    endif
  endif
endif

# OMTP IDS
if enable_omtp_ids
  # bin_PROGRAMS += \
  #     omtp-ids/OpenMTP-IDS_debug \
  #     omtp-ids/OpenMTP-IDS_sector \
  #     omtp-ids/OpenMTP-IDS_to_pgm \
  #     omtp-ids/OpenMTP-IDS_write

  openmpt_ids_debug = executable('OpenMTP-IDS_debug', ['omtp-ids/OpenMTP-IDS_debug.cc'],
        include_directories: toplevel_inc,
        link_with: [msat_base, libmsat],
        install: true)

  openmpt_ids_sector = executable('OpenMTP-IDS_sector', ['omtp-ids/OpenMTP-IDS_sector.cc'],
        include_directories: toplevel_inc,
        link_with: [msat_base, libmsat],
        install: true)

  openmpt_ids_to_pgm = executable('OpenMTP-IDS_to_pgm', ['omtp-ids/OpenMTP-IDS_to_pgm.cc'],
        include_directories: toplevel_inc,
        link_with: [msat_base, libmsat],
        install: true)

  openmpt_ids_write = executable('OpenMTP-IDS_write', ['omtp-ids/OpenMTP-IDS_write.cc'],
        include_directories: toplevel_inc,
        link_with: [msat_base, libmsat],
        install: true)
endif

# openmtp
if enable_openmtp
  # bin_PROGRAMS += openmtp/OpenMTP_info
  openmcp_info = executable('OpenMTP_info', [config, 'openmtp/OpenMTP_info.cc'],
        include_directories: toplevel_inc,
        link_with: [msat_base, libmsat],
        install: true)

  if netcdf_dep.found()
    openmcp_to_netcdf = executable('OpenMTP_to_NetCDF', [config, 'openmtp/OpenMTP_to_NetCDF.cpp'],
          include_directories: toplevel_inc,
          dependencies: [netcdf_dep, libnetcdfpp],
          link_with: [msat_base, libmsat],
          install: true)
  endif
endif

# thornsds_db1
if enable_thornsds_db1
  # bin_PROGRAMS += thornsds_db1/db1_to_netcdf
  db1_to_netcdf = executable('db1_to_netcdf', [config, 'thornsds_db1/db1_to_netcdf.cpp'],
          include_directories: toplevel_inc,
          dependencies: [netcdf_dep, libnetcdfpp],
          link_with: [msat_base, libmsat],
          install: true)

  if magickpp_dep.found()
    # bin_PROGRAMS += thornsds_db1/db1_to_image
    db1_to_image = executable('db1_to_image', [config, 'thornsds_db1/db1_to_image.cpp'],
            include_directories: toplevel_inc,
            dependencies: [netcdf_dep, magickpp_dep],
            link_with: [msat_base, libmsat],
            install: true)
  endif
endif


# Manpages

prog_manpages = [
  [msat, 'Manipulate satellite images'],
]

script_manpages = [
  ['msat-view', 'Interactively display satellite images'],
]

foreach manpage: prog_manpages
    custom_target(manpage[0].name() + '.1',
                  input: [manpage[0]],
                  output: manpage[0].name() + '.1',
                  command: [run_local, help2man, '--name=' + manpage[1], '--section=1', '--no-info', '--version-string=' + meson.project_version(), '@INPUT0@'],
                  depends: [manpage[0]],
                  capture: true,
                  install: true,
                  install_dir: get_option('mandir') / 'man1')
endforeach

foreach manpage: script_manpages
    custom_target(manpage[0] + '.1',
                  input: [manpage[0]],
                  output: manpage[0] + '.1',
                  command: [run_local, help2man, '--name=' + manpage[1], '--section=1', '--no-info', '--version-string=' + meson.project_version(), '@INPUT0@'],
                  capture: true,
                  install: true,
                  install_dir: get_option('mandir') / 'man1')
endforeach

# TODO
# man_MANS = msat.1 msat-view.1
# 
# msat.1: msat
# 	help2man -n "Manipulate satellite images" --no-info -o $@ --version-string=@PACKAGE_VERSION@ ./msat
# 
# msat-view.1: msat-view
# 	help2man -n "Interactively display satellite images" --no-info -o $@ --version-string=@PACKAGE_VERSION@ ./msat-view
# 
# EXTRA_DIST = image.h msat2gdal

