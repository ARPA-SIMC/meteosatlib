#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>

namespace msat {

struct ImageData;

struct Image {
  /// Image name
  std::string name;

  /// Image time
  int year, month, day, hour, minute;

  std::string projection;
  int channel_id;
  int spacecraft_id;

	// TODO
  int column_factor;
	// TODO
  int line_factor;
	// TODO
  int column_offset;
	// TODO
  int line_offset;

	/// Pixel resolution at nadir point
	float pixelSize() const;

	/// Earth dimension scanned by Seviri in the X direction
	float seviriDX() const;

	/// Earth dimension scanned by Seviri in the Y direction
	float seviriDY() const;

  // Get the datetime as a string
  std::string datetime() const;

  // Get the image time as number of seconds since 1/1/2000 UTC
  time_t forecastSeconds2000() const;

	// Image data
	ImageData* data;

	Image() : data(0) {}
	~Image();

	void setData(ImageData* data);
};

/// Interface for image data of various types
struct ImageData
{
  virtual ~ImageData() {}

  // Image metadata

  /// Number of columns
  size_t columns;

  /// Number of lines
  size_t lines;

  /// Scaling factor to apply to the raw image data to get the real physical
  /// values
  float slope;

  /// Reference value to add to the scaled raw image data to get the real
  /// physical values
  float offset;

  /// Number of bits per sample
  size_t bpp;

  /// Image sample as physical value (already scaled with slope and offset)
  virtual float scaled(int column, int line) const
  {
    return unscaled(column, line) * slope + offset;
  }

	/// Get all the lines * columns samples, scaled
	virtual float* allScaled() const;

  /// Image sample as unscaled int value (to be scaled with slope and offset)
  virtual int unscaled(int column, int line) const = 0;

	/// Get all the lines * columns samples, unscaled
	virtual int* allUnscaled() const;

	/// Return the decimal scaling factor that can be used before truncating
	/// scaled values as integers
	int decimalScale() const;
};

// Container for image data, which can be used with different sample sizes
template<typename EL>
struct ImageDataWithPixels : public ImageData
{
public:
  typedef EL Sample;
  Sample* pixels;

  ImageDataWithPixels() : pixels(0) { bpp = sizeof(Sample) * 8; }
  ImageDataWithPixels(size_t width, size_t height) : pixels(new Sample[width*height])
	{
		bpp = sizeof(Sample) * 8;
		columns = width;
		lines = height;
	}
  ~ImageDataWithPixels()
  {
    if (pixels)
      delete[] pixels;
  }

  virtual int unscaled(int column, int line) const
  {
      return static_cast<int>(pixels[line * columns + column]);
  }

	// Rotate the image by 180 degrees, in place
	void rotate180()
	{
		for (int y = 0; y < lines; ++y)
			for (int x = 0; x < columns; ++x)
			{
				Sample i = pixels[y * columns + x];
				pixels[y * columns + x] = pixels[(lines - y - 1) * columns + (columns - x - 1)];
				pixels[(lines - y - 1) * columns + (columns - x - 1)] = i;
			}
	}

	// Throw away all the samples outside of a given area
	void crop(int x, int y, int width, int height);
};

struct ImageConsumer
{
	virtual void processImage(const Image& image) = 0;
};

struct ImageImporter
{
	virtual void read(ImageConsumer& output) = 0;
};

std::auto_ptr<ImageConsumer> createImageDumper(bool withContents);

}

// vim:set ts=2 sw=2:
#endif
