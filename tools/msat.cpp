/*
 * Copyright (C) 2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Author: Enrico Zini <enrico@enricozini.com>
 */

#include <gdal.h>
#include <gdal_priv.h>
#include <msat/facts.h>

#include <msat/gdal/const.h>
#include <msat/gdal/gdaltranslate.h>
#include <msat/gdal/dataset.h>

#include "config.h"

#ifdef HAVE_MAGICKPP
#include "image.h"
#endif

#if 0
#include <msat/Progress.h>
#endif
#include <stdexcept>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <iomanip>

#include <cstdlib>
#include <getopt.h>

using namespace std;

void do_help(const char* argv0, ostream& out)
{
        out << "Usage: " << "msat" << " [options] file(s)..." << endl
            << "msat can read and write various formats of satellite images or other" << endl
            << "georeferentiated raster data." << endl
            << "Images can be displayed, cropped and converted from a format to another." << endl
            << endl
            << "Options:" << endl
            << "  --help           Print detailed usage information." << endl
            << "  --version        Print the program version and exit." << endl
            << "  -q, --quiet      Work silently." << endl
            << "  --view           View the contents of a file." << endl
            << "  --viewmore       View the contents of a file, including computed pixel information." << endl
            << "  -c, --conv FMT   Convert to the given format (see gdalinfo --formats for a list)" << endl
            << "  --copymd FILE    Amend the dataset with the metadata from the given file" << endl
#ifdef HAVE_MAGICKPP
            << "  --jpg            Convert to JPEG (with gray scale normalization)." << endl
            << "  --png            Convert to PNG (with gray scale normalization)." << endl
            << "  --display        Display the image on a X11 window." << endl
#endif
            << "  --area='x,dx,y,dy'       Crop the source image(s) to the given area in pixels." << endl
            << "  --Area='latmin,latmax,lonmin,lonmax'  Crop the source image(s) to the given coordinates." << endl
            << "  --around='lat,lon,lath,lonw'          Create an image centered at the given location and with the given width and height." << endl
            << "  --resize='columns,lines' Scale the output image to the given size." << endl
            << "  --resize='xx%,yy%'       Scale the output image by a given percentage." << endl
            << "  -b, --band='idx|name'    Raster band to process. Prefix with '!' to force interpretation as a name. Can be given multiple times." << endl
            << "  --force-calibration      Always calibrate, even if it would result in larger-than-needed output images" << endl
            << endl
            << "Examples:" << endl
            << endl
            << " $ msat --display --Area=30,60,-10,40 file.grb" << endl
            << " $ msat --jpg file.grb" << endl
            << " $ msat --conv MsatGRIB dir/H:MSG1:HRV:200611130800" << endl
            << endl
            << "Report bugs to " << PACKAGE_BUGREPORT << endl;
        ;
}
void usage(char *pname)
{
        cout << pname << ": Convert meteosat image files from and to various formats."
             << endl;
        cout << endl << "Usage:" << endl << "\t"
             << pname << "<output option> file(s)..."
             << endl << endl
             << "Examples: " << endl << "\t" << pname
             << "  " << pname << " --grib Data0.nc SAFNWC_MSG1_CT___04356_051_EUROPE______.h5 hritdir/H:MSG1:HRV:200605031200"
             << endl;
        return;
}

vector<string> split(const std::string& str, char sep)
{
        vector<string> res;

        string::const_iterator s = str.begin();
        for (string::const_iterator i = str.begin(); i != str.end(); ++i)
        {
                if (*i != sep) continue;
                res.push_back(string(s, i));
                s = i + 1;
        }
        if (s != str.end())
                res.push_back(string(s, str.end()));
        return res;
}

static const char* known_md_domains[] = {
        "SUBDATASETS",
        "IMAGE_STRUCTURE",
        "RPC",
        // MD_DOMAIN_MSAT,
        NULL
};

