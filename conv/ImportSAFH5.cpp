#include "ImportSAFH5.h"
#include <string>

#include <hdf5/Utils.h>

#include <H5Cpp.h>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace H5;
using namespace std;

struct H5Image
{
  virtual void acquire(DataSet& dataset) = 0;
};

// Container for image data, which can be used with different sample sizes
template<typename Item>
struct H5ImageData : public ImageDataWithPixels<Item>, public H5Image
{
public:
	// This is not const, but it's used for once-only initialization of mutable
	// m_pixels, and needed to allow floats() to be const
  void acquire(DataSet& dataset)
  {
		if (this->pixels)
			delete[] this->pixels;
    this->pixels = new Item[this->columns * this->lines];

    DataSpace space = dataset.getSpace();
    int ndims = space.getSimpleExtentNdims();
    hsize_t *dims = new hsize_t[ndims];
    space.getSimpleExtentDims(dims);
    hsize_t size = 1;
    for (int i=0; i<ndims; ++i)
			size = size * dims[i];
    delete[] dims;
    if (size != this->columns * this->lines)
    {
			stringstream err;
			err << "Image declares " << this->columns * this->lines << " samples "
									 "but has " << size << " instead" << endl;
      throw std::runtime_error(err.str());
    }
    dataset.read(this->pixels, dataset.getDataType());
  }
};

auto_ptr<ImageData> ImportSAFH5(const H5::Group& group, const std::string& name)
{
	DataSet dataset = group.openDataSet(name);

	// Get the group name
	std::string groupName = readStringAttribute(group, "PRODUCT_NAME");
	// Trim trailing '_' characters
	while (groupName.size() > 0 && groupName[groupName.size() - 1] == '_')
		groupName.resize(groupName.size() - 1);

  // Get image dataset
  auto_ptr<ImageData> data;

	// Create the right ImageData
  switch (dataset.getDataType().getSize())
  {
    case 1:
      data.reset(new H5ImageData<uint8_t>());
      break;
    case 2:
      data.reset(new H5ImageData<uint16_t>());
      break;
    case 4:
      data.reset(new H5ImageData<uint32_t>());
      break;
    default: {
      stringstream err;
      err << "Unsupported sample data size " << dataset.getDataType().getSize() << " in " << name << endl;
      throw std::runtime_error(err.str());
		}
  }

	// Read group metadata

	// Get date and time
	std::string datetime = readStringAttribute(group, "IMAGE_ACQUISITION_TIME");
	// Split datetime to year, month, etc
	if (sscanf(datetime.c_str(), "%04d%02d%02d%02d%02d", &data->year, &data->month, &data->day, &data->hour, &data->minute) != 5)
	{
		stringstream err;
		err << "Unable to parse datetime " << datetime << endl;
		throw std::runtime_error(err.str());
	}

	// Get projection name
	data->projection = readStringAttribute(group, "PROJECTION_NAME");
	
	// Get channel ID
	data->channel_id = readIntAttribute(group, "SPECTRAL_CHANNEL_ID");
	
	// Get spacecraft ID
	data->spacecraft_id = readIntAttribute(group, "GP_SC_ID");
	
	// Get scale factor
	data->column_factor = readIntAttribute(group, "CFAC"); 
	data->line_factor = readIntAttribute(group, "LFAC");
	
	// Get offset
	data->column_offset = readIntAttribute(group, "COFF");
	data->line_offset = readIntAttribute(group, "LOFF");


  // Read image metadata

  // Compute/invent the spectral channel id
	SAFChannelInfo* ci = SAFChannelByName(name);
	data->channel_id = ci == NULL ? 0 : ci->channelID;

  data->slope = readFloatAttribute(dataset, "SCALING_FACTOR");
  data->offset = readFloatAttribute(dataset, "OFFSET");

	data->columns = readIntAttribute(dataset, "N_COLS");
	data->lines = readIntAttribute(dataset, "N_LINES");

	// Read image data
	dynamic_cast<H5Image*>(data.get())->acquire(dataset);

	// Consistency checks

	// Check that slope, offset and bpp match the ones that we have in
	// Utils, otherwise warning that the conversion can be irreversible
	if (ci == NULL)
		cerr << "Warning: unknown channel informations for product " << name << endl;
	else {
		if (ci->slope != data->slope)
			cerr << "Warning: slope for image (" << data->slope << ") is different from the usual one (" << ci->slope << ")" << endl;
		if (ci->offset != data->offset)
			cerr << "Warning: offset for image (" << data->offset << ") is different from the usual one (" << ci->offset << ")" << endl;
		if (ci->bpp != data->bpp)
			cerr << "Warning: bpp for image (" << data->bpp << ") is different from the usual one (" << ci->bpp << ")" << endl;
	}

  return data;
}


bool isSAFH5(const std::string& filename)
{
	return H5File::isHdf5(filename);
}

class SAFH5ImageImporter : public ImageImporter
{
	std::string filename;
	H5File HDF5_source;

public:
	SAFH5ImageImporter(const std::string& filename)
		: filename(filename), HDF5_source(filename, H5F_ACC_RDONLY) {}

	virtual void read(ImageConsumer& output)
	{
		Group group = HDF5_source.openGroup("/");

		// Iterate on all the images within
		for (hsize_t i = 0; i < group.getNumObjs(); ++i)
		{
			string name = group.getObjnameByIdx(i);
			DataSet dataset = group.openDataSet(name);
			string c = readStringAttribute(dataset, "CLASS");
			if (c != "IMAGE")
				continue;
			std::auto_ptr<ImageData> img = ImportSAFH5(group, name);
			output.processImage(*img);
		}
	}
};

std::auto_ptr<ImageImporter> createSAFH5Importer(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new SAFH5ImageImporter(filename));
}

// vim:set ts=2 sw=2: