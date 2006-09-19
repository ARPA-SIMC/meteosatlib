#ifndef MSATLIB_XRIT_IMPORT_H
#define MSATLIB_XRIT_IMPORT_H

#include <conv/ImageData.h>
#include <memory>

struct XRITImportOptions
{
  std::string directory;
  std::string resolution;
  std::string productid1;
  std::string productid2;
  std::string timing;
  bool subarea;
  int AreaLinStart, AreaNlin, AreaPixStart, AreaNpix;

  XRITImportOptions() :
    directory("."), subarea(false) {}

  void ensureComplete();

  std::string prologueFile();
  std::vector<std::string> segmentFiles();
}

std::auto_ptr<ImageImporter> createXRITImporter(const XRITImportOptions& opts);

// vim:set sw=2:
#endif
