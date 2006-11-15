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

#include <conv/ImportXRIT.h>
#include <proj/const.h>
#include <proj/Geos.h>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <hrit/MSG_HRIT.h>
#include <glob.h>

#include <conv/Image.tcc>
#include <conv/Progress.h>

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
           + timing + "-" + "?_";

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
	if (pixelSubarea)
	{
		str
			<<      " area: " << AreaLinStart << "," << AreaPixStart
			<<                " + " << AreaNlin << "," << AreaNpix;
	}
	if (coordSubarea)
	{
		str
			<<      " area: lat " << AreaLatMin << ".." << AreaLatMax << " lon " << AreaLonMin << ".." << AreaLonMax;
	}
	return str.str();
}

struct Decoder
{
	/// HRIT HRV parameters used to locate the two image parts
	int LowerEastColumnActual;
	int LowerNorthLineActual;
	int LowerWestColumnActual;
	int UpperEastColumnActual;
	int UpperSouthLineActual;
	int UpperWestColumnActual;

	ProgressTask& p;
	const XRITImportOptions& opts;
	std::vector<string> segnames;
	int seglines;
	int columns;
	int lines;
	size_t npixperseg;
	int bpp;
	MSG_data* data;
	int cur_data;
	bool swapX;
	bool swapY;
	bool hrv;

	Decoder(const XRITImportOptions& opts, Image& img, ProgressTask& p)
		: p(p), opts(opts), seglines(0), columns(0), lines(0), npixperseg(0), data(0), cur_data(-1)
	{
		MSG_header EPI_head;
		MSG_data EPI_data;
		p.activity("Reading epilogue " + opts.epilogueFile());
		std::ifstream hrit(opts.epilogueFile().c_str(), (std::ios::binary | std::ios::in));
		if (hrit.fail())
			throw std::runtime_error("Cannot open input hrit file " + opts.prologueFile());
		EPI_head.read_from(hrit);
		EPI_data.read_from(hrit, EPI_head);
		hrit.close();

		// Subtracting one because they start from 1 instead of 0
		LowerEastColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerEastColumnActual - 1;
		LowerNorthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerNorthLineActual - 1;
		LowerWestColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerWestColumnActual - 1;
		//<< " LSLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerSouthLineActual
		UpperEastColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperEastColumnActual - 1;
		UpperSouthLineActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperSouthLineActual - 1;
		UpperWestColumnActual = EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperWestColumnActual - 1;
		//<< " UNLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperNorthLineActual

#if 0
		cerr << " LSLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.LowerSouthLineActual - 1
			   << " LNLA: " << LowerNorthLineActual
			   << " LECA: " << LowerEastColumnActual
			   << " LWCA: " << LowerWestColumnActual
			   << " USLA: " << UpperSouthLineActual
			   << " UNLA: " << EPI_data.epilogue->product_stats.ActualL15CoverageHRV.UpperNorthLineActual - 1
			   << " UECA: " << UpperEastColumnActual
			   << " UWCA: " << UpperWestColumnActual
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
			if (npixperseg == 0)
			{
				int totalsegs = header.segment_id->planned_end_segment_sequence_number;

				// Decoding informations
				seglines = header.image_structure->number_of_lines;
				columns = header.image_structure->number_of_columns;
				lines = seglines * totalsegs;
				npixperseg = columns * seglines;

				// Image metadata
				img.proj.reset(new proj::Geos(header.image_navigation->subsatellite_longitude, ORBIT_RADIUS));
				img.channel_id = header.segment_id->spectral_channel_id;
				hrv = img.channel_id == MSG_SEVIRI_1_5_HRV;
				img.spacecraft_id = Image::spacecraftIDFromHRIT(header.segment_id->spacecraft_id);
				// See if the image needs to be rotated
				swapX = header.image_navigation->column_scaling_factor < 0;
				swapY = header.image_navigation->line_scaling_factor < 0;
				img.column_factor = abs(header.image_navigation->column_scaling_factor);
				img.line_factor = abs(header.image_navigation->line_scaling_factor);
				if (hrv)
				{
					// Since we are omitting the first (11136-7631) of the rotated image,
					// we need to shift the column offset accordingly
					img.column_offset = 5566 - (11136 - 7630);
					img.line_offset = 5566;
				} else {
					img.column_offset = header.image_navigation->column_offset;
					img.line_offset = header.image_navigation->line_offset;
				}
				img.x0 = 1;
				img.y0 = 1;
				bpp = header.image_structure->number_of_bits_per_pixel;
			}

			int idx = header.segment_id->sequence_number-1;
			if (idx < 0) continue;
			if ((size_t)idx >= segnames.size())
				segnames.resize(idx + 1);
			segnames[idx] = *i;

#if 0
			// Print the data of the holey segment
			if (idx == 17)
			{
				std::ifstream hrit(segnames[idx].c_str(), (std::ios::binary | std::ios::in));
				if (hrit.fail())
					throw std::runtime_error("Cannot open input hrit segment " + segnames[idx]);
				MSG_header header;
				header.read_from(hrit);
				if (header.segment_id->data_field_format == MSG_NO_FORMAT)
					throw std::runtime_error("Product dumped in binary format.");
				MSG_data d;
				d.read_from(hrit, header);
				hrit.close( );
				for (int i = 0; i < npixperseg; ++i)
				{
					cout << " " << d.image->data[i];
					if ((i % columns) == 0)
						cout << endl;
				}
				cout << endl;
			}
#endif
		}

