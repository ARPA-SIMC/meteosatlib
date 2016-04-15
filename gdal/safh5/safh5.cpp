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

#include "safh5.h"
#include <msat/gdal/const.h>
#include <msat/gdal/dataset.h>
#include "utils.h"
#include <msat/facts.h>
#include <gdal.h>
#include <gdal_priv.h>

#include <string>
#include <limits>
#include <cstdio>
#include <memory>
#include <math.h>

using namespace std;

namespace msat {
namespace safh5 {

class SAFH5Dataset : public GDALDataset
{
public:
    mh5::File h5;
    mh5::Group group;
    string projWKT;
    int spacecraft_id;

    SAFH5Dataset(mh5::File& h5) : h5(h5) {}
    ~SAFH5Dataset() {}

        virtual bool init();

        virtual const char* GetProjectionRef()
        {
                return projWKT.c_str();
        }

        virtual CPLErr GetGeoTransform(double* tr)
        {
                // SAF COFF and LOFF represent the distance in pixels from the top-left
                // cropped image point to the subsatellite point, increasing with increasing
                // latitudes and increasing longitudes
                const int column_offset = 1856;
                const int line_offset = 1856;
                const int x0 = 1856 - group.attr("COFF").as_int() + 1;
                const int y0 = 1856 - group.attr("LOFF").as_int() + 1;

                // Compute geotransform matrix

                // Only valid for non-HRV
                const double psx = facts::pixelHSizeFromCFAC(abs(group.attr("CFAC").as_int()) * exp2(-16));
                const double psy = facts::pixelVSizeFromLFAC(abs(group.attr("LFAC").as_int()) * exp2(-16));
#if 0
                if (readIntAttribute(group, "CFAC") != 13642337)
                        throw std::runtime_error("CFAC attribute is not 13642337");
                if (readIntAttribute(group, "LFAC") != 13642337)
                        throw std::runtime_error("CFAC attribute is not 13642337");
#endif

                tr[0] = -(column_offset - x0) * psx;
                tr[3] = (line_offset   - y0) * psy;
                tr[1] = psx;
                tr[5] = -psy;
                tr[2] = 0.0;
                tr[4] = 0.0;
                return CE_None;
        }
};

class SAFH5RasterBand : public GDALRasterBand
{
public:
    mh5::Dataset dataset;

    SAFH5RasterBand(SAFH5Dataset* ds, int idx, mh5::Dataset dataset)
            : dataset(dataset)
    {
        poDS = ds;
        nBand = idx;
    }

        bool init(SAFH5Dataset* hds, const std::string& name)
        {
                nBlockXSize = hds->GetRasterXSize();
                nBlockYSize = hds->GetRasterYSize();

                // Set name
                SetDescription(name.c_str());

                /// Channel
                int channel_id;
                if (SAFChannelInfo* ci = SAFChannelByName(name))
                        channel_id = ci->channelID;
                else
                        channel_id = hds->group.attr("SPECTRAL_CHANNEL_ID").as_int();
                char buf[25];
                snprintf(buf, 25, "%d", channel_id);
                SetMetadataItem(MD_MSAT_CHANNEL_ID, buf, MD_DOMAIN_MSAT);
                SetMetadataItem(MD_MSAT_CHANNEL,
                                facts::channelName(hds->spacecraft_id, channel_id),
                                MD_DOMAIN_MSAT);

                // Read image data
                switch (dataset.datatype().get_size())
                {
                        case 1: eDataType = GDT_Byte; break;
                        case 2: eDataType = GDT_UInt16; break;
                        case 4: eDataType = GDT_UInt32; break;
                        default:
                                CPLError(CE_Failure, CPLE_AppDefined, "Unsupported sample data size %zd in %s", dataset.datatype().get_size(), name.c_str());
                                return false;
                }

                return true;
        }

        virtual const char* GetUnitType()
        {
                return "NUMERIC";
        }

        virtual CPLErr IReadBlock(int xblock, int yblock, void *buf)
        {
                if (xblock != 0 || yblock != 0)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Invalid block number");
                        return CE_Failure;
                }

                // TODO: see if/how we can do partial (segmented) reads efficiently
#if 0
                ProgressTask p1(string("Reading NetCDF variable ") + var->name());
                img->defaultFilename = util::satelliteSingleImageFilename(*img);
                img->shortName = util::satelliteSingleImageShortName(*img);
                img->addToHistory("Imported from NetCDF " + img->defaultFilename);
                cropIfNeeded(*img);
                output.processImage(img);
#endif
                size_t size = dataset.npoints();
                if ((int)size != nBlockXSize * nBlockYSize)
                {
                        CPLError(CE_Failure, CPLE_AppDefined, "Image declares %d samples but has %d instead", nBlockXSize * nBlockYSize, (int)size);
                        return CE_Failure;
                }

                switch (eDataType)
                {
                        case GDT_Byte:   dataset.read((uint8_t*)buf); break;
                        case GDT_UInt16: dataset.read((uint16_t*)buf); break;
                        case GDT_UInt32: dataset.read((uint32_t*)buf); break;
                        default:
                                CPLError(CE_Failure, CPLE_AppDefined, "Unsupported data type %d", (int)eDataType);
                                return CE_Failure;
                }

                return CE_None;
        }

        virtual double GetOffset(int* pbSuccess=NULL)
        {
                if (pbSuccess) *pbSuccess = TRUE;
                return dataset.attr("OFFSET").as_float();
        }

        virtual double GetScale(int* pbSuccess=NULL)
        {
                if (pbSuccess) *pbSuccess = TRUE;
                return dataset.attr("SCALING_FACTOR").as_float();
        }

        virtual double GetNoDataValue(int* pbSuccess=NULL)
        {
                if (pbSuccess) *pbSuccess = TRUE;
                switch (eDataType)
                {
                        case GDT_Byte:   return std::numeric_limits<uint8_t>::max();
                        case GDT_UInt16: return std::numeric_limits<uint16_t>::max();
                        case GDT_UInt32: return std::numeric_limits<uint32_t>::max();
                        default:
                                if (pbSuccess) *pbSuccess = FALSE;
                                return 0.0;
                }
        }
};

bool SAFH5Dataset::init()
{
        char buf[25];

        group = h5.group("/");
        nRasterXSize = group.attr("NC").as_int();
        nRasterYSize = group.attr("NL").as_int();

        // Get the group name
        std::string groupName = group.attr("PRODUCT_NAME").as_string();
        // Trim trailing '_' characters
        while (groupName.size() > 0 && groupName[groupName.size() - 1] == '_')
                groupName.resize(groupName.size() - 1);


        /// Spacecraft
        spacecraft_id = facts::spacecraftIDFromHRIT(group.attr("GP_SC_ID").as_int());
        snprintf(buf, 25, "%d", spacecraft_id);
        if (SetMetadataItem(MD_MSAT_SPACECRAFT_ID, buf, MD_DOMAIN_MSAT) != CE_None)
                return false;
        string spacecraft_name = facts::spacecraftName(spacecraft_id);
        if (SetMetadataItem(MD_MSAT_SPACECRAFT, spacecraft_name.c_str(), MD_DOMAIN_MSAT) != CE_None)
                return false;


        /// Image time
        std::string datetime = group.attr("IMAGE_ACQUISITION_TIME").as_string();
        int year, month, day, hour, minute;
        if (sscanf(datetime.c_str(), "%04d%02d%02d%02d%02d", &year, &month, &day, &hour, &minute) != 5)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "Unable to parse datetime %s", datetime.c_str());
                return false;
        }
        snprintf(buf, 20, "%04d-%02d-%02d %02d:%02d:00", year, month, day, hour, minute);
        if (SetMetadataItem(MD_MSAT_DATETIME, buf, MD_DOMAIN_MSAT) != CE_None)
                return false;


        /// Projection
        string proj = group.attr("PROJECTION_NAME").as_string();
        if (proj.size() < 8)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "projection name '%s' is too short to contain subsatellite longitude", proj.c_str());
                return false;
        }
        const char* s = proj.c_str() + 6;
        // skip initial zeros
        while (*(s+1) && *(s+1) == '0') ++s;
        double sublon;
        if (sscanf(s, "%lf", &sublon) != 1)
        {
                CPLError(CE_Failure, CPLE_AppDefined, "cannot read subsatellite longitude");
                return false;
        }
        projWKT = dataset::spaceviewWKT(sublon);


        // Datasets
        int next_rb = 1;
        for (hsize_t i = 0; i < group.get_num_objs(); ++i)
        {
                string name = group.get_objname_by_idx(i);
                mh5::Dataset dataset = group.dataset(name);
                string c = dataset.attr("CLASS").as_string();
                if (c != "IMAGE") continue;

                auto_ptr<SAFH5RasterBand> rb(new SAFH5RasterBand(this, next_rb, dataset));
                rb->init(this, name);
                SetBand(next_rb, rb.release());
                ++next_rb;
        }


