//---------------------------------------------------------------------------
//
//  File        :   ExportNetCDF.cpp
//  Description :   Export data from an ImageData into a NetCDF file
//  Project     :   ?
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
//  Source      :   derived from SAFH5CT2NetCDF.cpp by Le Duc, as modified by
//                  Francesca Di Giuseppe and from XRIT2Grib.cpp by Graziano
//                  Giuliani (Lamma Regione Toscana)
//  RCS ID      :   $Id: /local/meteosatlib/conv/ExportNetCDF.cpp 2065 2006-11-14T13:45:01.650692Z enrico  $
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

#include <config.h>

#include <msat/ExportImage.h>
#include <msat/ImportXRIT.h>
#include <msat/Progress.h>

#include <Magick++.h>

#include <sstream>
#include <iostream>
#include <stdexcept>
#include <limits>

using namespace std;

namespace msat {

template<typename SampleIn, typename SampleOut>
static SampleOut* normalise(ProgressTask& p, SampleIn* data, SampleIn missing, size_t size)
{
	// Compute minimum and maximum values
	p.activity("Finding minimum and maximum sample");
	SampleIn min = missing;
	SampleIn max = missing;
	for (size_t i = 0; i < size; ++i)
	{
		if (data[i] == missing) continue;
		if (min == missing || data[i] < min) min = data[i];
		if (max == missing || data[i] > max) max = data[i];
	}

	// Normalise the values to fit between 0 and 1
	p.activity("Normalising samples");
	SampleOut* res = new SampleOut[size];
	for (size_t i = 0; i < size; ++i)
	{
		if (data[i] == missing)
			res[i] = std::numeric_limits<SampleOut>::min();
		else
			res[i] = static_cast<SampleOut>((data[i] - min) * std::numeric_limits<SampleOut>::max() / max);
	}
	return res;
}

static std::auto_ptr<Magick::Image> imageToMagick(ProgressTask& p, const Image& img)
{
	auto_ptr<Magick::Image> image;
	if (ImageDataWithPixels<unsigned char>* i = dynamic_cast< ImageDataWithPixels<unsigned char>* >(img.data))
	{
		p.activity("Creating image");
		image.reset(new Magick::Image(img.data->columns, img.data->lines, "I", Magick::CharPixel, i->pixels));
		p.activity("Normalising image");
		image->normalize();
	}
	else if (ImageDataWithPixels<unsigned short>* i = dynamic_cast< ImageDataWithPixels<unsigned short>* >(img.data))
	{
		p.activity("Creating image");
		image.reset(new Magick::Image(img.data->columns, img.data->lines, "I", Magick::ShortPixel, i->pixels));
		p.activity("Normalising image");
		image->normalize();
	}
	else if (ImageDataWithPixels<unsigned int>* i = dynamic_cast< ImageDataWithPixels<unsigned int>* >(img.data))
	{
		p.activity("Creating image");
		image.reset(new Magick::Image(img.data->columns, img.data->lines, "I", Magick::IntegerPixel, i->pixels));
		p.activity("Normalising image");
		image->normalize();
	}
#ifdef HAVE_HRIT
	else if (HRITImageData* i = dynamic_cast< HRITImageData* >(img.data))
	{
		p.activity("Creating image");
		unsigned short* pixels = new unsigned short[img.data->columns * img.data->lines];
		for (size_t y = 0; y < img.data->lines; ++y)
			for (size_t x = 0; x < img.data->columns; ++x)
				pixels[y*img.data->columns+x] = i->sample(x, y);
		image.reset(new Magick::Image(img.data->columns, img.data->lines, "I", Magick::ShortPixel, pixels));
		delete[] pixels;
		p.activity("Normalising image");
		image->normalize();
	}
#endif
#if 0
	else if (ImageDataWithPixels<int>* i = dynamic_cast< ImageDataWithPixels<int>* >(img.data))
	{
		// TODO: check if there are negative values; if yes, convert to unsigned by
		// adding abs(smallest value)
		cerr << "int" << endl;
	}
#endif
	else if (ImageDataWithPixels<float>* i = dynamic_cast< ImageDataWithPixels<float>* >(img.data))
	{
		size_t size = img.data->columns * img.data->lines;
		unsigned short* res = normalise<float, unsigned short>(p, i->pixels, i->missing, size);
		p.activity("Creating image");
		image.reset(new Magick::Image(img.data->columns, img.data->lines, "I", Magick::ShortPixel, res));
		delete[] res;
	}
	else if (ImageDataWithPixels<double>* i = dynamic_cast< ImageDataWithPixels<double>* >(img.data))
	{
		size_t size = img.data->columns * img.data->lines;
		unsigned short* res = normalise<double, unsigned short>(p, i->pixels, i->missing, size);
		p.activity("Creating image");
		image.reset(new Magick::Image(img.data->columns, img.data->lines, "I", Magick::ShortPixel, res));
		delete[] res;
	}
	else
		throw std::runtime_error("Trying to create an image from an unknown kind of image data");

	return image;
}

//
// Creates NetCDF product
//
void ExportImage(const Image& img, const std::string& fileName)
{
	ProgressTask p("Exporting image to " + fileName);

	size_t pos = fileName.rfind('.');
	if (pos == string::npos)
		throw std::runtime_error("file name " + fileName + " has no extension, so I cannot guess the file format");

	auto_ptr<Magick::Image> image = imageToMagick(p, img);

#if 0
  if (normalize) image->normalize( );
  if (header[0].segment_id->spectral_channel_id < 12)
  {
    if (do_overlay)
    {
      Magick::Image overlay;
      overlay.read(overlay_image);
      image->composite(overlay, 0, 0, Magick::PlusCompositeOp);
    }
  }

  if (geometry < 1.0)
  {
    Magick::Geometry geom((int) ((float) npix*geometry),
                  (int) ((float) nlin*totalsegs*geometry));
    image->scale(geom);
  }
#endif

	p.activity("Writing image to file");
  image->write(fileName);
}

void DisplayImage(const Image& img)
{
	ProgressTask p("Displaying image " + img.defaultFilename);
	auto_ptr<Magick::Image> image = imageToMagick(p, img);
	p.activity("Displaying image");
	image->display();
}


class ImageExporter : public ImageConsumer
{
	std::string format;

public:
	ImageExporter(const std::string& format) : format(format) {}
	virtual void processImage(auto_ptr<Image> img)
	{
		ExportImage(*img, img->defaultFilename + "." + format);
	}
};

std::auto_ptr<ImageConsumer> createImageExporter(const std::string& format)
{
	return std::auto_ptr<ImageConsumer>(new ImageExporter(format));
}


class ImageDisplayer : public ImageConsumer
{
	virtual void processImage(auto_ptr<Image> img)
	{
		DisplayImage(*img);
	}
};

std::auto_ptr<ImageConsumer> createImageDisplayer()
{
	return std::auto_ptr<ImageConsumer>(new ImageDisplayer());
}

}

// vim:set ts=2 sw=2:
