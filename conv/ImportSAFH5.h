#ifndef MSATLIB_HDF5_IMPORT_H
#define MSATLIB_HDF5_IMPORT_H

#include <conv/ImageData.h>
#include <memory>

namespace H5 {
class Group;
class DataSet;
}

std::auto_ptr<ImageData> ImportSAFH5(const H5::Group& group, const std::string& name);

std::auto_ptr<ImageImporter> createSAFH5Importer();

// vim:set sw=2:
#endif
