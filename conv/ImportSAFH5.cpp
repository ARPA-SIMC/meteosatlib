#include "ImportSAFH5.h"
#include <string>

#include <hdf5/Utils.h>

#include <H5Cpp.h>
#include <sstream>
#include <iostream>
#include <stdexcept>

using namespace H5;
using namespace std;

struct H5Image : public ImageData
{
  virtual void acquire(DataSet& dataset) = 0;
};

// Container for image data, which can be used with different sample sizes
template<typename EL>
struct H5ImageData : public H5Image
{
public:
  typedef EL Item;
  Item* pixels;

  H5ImageData() : pixels(0) { bpp = sizeof(Item) * 8; }
  ~H5ImageData()
  {
    if (pixels)
      delete[] pixels;
  }

  virtual int unscaled(int column, int line) const
  {
      return pixels[line * columns + column];
  }

	// This is not const, but it's used for once-only initialization of mutable
	// m_pixels, and needed to allow floats() to be const
  void acquire(DataSet& dataset)
  {
		if (pixels)
			delete[] pixels;
    pixels = new Item[columns * lines];

    DataSpace space = dataset.getSpace();
    int ndims = space.getSimpleExtentNdims();
    hsize_t *dims = new hsize_t[ndims];
    space.getSimpleExtentDims(dims);
    hsize_t size = 1;
    for (int i=0; i<ndims; ++i)
			size = size * dims[i];
    delete[] dims;
    if (size != columns * lines)
    {
			stringstream err;
			err << "Image declares " << columns * lines << " samples "
									 "but has " << size << " instead" << endl;
      throw std::runtime_error(err.str());
    }
    dataset.read(pixels, dataset.getDataType());
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

// vim:set ts=2 sw=2:
