#ifndef MSATLIB_XRIT_IMPORT_H
#define MSATLIB_XRIT_IMPORT_H

#include <msat/Image.h>
#include <msat/xrit/fileaccess.h>
#include <msat/xrit/dataaccess.h>
#include <memory>
#include <vector>
#include <deque>

namespace msat {

typedef xrit::FileAccess XRITImportOptions;

struct HRITImageData : public ImageData
{
	xrit::DataAccess da;

        /// Calibration vector
        float* calibration;

  HRITImageData() : calibration(0) {}
  virtual ~HRITImageData();

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
