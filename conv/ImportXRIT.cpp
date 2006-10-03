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
#include <stdexcept>
#include <fstream>
#include <hrit/MSG_HRIT.h>
#include <glob.h>

#include <conv/Image.tcc>

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
	for (int i = 0; i < globbuf.gl_pathc; ++i)
		res.push_back(globbuf.gl_pathv[i+1]);
  globfree(&globbuf);
	return res;
}

struct Decoder
{
	struct Segment
	{
		std::string file;
		MSG_header header;
		MSG_data msgdat;
		bool loaded;

		Segment(const std::string& file) : file(file), loaded(false)
		{
			std::ifstream hrit(file.c_str(), (std::ios::binary | std::ios::in));
			if (hrit.fail())
				throw std::runtime_error("Cannot open input hrit segment " + file);
			header.read_from(hrit);
			hrit.close( );

			if (header.segment_id->data_field_format == MSG_NO_FORMAT)
				throw std::runtime_error("Product dumped in binary format.");
		}

		int index() const { header.segment_id->sequence_number-1; }

		MSG_data& data()
		{
			// Load segment data only on demand
			if (!loaded)
			{
				std::ifstream hrit(file.c_str(), (std::ios::binary | std::ios::in));
				if (hrit.fail())
					throw std::runtime_error("Cannot open input hrit segment " + file);
				header.read_from(hrit);
				msgdat.read_from(hrit, header);
				hrit.close( );
				loaded = true;
			}
			return msgdat;
		}
	};

	const XRITImportOptions& opts;
  MSG_header PRO_head;
  MSG_data PRO_data;
	std::vector<Segment*> segments;
	Segment* anySegment;
	int seglines;
	int columns;
	int lines;
	size_t npixperseg;

	Decoder(const XRITImportOptions& opts)
		: opts(opts), anySegment(0), seglines(0), columns(0), lines(0), npixperseg(0)
	{
		std::ifstream hrit(opts.prologueFile().c_str(), (std::ios::binary | std::ios::in));
		if (hrit.fail())
			throw std::runtime_error("Cannot open input hrit file " + opts.prologueFile());

		PRO_head.read_from(hrit);
		PRO_data.read_from(hrit, PRO_head);

		hrit.close();
		//std::cout << PRO_head;

		// Read the segment headers
		vector<string> segfiles = opts.segmentFiles();
		for (vector<string>::const_iterator i = segfiles.begin();
					i != segfiles.end(); ++i)
		{
			Segment* s = new Segment(*i);
			if (anySegment == 0) anySegment = s;
			if (s->index() >= segments.size())
				segments.resize(s->index()+1, 0);
			segments[s->index()] = s;
		}

		int totalsegs = anySegment->header.segment_id->planned_end_segment_sequence_number;

		// Number of lines per segment
		seglines = anySegment->header.image_structure->number_of_lines;
		columns = anySegment->header.image_structure->number_of_columns;
		lines = seglines * totalsegs;
		npixperseg = columns * seglines;
	}

	~Decoder()
	{
		for (vector<Segment*>::iterator i = segments.begin();
					i != segments.end(); ++i)
			if (*i)
				delete *i;
	}

	MSG_header& anyHeader() const { return anySegment->header; }

	MSG_SAMPLE get(size_t x, size_t y)
	{
		// Rotate by 180deg
		x = columns - x;
		y = lines - y;

		// Absolute position in image data
		size_t pos = y * columns + x;

		// Segment number where is the pixel
		size_t segno = pos / npixperseg;

		Segment* s = segments[segno];
		if (s == 0) return 0;

		// Offset of the pixel in the segment
		size_t segoff = pos - (segno * npixperseg);
		return s->data().image->data[segoff];
	}
};

