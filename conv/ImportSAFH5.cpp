#include "ImportSAFH5.h"
#include "SAFH5Utils.h"
#include <string>

#include <H5Cpp.h>
#include <sstream>
#include <iostream>
#include <stdexcept>

#include <math.h>

#include <conv/parameters.h>
#include <proj/Geos.h>

#include <conv/Image.tcc>

using namespace H5;
using namespace std;

namespace msat {

bool isSAFH5(const std::string& filename)
{
	try {
		if (access(filename.c_str(), F_OK) == 0)
			return H5File::isHdf5(filename);
		return false;
	} catch (FileIException& e) {
		e.printError(stderr);
		return false;
	}
}

template<typename Sample>
static ImageData* acquireImage(const DataSet& dataset)
{
	int cols = readIntAttribute(dataset, "N_COLS");
	int lines = readIntAttribute(dataset, "N_LINES");
	auto_ptr< ImageDataWithPixels<Sample> > res(new ImageDataWithPixels<Sample>(cols, lines));

  res->slope = readFloatAttribute(dataset, "SCALING_FACTOR");
  res->offset = readFloatAttribute(dataset, "OFFSET");

	DataSpace space = dataset.getSpace();
	int ndims = space.getSimpleExtentNdims();
	hsize_t *dims = new hsize_t[ndims];
	space.getSimpleExtentDims(dims);
	hsize_t size = 1;
	for (int i=0; i<ndims; ++i)
		size = size * dims[i];
	delete[] dims;
	if (size != res->columns * res->lines)
	{
		stringstream err;
		err << "Image declares " << res->columns * res->lines << " samples "
								 "but has " << size << " instead" << endl;
		throw std::runtime_error(err.str());
	}
	dataset.read(res->pixels, dataset.getDataType());

	// Compute real number of BPPs
	Sample max = res->pixels[0];
	for (size_t i = 1; i < res->columns * res->lines; ++i)
		if (res->pixels[i] > max)
			max = res->pixels[i];
	res->bpp = (int)ceil(log2(max + 1));

	return res.release();
}

auto_ptr<Image> ImportSAFH5(const H5::Group& group, const std::string& name)
{
	DataSet dataset = group.openDataSet(name);

	// Get the group name
	std::string groupName = readStringAttribute(group, "PRODUCT_NAME");
	// Trim trailing '_' characters
	while (groupName.size() > 0 && groupName[groupName.size() - 1] == '_')
		groupName.resize(groupName.size() - 1);

  // Get image dataset
  auto_ptr<Image> img(new Image);

	// Read group metadata

	// Get date and time
	std::string datetime = readStringAttribute(group, "IMAGE_ACQUISITION_TIME");
	// Split datetime to year, month, etc
	if (sscanf(datetime.c_str(), "%04d%02d%02d%02d%02d", &img->year, &img->month, &img->day, &img->hour, &img->minute) != 5)
	{
		stringstream err;
		err << "Unable to parse datetime " << datetime << endl;
		throw std::runtime_error(err.str());
	}

	// Get projection name
	string proj = readStringAttribute(group, "PROJECTION_NAME");
	if (proj.size() < 8)
		throw std::runtime_error("projection name '"+proj+"' is too short to contain subsatellite longitude");
	const char* s = proj.c_str() + 6;
	// skip initial zeros
	while (*(s+1) && *(s+1) == '0') ++s;
	double sublon;
	if (sscanf(s, "%lf", &sublon) != 1)
		throw std::runtime_error("cannot read subsatellite longitude from projection name '" + proj + "' at '" + s + "'");
	img->proj.reset(new proj::Geos(sublon, ORBIT_RADIUS));
	
	img->channel_id = readIntAttribute(group, "SPECTRAL_CHANNEL_ID");
	img->spacecraft_id = Image::spacecraftIDFromHRIT(readIntAttribute(group, "GP_SC_ID"));
	img->column_factor = readIntAttribute(group, "CFAC"); 
	img->line_factor = readIntAttribute(group, "LFAC");
	img->column_offset = readIntAttribute(group, "COFF");
	img->line_offset = readIntAttribute(group, "LOFF");


  // Read image metadata

  // Compute/invent the spectral channel id
	SAFChannelInfo* ci = SAFChannelByName(name);
	//img->channel_id = ci == NULL ? 0 : ci->channelID;

	// Read image data
  switch (dataset.getDataType().getSize())
  {
    case 1:
      img->setData(acquireImage<uint8_t>(dataset));
      break;
    case 2:
      img->setData(acquireImage<uint16_t>(dataset));
      break;
    case 4:
      img->setData(acquireImage<uint32_t>(dataset));
      break;
    default: {
      stringstream err;
      err << "Unsupported sample data size " << dataset.getDataType().getSize() << " in " << name << endl;
      throw std::runtime_error(err.str());
		}
  }
	img->data->scalesToInt = true;

	// Consistency checks

	// Check that slope, offset and bpp match the ones that we have in
	// Utils, otherwise warning that the conversion can be irreversible
	if (ci == NULL)
		cerr << "Warning: unknown channel informations for product " << name << endl;
	else {
		if (ci->slope != img->data->slope)
			cerr << "Warning: slope for image (" << img->data->slope << ") is different from the usual one (" << ci->slope << ")" << endl;
		if (ci->offset != img->data->offset)
			cerr << "Warning: offset for image (" << img->data->offset << ") is different from the usual one (" << ci->offset << ")" << endl;
		if (ci->bpp < img->data->bpp)
			cerr << "Warning: bpp for image (" << img->data->bpp << ") is more than the usual one (" << ci->bpp << ")" << endl;
	}

  return img;
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
			std::auto_ptr<Image> img = ImportSAFH5(group, name);
			if (shouldCrop())
				img->crop(cropX, cropY, cropWidth, cropHeight);
			output.processImage(img);
		}
	}
};

std::auto_ptr<ImageImporter> createSAFH5Importer(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new SAFH5ImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
