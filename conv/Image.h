#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>
#include <vector>

namespace msat {

struct ImageData;

class Image
{
private:
	// Copy is not straightforward: disable it
	Image(const Image& img);
	Image& operator=(const Image& img);

#if 0
	--- Prototypes that are still not working
	Image(const Image& img) : data(0)
	{
		year = img.year;
		month = img.month;
		day = img.day;
		hour = img.hour;
		minute = img.minute;
		sublon = img.sublon;
		channel_id = img.channel_id;
		spacecraft_id = img.spacecraft_id;
		column_factor = img.column_factor;
		line_factor = img.line_factor;
		column_offset = img.column_offset;
		line_offset = img.line_offset;
		if (img.data)
			data = img.data->clone();
	}

	/// Copy constructor
	Image& operator=(const Image& img)
	{
		if (this == &img) return *this;
		year = img.year;
		month = img.month;
		day = img.day;
		hour = img.hour;
		minute = img.minute;
		sublon = img.sublon;
		channel_id = img.channel_id;
		spacecraft_id = img.spacecraft_id;
		column_factor = img.column_factor;
		line_factor = img.line_factor;
		column_offset = img.column_offset;
		line_offset = img.line_offset;
		if (data) { delete data; data = 0; }
		if (img.data)
			data = img.data->clone();
		return *this;
	}
#endif

public:
  /// Image time
  int year, month, day, hour, minute;

	/// Longitude of sub-satellite point
	double sublon;

	/// Channel ID (from table TODO)
  int channel_id;

	/// Spacecraft identifier (from table TODO)
  int spacecraft_id;

	// TODO: description missing
  int column_factor;
	// TODO: description missing
  int line_factor;

	// Horizontal position of the image on the entire world view
  int column_offset;

	// Vertical position of the image on the entire world view
  int line_offset;

	// Image data
	ImageData* data;

	Image() : data(0) {}
	~Image();


	/// Horizontal pixel resolution at nadir point
	double pixelHSize() const;

	/// Vertical pixel resolution at nadir point
	double pixelVSize() const;

	/// Earth dimension scanned by Seviri in the X direction
	int seviriDX() const { return seviriDXFromColumnFactor(column_factor); }

	/// Earth dimension scanned by Seviri in the Y direction
	int seviriDY() const { return seviriDYFromLineFactor(column_factor); }

  // Get the datetime as a string
  std::string datetime() const;

  // Get the image time as number of seconds since 1/1/2000 UTC
  time_t forecastSeconds2000() const;

	/// Return the number of significant decimal digits for the scaled values.
	/// It can be negative in case the values are scaled up
	int decimalDigitsOfScaledValues() const;

	/// Set the image data for this image
	void setData(ImageData* data);

	/**
	 * Crop the image to the given rectangle specified in coordinates relative
	 * to the image itself
	 */
	void crop(int x, int y, int width, int height);


	/// Earth dimension scanned by Seviri in the X direction
	static int seviriDXFromColumnFactor(int column_factor);

	/// Earth dimension scanned by Seviri in the Y direction
	static int seviriDYFromLineFactor(int line_factor);

	/// Set the column factor from a seviri DX value
	static int columnFactorFromSeviriDX(int seviriDX);

	/// Set the column factor from a seviri DY value
	static int lineFactorFromSeviriDY(int seviriDY);

	/// Convert the HRIT spacecraft ID to the ID as in WMO Common code table C-5
	static int spacecraftIDFromHRIT(int id);

	/// Convert the spacecraft ID as in WMO Common code table C-5 to the value
	/// used by HRIT
	static int spacecraftIDToHRIT(int id);
};

/// Interface for image data of various types
struct ImageData
{
	ImageData();
  virtual ~ImageData() {}

  // Image metadata

  /// Number of columns
  size_t columns;

  /// Number of lines
  size_t lines;

  /// Scaling factor to apply to the raw image data to get the real physical
  /// values
  double slope;

  /// Reference value to add to the scaled raw image data to get the real
  /// physical values
  double offset;

  /// Number of bits per sample
  size_t bpp;

	/// True if the result of dividing by slope and subtracting offset can be
	/// rounded to an int without losing information
	bool scalesToInt;

	/// Value used to represent a missing value in the scaled data
	float missingValue;

  /// Image sample as physical value (already scaled with slope and offset)
  virtual float scaled(int column, int line) const = 0;

	/// Image sample scaled to int using slope and offset.
	/// The function throws if scalesToInt is false.
	virtual int scaledToInt(int column, int line) const = 0;

	/// Value used to represent a missing value in the unscaled int
	/// data, if available
	virtual int unscaledMissingValue() const = 0;

	/// Get all the lines * columns samples, scaled
	virtual float* allScaled() const;

	/// Crop the image to the given rectangle
	virtual void crop(int x, int y, int width, int height) = 0;
};

// Container for image data, which can be used with different sample sizes
template<typename EL>
struct ImageDataWithPixels : public ImageData
{
public:
  typedef EL Sample;
  Sample* pixels;
	Sample missing;

  ImageDataWithPixels();
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

  virtual float scaled(int column, int line) const
  {
		Sample s = this->pixels[line * columns + column];
    return s == missing ? missingValue : s * slope + offset;
  }

	virtual int scaledToInt(int column, int line) const;

	virtual int unscaledMissingValue() const;

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

// Container for image data, which can be used with different sample sizes
template<typename EL>
struct ImageDataWithPixelsPrescaled : public ImageDataWithPixels<EL>
{
public:
  ImageDataWithPixelsPrescaled() : ImageDataWithPixels<EL>() {}
  ImageDataWithPixelsPrescaled(size_t width, size_t height) : ImageDataWithPixels<EL>(width, height) {}

  virtual float scaled(int column, int line) const
  {
    return this->pixels[line * this->columns + column];
  }

	virtual int scaledToInt(int column, int line) const;
};


struct ImageConsumer
{
	virtual ~ImageConsumer() {}
	virtual void processImage(std::auto_ptr<Image> image) = 0;
};

struct ImageImporter
{
	int cropX, cropY, cropWidth, cropHeight;

	ImageImporter() : cropX(0), cropY(0), cropWidth(0), cropHeight(0) {}
	virtual ~ImageImporter() {}

	bool shouldCrop() const { return cropWidth != 0 && cropHeight != 0; }

	virtual void read(ImageConsumer& output) = 0;
};

struct ImageVector : public std::vector<Image*>, ImageConsumer
{
	virtual ~ImageVector();
	virtual void processImage(std::auto_ptr<Image> image)
	{
		push_back(image.release());
	}
	// Remove the first image from the vector, and returns it.
	virtual std::auto_ptr<Image> shift()
	{
		std::auto_ptr<Image> res(*begin());
		erase(begin());
		return res;
	}
};

std::auto_ptr<ImageConsumer> createImageDumper(bool withContents);

}

// vim:set ts=2 sw=2:
#endif
