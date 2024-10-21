config = configure_file(output: 'config.h', configuration: conf_data,
  install_dir: get_option('includedir') / 'msat')

libmsat_deps = []
libmsat_link_with = []

# Common code
install_headers([
  'config.h',
  'iniparser.h',
  'Progress.h',
  'auto_arr_ptr.h',
  'facts.h',
  'utils/string.h',
  'utils/subprocess.h',
  'utils/sys.h',
  'utils/term.h',
  'utils/testrunner.h',
  'utils/tests.h',
], subdir: 'msat')

libmsat_sources = [
    config,
    'iniparser.c',
    'Progress.cpp',
    'auto_arr_ptr.cpp',
    'utils/string.cc',
    'utils/subprocess.cc',
    'utils/sys.cc',
    'utils/term.cc',
    'utils/testrunner.cc',
    'utils/tests.cc',
]

msat_base = static_library(
  'msat-base', ['facts.cpp'],
  include_directories: toplevel_inc,
  pic: true,
)
libmsat_link_with += [msat_base]

if gdal_dep.found()
  install_headers([
    'gdal/clean_cpl_error.h',
    'gdal/clean_gdal_priv.h',
    'gdal/const.h',
    'gdal/points.h',
    'gdal/dataset.h',
    'gdal/gdaltranslate.h',
  ], subdir: 'msat/gdal')

  libmsat_sources += [
      'gdal/dataset.cpp',
      'gdal/gdaltranslate.cpp',
  ]

  libmsat_deps += [gdal_dep]
endif

if enable_hri
  install_headers([
    'hri/HRI.h',
    'hri/HRI_geolocation.h',
    'hri/HRI_image.h',
    'hri/HRI_machine.h',
    'hri/HRI_subframe_identification.h',
    'hri/HRI_subframe_interpretation.h',
    'hri/HRI_subframe_keyslot.h',
    'hri/HRI_subframe_label.h',
  ], subdir: 'msat/hri')

  libmsat_sources += [
    'hri/HRI.cpp',
    'hri/HRI_image.cpp',
    'hri/HRI_machine.cpp',
    'hri/HRI_subframe_identification.cpp',
    'hri/HRI_subframe_interpretation.cpp',
    'hri/HRI_subframe_keyslot.cpp',
    'hri/HRI_subframe_label.cpp',
  ]

  libmsat_deps += [netcdf_dep]
endif

if enable_hrit
  install_headers([
    'hrit/MSG_channel.h',
    'hrit/MSG_compression.h',
    'hrit/MSG_data_CelestialEvents.h',
    'hrit/MSG_data_format.h',
    'hrit/MSG_data_GeometricProc.h',
    'hrit/MSG_data_GeometricQuality.h',
    'hrit/MSG_data_gts.h',
    'hrit/MSG_data.h',
    'hrit/MSG_data_ImageAcquisition.h',
    'hrit/MSG_data_ImageDescription.h',
    'hrit/MSG_data_image.h',
    'hrit/MSG_data_image_mpef.h',
    'hrit/MSG_data_ImageProdStats.h',
    'hrit/MSG_data_IMPFConfiguration.h',
    'hrit/MSG_data_key.h',
    'hrit/MSG_data_NavigExtrResult.h',
    'hrit/MSG_data_RadiometricProc.h',
    'hrit/MSG_data_RadiometricQuality.h',
    'hrit/MSG_data_SatelliteStatus.h',
    'hrit/MSG_data_SGS_header.h',
    'hrit/MSG_data_text.h',
    'hrit/MSG_data_TimelinComple.h',
    'hrit/MSG_filetype.h',
    'hrit/MSG_header_ancillary_text.h',
    'hrit/MSG_header_annotation.h',
    'hrit/MSG_header.h',
    'hrit/MSG_header_image_datafunc.h',
    'hrit/MSG_header_image_navig.h',
    'hrit/MSG_header_image_struct.h',
    'hrit/MSG_header_key.h',
    'hrit/MSG_header_segment_id.h',
    'hrit/MSG_header_segment_quality.h',
    'hrit/MSG_header_timestamp.h',
    'hrit/MSG_HRIT.h',
    'hrit/MSG_hrit_specdoc.h',
    'hrit/MSG_machine.h',
    'hrit/MSG_projection.h',
    'hrit/MSG_quality.h',
    'hrit/MSG_spacecraft.h',
    'hrit/MSG_time_cds.h',
  ], subdir: 'msat/hrit')

  install_headers([
    'xrit/dataaccess.h',
    'xrit/fileaccess.h',
  ], subdir: 'msat/xrit')

  msat_hrit_sources = [
    'hrit/MSG_channel.cpp',
    'hrit/MSG_compression.cpp',
    'hrit/MSG_data_CelestialEvents.cpp',
    'hrit/MSG_data.cpp',
    'hrit/MSG_data_format.cpp',
    'hrit/MSG_data_GeometricProc.cpp',
    'hrit/MSG_data_GeometricQuality.cpp',
    'hrit/MSG_data_gts.cpp',
    'hrit/MSG_data_ImageAcquisition.cpp',
    'hrit/MSG_data_image.cpp',
    'hrit/MSG_data_ImageDescription.cpp',
    'hrit/MSG_data_ImageProdStats.cpp',
    'hrit/MSG_data_IMPFConfiguration.cpp',
    'hrit/MSG_data_key.cpp',
    'hrit/MSG_data_NavigExtrResult.cpp',
    'hrit/MSG_data_RadiometricProc.cpp',
    'hrit/MSG_data_RadiometricQuality.cpp',
    'hrit/MSG_data_SatelliteStatus.cpp',
    'hrit/MSG_data_SGS_header.cpp',
    'hrit/MSG_data_text.cpp',
    'hrit/MSG_data_TimelinComple.cpp',
    'hrit/MSG_filetype.cpp',
    'hrit/MSG_header_ancillary_text.cpp',
    'hrit/MSG_header_annotation.cpp',
    'hrit/MSG_header.cpp',
    'hrit/MSG_header_image_datafunc.cpp',
    'hrit/MSG_header_image_navig.cpp',
    'hrit/MSG_header_image_struct.cpp',
    'hrit/MSG_header_key.cpp',
    'hrit/MSG_header_segment_id.cpp',
    'hrit/MSG_header_segment_quality.cpp',
    'hrit/MSG_header_timestamp.cpp',
    'hrit/MSG_HRIT.cpp',
    'hrit/MSG_machine.cpp',
    'hrit/MSG_quality.cpp',
    'hrit/MSG_spacecraft.cpp',
    'hrit/MSG_time_cds.cpp',
    'xrit/dataaccess.cpp',
    'xrit/fileaccess.cpp',
  ]

  # we use publicdecompwt code which is outside our control
  msat_hrit_warning_control = [
    '-Wno-catch-value',
    '-Wno-dangling-else',
    '-Wno-deprecated-copy',
    '-Wno-deprecated-declarations',
    '-Wno-format',
    '-Wno-format-extra-args',
    '-Wno-extra',
    '-Wno-logical-op',
    '-Wno-overloaded-virtual',
    '-Wno-reorder',
    '-Wno-sign-compare',
    '-Wno-suggest-override',
    '-Wno-switch',
    '-Wno-type-limits',
    '-Wno-unknown-pragmas',
    '-Wno-unused-value',
    '-Wno-unused-variable',
  ]

  msat_hrit = static_library(
    'msat-hrit', msat_hrit_sources,
    include_directories: toplevel_inc,
    pic: true,
    dependencies: [libpublicdecompwt_dep],
    cpp_args: cpp.get_supported_arguments(msat_hrit_warning_control),
    link_with: [msat_base],
  )
  libmsat_link_with += [msat_hrit]
endif

if enable_msg_native
  install_headers([
    'msg-native/MSG_native.h',
    'msg-native/MSG_native_packetheader.h',
    'msg-native/MSG_native_header.h',
    'msg-native/MSG_native_trailer.h',
    'msg-native/MSG_native_line.h',
  ], subdir: 'msat/msg-native')

  libmsat_sources += [
    'msg-native/MSG_native.cpp',
    'msg-native/MSG_native_packetheader.cpp',
    'msg-native/MSG_native_header.cpp',
    'msg-native/MSG_native_trailer.cpp',
    'msg-native/MSG_native_line.cpp',
  ]
endif

if enable_omtp_ids
  install_headers([
    'omtp-ids/Constants.hh',
    'omtp-ids/FileHeader.hh',
    'omtp-ids/LineHeader.hh',
    'omtp-ids/OpenMTP-IDS.hh',
    'omtp-ids/RecordHeader.hh',
    'omtp-ids/Record.hh',
    'omtp-ids/ScanLine.hh',
  ], subdir: 'msat/omtp-ids')

# noinst_HEADERS += \
#     omtp-ids/ByteSex.hh \
#     omtp-ids/sysdep.h

  libmsat_sources += [
    'omtp-ids/ByteSex.cc',
    'omtp-ids/FileHeader.cc',
    'omtp-ids/LineHeader.cc',
    'omtp-ids/OpenMTP-IDS.cc',
    'omtp-ids/Record.cc',
    'omtp-ids/RecordHeader.cc',
    'omtp-ids/ScanLine.cc',
  ]
endif

if enable_openmtp
  install_headers([
    'openmtp/OpenMTP_ascii_header.h',
    'openmtp/OpenMTP_binary_header.h',
    'openmtp/OpenMTP.h',
    'openmtp/OpenMTP_image.h',
    'openmtp/OpenMTP_image_line.h',
    'openmtp/OpenMTP_machine.h',
  ], subdir: 'msat/openmtp')

  libmsat_sources += [
    'openmtp/OpenMTP_ascii_header.cpp',
    'openmtp/OpenMTP_binary_header.cpp',
    'openmtp/OpenMTP.cpp',
    'openmtp/OpenMTP_image.cpp',
    'openmtp/OpenMTP_image_line.cpp',
    'openmtp/OpenMTP_machine.cpp',
  ]
endif

if enable_thornsds_db1
  install_headers([
    'thornsds_db1/thornsds_db1.h',
  ], subdir: 'msat/thornsds_db1')

  libmsat_sources += [                              
    'thornsds_db1/thornsds_db1.cpp',
  ]
endif

libmsat = both_libraries('msat',
        libmsat_sources,
        version: libmsat_so_version,
        install: true,
        include_directories: toplevel_inc,
        dependencies: libmsat_deps,
        link_whole: libmsat_link_with)
