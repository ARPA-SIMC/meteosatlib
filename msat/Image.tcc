#ifndef IMAGE_DATA_TCC
#define IMAGE_DATA_TCC

#include "Image.h"
#include <stdexcept>
#include <math.h>
#include <limits>
#include <cstring>

namespace msat {

template<typename EL>
ImageDataWithPixels<EL>::ImageDataWithPixels() : pixels(0)
{
	bpp = sizeof(Sample) * 8;
	if (std::numeric_limits<EL>::has_quiet_NaN)
		missing = std::numeric_limits<EL>::quiet_NaN();
	else if (std::numeric_limits<EL>::is_signed)
		missing = std::numeric_limits<EL>::min();
	else
		missing = std::numeric_limits<EL>::max();
}

template<typename EL>
int ImageDataWithPixels<EL>::scaledToInt(int column, int line) const
{
	if (!this->scalesToInt)
		throw std::runtime_error("Image samples cannot be scaled to int");
	EL s = this->pixels[line * this->columns + column];
	if (s == missing)
		return unscaledMissingValue();
	else
		return (int)s;
}

template<typename EL>
int ImageDataWithPixels<EL>::unscaledMissingValue() const
{
	if (!this->scalesToInt)
		throw std::runtime_error("Image samples cannot be scaled to int");
	return (int)missing;
}

template<typename EL>
void ImageDataWithPixels<EL>::crop(size_t x, size_t y, size_t width, size_t height)
{
	using namespace std;
	// Consistency checks
	if (x < 0  || x > columns - width)
		throw std::runtime_error("Area to crop does not fit horizontally inside image data");
	if (y < 0  || y > lines - height)
		throw std::runtime_error("Area to crop does not fit vertically inside image data");

	size_t newsize = width * height;
	Sample *newpix = new Sample[newsize];
	for (size_t i = 0; i < height; ++i)
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
	EL s = this->pixels[line * this->columns + column];
	if (s == this->missing)
		return this->unscaledMissingValue();
	else
		return (int)round((s - this->offset) / this->slope);
}

}

// vim:set ts=2 sw=2:
#endif
