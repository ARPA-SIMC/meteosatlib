#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>
#include <vector>
#include <memory>
#include <gdal/gdal_priv.h>
#include <msat/proj/Points.h>

struct GeoReferencer;

namespace msat {

struct ImageData;

class Image : public GDALDataset
{
public:
#if 0
	/// Functor interface for mapping pixels between two images
	struct PixelMapper
	{
		virtual ~PixelMapper() {}
		virtual void operator()(size_t x, size_t y, int& nx, int& xy) const = 0;
	};
#endif

	/// Get image time
	virtual void getTime(int& year, int& month, int& day, int& hour, int& minute, int& second) = 0;

	/// Set image time
	virtual void setTime(int year, int month, int day, int hour, int minute, int second);

	/// Spacecraft identifier (from WMO Common code table C-5)
	/// http://www.emc.ncep.noaa.gov/mmb/data_processing/common_tbl_c1-c5.htm
	virtual int getSpacecraft() = 0;

	/// Spacecraft identifier (from WMO Common code table C-5)
	/// http://www.emc.ncep.noaa.gov/mmb/data_processing/common_tbl_c1-c5.htm
	virtual void setSpacecraft(int id);

	/// TODO: instrument identifier from WMO Common code table C-8
	/// http://www.emc.ncep.noaa.gov/mmb/data_processing/common_tbl_c8-c13.htm

	/**
	 * Get the geotransformation matrix from a meteosat point of view.
	 *
	 * @retval xs
	 *  X pixel coordinates of the subsatellite point
	 * @retval ys
	 *  Y pixel coordinates of the subsatellite point
	 * @retval psx
	 *  Horizontal pixel size
	 * @retval psy
	 *  Vertical pixel size
	 */
	void getMsatGeotransform(int& xs, int& ys, double& psx, double& psy);

	/**
	 * Set the geotransformation matrix from a meteosat point of view.
	 *
	 * @param xs
	 *  X pixel coordinates of the subsatellite point
	 * @param ys
	 *  Y pixel coordinates of the subsatellite point
	 * @param psx
	 *  Horizontal pixel size
	 * @param psy
	 *  Vertical pixel size
	 */
	void setMsatGeotransform(int xs, int ys, double psx, double psy);

	/**
	 * Character indicating image quality: "H" for high, "M" for medium, "L" for
	 * low, "_" for unknown.
	 */
	char quality;

#if 0
	/**
	 * Default file name (without extension) that can be used to store this image
	 */
	std::string defaultFilename;
#endif


	Image() : quality('_') {}
	~Image();

	/**
	 * Set the quality character using the first letters of the file name
	 * pointed by the given pathname
	 */
	void setQualityFromPathname(const std::string& pathname);

	/**
	 * History of this image, as comma-separated descriptions of events,
	 * earliest first
	 */
	virtual std::string getHistory() = 0;

	/// Add an event to the image history
	virtual void addToHistory(const std::string& event) = 0;

	/// Get the image history, with the given event appended.
	std::string historyPlusEvent(const std::string& event);

	// Get the datetime as a string
	std::string datetime();

#if 0
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
#endif

	/// Convert the HRIT spacecraft ID to the ID as in WMO Common code table C-5
	static int spacecraftIDFromHRIT(int id);

	/// Convert the spacecraft ID as in WMO Common code table C-5 to the value
	/// used by HRIT
	static int spacecraftIDToHRIT(int id);

	/// Get the spacecraft name from the WMO Common code table C-5 satellite ID
	static std::string spacecraftName(int hritID);

	/// Get the spacecraft id by name from the WMO Common code table C-5 satellite ID
	static int spacecraftID(const std::string& name);

	/// Get the sensor name from the given WMO Common code table C-5 satellite ID
	static std::string sensorName(int spacecraftID);

	/// Get the channel name from the given WMO Common code table C-5 satellite ID and channel ID
	static std::string channelName(int spacecraftID, int channelID);

	/// Get the measure unit name from the given WMO Common code table C-5 satellite ID and channel ID
	static const char* channelUnit(int spacecraftID, int channelID);