static bool dumpMetadata(GDALMajorObject* o, const char* prefix = "")
{
        // Default domain
        if (char **md = o->GetMetadata())
        {
                cout << prefix << "Metadata:" << endl;
                for (char** s = md; md && *s; ++s)
                        cout << prefix << "  " << *s << endl;
        }
        // Known domains
        for (const char** d = known_md_domains; *d; ++d)
        {
                if (char** md = o->GetMetadata(*d))
                {
                        cout << prefix << "Metadata (" << *d << "):" << endl;
                        for (char** s = md; md && *s; ++s)
                                cout << prefix << "  " << *s << endl;
                }
        }
        return true;
}

const char* gdalTypeName(GDALDataType dt)
{
        switch (dt)
        {
                case GDT_Unknown:  return "GDT_Unknown";
                case GDT_Byte:     return "GDT_Byte";
                case GDT_Int16:    return "GDT_Int16";
                case GDT_UInt16:   return "GDT_UInt16";
                case GDT_Int32:    return "GDT_Int32";
                case GDT_UInt32:   return "GDT_UInt32";
                case GDT_Float32:  return "GDT_Float32";
                case GDT_Float64:  return "GDT_Float64";
                case GDT_CInt16:   return "GDT_CInt16";
                case GDT_CInt32:   return "GDT_CInt32";
                case GDT_CFloat32: return "GDT_CFloat32";
                case GDT_CFloat64: return "GDT_CFloat64";
                default:           return "(unknown GDALDataType)";
        }
}

static bool printRasterBand(GDALRasterBand* band, bool withContents=false, const char* prefix="")
{
        cout << prefix << "Size: " << band->GetXSize() << "x" << band->GetYSize() << endl;
        cout << prefix << "Type: " << gdalTypeName(band->GetRasterDataType()) << endl;
        cout << prefix << "Offset: " << band->GetOffset() << endl;
        cout << prefix << "Scale: " << band->GetScale() << endl;
        cout << prefix << "Unit: " << band->GetUnitType() << endl;
        //cout << "Default filename: " << defaultFilename(*band) << endl;

        if (!dumpMetadata(band, prefix))
                return false;

        double pixmin, pixmax, pixmean, pixstddev;
        CPLErr res = band->GetStatistics(false, withContents,
                &pixmin, &pixmax, &pixmean, &pixstddev);

        if (res == CE_None)
        {
            cout << prefix << "Pixel range: min " << pixmin << " max " << pixmax
                 << " mean " << pixmean << " stddev " << pixstddev << endl;
        }

        return true;
}

static bool printDataset(GDALDataset* ds, bool withContents=false)
{
        cout << "Dataset: " << ds->GetDescription() << endl;
        cout << "Size: " << ds->GetRasterXSize() << "x" << ds->GetRasterYSize() << endl;
        cout << "Projection: " << ds->GetProjectionRef() << endl;

        double geoTransform[6];
        ds->GetGeoTransform(geoTransform);
        cout << "Geotransform matrix: " << endl
                 << "  "
                 << setw(10) << geoTransform[0] << ",\t" << setw(10) << geoTransform[1] << ",\t" << setw(10) << geoTransform[2]
                 << endl
                 << "  "
                 << setw(10) << geoTransform[3] << ",\t" << setw(10) << geoTransform[4] << ",\t" << setw(10) << geoTransform[5]
                 << endl;

        if (!dumpMetadata(ds))
                return false;

#if 0
        // TODO
        GeoReferencer georef(&img);
        double lat, lon;
        georef.pixelToLatlon(0, 0, lat, lon);
        cout << "Coordinates of the top left pixel " << lat << "," << lon << endl;
        georef.pixelToLatlon(ds->GetRasterXSize(), ds->GetRasterYSize(), lat, lon);
        cout << "Coordinates of the bottom right pixel " << lat << "," << lon << endl;
        cout << "Default filename: " << defaultFilename(img) << endl;
#endif

        for (int i = 1; i <= ds->GetRasterCount(); ++i)
        {
                GDALRasterBand* rb = ds->GetRasterBand(i);
                cout << "Raster band "
                     << i << "/" << ds->GetRasterCount()
                     << ": " << rb->GetDescription() << endl;
                if (!printRasterBand(rb, withContents, "  "))
                        return false;
        }

        return true;
}

static void escapeSpacesAndDots(std::string& str)
{
        for (string::iterator i = str.begin(); i != str.end(); ++i)
                if (*i == ' ' || *i == '.')
                        *i = '_';
}

