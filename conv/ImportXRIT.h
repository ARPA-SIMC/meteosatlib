#ifndef MSATLIB_XRIT_IMPORT_H
#define MSATLIB_XRIT_IMPORT_H

#include <conv/Image.h>
#include <memory>
#include <vector>

namespace msat {

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
  XRITImportOptions(const std::string& filename);

  void ensureComplete() const;

  std::string prologueFile() const;
  std::vector<std::string> segmentFiles() const;
};

bool isXRIT(const std::string& filename);
std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts);
std::auto_ptr<ImageImporter> createXRITImporter(const std::string& filename);

}

// vim:set sw=2:
#endif
