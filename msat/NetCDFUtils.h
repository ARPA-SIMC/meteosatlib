#ifndef CONV_NETCDF_UTILS_H
#define CONV_NETCDF_UTILS_H

//---------------------------------------------------------------------------
//
//  File        :   NetCDFUtils.h
//  Description :   NetCDF utility functions
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  RCS ID      :   $Id: /local/meteosatlib/conv/ExportNetCDF.cpp 1778 2006-09-20T16:26:07.745577Z enrico  $
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//---------------------------------------------------------------------------

//#include "../config.h"

#include <netcdfcpp.h>

#include <sstream>
#include <stdexcept>
#include <memory>

#include <math.h>
#include <limits>

namespace msat {

template<typename NCObject, typename T>
static void ncfAddAttr(NCObject& ncf, const char* name, const T& val)
{
  if (!ncf.add_att(name, val))
  {
		std::stringstream msg;
    msg << "Adding '" << name << "' attribute " << val;
    throw std::runtime_error(msg.str());
  }
}

template<typename Sample>
static inline Sample getAttribute(const NcAtt& a) { throw std::runtime_error("requested to read attribute from unknown C++ type"); }
template<> static inline ncbyte getAttribute<ncbyte>(const NcAtt& a) { return a.as_ncbyte(0); }
template<> static inline char getAttribute<char>(const NcAtt& a) { return a.as_char(0); }
template<> static inline short getAttribute<short>(const NcAtt& a) { return a.as_short(0); }
template<> static inline int getAttribute<int>(const NcAtt& a) { return a.as_int(0); }
template<> static inline float getAttribute<float>(const NcAtt& a) { return a.as_float(0); }
template<> static inline double getAttribute<double>(const NcAtt& a) { return a.as_double(0); }

template<typename Sample>
static inline NcType getNcType() { throw std::runtime_error("requested NcType for unknown C++ type"); }
template<> static inline NcType getNcType<ncbyte>() { return ncByte; }
template<> static inline NcType getNcType<char>() { return ncChar; }
template<> static inline NcType getNcType<short>() { return ncShort; }
template<> static inline NcType getNcType<int>() { return ncInt; }
template<> static inline NcType getNcType<float>() { return ncFloat; }
template<> static inline NcType getNcType<double>() { return ncDouble; }

template<typename Sample>
static inline Sample getMissing()
{
	if (std::numeric_limits<Sample>::has_quiet_NaN)
		return std::numeric_limits<Sample>::quiet_NaN();
	else if (std::numeric_limits<Sample>::is_signed)
		return std::numeric_limits<Sample>::min();
	else
		return std::numeric_limits<Sample>::max();
}

struct NcEncoder
{
	virtual ~NcEncoder() {}
	virtual NcType getType() = 0;
	virtual void setData(NcVar& var, const Image& img) = 0;
};

template<typename Sample>
struct NcEncoderImpl : public NcEncoder
{
	virtual NcType getType() { return getNcType<Sample>(); }
	virtual void setData(NcVar& var, const Image& img)
	{
		Sample* pixels = new Sample[img.data->columns * img.data->lines];
		int missing = img.data->unscaledMissingValue();
		Sample encodedMissing = getMissing<Sample>();
		ncfAddAttr(var, "missing_value", encodedMissing);

		for (size_t y = 0; y < img.data->lines; ++y)
			for (size_t x = 0; x < img.data->columns; ++x)
			{
				int unscaled = img.data->scaledToInt(x, y);
				if (unscaled == missing)
					pixels[y * img.data->columns + x] = encodedMissing;
				else
					pixels[y * img.data->columns + x] = unscaled;
			}

		if (!var.put(pixels, 1, img.data->lines, img.data->columns))
			throw std::runtime_error("writing image values failed");

		delete[] pixels;
	}
};

static std::auto_ptr<NcEncoder> createEncoder(size_t bpp)
{
	if (bpp > 15)
		return std::auto_ptr<NcEncoder>(new NcEncoderImpl<int>);
	else if (bpp > 7)
		return std::auto_ptr<NcEncoder>(new NcEncoderImpl<short>);
	else
		return std::auto_ptr<NcEncoder>(new NcEncoderImpl<ncbyte>);
}

// Recompute the BPP for an image made of integer values, by looking at what is
// their maximum value.  Assume unsigned integers.
static void computeBPP(ImageData& img)
{
	if (img.scalesToInt)
	{
		int max = img.scaledToInt(0, 0);
		for (size_t y = 0; y < img.lines; ++y)
			for (size_t x = 0; x < img.columns; ++x)
			{
				int sample = img.scaledToInt(x, y);
				if (sample > max)
					max = sample;
			}
		img.bpp = (int)ceil(log2(max + 1));
	}
	// Else use the original BPPs
}

// Get the attribute if it exists, otherwise returns 0
static NcAtt* getAttrIfExists(const NcVar& var, const std::string& name)
{
	for (int i = 0; i < var.num_atts(); ++i)
	{
		NcAtt* cand = var.get_att(i);
		if (cand->name() == name)
			return cand;
	}
	return 0;
}

template<typename Sample>
void decodeMissing(const NcVar& var, ImageDataWithPixels<Sample>& img)
{
	NcAtt* attrMissing = getAttrIfExists(var, "missing_value");
	if (attrMissing != NULL)
		img.missing = getAttribute<Sample>(*attrMissing);
}

template<typename Sample>
static ImageData* acquireImage(const NcVar& var)
{
	if (var.num_dims() != 3)
	{
		std::stringstream msg;
		msg << "Number of dimensions for " << var.name() << " should be 3 but is " << var.num_dims() << " instead";
		throw std::runtime_error(msg.str());
	}

	int tsize = var.get_dim(0)->size();
	if (tsize != 1)
	{
		std::stringstream msg;
		msg << "Size of the time dimension for " << var.name() << " should be 1 but is " << tsize << " instead";
		throw std::runtime_error(msg.str());
	}

	std::auto_ptr< ImageDataWithPixels<Sample> > res(new ImageDataWithPixels<Sample>(var.get_dim(2)->size(), var.get_dim(1)->size()));
#warning should this not be stored in / read from the NetCDF file itself?
	res->missing = getMissing<Sample>();

	decodeMissing(var, *res);

	if (!var.get(res->pixels, 1, res->lines, res->columns))
		throw std::runtime_error("reading image pixels failed");

	return res.release();
}

}

// vim:set ts=2 sw=2:
#endif