		// Special handling for HRV images
		if (hrv)
		{
			// Widen the image to include both image parts, placed in their right
			// position
			columns += UpperEastColumnActual + 1;
		}
	}

	~Decoder()
	{
		if (data)
			delete data;
	}

	MSG_data* getData(size_t idx)
	{
		if ((int)idx != cur_data)
		{
			// Delete old segment if any
			if (data) delete data;
			data = 0;
			if (idx >= segnames.size()) return 0;
			if (segnames[idx].empty()) return data;

			p.activity("Reading segment " + segnames[idx]);
			std::ifstream hrit(segnames[idx].c_str(), (std::ios::binary | std::ios::in));
			if (hrit.fail())
				throw std::runtime_error("Cannot open input hrit segment " + segnames[idx]);
			MSG_header header;
			header.read_from(hrit);
			if (header.segment_id->data_field_format == MSG_NO_FORMAT)
				throw std::runtime_error("Product dumped in binary format.");
			data = new MSG_data;
			data->read_from(hrit, header);
			hrit.close( );

			cur_data = idx;
		}
		return data;
	}

	MSG_SAMPLE get(size_t x, size_t y)
	{
		//bool debug = x == 3000;
		//bool shift;
		//size_t ypre = y;

		// Rotate if needed
		if (swapX) x = columns - x - 1;
		if (swapY) y = lines - y - 1;

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
				//shift = true;
				x -= UpperEastColumnActual;
			} else {
				if (x < LowerEastColumnActual)
					return 0;
				if (x > LowerWestColumnActual)
					return 0;
				//shift = false;
				x -= LowerEastColumnActual;
			}
			pos = y * (columns - UpperEastColumnActual - 1) + x;
		} else
			pos = y * columns + x;

		// Segment number where is the pixel
		size_t segno = pos / npixperseg;
		MSG_data* d = getData(segno);
		if (d == 0) return 0;

		// Offset of the pixel in the segment
		size_t segoff = pos - (segno * npixperseg);
		//if (debug) cerr << "Ypre: " << ypre << " post: " << x << "," << y << " shift: " << shift << " seg " << segno << " val " << d->image->data[segoff] << endl;
		return d->image->data[segoff];
	}
};