std::auto_ptr<Image> importXRIT(const XRITImportOptions& opts)
{
	opts.ensureComplete();

	Decoder h(opts);

#if 0
  for (int i = 0; i < segments.size(); ++i)
  {
    std::ifstream hrit(segments[i].c_str(), (std::ios::binary | std::ios::in));
    if (hrit.fail())
			throw std::runtime_error("Cannot open input hrit file " + segments[i]);
    header[i].read_from(hrit);
    msgdat[i].read_from(hrit, header[i]);
    hrit.close( );
    // std::cout << header[i];
  }
#endif

#if 0
	// Perform image rotation and cropping on the uncalibrated data
	ImageDataWithPixels<MSG_SAMPLE> work_img;

  // Assemble all the segments together
	size_t total_size = work_img.columns * work_img.lines;
  work_img.pixels = new MSG_SAMPLE[total_size];
  bzero(work_img.pixels, total_size * sizeof(MSG_SAMPLE));
  size_t pos = 0;
  for (int i = 0; i < totalsegs; i ++)
  {
    if (segsindexes[i] >= 0)
			memcpy(work_img.pixels+pos, msgdat[segsindexes[i]].image->data,
					npixperseg*sizeof(MSG_SAMPLE));
		pos += npixperseg;
  }
#endif

	int x = 0;
	int y = 0;
	int width = h.columns;
	int height = h.lines;

  // Slice the subarea if needed
  if (opts.subarea)
	{
		x = opts.AreaPixStart;
		y = opts.AreaLinStart;
		width = opts.AreaNpix;
		height = opts.AreaNlin;
	}

	// Final, calibrated image

  // Get calibration values
  float *cal = h.PRO_data.prologue->radiometric_proc.get_calibration(
								h.anyHeader().segment_id->spectral_channel_id,
								h.anyHeader().image_structure->number_of_bits_per_pixel);
	float base;
	switch (h.anyHeader().segment_id->spectral_channel_id)
	{
  	case MSG_SEVIRI_1_5_VIS_0_6:
  	case MSG_SEVIRI_1_5_VIS_0_8:
  	case MSG_SEVIRI_1_5_HRV:
			base = 0.0;
			break;
		default:
			base = 145.0;
			break;
	}
	// Perform calibration and copy the data to the result image
	auto_ptr< ImageDataWithPixels<float> > data(new ImageDataWithPixels<float>(width, height));
	for (size_t iy = 0; iy < height; ++iy)
		for (size_t ix = 0; ix < width; ++ix)
		{
			MSG_SAMPLE sample = h.get(x + ix, y + iy);
			if (sample > 0)
				data->pixels[iy*width+ix] = cal[sample] - base;
			else
				data->pixels[iy*width+ix] = 0.0;
		}
  delete [ ] cal;

  std::auto_ptr<Image> img(new Image);
	img->setData(data.release());

	// TODO
#if 0
  char *channelstring = strdup(MSG_channel_name((t_enum_MSG_spacecraft) pds.spc,
                               pds.chn).c_str( ));
  char *channel = chname(channelstring, strlen(channelstring) + 1);

  // Build up output Grib file name and open it
  sprintf( GribName, "%s_%4d%02d%02d_%02d%02d.grb", channel,
           tmtime->tm_year + 1900, tmtime->tm_mon + 1, tmtime->tm_mday,
	   tmtime->tm_hour, tmtime->tm_min );
#endif
	//img->name = "" /* TODO */;

#if 0
  MSG_data_level_15_header *p = PRO_data->prologue;
  pds.cal_offset = p->radiometric_proc.ImageCalibration[pds.chn-1].Cal_Offset;
  pds.cal_slope = p->radiometric_proc.ImageCalibration[pds.chn-1].Cal_Slope;
#endif
  img->data->slope = 1 /* TODO */;
  img->data->offset = 0 /* TODO */;

	// Image time
  struct tm *tmtime = h.PRO_data.prologue->image_acquisition.PlannedAquisitionTime.TrueRepeatCycleStart.get_timestruct( );
  img->year = tmtime->tm_year+1900;
	img->month = tmtime->tm_mon+1;
	img->day = tmtime->tm_mday;
	img->hour = tmtime->tm_hour;
	img->minute = tmtime->tm_min;



#if 0
  g.set_size(AreaNpix, AreaNlin);
  g.set_earth_spheroid( );
  g.is_dirincgiven = true;
  // Earth Equatorial Radius is 6378.160 Km (IAU 1965)
  g.set_spaceview(0.0, pds.sublon, SEVIRI_DX, SEVIRI_DY,
                  SP_X0, SP_Y0, SEVIRI_ORIENTATION,
                  SEVIRI_CAMERA_H, AreaPixStart+1, AreaLinStart+1);
#endif
	img->sublon = h.anyHeader().image_navigation->subsatellite_longitude;


  img->channel_id = (unsigned char ) h.anyHeader().segment_id->spectral_channel_id;
  img->spacecraft_id = h.anyHeader().segment_id->spacecraft_id;
  img->column_factor = h.anyHeader().image_navigation->column_scaling_factor;
  img->line_factor = h.anyHeader().image_navigation->line_scaling_factor;
  img->column_offset = h.anyHeader().image_navigation->column_offset + x;
  img->line_offset = h.anyHeader().image_navigation->line_offset + y;

  // FIXME: and this? pds.sublon = header[0].image_navigation->subsatellite_longitude;
  // FIXME: and this? pds.sh = header[0].image_navigation->satellite_h;

	/* FIXME: and this?
  int fakechan = 0;
  float abase = 145.0;
  if (pds.chn == MSG_SEVIRI_1_5_VIS_0_6) { fakechan = 10; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_VIS_0_8) { fakechan = 11; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_HRV)     { fakechan = 12; abase = 0.0; }
  if (pds.chn == MSG_SEVIRI_1_5_IR_1_6)   fakechan = 0;
  if (pds.chn == MSG_SEVIRI_1_5_IR_3_9)   fakechan = 1;
  if (pds.chn == MSG_SEVIRI_1_5_IR_8_7)   fakechan = 2;
  if (pds.chn == MSG_SEVIRI_1_5_IR_9_7)   fakechan = 3;
  if (pds.chn == MSG_SEVIRI_1_5_IR_10_8)  fakechan = 4;
  if (pds.chn == MSG_SEVIRI_1_5_IR_12_0)  fakechan = 5;
  if (pds.chn == MSG_SEVIRI_1_5_IR_13_4)  fakechan = 6;
  if (pds.chn == MSG_SEVIRI_1_5_WV_6_2)   fakechan = 20;
  if (pds.chn == MSG_SEVIRI_1_5_WV_7_3)   fakechan = 21;
  fakechan = pds.chn;
	*/
  return img;
}

class XRITImageImporter : public ImageImporter
{
	XRITImportOptions opts;

public:
	XRITImageImporter(const std::string& filename) : opts(filename) {}

	virtual void read(ImageConsumer& output)
	{
		if (shouldCrop())
		{
			opts.subarea = true;
			opts.AreaPixStart = cropX;
			opts.AreaLinStart = cropY;
			opts.AreaNpix = cropWidth;
			opts.AreaNlin = cropHeight;
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
