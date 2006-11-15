#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>
#include <vector>
#include <proj/Projection.h>

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

	/// Projection
	std::auto_ptr<proj::Projection> proj;

	/// Channel ID (from table TODO)
  int channel_id;

	/// Spacecraft identifier (from table TODO)
  int spacecraft_id;

	/**
	 * Horizontal scaling coefficient computed as (2**16)/delta, where delta is
	 * the size in micro Radians of one pixel
	 */
  int column_factor;

	/**
	 * Vertical scaling coefficient computed as (2**16)/delta, where delta is the
	 * size in micro Radians of one pixel
	 */
  int line_factor;

	/// Horizontal offset in pixels of the sub-satellite point
	int column_offset;

	/// Vertical offset in pixels of the sub-satellite point
  int line_offset;

	/**
	 * Pixel coordinates of the top left point in the image, related to the
	 * uncropped satellite image
	 */
	int x0, y0;

	/**
	 * Character indicating image quality: "H" for high, "M" for medium, "L" for
	 * low, "_" for unknown.
	 */
	char quality;

	/**
	 * History of this image, as comma-separated descriptions of events,
	 * earliest first
	 */
	std::string history;

	// Image data
	ImageData* data;

	Image() : quality('_'), data(0) {}
	~Image();

	/**
	 * Set the quality character using the first letters of the file name
	 * pointed by the given pathname
	 */
	void setQualityFromPathname(const std::string& pathname);

	/// Add an event to the image history
	void addToHistory(const std::string& event);

	/// Get the image history, with the given event appended.
	std::string historyPlusEvent(const std::string& event) const;

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

	/// Return the nearest pixel coordinates to the given geographical place
	void coordsToPixels(double lat, double lon, size_t& x, size_t& y) const;

	/**
	 * Crop the image to the given rectangle specified in pixel coordinates
	 * relative to the image itself
	 */
	void crop(int x, int y, int width, int height);

	/**
	 * Crop the image to the minimum rectangle of pixels containing the area
	 * defined with the given geographical coordinates
	 */
	void cropByCoords(double latmin, double latmax, double lonmin, double lonmax);

	/**
	 * Compute a meaningful default file name (without extension) that can be
	 * used to store this image
	 */
	std::string defaultFilename() const;

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
	double cropLatMin, cropLonMin, cropLatMax, cropLonMax;

	ImageImporter() :
		cropX(0), cropY(0), cropWidth(0), cropHeight(0),
		cropLatMin(1000), cropLonMin(1000), cropLatMax(1000), cropLonMax(1000) {}
	virtual ~ImageImporter() {}

	void cropIfNeeded(Image& img)
	{
		if (cropWidth != 0 && cropHeight != 0)
			img.crop(cropX, cropY, cropWidth, cropHeight);
		else if (cropLatMin != 1000 && cropLatMax != 1000 && cropLonMin != 1000 && cropLonMax != 1000)
			img.cropByCoords(cropLatMin, cropLatMax, cropLonMin, cropLonMax);
	}

	virtual void read(ImageConsumer& output) = 0;
};

struct ImageVector : public std::vector<Image*>, ImageConsumer
{
	ImageVector() {}
	// Convenience constructor to automatically fill in using an importer
	ImageVector(ImageImporter& imp) { imp.read(*this); }
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
