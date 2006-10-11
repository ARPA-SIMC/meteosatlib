#ifndef MSATLIB_NETCDF24_IMPORT_H
#define MSATLIB_NETCDF24_IMPORT_H

#include <conv/Image.h>
#include <memory>

namespace msat {

bool isNetCDF24(const std::string& filename);
std::auto_ptr<ImageImporter> createNetCDF24Importer(const std::string& filename);

}

// vim:set sw=2:
#endif
