#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>
#include <vector>
#include <msat/proj/Projection.h>

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
	/// Functor interface for mapping pixels between two images
	struct PixelMapper
	{
		virtual ~PixelMapper() {}
		virtual void operator()(size_t x, size_t y, int& nx, int& xy) const = 0;
	};

  /// Image time
  int year, month, day, hour, minute;

	/// Projection
	std::auto_ptr<proj::Projection> proj;

	/// Channel ID (from table TODO)
  int channel_id;

	/// Spacecraft identifier (from table TODO)
  int spacecraft_id;

	/**
	 * Horizontal resolution: how many pixels are in one projection unit
	 */
  double column_res;

	/**
	 * Vertical resolution: how many pixels are in one projection unit
	 */
  double line_res;

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
	 * Default file name (without extension) that can be used to store this image
	 */
	std::string defaultFilename;

	/**
	 * Short image name, such as product name for multi-product images
	 */
	std::string shortName;

	/**
	 * Measurement units for the scaled image samples, following COARDS
	 * conventions, or "NUMBER" for pure numbers like palette indexes.
	 */
	std::string unit;

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
	int seviriDX() const { return seviriDXFromColumnRes(column_res); }

	/// Earth dimension scanned by Seviri in the Y direction
	int seviriDY() const { return seviriDYFromLineRes(line_res); }

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
	void coordsToPixels(double lat, double lon, int& x, int& y) const;

	/// Return the coordinates of the place corresponding to the given pixel
	void pixelsToCoords(int x, int y, double& lat, double& lon) const;

	/**
	 * Crop the image to the given rectangle specified in pixel coordinates
	 * relative to the image itself
	 */
	void crop(const proj::ImageBox& area);

	/**
	 * Crop the image to the minimum rectangle of pixels containing the area
	 * defined with the given geographical coordinates
	 */
	void cropByCoords(const proj::MapBox& area);

	/**
	 * Return a new image scaled to the given width and height
	 */
	std::auto_ptr<Image> rescaled(size_t width, size_t height) const;

#ifdef EXPERIMENTAL_REPROJECTION
	/**
	 * Return a new image with the given width and height, containing the same
	 * data of this image but using a different projection
	 */
	std::auto_ptr<Image> reproject(size_t width, size_t height, std::auto_ptr<proj::Projection> proj, const proj::MapBox& box) const;
#endif

	/// Earth dimension scanned by Seviri in the X direction
	static int seviriDXFromColumnRes(double column_res);

	/// Earth dimension scanned by Seviri in the Y direction
	static int seviriDYFromLineRes(double line_res);

	/// Set the column factor from a seviri DX value
	static double columnResFromSeviriDX(int seviriDX);

	/// Set the column factor from a seviri DY value
	static double lineResFromSeviriDY(int seviriDY);

	/// Convert the HRIT spacecraft ID to the ID as in WMO Common code table C-5
	static int spacecraftIDFromHRIT(int id);

	/// Convert the spacecraft ID as in WMO Common code table C-5 to the value
	/// used by HRIT
	static int spacecraftIDToHRIT(int id);

	/// Get the spacecraft name from the given HRIT satellite ID
	static std::string spacecraftName(int hritID);

	/// Get the sensor name from the given HRIT satellite ID
	static std::string sensorName(int hritSpacecraftID);

	/// Get the channel name from the given HRIT satellite ID and channel ID
	static std::string channelName(int hritSpacecraftID, int channelID);

	/// Get the measure unit name from the given HRIT satellite ID and channel ID
	static std::string channelUnit(int hritSpacecraftID, int channelID);
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

	virtual ImageData* createResampled(size_t width, size_t height) const = 0;
#ifdef EXPERIMENTAL_REPROJECTION
	/**
	 * Create a new image with the given size using the same kind of image data
	 * as this one.  The new image will be initialized with all missing values.
	 */
	virtual ImageData* createReprojected(size_t width, size_t height, const Image::PixelMapper& mapper) const = 0;
