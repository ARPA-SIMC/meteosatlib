#ifndef MSATLIB_XRIT_IMPORT_H
#define MSATLIB_XRIT_IMPORT_H

#include <msat/Image.h>
#include <msat/xrit/fileaccess.h>
#include <hrit/MSG_data_image.h>
#include <memory>
#include <vector>
#include <deque>

struct MSG_data;

namespace msat {

typedef xrit::FileAccess XRITImportOptions;

struct HRITImageData : public ImageData
{
	/**
	 * Map an area on the virtual image to an area in the real image data
	 */
	struct AreaMap
	{
		int x, y;
		size_t width, height;
		size_t startcolumn, startline;
		AreaMap() : x(0), y(0), width(0), height(0), startcolumn(0), startline(0) {}
		bool contains(size_t x, size_t y) const
		{
			if (this->x >= 0 && (size_t)this->x > x)
				return false;
			if (this->y >= 0 && (size_t)this->y > y)
				return false;
			return x < (size_t)(this->x + width) && y < (size_t)(this->y + height);
		}
		void remap(size_t x, size_t y, size_t& raw_x, size_t& raw_y) const
		{
			raw_x = startcolumn + x - this->x;
			raw_y = startline + y - this->y;
		}
	};

	/// HRV North area, or complete area for non-HRV images
	AreaMap hrvNorth;
	/// HRV South area
	AreaMap hrvSouth;

	/*
  /// HRV parameters used to locate the two image parts
  size_t LowerEastColumnActual;
  size_t LowerNorthLineActual;
  size_t LowerWestColumnActual;
  size_t UpperEastColumnActual;
  size_t UpperSouthLineActual;
  size_t UpperWestColumnActual;
	*/

  /// Number of pixels in every segment
  size_t npixperseg;

  /// True if the image needs to be swapped horizontally
  bool swapX;

  /// True if the image needs to be swapped vertically
  bool swapY;

  /// True if the image is an HRV image divided in two parts
  bool hrv;

  /// Pathnames of the segment files, indexed with their index
  std::vector<std::string> segnames;

	struct scache
	{
		MSG_data* segment;
		size_t segno;
	};
	/// Segment cache
	mutable std::deque<scache> segcache;

  /// Calibration vector
  float* calibration;

  /// Number of columns in the uncropped image
  size_t origColumns;

  /// Number of lines in the uncropped image
  size_t origLines;

  /// Cropping edges
  int cropX, cropY;

  HRITImageData() : npixperseg(0), calibration(0), cropX(0), cropY(0) {}
  virtual ~HRITImageData();

  /**
   * Return the MSG_data corresponding to the segment with the given index.
   *
   * The pointer could be invalidated by another call to segment()
   */
  MSG_data* segment(size_t idx) const;

  /// Get an unscaled sample from the given coordinates in the normalised image
  MSG_SAMPLE sample(size_t x, size_t y) const;

  /// Image sample as physical value (already scaled with slope and offset)
  float scaled(int column, int line) const;

	virtual ImageData* createResampled(size_t width, size_t height) const;
#ifdef EXPERIMENTAL_REPROJECTION
	/**
	 * Create a new image with the given size using the same kind of image data
	 * as this one.  The new image will be initialized with all missing values.
	 */
	virtual ImageData* createReprojected(size_t width, size_t height, const Image::PixelMapper& mapper) const;
#endif

  /// Image sample scaled to int using slope and offset.
  /// The function throws if scalesToInt is false.
  virtual int scaledToInt(int column, int line) const;

  /// Value used to represent a missing value in the unscaled int
  /// data, if available
  virtual int unscaledMissingValue() const;

  /// Crop the image to the given rectangle
  virtual void crop(size_t x, size_t y, size_t width, size_t height);
};

static inline bool isXRIT(const std::string& filename)
{
  return xrit::isValid(filename);
}

std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts);
std::auto_ptr<ImageImporter> createXRITImporter(const std::string& filename);

}

// vim:set ts=2 sw=2:
#endif
