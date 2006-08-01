#ifndef MSATLIB_NETCDF_IMPORT_H
#define MSATLIB_NETCDF_IMPORT_H

#include <conv/ImageData.h>
#include <memory>

bool isNetCDF(const std::string& filename);
std::auto_ptr<ImageImporter> createNetCDFImporter(const std::string& filename);

// vim:set sw=2:
#endif