static std::string output_file_name(GDALDataset* ds, GDALRasterBand* rb = NULL)
{
        if (rb == NULL && ds->GetRasterCount() == 1)
                rb = ds->GetRasterBand(1);

        // Get the spacecraft ID
        int spacecraft_id = -1;
        const char* val = ds->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
        if (val != NULL) spacecraft_id = strtoul(val, NULL, 10);

        // Get the spacecraft name
        string spacecraft_name;
        val = ds->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
        if (val != NULL)
                spacecraft_name = val;
        else
                spacecraft_name = "unknown";

        // Get the string describing the sensor
        std::string sensor_name = msat::facts::sensorName(spacecraft_id);

        escapeSpacesAndDots(spacecraft_name);
        escapeSpacesAndDots(sensor_name);

        // Format the date
        string time;
        val = ds->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
        if (val != NULL)
        {
                for (const char* s = val; *s && (s-val) < 16; ++s)
                {
                        if (*s == '-' || *s == ':') continue;
                        if (*s == ' ')
                                time += '_';
                        else
                                time += *s;
                }
        }

        string res;
        // TODO if (i->quality != '_')
        // TODO         res = string() + i->quality + "_";
        res += spacecraft_name + "_" + sensor_name + "_";

        if (rb != NULL)
        {
                int channel_id = -1;
                val = rb->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                if (val != NULL) channel_id = strtoul(val, NULL, 10);

                string channel_name;
                val = rb->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
                if (val != NULL) channel_name = val;

                string channel_level = msat::facts::channelLevel(spacecraft_id, channel_id);

                escapeSpacesAndDots(channel_name);
                escapeSpacesAndDots(channel_level);

                if (!channel_level.empty())
                        res += channel_level + "_";
                res += channel_name + "_";
        }
        res += time;
        return res;
}

void parseBands(GDALTranslate& translate, const std::string& arg)
{

}

enum Action { VIEW, VIEWMORE, CONVERT, JPG, PNG, DISPLAY };

struct Msat
{
    // Defaults to view
    Action action;

    // Dataset transformation
    GDALTranslate translate;
    string scaleX, scaleY;
    double lat[2];
    double lon[2];

    // Output driver
    string outdriver;

    // File to use for copying metadata over
    string mdtemplate;

    // Should we be verbose?
    bool quiet;

    // Input files to process
    vector<string> input_files;

    // Cap to the maximum image size to generate (0 for no cap)
    size_t maxx;
    size_t maxy;

    // List of raster bands to process (indices or names)
    vector<string> band_list;

    bool force_calibration;

    Msat()
        : action(VIEW), quiet(false), force_calibration(false)
    {
        lat[0] = lat[1] = 0;
        lon[0] = lon[1] = 0;
        maxx = maxy = 0;
    }
    void parse_cmdline(int argc, char* argv[]);
    int main();

    void scale_if_needed(GDALDataset& ds)
    {
        // Scale down image to fit in maxx x maxy
        size_t sx = ds.GetRasterXSize();
        size_t sy = ds.GetRasterYSize();
        size_t tx = sx;
        size_t ty = sy;
        if (maxx != 0 && tx > maxx)
        {
                ty = ty * maxx / tx;
                tx = maxx;
        }
        if (maxy != 0 && ty > maxy)
        {
                tx = tx * maxy / ty;
                ty = maxy;
        }
        if (tx != sx && ty != sy)
        {
            cerr << "Note: image scaled down to " << tx << "x" << ty << " to prevent excessive memory usage." << endl;
            cerr << "      you can set one of --area, --Area, --around, or --resize to prevent this." << endl;
            char buf[16];
            snprintf(buf, 16, "%zd", tx); scaleX = buf;
            snprintf(buf, 16, "%zd", ty); scaleY = buf;
            translate.pszOXSize = scaleX.c_str();
            translate.pszOYSize = scaleY.c_str();
        }
    }
};