std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts)
{
	opts.ensureComplete();

	ProgressTask p("Reading HRIT from " + opts.toString());

  std::auto_ptr<Image> img(new Image);

	img->quality = opts.resolution[0];
	img->addToHistory("Imported from HRIT " + opts.resolution + ' ' + opts.productid1 + ' ' + opts.productid2 + ' ' + opts.timing);
	if (opts.pixelSubarea)
	{
		std::stringstream str;
		str << "Cropped area: " << opts.AreaLinStart << "," << opts.AreaPixStart << " + " << opts.AreaNlin << "," << opts.AreaNpix;
		img->addToHistory(str.str());
	}
	if (opts.coordSubarea)
	{
		std::stringstream str;
		str << "Cropped area: lat " << opts.AreaLatMin << ".." << opts.AreaLatMax << " lon " << opts.AreaLonMin << ".." << opts.AreaLonMax;
		img->addToHistory(str.str());
	}

	Decoder d(opts, *img, p);

  MSG_header PRO_head;
  MSG_data PRO_data;

	p.activity("Reading prologue " + opts.prologueFile());
	std::ifstream hrit(opts.prologueFile().c_str(), (std::ios::binary | std::ios::in));
	if (hrit.fail())
		throw std::runtime_error("Cannot open input hrit file " + opts.prologueFile());

	PRO_head.read_from(hrit);
	PRO_data.read_from(hrit, PRO_head);

	hrit.close();

	size_t x = 0;
	size_t y = 0;
	size_t width = d.columns;
	size_t height = d.lines;

  // Slice the subarea if needed
  if (opts.pixelSubarea)
	{
		// Slice to the given pixel coordinates
		x = opts.AreaPixStart;
		y = opts.AreaLinStart;
		width = opts.AreaNpix;
		height = opts.AreaNlin;
		std::stringstream str;
		str << "Import limited to " << x << "," << y << " " << width << "x" << height;
		p.activity(str.str());
	}
	if (opts.coordSubarea)
	{
		size_t x1, y1;
		// Convert to pixel coordinates
		img->coordsToPixels(opts.AreaLatMin, opts.AreaLonMin, x, y);
		img->coordsToPixels(opts.AreaLatMax, opts.AreaLonMax, x1, y1);
		// Slice to the bounding box for the 2 coordinates
		width = x > x1 ? x-x1 : x1-x;
		height = y > y1 ? y-y1 : y1-y;
		x = x < x1 ? x : x1;
		y = y < y1 ? y : y1;
		if (x + width >= d.columns) width = d.columns - x - 1;
		if (y + height >= d.lines) height = d.lines - y - 1;
		std::stringstream str;
		str << "Import limited to " << x << "," << y << " " << width << "x" << height;
		p.activity(str.str());
	}

	// Final, calibrated image

  // Get calibration values
  float *cal = PRO_data.prologue->radiometric_proc.get_calibration(
								img->channel_id, d.bpp);

#if 0
	for (int i = 0; i < 256; ++i)
		cerr << "cal["<<i<<"] " << cal[i] << endl;
#endif

	// Perform calibration and copy the data to the result image
	auto_ptr< ImageDataWithPixelsPrescaled<float> > data;
	data.reset(new ImageDataWithPixelsPrescaled<float>(width, height));
	data->missing = 0.0;
	for (size_t iy = 0; iy < height; ++iy)
		for (size_t ix = 0; ix < width; ++ix)
		{
			MSG_SAMPLE sample = d.get(x + ix, y + iy);
			// To avoid spurious data, negative values after calibration are
			// converted to missing values
			if (sample > 0 && cal[sample] >= 0)
				data->pixels[iy*width+ix] = cal[sample];
			else
				data->pixels[iy*width+ix] = data->missingValue;
		}
  delete [ ] cal;

	img->setData(data.release());

	img->x0 += x;
	img->y0 += y;
#if 0
	Column offset and line offset should not be affected by cropping
	img->column_offset -= x;
	img->line_offset -= y;
#endif

	double slope;
	double offset;
  PRO_data.prologue->radiometric_proc.get_slope_offset(
			img->channel_id, slope, offset, img->data->scalesToInt);
  img->data->slope = slope;
  img->data->offset = offset;
	img->data->bpp = d.bpp;

	// Image time
  struct tm *tmtime = PRO_data.prologue->image_acquisition.PlannedAquisitionTime.TrueRepeatCycleStart.get_timestruct( );
  img->year = tmtime->tm_year+1900;
	img->month = tmtime->tm_mon+1;
	img->day = tmtime->tm_mday;
	img->hour = tmtime->tm_hour;
	img->minute = tmtime->tm_min;

  // FIXME: and this? pds.sh = header[0].image_navigation->satellite_h;

  return img;
}

class XRITImageImporter : public ImageImporter
{
	XRITImportOptions opts;

public:
	XRITImageImporter(const std::string& filename) : opts(filename) {}

	virtual void read(ImageConsumer& output)
	{
		if (cropWidth != 0 && cropHeight != 0)
		{
			opts.pixelSubarea = true;
			opts.AreaPixStart = cropX;
			opts.AreaLinStart = cropY;
			opts.AreaNpix = cropWidth;
			opts.AreaNlin = cropHeight;
		} else if (cropLatMin != 1000 && cropLatMax != 1000 && cropLonMin != 1000 && cropLonMax != 1000) {
			opts.coordSubarea = true;
			opts.AreaLatMin = cropLatMin;
			opts.AreaLatMax = cropLatMax;
			opts.AreaLonMin = cropLonMin;
			opts.AreaLonMax = cropLonMax;
		}
		std::auto_ptr<Image> img = importXRIT(opts);
		output.processImage(img);
	}
};

std::auto_ptr<ImageImporter> createXRITImporter(const std::string& filename)
{
	return std::auto_ptr<ImageImporter>(new XRITImageImporter(filename));
}

}

// vim:set ts=2 sw=2:
