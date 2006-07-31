#ifndef MSATLIB_HDF5_IMPORT_H
#define MSATLIB_HDF5_IMPORT_H

#include <conv/ImageData.h>
#include <memory>

std::auto_ptr<ImageData> ImportNetCDF(const std::string& filename);

bool isNetCDF(const std::string& filename);
std::auto_ptr<ImageImporter> createNetCDFImporter();

// vim:set sw=2:
#endif