void Msat::parse_cmdline(int argc, char* argv[])
{
    // Initialise GDAL and run GDAL's command line processing
    GDALAllRegister();
    argc = GDALGeneralCmdLineProcessor(argc, &argv, 0);
    if (argc < 1)
            exit(-argc);

    static struct option longopts[] = {
            { "help", 0, NULL, 'H' },
            { "version",  0, NULL, 'v' },
            { "quiet", 0, NULL, 'q' },
            { "view", 0, NULL, 'V' },
            { "viewmore", 0, NULL, 'D' },
            { "conv", 1, NULL, 'c' },
            { "copymd", 1, NULL, 'M' },
            { "area", 1, 0, 'a' },
            { "Area", 1, 0, 'A' },
            { "around", 1, 0, 'C' },
            { "resize", 1, 0, 'r' },
            { "band", 1, 0, 'b' },
            { "force-calibration", 0, NULL, 'F' },
#ifdef HAVE_MAGICKPP
            { "jpg",  0, NULL, 'j' },
            { "png",  0, NULL, 'p' },
            { "display",  0, NULL, 'd' },
#endif
            { 0, 0, 0, 0 },
    };

    bool done = false;
    bool has_size = false;
    bool is_image = false;
    while (!done) {
            int c = getopt_long(argc, argv, "qRc:b:", longopts, (int*)0);
            switch (c) {
                    case 'H': // --help
                            do_help(argv[0], cout);
                            exit(0);
                    case 'v': // --version
                            cout << "msat version " PACKAGE_VERSION << endl;
                            exit(0);
                    case 'q': // -q,--quiet
                            quiet = true;
                            break;
                    case 'V': // --view
                            action = VIEW;
                            break;
                    case 'D': // --dump
                            action = VIEWMORE;
                            break;
                    case 'c': // --conv
                            action = CONVERT;
                            outdriver = optarg;
                            break;
                    case 'M': // --copymd
                            mdtemplate = optarg;
                            break;
                    case 'a': // --area
                            if (sscanf(optarg, "%d,%d,%d,%d", 
                                    &(translate.anSrcWin[0]),
                                    &(translate.anSrcWin[2]),
                                    &(translate.anSrcWin[1]),
                                    &(translate.anSrcWin[3])) != 4)
                            {
                                    cerr << "Area value should be in the format x,dx,y,dy" << endl;
                                    do_help(argv[0], cerr);
                                    exit(1);
                            }
                            has_size = true;
                            break;
                    case 'A': // --Area
                            if (sscanf(optarg, "%lf,%lf,%lf,%lf", &lat[0], &lat[1], &lon[0], &lon[1]) != 4)
                            {
                                    cerr << "Area value should be in the format latmin,latmax,lonmin,lonmax" << endl;
                                    do_help(argv[0], cerr);
                                    exit(1);
                            }
                            has_size = true;
                            break;
                    case 'C': { // --around
                            double lat, lon, h, w;
                            if (sscanf(optarg, "%lf,%lf,%lf,%lf", &lat,&lon,&h,&w) != 4)
                            {
                                    cerr << "around value should be in the format lat,lon,lath,lonw" << endl;
                                    do_help(argv[0], cerr);
                                    exit(1);
                            }
                            translate.dfLRY = lat - h/2;
                            translate.dfULY = lat + h/2;
                            translate.dfULX = lon - w/2;
                            translate.dfLRX = lon + w/2;
                            has_size = true;
                            break;
                    }
                    case 'r': { // --resize
                            string arg(optarg);
                            size_t pos = arg.find(",");
                            if (pos == string::npos)
                            {
                                    cerr << "size value should be in the format width[%%],height[%%]" << endl;
                                    do_help(argv[0], cerr);
                                    exit(1);
                            }
                            scaleX = arg.substr(0, pos);
                            scaleY = arg.substr(pos+1);
                            translate.pszOXSize = scaleX.c_str();
                            translate.pszOYSize = scaleY.c_str();
                            has_size = true;
                            break;
                    }
                    case 'b': { // --band
                            band_list.push_back(string(optarg));
                            break;
                    }
                    case 'F': // --force-calibration
                        force_calibration = true;
                        break;
#ifdef HAVE_MAGICKPP
                    case 'j': // --jpg
                            action = JPG;
                            is_image = true;
                            break;
                    case 'p': // --png
                            action = PNG;
                            is_image = true;
                            break;
                    case 'd': // --display
                            action = DISPLAY;
                            is_image = true;
                            break;
#endif
                    case -1:
                              done = true;
                              break;
                    default:
                              cerr << "Error parsing commandline." << endl;
                              do_help(argv[0], cerr);
                              exit(1);
            }
    }

    if (optind == argc)
    {
            do_help(argv[0], cerr);
            exit(1);
    }

    for (int i = optind; i < argc; ++i)
        input_files.push_back(argv[i]);

#if 0 // TODO
    if (!quiet)
            Progress::get().setHandler(new StreamProgressHandler(cerr));
#endif

    // If no size was chosen and we are generating images, cap the output image
    // size to avoid exploding in RAM use
    if (is_image && !has_size)
    {
        maxx = 1280;
        maxy = 1280;
    }
}

