#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include <string>

/// Interface for image data of various types
struct ImageData
{
  virtual ~ImageData() {}

  // Image metadata

  /// Image name
  std::string name;

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

  /// Image time
  int year, month, day, hour, minute;

  std::string projection;
  int channel_id;
  int spacecraft_id;
  int column_factor;
  int line_factor;
  int column_offset;
  int line_offset;

  /// Number of bits per sample
  size_t bpp;

  /// Image sample as physical value (already scaled with slope and offset)
  virtual float scaled(int column, int line) const
  {
    return unscaled(column, line) * slope + offset;
  }

	/// Pixel resolution at nadir point
	virtual float pixelSize() const;

	/// Earth dimension scanned by Seviri in the X direction
	virtual float seviriDX() const;

	/// Earth dimension scanned by Seviri in the Y direction
	virtual float seviriDY() const;

	/// Get all the lines * columns samples, scaled
	virtual float* allScaled() const;

  /// Image sample as unscaled int value (to be scaled with slope and offset)
  virtual int unscaled(int column, int line) const = 0;

	/// Get all the lines * columns samples, unscaled
	virtual int* allUnscaled() const;

	/// Return the decimal scaling factor that can be used before truncating
	/// scaled values as integers
	int decimalScale() const;

  // Get the datetime as a string
  std::string datetime();

  // Get the image time as number of seconds since 1/1/2000 UTC
  time_t forecastSeconds2000();
};

// vim:set ts=2 sw=2:
#endif
