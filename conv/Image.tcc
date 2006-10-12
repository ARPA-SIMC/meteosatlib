#ifndef IMAGE_DATA_TCC
#define IMAGE_DATA_TCC

#include "Image.h"
#include <stdexcept>
#include <math.h>

namespace msat {

template<typename EL>
int ImageDataWithPixels<EL>::scaledToInt(int column, int line) const
{
	if (!this->scalesToInt)
		throw std::runtime_error("Image samples cannot be scaled to int");
	return (int)this->pixels[line * this->columns + column];
}

template<typename EL>
void ImageDataWithPixels<EL>::crop(int x, int y, int width, int height)
{
	using namespace std;
	// Consistency checks
	if (x < 0  || x > columns - width)
		throw std::runtime_error("Area to crop does not fit horizontally inside image data");
	if (y < 0  || y > lines - height)
		throw std::runtime_error("Area to crop does not fit vertically inside image data");

	size_t newsize = width * height;
	Sample *newpix = new Sample[newsize];
	for (int i = 0; i < height; ++i)
		memcpy(newpix + i * width, pixels + (y + i) * columns + x, width * sizeof(Sample));
	delete [ ] pixels;
	pixels = newpix;
	lines = height;
	columns = width;

	// Record the change of the starting point
	//column_offset += x;
	//line_offset += y;
}

template<typename EL>
int ImageDataWithPixelsPrescaled<EL>::scaledToInt(int column, int line) const
{
	if (!this->scalesToInt)
		throw std::runtime_error("Image samples cannot be scaled to int");
	return (int)round((this->pixels[line * this->columns + column] - this->offset) / this->slope);
}

}

// vim:set ts=2 sw=2:
#endif