namespace {
int rbindex_by_name(GDALDataset& ds, const std::string& name)
{
    // Copy raster band metadata from mdds to vds
    for (int i = 1; i <= ds.GetRasterCount(); ++i)
    {
        GDALRasterBand* rb = ds.GetRasterBand(i);
        if (rb == NULL) continue;
        if (rb->GetDescription() == name)
            return i;
    }
    return 0;
}
}

int Msat::main()
{
    for (vector<string>::const_iterator i = input_files.begin(); i != input_files.end(); ++i)
    {
            auto_ptr<GDALDataset> dataset((GDALDataset*)GDALOpen(i->c_str(), GA_ReadOnly));
            if (dataset.get() == NULL)
            {
                    cerr << CPLGetLastErrorMsg() << endl;
                    return 1;
            }

            auto_ptr<GDALDataset> ds_orig;
            if (force_calibration)
            {
                ds_orig = dataset;
                dataset.reset(new msat::dataset::CalibratedDataset(*ds_orig));
            }

            // Create source band list using band_list
            if (!band_list.empty())
            {
                int* bands = (int*)CPLMalloc(band_list.size() * sizeof(int));
                int band_count = 0;
                for (vector<string>::const_iterator bi = band_list.begin();
                        bi != band_list.end(); ++bi)
                {
                    if (bi->empty()) continue;
                    const char* sptr = bi->c_str();
                    char* endptr;
                    unsigned long int idx = strtoul(sptr, &endptr, 10);
                    if (endptr - sptr == (signed)bi->size())
                    {
                        // If it is an integer, use it literally
                        bands[band_count++] = idx;
                    } else {
                        // If it is a string, remove leading bang (if any)
                        // and lookup in raster band descriptions
                        string name = *bi;
                        if (name[0] == '!')
                            name = name.substr(1);
                        idx = rbindex_by_name(*dataset, name);
                        if (idx != 0)
                            bands[band_count++] = idx;
                    }
                }
                // If there are no bands to process in this dataset, move on to the next one
                if (band_count == 0)
                {
                    CPLFree(bands);
                    continue;
                }

                translate.panBandList = bands;
                translate.nBandCount = band_count;
                translate.bDefBands = TRUE;
                if (band_count != dataset->GetRasterCount())
                    translate.bDefBands = FALSE;
                else
                {
                    for (int ci = 0; ci < band_count; ++ci)
                        if (bands[ci] != ci + 1)
                        {
                            translate.bDefBands = FALSE;
                            break;
                        }
                }
            }

            // If --Area is given, we can only resolve it to pixel
            // sizes once we have the dataset
            if (lat[0] != 0 || lat[1] != 0 || lon[0] != 0 || lon[1] != 0)
            {
                    msat::dataset::GeoReferencer gr;
                    if (gr.init(dataset.get()) != CE_None)
                    {
                            cerr << CPLGetLastErrorMsg() << endl;
                            return 1;
                    }
                    int xmin = dataset->GetRasterXSize(),
                        ymin = dataset->GetRasterYSize(),
                        xmax = 0, ymax = 0;
                    for (size_t i = 0; i < 2; ++i)
                            for (size_t j = 0; j < 2; ++j)
                            {
                                    int x, y;
                                    if (gr.latlonToPixel(lat[i], lon[j], x, y) != CE_None)
                                    {
                                            cerr << CPLGetLastErrorMsg() << endl;
                                            return 1;
                                    }
                                    if (x < xmin) xmin = x;
                                    if (x > xmax) xmax = x;
                                    if (y < ymin) ymin = y;
                                    if (y > ymax) ymax = y;
                            }

                    translate.anSrcWin[0] = xmin;
                    translate.anSrcWin[1] = ymin;
                    translate.anSrcWin[2] = xmax - xmin;
                    translate.anSrcWin[3] = ymax - ymin;
            }

            scale_if_needed(*dataset);

            GDALDataset* vds = translate.translate(dataset.get());
            //translate.dump(cerr);

            if (!mdtemplate.empty())
            {
                    auto_ptr<GDALDataset> mdds((GDALDataset*)GDALOpen(mdtemplate.c_str(), GA_ReadOnly));

                    // Copy metadata from mdds to vds
                    vds->SetDescription(mdds->GetDescription());
                    vds->SetMetadata(mdds->GetMetadata());

                    // Copy raster band metadata from mdds to vds
                    for (int i = 1; i <= vds->GetRasterCount(); ++i)
                    {
                            GDALRasterBand* mdr = mdds->GetRasterBand(i);
                            if (mdr != NULL)
                            {
                                    GDALRasterBand* vr = vds->GetRasterBand(i);
                                    vr->SetDescription(mdr->GetDescription());
                                    vr->SetMetadata(mdr->GetMetadata());
                            }
                    }
            }

            switch (action)
            {
                    case VIEW:
                            printDataset(vds, false);
                            break;
                    case VIEWMORE:
                            printDataset(vds, true);
                            break;
                    case CONVERT: {
                            GDALDriverH driver = GDALGetDriverByName(outdriver.c_str());
                            if (driver == NULL)
                            {

                                    cerr << "Driver for \"" << outdriver << "\" not found (see gdalinfo --formats)" << endl;
                                    return 1;
                            }

                            const char* ext = GDALGetMetadataItem(driver, GDAL_DMD_EXTENSION, NULL);

                            string fname = output_file_name(vds);
                            if (ext != NULL)
                            {
                                    fname += ".";
                                    fname += ext;
                            }
                            GDALDatasetH outds = GDALCreateCopy(driver, fname.c_str(), vds,
                                            TRUE, NULL,
                                            GDALDummyProgress, NULL);
                            if (outds == NULL)
                            {
                                    cerr << CPLGetLastErrorMsg() << endl;
                                    return 1;
                            }
                            GDALClose(outds);
                            break;
                    }
#ifdef HAVE_MAGICKPP
                    case JPG:
                            for (int i = 1; i <= vds->GetRasterCount(); ++i)
                            {
                                    GDALRasterBand* rb = vds->GetRasterBand(i);
                                    string fname = output_file_name(vds, rb) + ".jpg";
                                    if (!msat::export_image(rb, fname.c_str()))
                                    {
                                            cerr << CPLGetLastErrorMsg() << endl;
                                            return 1;
                                    }
                            }
                            break;
                    case PNG:
                            for (int i = 1; i <= vds->GetRasterCount(); ++i)
                            {
                                    GDALRasterBand* rb = vds->GetRasterBand(i);
                                    string fname = output_file_name(vds, rb) + ".png";
                                    if (!msat::export_image(rb, fname.c_str()))
                                    {
                                            cerr << CPLGetLastErrorMsg() << endl;
                                            return 1;
                                    }
                            }
                            break;
                    case DISPLAY:
                            for (int i = 1; i <= vds->GetRasterCount(); ++i)
                                    if (!msat::display_image(vds->GetRasterBand(i)))
                                    {
                                            cerr << CPLGetLastErrorMsg() << endl;
                                            return 1;
                                    }
                            break;
#endif
                    default:
                            throw std::runtime_error("unsupported action");
            }

            if (vds != dataset.get())
                    GDALClose( (GDALDatasetH) vds );
    }
    return 0;
}

int main( int argc, char* argv[] )
{
    Msat app;

    app.parse_cmdline(argc, argv);

    try
    {
        return app.main();
    }
    catch (std::exception& e)
    {
            cerr << e.what() << endl;
            return 1;
    }

    return 0;
}
