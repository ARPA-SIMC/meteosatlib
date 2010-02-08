//---------------------------------------------------------------------------
//
//  File        :   ImportXRIT.cpp
//  Description :   Import MSG HRIT format
//  Project     :   Lamma 2004
//  Author      :   Graziano Giuliani (Lamma Regione Toscana)
//                  modified by Enrico Zini (ARPA Emilia Romagna)
//  Source      :   n/a
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

#include "msat/ImportXRIT.h"
#include "msat/ImportUtils.h"
#include "facts.h"
#include "proj/Geos.h"
#include <hrit/MSG_HRIT.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <glob.h>

#include <msat/Image.tcc>
#include <msat/Progress.h>

namespace std {
ostream& operator<<(ostream& out, const msat::HRITImageData::AreaMap& a)
{
	return out << a.x << "," << a.y << " dim: " << a.width << "," << a.height << " start: " << a.startcolumn << ", " << a.startline;
}
}

using namespace std;

#define PATH_SEPARATOR "/"
// For windows use #define PATH_SEPARATOR "\"

// Pad the string 'base' with trailing underscores to ensure it's at least
// final_len characters long
static std::string underscoreit(const std::string& base, int final_len)
{
	string res = base;
	res.resize(final_len, '_');
	return res;
}

namespace msat {

HRITImageData::~HRITImageData()
{
        if (calibration) delete[] calibration;
}

// #define tmprintf(...) fprintf(stderr, __VA_ARGS__)
#define tmprintf(...) do {} while(0)

MSG_SAMPLE HRITImageData::sample(size_t x, size_t y) const
{
	// Shift and cut the area according to cropping
	tmprintf("%zd,%zd -> ", x, y);
	x += cropX;
	y += cropY;

	tmprintf("%zd,%zd -> ", x, y);

	// Absolute position in image data
	size_t rx, ry;
	if (hrvNorth.contains(x, y))
	{
		hrvNorth.remap(x, y, rx, ry);
		tmprintf("(north) -> ");
	} else if (hrvSouth.contains(x, y)) {
		hrvSouth.remap(x, y, rx, ry);
		tmprintf("(south) -> ");
	} else {
		tmprintf("discarded.\n");
		return 0;
	}

	tmprintf("%zd,%zd -> ", rx, ry);

	// Rotate if needed
	if (da.swapX) rx = da.columns - rx - 1;
	if (da.swapY) ry = da.lines - ry - 1;

	size_t pos = ry * da.columns + rx;
	tmprintf("%zd,%zd -> %zd -> ", rx, ry, pos);

	// Segment number where is the pixel
	size_t segno = pos / da.npixperseg;
	tmprintf("seg %zd/%zd=%zd -> ", pos, da.npixperseg, segno);
	MSG_data* d = da.segment(segno);
	if (d == 0)
	{
		tmprintf("discarded.\n");
		return 0;
	}

	// Offset of the pixel in the segment
	size_t segoff = pos - (segno * da.npixperseg);
	tmprintf("segoff %zd -> val %d\n", segoff, d->image->data[segoff]);
	return d->image->data[segoff];
}

float HRITImageData::scaled(int column, int line) const
{
	// Get the wanted sample
	MSG_SAMPLE s = sample(column, line);

	// Perform calibration

	// To avoid spurious data, negative values after calibration are
	// converted to missing values
	if (s > 0 && calibration[s] >= 0)
		return calibration[s];
	else
		return missingValue;
}

int HRITImageData::scaledToInt(int column, int line) const
{
	if (!this->scalesToInt)
		throw std::runtime_error("Image samples cannot be scaled to int");
	return sample(column, line);
}

int HRITImageData::unscaledMissingValue() const
{
	if (!this->scalesToInt)
		throw std::runtime_error("Image samples cannot be scaled to int");
	// HRIT samples have 0 as missing value
	return 0;
}

void HRITImageData::crop(size_t x, size_t y, size_t width, size_t height)
{
	// Virtual cropping: we just limit the area of the image we read
	cropX += x;
	cropY += y;
	columns = width;
	lines = height;
}

ImageData* HRITImageData::createResampled(size_t width, size_t height) const
{
	ImageDataWithPixels<float>* res(new ImageDataWithPixelsPrescaled<float>(width, height));
	res->slope = slope;
	res->offset = offset;
	res->bpp = bpp;
	res->scalesToInt = scalesToInt;
	res->missingValue = missingValue;
	res->missing = missingValue;
	for (size_t y = 0; y < height; ++y)
		for (size_t x = 0; x < height; ++x)
		{
			size_t nx = x * this->columns / width;
			size_t ny = y * this->lines / height;
			res->pixels[y*width+x] = scaled(nx, ny);
		}
	return res;
}

#ifdef EXPERIMENTAL_REPROJECTION
ImageData* HRITImageData::createReprojected(size_t width, size_t height, const Image::PixelMapper& mapper) const
{
	ImageDataWithPixels<float>* res(new ImageDataWithPixelsPrescaled<float>(width, height));
	res->slope = slope;
	res->offset = offset;
	res->bpp = bpp;
	res->scalesToInt = scalesToInt;
	res->missingValue = missingValue;
	res->missing = missingValue;
	for (size_t y = 0; y < height; ++y)
	{
		//cout << "Line " << y << "/" << height << endl;
		for (size_t x = 0; x < width; ++x)
		{
			int nx = 0, ny = 0;
			mapper(x, y, nx, ny);
			//cout << "  map " << x << "," << y << " -> " << nx << "," << ny << endl;
			if (nx >= 0 && ny >= 0 && (unsigned)nx < columns && (unsigned)ny < lines)
				res->pixels[y*width+x] = scaled(nx, ny);
			else
				res->pixels[y*width+x] = missingValue;
		}
	}
	return res;
}
#endif

std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts)
{
	opts.ensureComplete();

	ProgressTask p("Reading HRIT from " + opts.toString());

  std::auto_ptr<Image> img(new Image);

	img->quality = opts.resolution[0];

	auto_ptr<HRITImageData> data(new HRITImageData);

	// Read prologue
  MSG_header PRO_head;
  MSG_data PRO_data;
	p.activity("Reading prologue " + opts.prologueFile());
	data->da.read_file(opts.prologueFile(), PRO_head, PRO_data);

	// Image time
  struct tm *tmtime = PRO_data.prologue->image_acquisition.PlannedAquisitionTime.TrueRepeatCycleStart.get_timestruct( );
  img->year = tmtime->tm_year+1900;
	img->month = tmtime->tm_mon+1;
	img->day = tmtime->tm_mday;
	img->hour = tmtime->tm_hour;
	img->minute = tmtime->tm_min;

  // FIXME: and this? pds.sh = header[0].image_navigation->satellite_h;


	// Read epilogue
	MSG_header EPI_head;
	MSG_data EPI_data;
	p.activity("Reading epilogue " + opts.epilogueFile());
	data->da.read_file(opts.epilogueFile(), EPI_head, EPI_data);

  size_t LowerEastColumnActual;
  size_t LowerSouthLineActual;
  size_t LowerWestColumnActual;
  size_t LowerNorthLineActual;
  size_t UpperEastColumnActual;
  size_t UpperSouthLineActual;
  size_t UpperWestColumnActual;
  size_t UpperNorthLineActual;

	// Subtracting one because they start from 1 instead of 0
	LowerEastColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerEastColumnActual - 1;
	LowerNorthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerNorthLineActual - 1;
	LowerWestColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerWestColumnActual - 1;
	LowerSouthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerSouthLineActual - 1;
	UpperEastColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperEastColumnActual - 1;
	UpperSouthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperSouthLineActual - 1;
	UpperWestColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperWestColumnActual - 1;
	UpperNorthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperNorthLineActual - 1;

#if 0
	data->LowerEastColumnActual = 1;
	data->LowerNorthLineActual = 8064;
	data->LowerWestColumnActual = 5568;
	//" LSLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerSouthLineActual
	data->UpperEastColumnActual = 2064;
	data->UpperSouthLineActual = 8065;
	data->UpperWestColumnActual = 7631;
#endif

#if 0
		cerr << " LSLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerSouthLineActual - 1
			   << " LNLA: " << data->LowerNorthLineActual
			   << " LECA: " << data->LowerEastColumnActual
			   << " LWCA: " << data->LowerWestColumnActual
			   << " USLA: " << data->UpperSouthLineActual
			   << " UNLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperNorthLineActual - 1
			   << " UECA: " << data->UpperEastColumnActual
			   << " UWCA: " << data->UpperWestColumnActual
			   << endl;
#endif

	// Sort the segment names by their index
	vector<string> segfiles = opts.segmentFiles();

	// Scan segment headers
	MSG_header header;
	data->da.scan(segfiles, header);

	// Fill in image information
        img->proj.reset(new proj::Geos(header.image_navigation->subsatellite_longitude, ORBIT_RADIUS));
        img->projWKT = facts::spaceviewWKT(header.image_navigation->subsatellite_longitude);
        img->spacecraft_id = facts::spacecraftIDFromHRIT(header.segment_id->spacecraft_id);
        img->channel_id = header.segment_id->spectral_channel_id;
        data->bpp = header.image_structure->number_of_bits_per_pixel;
	img->unit = facts::channelUnit(img->spacecraft_id, img->channel_id);

        if (data->da.hrv)
        {
                data->hrvNorth.x = 11136 - UpperWestColumnActual - 1;
                data->hrvNorth.y = 11136 - UpperNorthLineActual - 1;
                data->hrvNorth.width = UpperWestColumnActual - UpperEastColumnActual;
                data->hrvNorth.height = UpperNorthLineActual - UpperSouthLineActual;
                data->hrvNorth.startcolumn = 0;
                data->hrvNorth.startline = 0;

                data->hrvSouth.x = 11136 - LowerWestColumnActual - 1;
                data->hrvSouth.y = 11136 - LowerNorthLineActual - 1;
                data->hrvSouth.width = LowerWestColumnActual - LowerEastColumnActual;
                data->hrvSouth.height = LowerNorthLineActual - LowerSouthLineActual;
                data->hrvSouth.startcolumn = 0;
                data->hrvSouth.startline = data->hrvNorth.height - 1;

                // Since we are omitting the first (11136-UpperWestColumnActual) of the
                // rotated image, we need to shift the column offset accordingly
                // FIXME: don't we have a way to compute this from the HRIT data?
                //img->column_offset = 5568 - (11136 - data->UpperWestColumnActual - 1);
                img->column_offset = 5566;
                img->line_offset = 5566;
#if 0
                cerr << "COFF " << header.image_navigation->column_offset << endl;
                cerr << "LOFF " << header.image_navigation->line_offset << endl;
                cerr << "COFF " << header.image_navigation->COFF << endl;
                cerr << "LOFF " << header.image_navigation->LOFF << endl;
                cerr << "cCOFF " << img->column_offset << endl;
                cerr << "cLOFF " << img->line_offset << endl;
#endif
                data->columns = 11136;
                data->lines = 11136;
        } else {
                data->hrvNorth.x = 1856 - header.image_navigation->column_offset;
                data->hrvNorth.y = 1856 - header.image_navigation->line_offset;
                data->hrvNorth.width = UpperWestColumnActual - UpperEastColumnActual;
                data->hrvNorth.height = data->da.lines;
                data->hrvNorth.startcolumn = 0;
                data->hrvNorth.startline = 0;

                img->column_offset = 1856;
                img->line_offset = 1856;

                // img->column_offset = header.image_navigation->column_offset;
                // img->line_offset = header.image_navigation->line_offset;
                data->columns = 3712;
                data->lines = 3712;
        }

        // Horizontal scaling coefficient computed as (2**16)/delta, where delta is
        // size in micro Radians of one pixel
        img->column_res = abs(header.image_navigation->column_scaling_factor) * exp2(-16);
        // Vertical scaling coefficient computed as (2**16)/delta, where delta is the
        // size in micro Radians of one pixel
        img->line_res = abs(header.image_navigation->line_scaling_factor) * exp2(-16);

	double pixelSizeX, pixelSizeY;
	int column_offset, line_offset, x0 = 0, y0 = 0;
	if (data->da.hrv)
	{
		pixelSizeX = 1000 * PRO_data.prologue->image_description.ReferenceGridHRV.ColumnDirGridStep;
		pixelSizeY = 1000 * PRO_data.prologue->image_description.ReferenceGridHRV.LineDirGridStep;

		// Since we are omitting the first (11136-UpperWestColumnActual) of the
		// rotated image, we need to shift the column offset accordingly
		// FIXME: don't we have a way to compute this from the HRIT data?
		//img->column_offset = 5568 - (11136 - data->UpperWestColumnActual - 1);
		column_offset = 5568;
		line_offset = 5568;
	} else {
		pixelSizeX = 1000 * PRO_data.prologue->image_description.ReferenceGridVIS_IR.ColumnDirGridStep;
		pixelSizeY = 1000 * PRO_data.prologue->image_description.ReferenceGridVIS_IR.LineDirGridStep;

		column_offset = 1856;
		line_offset = 1856;
	}
	//img->geotransform[0] = -(band->column_offset - band->x0) * band->column_res;
	//img->geotransform[3] = -(band->line_offset   - band->y0) * band->line_res;
	img->geotransform[0] = -(column_offset - x0) * fabs(pixelSizeX);
	img->geotransform[3] = (line_offset   - y0) * fabs(pixelSizeY);
	//img->geotransform[1] = band->column_res;
	//img->geotransform[5] = band->line_res;
	img->geotransform[1] = fabs(pixelSizeX);
	img->geotransform[5] = -fabs(pixelSizeY);
	img->geotransform[2] = 0.0;
	img->geotransform[4] = 0.0;

	img->x0 = 0;
	img->y0 = 0;




	//cerr << "HRVNORTH " << data->hrvNorth << endl;
	//cerr << "HRVSOUTH " << data->hrvSouth << endl;

	/*
	// Special handling for HRV images
	if (data->hrv)
	{
		// Widen the image to include both image parts, placed in their right
		// position
		data->origColumns += data->UpperEastColumnActual + 1;
	}
	*/

	//data->columns = data->origColumns;
	//data->lines = data->origLines;

  // Get calibration values
  data->calibration = PRO_data.prologue->radiometric_proc.get_calibration(img->channel_id, data->bpp);

	// Get offset and slope
	double slope;
	double offset;
  PRO_data.prologue->radiometric_proc.get_slope_offset(img->channel_id, slope, offset, data->scalesToInt);
  data->slope = slope;
  data->offset = offset;

	img->setData(data.release());

  return img;
}

class XRITImageImporter : public ImageImporter
{
	XRITImportOptions opts;

public:
	XRITImageImporter(const std::string& filename) : opts(filename) {}

	virtual void read(ImageConsumer& output)
	{
		std::auto_ptr<Image> img = importXRIT(opts);
		cropIfNeeded(*img);
		img->defaultFilename = util::satelliteSingleImageFilename(*img);
		img->shortName = util::satelliteSingleImageShortName(*img);
		img->addToHistory("Imported from HRIT " + opts.resolution + ' ' + opts.productid1 + ' ' + opts.productid2 + ' ' + opts.timing);
		output.processImage(img);
	}
};

std::auto_ptr<ImageImporter> createXRITImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new XRITImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
