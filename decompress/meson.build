# project('publicdecompwt', 'cpp')

# publicdecompwt code is outside our control
pdwt_warning_control = [
  '-Wno-catch-value',
  '-Wno-dangling-else',
  '-Wno-deprecated-copy',
  '-Wno-deprecated-declarations',
  '-Wno-error=format',
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

pdwt_includes = include_directories([
  'PublicDecompWT/DISE/',
  'PublicDecompWT/COMP/Inc/',
  'PublicDecompWT/COMP/Src/',
  'PublicDecompWT/COMP/T4/Inc/',
  'PublicDecompWT/COMP/JPEG/Inc/',
  'PublicDecompWT/COMP/WT/Inc/',
])

pdwt_sources = [
  'PublicDecompWT/DISE/CxRITFile.cpp',
  'PublicDecompWT/DISE/CxRITFileHeaderRecords.cpp',
  'PublicDecompWT/DISE/CxRITAnnotation.cpp',
  'PublicDecompWT/DISE/ErrorHandling.cpp',
  'PublicDecompWT/DISE/TimeSpan.cpp',
  'PublicDecompWT/DISE/UTCTime.cpp',
  'PublicDecompWT/COMP/Src/CBuffer.cpp',
  'PublicDecompWT/COMP/Src/CImage.cpp',
  'PublicDecompWT/COMP/Src/CQualityMeasures.cpp',
  'PublicDecompWT/COMP/T4/Src/CBitBuffer.cpp',
  'PublicDecompWT/COMP/T4/Src/CompressT4.cpp',
  'PublicDecompWT/COMP/T4/Src/T4Coder.cpp',
  'PublicDecompWT/COMP/T4/Src/T4Codes.cpp',
  'PublicDecompWT/COMP/T4/Src/T4Decoder.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CHOptim.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CHcodec.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CHufftables.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CJBlock.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CJPEGDecoder.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CJPEGLossLessCoder.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CJPEGLossyCoder.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CQuantizationTable.cpp',
  'PublicDecompWT/COMP/JPEG/Src/CompressJPEG.cpp',
  'PublicDecompWT/COMP/WT/Src/CACCoder.cpp',
  'PublicDecompWT/COMP/WT/Src/CACDecoder.cpp',
  'PublicDecompWT/COMP/WT/Src/CACModel.cpp',
  'PublicDecompWT/COMP/WT/Src/CVLCCoder.cpp',
  'PublicDecompWT/COMP/WT/Src/CVLCDecoder.cpp',
  'PublicDecompWT/COMP/WT/Src/CWBlock.cpp',
  'PublicDecompWT/COMP/WT/Src/CWTCoder.cpp',
  'PublicDecompWT/COMP/WT/Src/CWTDecoder.cpp',
  'PublicDecompWT/COMP/WT/Src/CompressWT.cpp',
]

libpublicdecompwt = static_library(
  'publicdecompwt', pdwt_sources,
  include_directories: pdwt_includes,
  pic: true,
  cpp_args: cpp.get_supported_arguments(pdwt_warning_control),
)

publicdecompwt_dep = declare_dependency(
  include_directories: pdwt_includes,
  link_whole: libpublicdecompwt)