	/// Get the data level string "1", "1.5", "2", "3"... for the given channel
	static std::string channelLevel(int spacecraftID, int channelID);
};

class Band : public GDALRasterBand
{
public:
	virtual ~Band() {}

#if 0
	/**
	 * Default file name (without extension) that can be used to store this
	 * raster band only
	 */
	std::string defaultFilename;
#endif

	/// Get the channel ID (from table TODO)
	/// http://www.ecmwf.int/publications/manuals/libraries/gribex/Proposed_Channel_ID_tables.doc
	virtual int getChannel() = 0;

	/// Set the channel ID (from table TODO)
	virtual void setChannel(int id);

	Image& img() { return *dynamic_cast<Image*>(poDS); }

	/// Get the band short name
	virtual std::string getName() = 0;

	/// Set the band short name
	virtual void setName(const std::string& name);

	/**
	 * Bits per pixel of information conveyed by this raster band.  This is not
	 * necessarily the size of the samples read with the I/O functions, but
	 * rather the expected precision of the data.
	 *
	 * For example, if a raster band originally stores data with 10 bits per
	 * pixel, the I/O functions will have to read at least 16 bits per pixel.
	 *
	 * In such cases, originalBpp can be used to access the amount of
	 * information that can actually be trusted, to be used, for example, in
	 * decimal roundings, or when choosing the sample size to use for recoding
	 * into other formats.
	 */
	virtual int getOriginalBpp() = 0;

	/**
	 * Set the preferred bpp to use to encode the image.
	 *
	 * This can provide a finer grain of control for those datasets that
	 * support it, and is just ignored for all the others.
	 */
	virtual void setPreferredBpp(int bpp);

	/// True if the result of dividing by slope and subtracting offset can be
	/// rounded to an int without losing information
	bool scalesToInt();

#if 0
	/// Number of bits per sample
	int bpp();
#endif

	/// Return the number of significant decimal digits for the scaled values.
	/// It can be negative in case the values are scaled up
	/// Use GetScale() as a scale.
	int decimalDigitsOfScaledValues();

	/// Return the number of significant decimal digits for the scaled values.
	/// It can be negative in case the values are scaled up
	int decimalDigitsOfScaledValues(double scale);

#if 0
	/// Horizontal pixel resolution at nadir point
	double pixelHSize() const;

	/// Vertical pixel resolution at nadir point
	double pixelVSize() const;
#endif

	/// Image sample as physical value (already scaled with slope and offset)
	virtual double scaled(int column, int line);

	/// No data value (already scaled with slope and offset)
	virtual double scaledNoDataValue();

	static int seviriDXFromPixelHSize(double psx);
	static int seviriDYFromPixelVSize(double psy);
	static double pixelHSizeFromSeviriDX(int dx);
	static double pixelVSizeFromSeviriDY(int dy);

	/// Set the column factor from a seviri DX value
	static double CFACFromSeviriDX(int seviriDX);

	/// Set the column factor from a seviri DY value
	static double LFACFromSeviriDY(int seviriDY);

	/// Earth dimension scanned by Seviri in the X direction
	static int seviriDXFromCFAC(double column_res);

	/// Earth dimension scanned by Seviri in the Y direction
	static int seviriDYFromLFAC(double line_res);


	/// Pixel size in "metres" from column resolution in microradians
	static double pixelHSizeFromCFAC(double cfac);

	/// Inverse of the above
	static double CFACFromPixelHSize(double psx);

	/// Pixel size in "metres" from line resolution in microradians
	static double pixelVSizeFromLFAC(double lfac);

	/// Inverse of the above
	static double LFACFromPixelVSize(double psy);


	/// Return the bet encoding the dst driver supports for storing the data in src
	static GDALDataType selectType(GDALRasterBand& src, GDALDriver& dst);

	/// Get the default missing value that can be used for the given channel
	static double defaultPackedMissing(int channel);

