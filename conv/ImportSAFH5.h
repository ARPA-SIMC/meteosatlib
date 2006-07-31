#ifndef MSATLIB_HDF5_IMPORT_H
#define MSATLIB_HDF5_IMPORT_H

#include <conv/ImageData.h>
#include <memory>

namespace H5 {
class Group;
class DataSet;
}

std::auto_ptr<ImageData> ImportSAFH5(const H5::Group& group, const std::string& name);

bool isSAFH5(const std::string& filename);
std::auto_ptr<ImageImporter> createSAFH5Importer(const std::string& filename);

// vim:set sw=2:
#endif
