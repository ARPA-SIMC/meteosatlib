#ifndef MSATLIB_XRIT_IMPORT_H
#define MSATLIB_XRIT_IMPORT_H

#include <msat/Image.h>
#include <hrit/MSG_data_image.h>
#include <memory>
#include <vector>

struct MSG_data;

namespace msat {

struct XRITImportOptions
{
  std::string directory;
  std::string resolution;
  std::string productid1;
  std::string productid2;
  std::string timing;

  XRITImportOptions() : directory(".") {}
  XRITImportOptions(const std::string& filename);

  void ensureComplete() const;

  std::string prologueFile() const;
  std::string epilogueFile() const;
  std::vector<std::string> segmentFiles() const;

  std::string toString() const;
};

struct HRITImageData : public ImageData
{
  /// HRV parameters used to locate the two image parts
  size_t LowerEastColumnActual;
  size_t LowerNorthLineActual;
  size_t LowerWestColumnActual;
  size_t UpperEastColumnActual;
  size_t UpperSouthLineActual;
  size_t UpperWestColumnActual;

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

  /// Cached segment
  mutable MSG_data* m_segment;

  /// Index of the currently cached segment
  mutable int m_segment_idx;

  /// Calibration vector
  float* calibration;

  /// Number of columns in the uncropped image
  size_t origColumns;

  /// Number of lines in the uncropped image
  size_t origLines;

  /// Cropping edges
  int cropX, cropY;

  HRITImageData() : npixperseg(0), m_segment(0), m_segment_idx(-1), calibration(0), cropX(0), cropY(0) {}
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

  /// Image sample scaled to int using slope and offset.
  /// The function throws if scalesToInt is false.
  virtual int scaledToInt(int column, int line) const;

  /// Value used to represent a missing value in the unscaled int
  /// data, if available
  virtual int unscaledMissingValue() const;

  /// Crop the image to the given rectangle
  virtual void crop(size_t x, size_t y, size_t width, size_t height);
};

bool isXRIT(const std::string& filename);
std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts);
std::auto_ptr<ImageImporter> createXRITImporter(const std::string& filename);

}

// vim:set ts=2 sw=2:
#endif
