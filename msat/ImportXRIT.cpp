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
#include "proj/const.h"
#include "proj/Geos.h"
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <hrit/MSG_HRIT.h>
#include <glob.h>

#include <msat/Image.tcc>
#include <msat/Progress.h>

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

// dir/res:prodid1:prodid2:time
bool isXRIT(const std::string& filename)
{
	// check that it contains at least 3 ':' signs
	size_t pos = 0;
	for (int i = 0; i < 3; ++i, ++pos)
		if ((pos = filename.find(':', pos)) == string::npos)
			return false;
	return true;
}

// dir/res:prodid1:prodid2:time
XRITImportOptions::XRITImportOptions(const std::string& filename)
{
	size_t beg;
	size_t end = filename.rfind('/');
	if (end == string::npos)
	{
		directory = ".";
		beg = 0;
	}
	else
	{
		directory = filename.substr(0, end);
		if (directory.size() == 0) directory = "/";
		beg = end + 1;
	}

	if ((end = filename.find(':', beg)) == string::npos)
		throw std::runtime_error("XRIT name " + filename + " is not in the form [directory/]resolution:productid1:productid2:datetime");
	resolution = filename.substr(beg, end-beg);

	beg = end + 1;
	if ((end = filename.find(':', beg)) == string::npos)
		throw std::runtime_error("XRIT name " + filename + " is not in the form [directory/]resolution:productid1:productid2:datetime");
	productid1 = filename.substr(beg, end-beg);

	beg = end + 1;
	if ((end = filename.find(':', beg)) == string::npos)
		throw std::runtime_error("XRIT name " + filename + " is not in the form [directory/]resolution:productid1:productid2:datetime");
	productid2 = filename.substr(beg, end-beg);

	beg = end + 1;
	timing = filename.substr(beg);
}

void XRITImportOptions::ensureComplete() const
{
	if (directory.empty())
		throw std::runtime_error("source directory is missing");
	if (resolution.empty())
		throw std::runtime_error("resolution is missing");
	if (productid1.empty())
		throw std::runtime_error("first product ID is missing");
	if (productid2.empty())
		throw std::runtime_error("second product ID is missing");
	if (timing.empty())
		throw std::runtime_error("timing is missing");
}

std::string XRITImportOptions::prologueFile() const
{
  std::string filename = directory
		       + PATH_SEPARATOR
					 + resolution
		       + "-???-??????-"
					 + underscoreit(productid1, 12) + "-"
					 + underscoreit("_", 9) + "-"
					 + "PRO______-"
					 + timing
					 + "-__";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str(), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

  if (globbuf.gl_pathc > 1)
    throw std::runtime_error("Non univoque prologue file.... Do not trust calibration.");

	string res(globbuf.gl_pathv[1]);
  globfree(&globbuf);
  return res;
}

std::string XRITImportOptions::epilogueFile() const
{
  std::string filename = directory
		       + PATH_SEPARATOR
					 + resolution
		       + "-???-??????-"
					 + underscoreit(productid1, 12) + "-"
					 + underscoreit("_", 9) + "-"
					 + "EPI______-"
					 + timing
					 + "-__";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str(), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

  if (globbuf.gl_pathc > 1)
    throw std::runtime_error("Non univoque prologue file.... Do not trust calibration.");

	string res(globbuf.gl_pathv[1]);
  globfree(&globbuf);
  return res;
}

std::vector<std::string> XRITImportOptions::segmentFiles() const
{
  string filename = directory
					 + PATH_SEPARATOR
					 + resolution
           + "-???-??????-"
           + underscoreit(productid1, 12) + "-"
           + underscoreit(productid2, 9) + "-"
           + "0?????___" + "-"
           + timing + "-" + "C_";

  glob_t globbuf;
  globbuf.gl_offs = 1;

  if ((glob(filename.c_str( ), GLOB_DOOFFS, NULL, &globbuf)) != 0)
    throw std::runtime_error("No such file(s)");

	std::vector<std::string> res;
	for (size_t i = 0; i < globbuf.gl_pathc; ++i)
		res.push_back(globbuf.gl_pathv[i+1]);
  globfree(&globbuf);
	return res;
}

std::string XRITImportOptions::toString() const
{
	std::stringstream str;
	str <<        "dir: " << directory
		  <<       " res: " << resolution
			<<     " prod1: " << productid1
			<<     " prod2: " << productid2
			<<      " time: " << timing;
	return str.str();
}

HRITImageData::~HRITImageData()
{
	// Delete the segment cache
	//if (m_segment) delete m_segment;
	for (std::deque<scache>::iterator i = segcache.begin();
			 i != segcache.end(); ++i)
		delete i->segment;
	if (calibration) delete[] calibration;
}

MSG_data* HRITImageData::segment(size_t idx) const
{
	// Check to see if the segment we need is the current one
	if (segcache.empty() || segcache.begin()->segno != idx)
	{
		// If not, check to see if we can find the segment in the cache
		std::deque<scache>::iterator i = segcache.begin();
		for ( ; i != segcache.end(); ++i)
			if (i->segno == idx)
				break;
		if (i == segcache.end())
		{
			// Not in cache: we need to load it

			// Do not load missing segments
			if (idx >= segnames.size()) return 0;
			if (segnames[idx].empty()) return 0;

			// Remove the last recently used if the cache is full
			if (segcache.size() == 2)
			{
				delete segcache.rbegin()->segment;
				segcache.pop_back();
			}

			// Load the segment
			ProgressTask p("Reading segment " + segnames[idx]);
			std::ifstream hrit(segnames[idx].c_str(), (std::ios::binary | std::ios::in));
			if (hrit.fail())
				throw std::runtime_error("Cannot open input hrit segment " + segnames[idx]);
			MSG_header header;
			header.read_from(hrit);
			if (header.segment_id->data_field_format == MSG_NO_FORMAT)
				throw std::runtime_error("Product dumped in binary format.");
			scache new_scache;
			new_scache.segment = new MSG_data;
			new_scache.segment->read_from(hrit, header);
			new_scache.segno = idx;
			hrit.close();

			// Put it in the front
			segcache.push_front(new_scache);
		} else {
			// The segment is in the cache: bring it to the front
			scache tmp = *i;
			segcache.erase(i);
			segcache.push_front(tmp);
		}
	}
	return segcache.begin()->segment;
}

MSG_SAMPLE HRITImageData::sample(size_t x, size_t y) const
{
	// Shift and cut the area according to cropping
	if (x >= columns) return 0;
	if (y >= lines) return 0;
	x += cropX;
	y += cropY;

	// Rotate if needed
	if (swapX) x = origColumns - x - 1;
	if (swapY) y = origLines - y - 1;

	// Absolute position in image data
	size_t pos;
	if (hrv)
	{
		// Check if we are in the shifted HRV upper area
		// FIXME: the '-1' should not be there, but if I take it out I see one
		//        line badly offset
		if (y >= UpperSouthLineActual - 1)
		{
			if (x < UpperEastColumnActual)
				return 0;
			if (x > UpperWestColumnActual)
				return 0;
			x -= UpperEastColumnActual;
		} else {
			if (x < LowerEastColumnActual)
				return 0;
			if (x > LowerWestColumnActual)
				return 0;
			x -= LowerEastColumnActual;
		}
		//x -= 6;
		//y -= 2;
		pos = y * (origColumns - UpperEastColumnActual - 1) + x;
	} else
		pos = y * origColumns + x;

	// Segment number where is the pixel
	size_t segno = pos / npixperseg;
	MSG_data* d = segment(segno);
	if (d == 0) return 0;

	// Offset of the pixel in the segment
	size_t segoff = pos - (segno * npixperseg);
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
	std::ifstream hrit(opts.prologueFile().c_str(), (std::ios::binary | std::ios::in));
	if (hrit.fail())
		throw std::runtime_error("Cannot open input hrit file " + opts.prologueFile());
	PRO_head.read_from(hrit);
	PRO_data.read_from(hrit, PRO_head);
	hrit.close();

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
	hrit.open(opts.epilogueFile().c_str(), (std::ios::binary | std::ios::in));
	if (hrit.fail())
		throw std::runtime_error("Cannot open input hrit file " + opts.prologueFile());
	EPI_head.read_from(hrit);
	EPI_data.read_from(hrit, EPI_head);
	hrit.close();

	// Subtracting one because they start from 1 instead of 0
	data->LowerEastColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerEastColumnActual - 1;
	data->LowerNorthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerNorthLineActual - 1;
	data->LowerWestColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerWestColumnActual - 1;
	//" LSLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerSouthLineActual
	data->UpperEastColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperEastColumnActual - 1;
	data->UpperSouthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperSouthLineActual - 1;
	data->UpperWestColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperWestColumnActual - 1;
	//" UNLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperNorthLineActual

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
	for (vector<string>::const_iterator i = segfiles.begin();
				i != segfiles.end(); ++i)
	{
		p.activity("Scanning segment " + *i);
		std::ifstream hrit(i->c_str(), (std::ios::binary | std::ios::in));
		if (hrit.fail())
			throw std::runtime_error("Cannot open input hrit segment " + *i);
		MSG_header header;
		header.read_from(hrit);
		hrit.close( );

		if (header.segment_id->data_field_format == MSG_NO_FORMAT)
			throw std::runtime_error("Product dumped in binary format.");

		// Read common info just once from a random segment
		if (data->npixperseg == 0)
		{
			// Decoding information
			int totalsegs = header.segment_id->planned_end_segment_sequence_number;
			int seglines = header.image_structure->number_of_lines;
#if 0
			cerr << "NCOL " << header.image_structure->number_of_columns << endl; 
			cerr << "NLIN " << header.image_structure->number_of_lines << endl; 
#endif
			data->origColumns = header.image_structure->number_of_columns;
			data->origLines = seglines * totalsegs;
			data->npixperseg = data->origColumns * seglines;

			// Image metadata
			img->proj.reset(new proj::Geos(header.image_navigation->subsatellite_longitude, ORBIT_RADIUS));
			img->channel_id = header.segment_id->spectral_channel_id;
			data->hrv = img->channel_id == MSG_SEVIRI_1_5_HRV;
			img->spacecraft_id = Image::spacecraftIDFromHRIT(header.segment_id->spacecraft_id);
			img->unit = Image::channelUnit(header.segment_id->spacecraft_id, img->channel_id);

			// See if the image needs to be rotated
			data->swapX = header.image_navigation->column_scaling_factor < 0;
			data->swapY = header.image_navigation->line_scaling_factor < 0;
		  // Horizontal scaling coefficient computed as (2**16)/delta, where delta is
		  // size in micro Radians of one pixel
			img->column_res = abs(header.image_navigation->column_scaling_factor) * exp2(-16);
	    // Vertical scaling coefficient computed as (2**16)/delta, where delta is the
	    // size in micro Radians of one pixel
			img->line_res = abs(header.image_navigation->line_scaling_factor) * exp2(-16);
			if (data->hrv)
			{
				// Since we are omitting the first (11136-UpperWestColumnActual) of the
				// rotated image, we need to shift the column offset accordingly
				// FIXME: don't we have a way to compute this from the HRIT data?
				img->column_offset = 5568 - (11136 - data->UpperWestColumnActual - 1);
				img->line_offset = 5568;
#if 0
				cerr << "COFF " << header.image_navigation->column_offset << endl;
				cerr << "LOFF " << header.image_navigation->line_offset << endl;
				cerr << "COFF " << header.image_navigation->COFF << endl;
				cerr << "LOFF " << header.image_navigation->LOFF << endl;
				cerr << "cCOFF " << img->column_offset << endl;
				cerr << "cLOFF " << img->line_offset << endl;
#endif
			} else {
				img->column_offset = header.image_navigation->column_offset;
				img->line_offset = header.image_navigation->line_offset;
			}
			img->x0 = 1;
			img->y0 = 1;
			data->bpp = header.image_structure->number_of_bits_per_pixel;
		}

		int idx = header.segment_id->sequence_number-1;
		if (idx < 0) continue;
		if ((size_t)idx >= data->segnames.size())
			data->segnames.resize(idx + 1);
		data->segnames[idx] = *i;
	}

	// Special handling for HRV images
	if (data->hrv)
	{
		// Widen the image to include both image parts, placed in their right
		// position
		data->origColumns += data->UpperEastColumnActual + 1;
	}

	data->columns = data->origColumns;
	data->lines = data->origLines;

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