#endif

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
	virtual void crop(size_t x, size_t y, size_t width, size_t height) = 0;
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

	virtual ImageData* createResampled(size_t width, size_t height) const
	{
		ImageDataWithPixels<EL>* res(new ImageDataWithPixels<EL>(width, height));
		res->slope = slope;
		res->offset = offset;
		res->bpp = bpp;
		res->scalesToInt = scalesToInt;
		res->missingValue = missingValue;
		res->missing = missing;
		for (size_t y = 0; y < height; ++y)
			for (size_t x = 0; x < height; ++x)
			{
				size_t nx = x * this->columns / width;
				size_t ny = y * this->lines   / height;
				res->pixels[y*width+x] = pixels[ny*columns+nx];
			}
		return res;
	}

#ifdef EXPERIMENTAL_REPROJECTION
	virtual ImageData* createReprojected(size_t width, size_t height, const Image::PixelMapper& mapper) const
	{
		ImageDataWithPixels<EL>* res(new ImageDataWithPixels<EL>(width, height));
		res->slope = slope;
		res->offset = offset;
		res->bpp = bpp;
		res->scalesToInt = scalesToInt;
		res->missingValue = missingValue;
		res->missing = missing;
		for (size_t y = 0; y < height; ++y)
			for (size_t x = 0; x < width; ++x)
			{
				int nx = 0, ny = 0;
				mapper(x, y, nx, ny);
				if (nx < 0 || ny < 0 || (unsigned)nx > columns || (unsigned)ny > lines)
					res->pixels[y*width+x] = missing;
				else
					res->pixels[y*width+x] = pixels[ny*columns+nx];
			}
		return res;
	}
#endif

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
	void crop(size_t x, size_t y, size_t width, size_t height);
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

	virtual ImageData* createResampled(size_t width, size_t height) const
	{
		ImageDataWithPixelsPrescaled<EL>* res(new ImageDataWithPixelsPrescaled<EL>(width, height));
		res->slope = this->slope;
		res->offset = this->offset;
		res->bpp = this->bpp;
		res->scalesToInt = this->scalesToInt;
		res->missingValue = this->missingValue;
		res->missing = this->missing;
		for (size_t y = 0; y < height; ++y)
			for (size_t x = 0; x < height; ++x)
			{
				size_t nx = x * this->columns / width;
				size_t ny = y * this->lines   / height;
				res->pixels[y*width+x] = this->pixels[ny*this->columns+nx];
			}
		return res;
	}

#ifdef EXPERIMENTAL_REPROJECTION
	virtual ImageData* createReprojected(size_t width, size_t height, const Image::PixelMapper& mapper) const
	{
		ImageDataWithPixelsPrescaled<EL>* res(new ImageDataWithPixelsPrescaled<EL>(width, height));
		res->slope = this->slope;
		res->offset = this->offset;
		res->bpp = this->bpp;
		res->scalesToInt = this->scalesToInt;
		res->missingValue = this->missingValue;
		res->missing = this->missing;
		for (size_t y = 0; y < height; ++y)
			for (size_t x = 0; x < width; ++x)
			{
				int nx = 0, ny = 0;
				mapper(x, y, nx, ny);
				if (nx < 0 || ny < 0 || (unsigned)nx > this->columns || (unsigned)nx > this->lines)
					res->pixels[y*width+x] = this->missingValue;
				else
					res->pixels[y*width+x] = this->pixels[ny*this->columns+nx];
			}
		return res;
	}
#endif
};


struct ImageConsumer
{
	virtual ~ImageConsumer() {}
	virtual void processImage(std::auto_ptr<Image> image) = 0;
};

struct ImageImporter
{
	proj::ImageBox cropImgArea;
	proj::MapBox cropGeoArea;

	ImageImporter() {}
	virtual ~ImageImporter() {}

	void cropIfNeeded(Image& img)
	{
		if (cropImgArea.isNonZero())
			img.crop(cropImgArea);
		else if (cropGeoArea.isNonZero())
			img.cropByCoords(cropGeoArea);
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