#if 0
        // Consistency checks

        // Check that slope, offset and bpp match the ones that we have in
        // Utils, otherwise warning that the conversion can be irreversible
        if (ci == NULL)
                cerr << "Warning: unknown channel informations for product " << name << endl;
        else {
                if (ci->slope != img->data->slope)
                        cerr << "Warning: slope for image (" << img->data->slope << ") is different from the usual one (" << ci->slope << ")" << endl;
                if (ci->offset != img->data->offset)
                        cerr << "Warning: offset for image (" << img->data->offset << ") is different from the usual one (" << ci->offset << ")" << endl;
                if (ci->bpp < img->data->bpp)
                        cerr << "Warning: bpp for image (" << img->data->bpp << ") is more than the usual one (" << ci->bpp << ")" << endl;
        }


        // Output file name should be SAF_{REGION_NAME}_{nome dataset}_{date}.*
        string regionName;
        try {
                regionName = readStringAttribute(group, "REGION_NAME");
        } catch (...) {
                regionName = "unknown";
        }
        char datestring[15];
        snprintf(datestring, 14, "%04d%02d%02d_%02d%02d", img->year, img->month, img->day, img->hour, img->minute);
  img->defaultFilename = "SAF_" + regionName + "_" + name + "_" + datestring;

#endif



#if 0
        virtual int getOriginalBpp()
        {
                #if 0
                if (originalBPP == -1)
                {
                        if (scalesToInt())
                        {
                                T* buf = 0;
                                try {
                                        buf = new T[nBlockXSize * nBlockYSize];
                                        IReadBlock(0, 0, buf);
                                        // Compute the maximum value
                                        T max = buf[0];
                                        for (int i = 1; i < nBlockXSize * nBlockYSize; ++i)
                                                if (buf[i] > max)
                                                        max = buf[i];
                                        originalBPP = (int)ceil(log2(max + 1));
                                } catch (...) {
                                        originalBPP = sizeof(T) * 8;
                                }
                        }
                        else
                                originalBPP = sizeof(T) * 8;
                }
                return originalBPP;
                #endif
                return originalBPP = sizeof(T) * 8;
        }

        virtual void fillMetadata(const std::string& filename, const std::string& name)
        {
                #if 0
                // SAF images do not have missing values
                missing = missing_value<Sample>();
                #endif

                #if 0
        // Compute real number of BPPs
        Sample max = res->pixels[0];
        for (size_t i = 1; i < res->columns * res->lines; ++i)
                if (res->pixels[i] > max)
                        max = res->pixels[i];
        res->bpp = (int)ceil(log2(max + 1));
                #endif
        }
#endif

        return true;
}

GDALDataset* SAFH5Open(GDALOpenInfo* info)
{
    // We want a real file
    if (info->fpL == NULL)
        return NULL;

    string filename(info->pszFilename);

    // Open the file and fiddle around to see if everything is there
    mh5::File h5;
    try {
        if (!mh5::File::is_hdf5(filename))
            return NULL;
        h5.open(filename, H5F_ACC_RDONLY);
        if (!h5.has_attr("/", "SAF"))
            return NULL;
    } catch (mh5::Error& e) {
        //e.printError(stderr);
        return NULL;
    }

    std::auto_ptr<SAFH5Dataset> ds(new SAFH5Dataset(h5));

    if (!ds->init()) return NULL;

    return ds.release();
}

}
}

extern "C" {

void GDALRegister_MsatSAFH5()
{
        if (! GDAL_CHECK_VERSION("MsatSAFH5"))
                return;

        if (GDALGetDriverByName("MsatSAFH5") == NULL)
        {
                auto_ptr<GDALDriver> driver(new GDALDriver());
                driver->SetDescription("MsatSAFH5");
                driver->SetMetadataItem(GDAL_DMD_LONGNAME, "SAF HDF5 (via Meteosatlib)");
                //driver->SetMetadataItem(GDAL_DMD_HELPTOPIC, "frmt_various.html#JDEM");
                //driver->SetMetadataItem(GDAL_DMD_EXTENSION, "mem");
                driver->pfnOpen = msat::safh5::SAFH5Open;
                GetGDALDriverManager()->RegisterDriver(driver.release());
        }
}

}