	/// Get the default missing value that can be used for the given channel
	static double defaultScaledMissing(int channel);
};


#if 0
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
			for (size_t x = 0; x < width; ++x)
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
		//using namespace std;
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
				if (nx >= 0 && ny >= 0 && (unsigned)nx < columns && (unsigned)ny < lines)
				{
					//cerr << "sample is " << this->pixels[ny*this->columns+nx] << endl;
					res->pixels[y*width+x] = pixels[ny*columns+nx];
				} else {
					//cerr << "sample missing" << endl;
					res->pixels[y*width+x] = missing;
				}
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
			for (size_t x = 0; x < width; ++x)
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
		//using namespace std;
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
				if (nx >= 0 && ny >= 0 && (unsigned)nx < this->columns && (unsigned)ny < this->lines)
				{
					//cerr << "sample is " << this->pixels[ny*this->columns+nx] << endl;
					res->pixels[y*width+x] = this->pixels[ny*this->columns+nx];
				}
				else
				{
					//cerr << "sample missing" << endl;
					res->pixels[y*width+x] = this->missingValue;
				}
			}
		return res;
	}
#endif
};
#endif


struct ImageConsumer
{
	virtual ~ImageConsumer() {}
	virtual void processImage(GDALDataset& image) = 0;
};

struct ImageTransformation
{
	proj::ImageBox cropImgArea;
	size_t scaleX, scaleY;

	ImageTransformation() : scaleX(0), scaleY(0) {}

	/// Update cropImgArea using cropGeoArea
	void updateImgArea(GeoReferencer& georef, const proj::MapBox& cropGeoArea);

	bool hasScale() const { return scaleX || scaleY; }
};

struct ImageImporter
{
	ImageImporter() {}
	virtual ~ImageImporter() {}

#if 0
	void cropIfNeeded(Image& img)
	{
		if (cropImgArea.isNonZero())
			img.crop(cropImgArea);
		else if (cropGeoArea.isNonZero())
			img.cropByCoords(cropGeoArea);
	}
#endif

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

/// Replace spaces and dots in the string with underscores
void escapeSpacesAndDots(std::string& str);

/// Compute a default file name for a standard satellite file
std::string defaultFilename(GDALDataset& img);

/// Compute a default file name for a standard satellite file
std::string defaultFilename(GDALRasterBand& band);

/// Compute a default short name for a standard satellite file
std::string defaultShortName(GDALDataset& img);

/// Compute a default short name for a standard satellite file
std::string defaultShortName(GDALRasterBand& band);

/// Get the amount of bpp coded by the raster band
int getBPP(GDALRasterBand& rb);

/// Recode the raster band 'ridx' of the dataset using the given driver and data type
void recode(
		GDALDataset& img, int ridx,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1 = std::string(),
		const std::string& opt2 = std::string(),
		const std::string& opt3 = std::string());

/// Recode the raster band 'ridx' of the dataset, using the given crop area, driver and data type
void recode(
		GDALDataset& img, int ridx,
		const msat::proj::ImageBox& cropArea,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1 = std::string(),
		const std::string& opt2 = std::string(),
		const std::string& opt3 = std::string());

/// Recode the raster band 'ridx' of the dataset, using the given target size, driver and data type
void recode(
		GDALDataset& img, int ridx,
		int sx, int sy,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1 = std::string(),
		const std::string& opt2 = std::string(),
		const std::string& opt3 = std::string());

/// Recode the raster band 'ridx' of the dataset, using the given crop area, target size, driver and data type
void recode(
		GDALDataset& img, int ridx,
		const msat::proj::ImageBox& cropArea, int sx, int sy,
		const std::string& fileName, GDALDriver* driver, GDALDataType type,
		const std::string& opt1 = std::string(),
		const std::string& opt2 = std::string(),
		const std::string& opt3 = std::string());

// Get the time as number of seconds since 1/1/2000 UTC
time_t forecastSeconds2000(int year, int month, int day, int hour=0, int minute=0, int second=0);

}

// vim:set ts=2 sw=2:
#endif
