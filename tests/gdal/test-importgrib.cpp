/*
 * Copyright (C) 2005--2010  ARPA-SIM <urpsim@smr.arpa.emr.it>
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

#include "test-utils.h"
// TODO #include <gdal/gdalwarper.h>
// TODO #include <gdal/ogr_spatialref.h>

using namespace std;

namespace tut {

#define TESTFILE "MSG_Seviri_1_5_Infrared_9_7_channel_20060426_1945.grb"

struct importgrib_shar : public ImportTest
{
	importgrib_shar()
		: ImportTest("MsatGRIB", TESTFILE) {}

	~importgrib_shar() {}

        void checkGeneralImageData(GDALDataset* dataset)
        {
                const char* val = dataset->GetMetadataItem(MD_MSAT_DATETIME, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "2006-04-26 19:45:00");

                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "55");
                val = dataset->GetMetadataItem(MD_MSAT_SPACECRAFT, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "MSG1");

                GDALRasterBand* b = dataset->GetRasterBand(1);
                gen_ensure(b != NULL);

                val = b->GetMetadataItem(MD_MSAT_CHANNEL_ID, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "8");
                val = b->GetMetadataItem(MD_MSAT_CHANNEL, MD_DOMAIN_MSAT);
                gen_ensure(val != NULL);
                gen_ensure_equals(string(val), "IR_097");

                gen_ensure_equals(string(b->GetUnitType()), "K");

                // TODO gen_ensure_equals(defaultFilename(*dataset), "MSG1_Seviri_1_5_IR_097_20060426_1945");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_097");

                // TODO gen_ensure_equals(defaultFilename(*b), "MSG1_Seviri_1_5_IR_097_20060426_1945");
                // TODO gen_ensure_equals(defaultShortName(*dataset), "IR_097");

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);

                // TODO gen_ensure_equals(b->scalesToInt(), false);

#if 0
                gen_ensure_equals(b->column_offset, 1856);
                gen_ensure_equals(b->line_offset, 1856);
                gen_ensure_equals(b->column_res, Image::columnResFromSeviriDX(3608));
                gen_ensure_equals(b->line_res, Image::columnResFromSeviriDX(3608));
                gen_ensure_equals(b->x0, 1499);
                gen_ensure_equals(b->y0, 199);
#endif
                //gen_ensure_equals(b->getOriginalBpp(), 11);
                // TODO gen_ensure(b->getOriginalBpp() >= 11);

#if 0
                gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_IR_108_channel_20051219_1415");
                //gen_ensure_equals(img.channel_id, 2049);
                //gen_ensure_similar(img.column_res, Image::columnResFromSeviriDX(3608), 0.00001);
                //gen_ensure_similar(img.line_res, Image::columnResFromSeviriDX(3608), 0.00001);
                //gen_ensure_equals(img.column_offset, 1856);
                //gen_ensure_equals(img.line_offset, 1856);
                //gen_ensure_equals(img.defaultFilename, "MSG1_Seviri_unknown55_2049_channel_20060426_1945");
                //gen_ensure_equals(img.shortName, "unknown55_2049");
                //gen_ensure_equals(img.unit, "unknown");
#endif
        }

        void checkFullImageData(GDALDataset* dataset)
        {
                checkGeneralImageData(dataset);

                gen_ensure_equals(dataset->GetRasterXSize(), 1300);
                gen_ensure_equals(dataset->GetRasterYSize(), 700);

                int xs, ys;
                double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
                gen_ensure_equals(xs, 1856 - 1500);
                gen_ensure_equals(ys, 1856 - 200);
                gen_ensure_equals(rx, METEOSAT_PIXELSIZE_X);
                gen_ensure_equals(ry, METEOSAT_PIXELSIZE_Y);

                GDALRasterBand* b = dataset->GetRasterBand(1);

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);
                double missing = b->GetNoDataValue(&valid);
                gen_ensure_equals(missing, 0);
                gen_ensure_equals(valid, TRUE);

                gen_ensure_similar(readFloat32(b, 0, 0), 97.7, 0.001);
                gen_ensure_similar(readFloat32(b, 10, 10), 98.1, 0.001);
                //gen_ensure_similar(b->scaled(1299, 0), b->scaledNoDataValue(), 0.000001);
                // Unfortunately, this grib does not have missing values
                gen_ensure_equals(readFloat32(b, 1299, 0), 0);
        }

        void checkCroppedImageData(GDALDataset* dataset)
        {
                checkGeneralImageData(dataset);

                gen_ensure_equals(dataset->GetRasterXSize(), 200);
                gen_ensure_equals(dataset->GetRasterYSize(), 50);

                int xs, ys;
                double rx, ry;
                msat::dataset::decodeGeotransform(dataset, xs, ys, rx, ry);
                gen_ensure_equals(xs, 1856 - 1600);
                gen_ensure_equals(ys, 1856 - 300);
                gen_ensure_equals(rx, METEOSAT_PIXELSIZE_X);
                gen_ensure_equals(ry, METEOSAT_PIXELSIZE_Y);

                GDALRasterBand* b = dataset->GetRasterBand(1);

                int valid;
                gen_ensure_equals(b->GetOffset(&valid), 0);
                gen_ensure_equals(valid, TRUE);
                gen_ensure_equals(b->GetScale(&valid), 1);
                gen_ensure_equals(valid, TRUE);
                double missing = b->GetNoDataValue(&valid);
                gen_ensure_equals(missing, 0);
                gen_ensure_equals(valid, TRUE);

                gen_ensure_equals(readFloat32(b, 0, 0), 100.50f);
                gen_ensure_similar(readFloat32(b, 10, 10), 97.8f, 0.0001);
        }
};
TESTGRP(importgrib);

static msat::proj::ImageBox cropArea(msat::proj::ImagePoint(100, 100), msat::proj::ImagePoint(100+200, 100+50));

// Test opening
template<> template<>
void to::test<1>()
{
	auto_ptr<GDALDataset> dataset = openDS();
	gen_ensure(dataset.get() != 0);
	gen_ensure_equals(string(GDALGetDriverShortName(dataset->GetDriver())), "MsatGRIB");
	gen_ensure_equals(dataset->GetRasterCount(), 1);
}

// Test reading image
template<> template<>
void to::test<2>()
{
	test_tag("fullGrib");
	auto_ptr<GDALDataset> dataset = openPlain();
	test_untag();
}

// Try CreateCopy with msat/msat
template<> template<>
void to::test<3>()
{
	test_tag("fullGribRecodedGribMsat");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/msat");
	test_untag();

#if 0
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try CreateCopy with msat/ecmwf
template<> template<>
void to::test<4>()
{
	test_tag("fullGribRecodedGribEcmwf");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatGRIB", false, "TEMPLATE=msat/ecmwf");
	test_untag();

#if 0
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported subarea grib (msat/msat)
template<> template<>
void to::test<5>()
{
	test_tag("croppedGribRecodedGribMsat");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/msat");
	test_untag();

#if 0
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_similar(img->data->offset, -91, 0.001);
	gen_ensure_equals(img->data->bpp, 8);
	gen_ensure_equals(img->data->scalesToInt, true);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported subarea grib (msat/ecmwf)
template<> template<>
void to::test<8>()
{
	test_tag("croppedGribRecodedGribEcmwf");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatGRIB", false, "TEMPLATE=msat/ecmwf");
	test_untag();

#if 0
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_similar(img->data->offset, -91, 0.001);
	gen_ensure_equals(img->data->bpp, 8);
	gen_ensure_equals(img->data->scalesToInt, true);

	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported netcdf24
template<> template<>
void to::test<9>()
{
	test_tag("fullGribRecodedNetCDF24");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF24", false);
	test_untag();

#if 0
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting a subarea exported to netcdf24
template<> template<>
void to::test<10>()
{
	test_tag("croppedGribRecodedNetCDF24");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF24", false);
	test_untag();

#if 0
	gen_ensure_similar(img->data->slope, 0.1, 0.001);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 11);
	gen_ensure_equals(img->data->scalesToInt, true);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported netcdf
template<> template<>
void to::test<11>()
{
	test_tag("fullGribRecodedNetCDF");
        auto_ptr<GDALDataset> dataset = openRecoded("MsatNetCDF", false);
	test_untag();

#if 0
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Try reimporting an exported subarea netcdf
template<> template<>
void to::test<12>()
{
	test_tag("croppedGribRecodedNetCDF");
        auto_ptr<GDALDataset> dataset = openRecodedCropped(cropArea, "MsatNetCDF", false);
	test_untag();

#if 0
	gen_ensure_equals(img->data->slope, 1);
	gen_ensure_equals(img->data->offset, 0);
	gen_ensure_equals(img->data->bpp, 32);
	gen_ensure_equals(img->data->scalesToInt, false);
	gen_ensure_imagedata_similar(*img->data, *imgs[0]->data, 0.0001);
#endif
}

// Reproject to latlon
template<> template<>
void to::test<13>()
{
	FOR_DRIVER("MsatGRIB");
#if 0
    // Open the source file. 
	auto_ptr<msat::Image> dataset = openro(TESTFILE);

	test_tag("fullGribReprojectedNetCDF");

    // Create output with same datatype as first input band. 
    GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(dataset.get(),1));

    // Get output driver (GeoTIFF format)
    GDALDriverH hDriver = GDALGetDriverByName("MsatNetCDF");
	gen_ensure(hDriver != NULL);

    // Get Source coordinate system. 
    const char *pszSrcWKT = GDALGetProjectionRef(dataset.get());
    gen_ensure(pszSrcWKT != NULL);
	gen_ensure(strlen(pszSrcWKT) > 0);

    // Setup output coordinate system that is UTM 11 WGS84. 
    OGRSpatialReference oSRS;
	oSRS.SetGeogCS( NULL, NULL, NULL, 6378169, 295.488065897, NULL, 0, NULL, 0 );
    //oSRS.SetUTM(11, TRUE);
    //oSRS.SetWellKnownGeogCS("WGS84");
	char* pszDstWKT = 0;
    oSRS.exportToWkt(&pszDstWKT);

    // Create a transformer that maps from source pixel/line coordinates
    // to destination georeferenced coordinates (not destination 
    // pixel line).  We do that by omitting the destination dataset
    // handle (setting it to NULL). 
    void *hTransformArg = GDALCreateGenImgProjTransformer(
		dataset.get(), pszSrcWKT, NULL, pszDstWKT, FALSE, 0, 1 );
    gen_ensure(hTransformArg != NULL);

    // Get approximate output georeferenced bounds and resolution for file. 
    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;

    CPLErr eErr = GDALSuggestedWarpOutput(dataset.get(), 
                                    GDALGenImgProjTransform, hTransformArg, 
                                    adfDstGeoTransform, &nPixels, &nLines );
    gen_ensure_equals(eErr, CE_None);

    GDALDestroyGenImgProjTransformer(hTransformArg);

    // Create the output file.  
    GDALDataset* hDstDS = (GDALDataset*)GDALCreate(hDriver, "out.nc", nPixels, nLines, 
                         GDALGetRasterCount(dataset.get()), eDT, NULL );
    gen_ensure(hDstDS != NULL);

	gen_ensure_equals(hDstDS->GetRasterBand(1)->GetAccess(), GA_Update);
	
	// Setup warp options. 
    
    GDALWarpOptions *psWarpOptions = GDALCreateWarpOptions();

    psWarpOptions->hSrcDS = dataset.get();
    psWarpOptions->hDstDS = hDstDS;

    psWarpOptions->nBandCount = 1;
    psWarpOptions->panSrcBands = 
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panSrcBands[0] = 1;
    psWarpOptions->panDstBands = 
        (int *) CPLMalloc(sizeof(int) * psWarpOptions->nBandCount );
    psWarpOptions->panDstBands[0] = 1;

    psWarpOptions->pfnProgress = GDALDummyProgress;   

    psWarpOptions->papszWarpOptions = CSLSetNameValue(NULL, "INIT_DEST", "NO_DATA");

    // Establish reprojection transformer. 

    psWarpOptions->pTransformerArg = 
        GDALCreateGenImgProjTransformer( dataset.get(), 
                                         GDALGetProjectionRef(dataset.get()), 
                                         hDstDS,
                                         GDALGetProjectionRef(hDstDS), 
                                         FALSE, 0.0, 1 );
    psWarpOptions->pfnTransformer = GDALGenImgProjTransform;


    // Write out the projection definition. 
    GDALSetProjection(hDstDS, pszDstWKT);
    GDALSetGeoTransform(hDstDS, adfDstGeoTransform);

    GDALWarpOperation oOperation;
    oOperation.Initialize(psWarpOptions);
    oOperation.ChunkAndWarpImage(0, 0, 
                                  GDALGetRasterXSize(hDstDS), 
                                  GDALGetRasterYSize(hDstDS));
    GDALDestroyGenImgProjTransformer(psWarpOptions->pTransformerArg);
    GDALDestroyWarpOptions(psWarpOptions);
    GDALClose(hDstDS);

	test_untag();
#endif
}

}


