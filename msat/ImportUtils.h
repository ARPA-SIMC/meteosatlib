#ifndef MSAT_IMPOT_UTILS_H
#define MSAT_IMPOT_UTILS_H

#include <string>

namespace msat {

class Image;

namespace util {

/// Replace spaces and dots in the string with underscores
void escapeSpacesAndDots(std::string& str);

/// Compute a default file name for a standard satellite file
std::string satelliteSingleImageFilename(const Image& img);

}
}

// vim:set ts=2 sw=2:
#endif
