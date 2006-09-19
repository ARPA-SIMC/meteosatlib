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

  void ensureComplete() const;

  std::string prologueFile() const;
  std::vector<std::string> segmentFiles() const;
};

std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts);

}

// vim:set sw=2:
#endif
