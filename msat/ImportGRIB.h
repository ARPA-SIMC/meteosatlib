#ifndef MSATLIB_GRIB_IMPORT_H
#define MSATLIB_GRIB_IMPORT_H

#include <msat/Image.h>
#include <memory>

namespace msat {

bool isGrib(const std::string& filename);
std::auto_ptr<ImageImporter> createGribImporter(const std::string& filename);

}

// vim:set sw=2:
#endif
