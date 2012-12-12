/******************************************************************************
 * $Id: gdal_translate.cpp 15640 2008-10-29 20:19:56Z rouault $
 *
 * Project:  GDAL Utilities
 * Purpose:  GDAL Image Translator Program redesigned as a C++ class
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1998, 2002, Frank Warmerdam
 * Copyright (c) 2010, Enrico Zini
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ****************************************************************************/

// Forked off GDAL 1.6.3, released 2009/11/19

#include "gdaltranslate.h"
#include <cpl_vsi.h>
#include <cpl_conv.h>
#include <cpl_string.h>
#include <gdal_priv.h>
#include <ogr_spatialref.h>
#include <vrtdataset.h>
#include <ostream>

using namespace std;

CPL_CVSID("$Id: gdal_translate.cpp 15640 2008-10-29 20:19:56Z rouault $");

static void AttachMetadata( GDALDatasetH, char ** );
static int bSubCall = FALSE;

GDALTranslate::GDALTranslate()
{
    eOutputType = GDT_Unknown;

    panBandList = NULL;
    nBandCount = 0;
    bDefBands = TRUE;

    bStrict = FALSE;

    nGCPCount = 0;
    pasGCPs = NULL;

    bSetNoData = FALSE;
    dfNoDataReal = 0.0;

    for (int i = 0; i < 4; ++i)
        adfULLR[i] = 0;
    bGotBounds = FALSE;

    papszCreateOptions = NULL;

    bScale = FALSE; bHaveScaleSrc = FALSE;
    dfScaleSrcMin=0.0; dfScaleSrcMax=255.0;
    dfScaleDstMin=0.0; dfScaleDstMax=255.0;

    papszMetadataOptions = NULL;

    pszOXSize=NULL; pszOYSize=NULL;

    for (int i = 0; i < 4; ++i)
        anSrcWin[i] = 0;

    dfULX = dfULY = dfLRX = dfLRY = 0.0;

    pszOutputSRS = NULL;

    nRGBExpand = 0;
}

GDALTranslate::~GDALTranslate()
{
    CPLFree( panBandList );
    CSLDestroy( papszCreateOptions );
}

/************************************************************************/
/*                             ProxyMain()                              */
/************************************************************************/

GDALDataset* GDALTranslate::translate(GDALDatasetH hDataset)
{
    // GDALDatasetH        hOutDS;
    int                 i;
    int                 nRasterXSize, nRasterYSize;
    // const char          *pszSource=NULL, *pszDest=NULL, *pszFormat = "GTiff";
    double              adfGeoTransform[6];
    int                 nOXSize = 0, nOYSize = 0;
    const char          *pszProjection;
    // int                 bQuiet = FALSE;
    // GDALProgressFunc    pfnProgress = GDALTermProgress;
    // int                 iSrcFileArg = -1, iDstFileArg = -1;
    int                 bCopySubDatasets = FALSE;

    /* Check strict compilation and runtime library version as we use C++ API */
    if (! GDAL_CHECK_VERSION("GDALTranslate"))
        return NULL;

#if 0 // SUBDATASETS
/* -------------------------------------------------------------------- */
/*      Handle command line arguments.                                  */
/* -------------------------------------------------------------------- */
    for( i = 1; i < argc; i++ )
    {
        else if( EQUAL(argv[i],"-of") && i < argc-1 )
            pszFormat = argv[++i];

        else if( EQUAL(argv[i],"-quiet") )
        {
            bQuiet = TRUE;
            pfnProgress = GDALDummyProgress;
        }

        else if( EQUAL(argv[i],"-sds")  )
            bCopySubDatasets = TRUE;
    }

/* -------------------------------------------------------------------- */
/*      Handle subdatasets.                                             */
/* -------------------------------------------------------------------- */
    if( CSLCount(GDALGetMetadata( hDataset, "SUBDATASETS" )) > 0 )
    {
        if( !bCopySubDatasets )
        {
            fprintf( stderr,
                     "Input file contains subdatasets. Please, select one of them for reading.\n" );
        }
        else
        {
            char **papszSubdatasets = GDALGetMetadata(hDataset,"SUBDATASETS");
            char *pszSubDest = (char *) CPLMalloc(strlen(pszDest)+32);
            int i;
            int bOldSubCall = bSubCall;

            argv[iDstFileArg] = pszSubDest;
            bSubCall = TRUE;
            for( i = 0; papszSubdatasets[i] != NULL; i += 2 )
            {
                argv[iSrcFileArg] = strstr(papszSubdatasets[i],"=")+1;
                sprintf( pszSubDest, "%s%d", pszDest, i/2 + 1 );
                if( ProxyMain( argc, argv ) != 0 )
                    break;
            }

            bSubCall = bOldSubCall;
            CPLFree( pszSubDest );
        }

        GDALClose( hDataset );

        if( !bSubCall )
        {
            GDALDumpOpenDatasets( stderr );
            GDALDestroyDriverManager();
        }
        return 1;
    }
#endif

/* -------------------------------------------------------------------- */
/*      Collect some information from the source file.                  */
/* -------------------------------------------------------------------- */
    nRasterXSize = GDALGetRasterXSize( hDataset );
    nRasterYSize = GDALGetRasterYSize( hDataset );

#if 0
    if( !bQuiet )
        printf( "Input file size is %d, %d\n", nRasterXSize, nRasterYSize );
#endif

    if( anSrcWin[2] == 0 && anSrcWin[3] == 0 )
    {
        anSrcWin[2] = nRasterXSize;
        anSrcWin[3] = nRasterYSize;
    }

/* -------------------------------------------------------------------- */
/*	Build band list to translate					*/
/* -------------------------------------------------------------------- */
    if( nBandCount == 0 )
    {
        nBandCount = GDALGetRasterCount( hDataset );
        if( nBandCount == 0 )
        {
            CPLError(CE_Failure, CPLE_AppDefined, "Input file has no bands, and so cannot be translated.");
            return NULL;
        }

        panBandList = (int *) CPLMalloc(sizeof(int)*nBandCount);
        for( i = 0; i < nBandCount; i++ )
            panBandList[i] = i+1;
    }
    else
    {
        for( i = 0; i < nBandCount; i++ )
        {
            if( panBandList[i] < 1 || panBandList[i] > GDALGetRasterCount(hDataset) )
            {
                CPLError(CE_Failure, CPLE_AppDefined, 
                         "Band %d requested, but only bands 1 to %d available.",
                         panBandList[i], GDALGetRasterCount(hDataset) );
                return NULL;
            }
        }

        if( nBandCount != GDALGetRasterCount( hDataset ) )
            bDefBands = FALSE;
    }

/* -------------------------------------------------------------------- */
/*      Compute the source window from the projected source window      */
/*      if the projected coordinates were provided.  Note that the      */
/*      projected coordinates are in ulx, uly, lrx, lry format,         */
/*      while the anSrcWin is xoff, yoff, xsize, ysize with the         */
/*      xoff,yoff being the ulx, uly in pixel/line.                     */
/* -------------------------------------------------------------------- */
    if( dfULX != 0.0 || dfULY != 0.0 
        || dfLRX != 0.0 || dfLRY != 0.0 )
    {
        double	adfGeoTransform[6];

        GDALGetGeoTransform( hDataset, adfGeoTransform );

        if( adfGeoTransform[2] != 0.0 || adfGeoTransform[4] != 0.0 )
        {
            CPLError(CE_Failure, CPLE_AppDefined, 
                     "The -projwin option was used, but the geotransform is "
                     "rotated.  This configuration is not supported." );
            return NULL;
        }

        anSrcWin[0] = (int) 
            ((dfULX - adfGeoTransform[0]) / adfGeoTransform[1] + 0.001);
        anSrcWin[1] = (int) 
            ((dfULY - adfGeoTransform[3]) / adfGeoTransform[5] + 0.001);

        anSrcWin[2] = (int) ((dfLRX - dfULX) / adfGeoTransform[1] + 0.5);
        anSrcWin[3] = (int) ((dfLRY - dfULY) / adfGeoTransform[5] + 0.5);

#if 0
        if( !bQuiet )
            fprintf( stdout, 
                     "Computed -srcwin %d %d %d %d from projected window.\n",
                     anSrcWin[0], 
                     anSrcWin[1], 
                     anSrcWin[2], 
                     anSrcWin[3] );
#endif
        
        if( anSrcWin[0] < 0 || anSrcWin[1] < 0 
            || anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset) 
            || anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset) )
        {
            CPLError(CE_Failure, CPLE_AppDefined, 
                     "Computed -srcwin falls outside raster size of %dx%d.",
                     GDALGetRasterXSize(hDataset), 
                     GDALGetRasterYSize(hDataset) );
            return NULL;
        }
    }

/* -------------------------------------------------------------------- */
/*      Verify source window.                                           */
/* -------------------------------------------------------------------- */
    if( anSrcWin[0] < 0 || anSrcWin[1] < 0 
        || anSrcWin[2] <= 0 || anSrcWin[3] <= 0
        || anSrcWin[0] + anSrcWin[2] > GDALGetRasterXSize(hDataset) 
        || anSrcWin[1] + anSrcWin[3] > GDALGetRasterYSize(hDataset) )
    {
        CPLError(CE_Failure, CPLE_AppDefined, 
                 "-srcwin %d %d %d %d falls outside raster size of %dx%d "
                 "or is otherwise illegal.",
                 anSrcWin[0],
                 anSrcWin[1],
                 anSrcWin[2],
                 anSrcWin[3],
                 GDALGetRasterXSize(hDataset), 
                 GDALGetRasterYSize(hDataset) );
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      If nothing needs to be changed, we can return the dataset       */
/*      itself.                                                         */
/* -------------------------------------------------------------------- */
    if( eOutputType == GDT_Unknown 
        && !bScale && CSLCount(papszMetadataOptions) == 0 && bDefBands 
        && anSrcWin[0] == 0 && anSrcWin[1] == 0 
        && anSrcWin[2] == GDALGetRasterXSize(hDataset)
        && anSrcWin[3] == GDALGetRasterYSize(hDataset) 
        && pszOXSize == NULL && pszOYSize == NULL 
        && nGCPCount == 0 && !bGotBounds
        && pszOutputSRS == NULL && !bSetNoData
        && nRGBExpand == 0)
    {
        return (GDALDataset*)hDataset;
    }

/* -------------------------------------------------------------------- */
/*      Establish some parameters.                                      */
/* -------------------------------------------------------------------- */
    if( pszOXSize == NULL )
    {
        nOXSize = anSrcWin[2];
        nOYSize = anSrcWin[3];
    }
    else
    {
        nOXSize = (int) ((pszOXSize[strlen(pszOXSize)-1]=='%' 
                          ? atof(pszOXSize)/100*anSrcWin[2] : atoi(pszOXSize)));
        nOYSize = (int) ((pszOYSize[strlen(pszOYSize)-1]=='%' 
                          ? atof(pszOYSize)/100*anSrcWin[3] : atoi(pszOYSize)));
    }
    
/* ==================================================================== */
/*      Create a virtual dataset.                                       */
/* ==================================================================== */
    VRTDataset *poVDS;
        
/* -------------------------------------------------------------------- */
/*      Make a virtual clone.                                           */
/* -------------------------------------------------------------------- */
    poVDS = (VRTDataset *) VRTCreate( nOXSize, nOYSize );

    if( nGCPCount == 0 )
    {
        if( pszOutputSRS != NULL )
        {
            poVDS->SetProjection( pszOutputSRS );
        }
        else
        {
            pszProjection = GDALGetProjectionRef( hDataset );
            if( pszProjection != NULL && strlen(pszProjection) > 0 )
                poVDS->SetProjection( pszProjection );
        }
    }

    if( bGotBounds )
    {
        adfGeoTransform[0] = adfULLR[0];
        adfGeoTransform[1] = (adfULLR[2] - adfULLR[0]) / nOXSize;
        adfGeoTransform[2] = 0.0;
        adfGeoTransform[3] = adfULLR[1];
        adfGeoTransform[4] = 0.0;
        adfGeoTransform[5] = (adfULLR[3] - adfULLR[1]) / nOYSize;

        poVDS->SetGeoTransform( adfGeoTransform );
    }

    else if( GDALGetGeoTransform( hDataset, adfGeoTransform ) == CE_None 
        && nGCPCount == 0 )
    {
        adfGeoTransform[0] += anSrcWin[0] * adfGeoTransform[1]
            + anSrcWin[1] * adfGeoTransform[2];
        adfGeoTransform[3] += anSrcWin[0] * adfGeoTransform[4]
            + anSrcWin[1] * adfGeoTransform[5];
        
        adfGeoTransform[1] *= anSrcWin[2] / (double) nOXSize;
        adfGeoTransform[2] *= anSrcWin[3] / (double) nOYSize;
        adfGeoTransform[4] *= anSrcWin[2] / (double) nOXSize;
        adfGeoTransform[5] *= anSrcWin[3] / (double) nOYSize;
        
        poVDS->SetGeoTransform( adfGeoTransform );
    }

    if( nGCPCount != 0 )
    {
        const char *pszGCPProjection = pszOutputSRS;

        if( pszGCPProjection == NULL )
            pszGCPProjection = GDALGetGCPProjection( hDataset );
        if( pszGCPProjection == NULL )
            pszGCPProjection = "";

        poVDS->SetGCPs( nGCPCount, pasGCPs, pszGCPProjection );

        GDALDeinitGCPs( nGCPCount, pasGCPs );
        CPLFree( pasGCPs );
    }

    else if( GDALGetGCPCount( hDataset ) > 0 )
    {
        GDAL_GCP *pasGCPs;
        int       nGCPs = GDALGetGCPCount( hDataset );

        pasGCPs = GDALDuplicateGCPs( nGCPs, GDALGetGCPs( hDataset ) );

        for( i = 0; i < nGCPs; i++ )
        {
            pasGCPs[i].dfGCPPixel -= anSrcWin[0];
            pasGCPs[i].dfGCPLine  -= anSrcWin[1];
            pasGCPs[i].dfGCPPixel *= (nOXSize / (double) anSrcWin[2] );
            pasGCPs[i].dfGCPLine  *= (nOYSize / (double) anSrcWin[3] );
        }
            
        poVDS->SetGCPs( nGCPs, pasGCPs,
                        GDALGetGCPProjection( hDataset ) );

        GDALDeinitGCPs( nGCPs, pasGCPs );
        CPLFree( pasGCPs );
    }

/* -------------------------------------------------------------------- */
/*      Transfer generally applicable metadata.                         */
/* -------------------------------------------------------------------- */
    poVDS->SetMetadata( ((GDALDataset*)hDataset)->GetMetadata() );
    AttachMetadata( (GDALDatasetH) poVDS, papszMetadataOptions );

/* -------------------------------------------------------------------- */
/*      Transfer metadata that remains valid if the spatial             */
/*      arrangement of the data is unaltered.                           */
/* -------------------------------------------------------------------- */
    if( anSrcWin[0] == 0 && anSrcWin[1] == 0 
        && anSrcWin[2] == GDALGetRasterXSize(hDataset)
        && anSrcWin[3] == GDALGetRasterYSize(hDataset) 
        && pszOXSize == NULL && pszOYSize == NULL )
    {
        char **papszMD;

        papszMD = ((GDALDataset*)hDataset)->GetMetadata("RPC");
        if( papszMD != NULL )
            poVDS->SetMetadata( papszMD, "RPC" );
    }

    if (nRGBExpand != 0)
        nBandCount += nRGBExpand - 1;

/* ==================================================================== */
/*      Process all bands.                                              */
/* ==================================================================== */
    for( i = 0; i < nBandCount; i++ )
    {
        VRTSourcedRasterBand   *poVRTBand;
        GDALRasterBand  *poSrcBand;
        GDALDataType    eBandType;

        if (nRGBExpand != 0 && i < nRGBExpand)
        {
            poSrcBand = ((GDALDataset *) 
                     hDataset)->GetRasterBand(panBandList[0]);
            if (poSrcBand->GetColorTable() == NULL)
            {
                CPLError(CE_Failure, CPLE_AppDefined, "Error : band %d has no color table", panBandList[0]);
                GDALClose( (GDALDatasetH) poVDS );
                return NULL;
            }
        }
        else
            poSrcBand = ((GDALDataset *) 
                        hDataset)->GetRasterBand(panBandList[i]);

/* -------------------------------------------------------------------- */
/*      Select output data type to match source.                        */
/* -------------------------------------------------------------------- */
        if( eOutputType == GDT_Unknown )
            eBandType = poSrcBand->GetRasterDataType();
        else
            eBandType = eOutputType;

/* -------------------------------------------------------------------- */
/*      Create this band.                                               */
/* -------------------------------------------------------------------- */
        poVDS->AddBand( eBandType, NULL );
        poVRTBand = (VRTSourcedRasterBand *) poVDS->GetRasterBand( i+1 );
            
/* -------------------------------------------------------------------- */
/*      Do we need to collect scaling information?                      */
/* -------------------------------------------------------------------- */
        double dfScale=1.0, dfOffset=0.0;

        if( bScale && !bHaveScaleSrc )
        {
            double	adfCMinMax[2];
            GDALComputeRasterMinMax( poSrcBand, TRUE, adfCMinMax );
            dfScaleSrcMin = adfCMinMax[0];
            dfScaleSrcMax = adfCMinMax[1];
        }

        if( bScale )
        {
            if( dfScaleSrcMax == dfScaleSrcMin )
                dfScaleSrcMax += 0.1;
            if( dfScaleDstMax == dfScaleDstMin )
                dfScaleDstMax += 0.1;

            dfScale = (dfScaleDstMax - dfScaleDstMin) 
                / (dfScaleSrcMax - dfScaleSrcMin);
            dfOffset = -1 * dfScaleSrcMin * dfScale + dfScaleDstMin;
        }

/* -------------------------------------------------------------------- */
/*      Create a simple or complex data source depending on the         */
/*      translation type required.                                      */
/* -------------------------------------------------------------------- */
        if( bScale || (nRGBExpand != 0 && i < nRGBExpand) )
        {
            poVRTBand->AddComplexSource( poSrcBand,
                                         anSrcWin[0], anSrcWin[1], 
                                         anSrcWin[2], anSrcWin[3], 
                                         0, 0, nOXSize, nOYSize,
                                         dfOffset, dfScale,
                                         VRT_NODATA_UNSET,
                                         (nRGBExpand != 0 && i < nRGBExpand) ? i + 1 : 0 );
        }
        else
            poVRTBand->AddSimpleSource( poSrcBand,
                                        anSrcWin[0], anSrcWin[1], 
                                        anSrcWin[2], anSrcWin[3], 
                                        0, 0, nOXSize, nOYSize );

        /* In case of color table translate, we only set the color interpretation */
        /* other info copied by CopyCommonInfoFrom are not relevant in RGB expansion */
        if (nRGBExpand != 0 && i < nRGBExpand)
        {
            poVRTBand->SetColorInterpretation( (GDALColorInterp) (GCI_RedBand + i) );
        }
        else
        {
/* -------------------------------------------------------------------- */
/*      copy over some other information of interest.                   */
/* -------------------------------------------------------------------- */
            poVRTBand->CopyCommonInfoFrom( poSrcBand );
        }

/* -------------------------------------------------------------------- */
/*      Set a forcable nodata value?                                    */
/* -------------------------------------------------------------------- */
        if( bSetNoData )
            poVRTBand->SetNoDataValue( dfNoDataReal );
    }

    return poVDS;

#if 0
    if( !bSubCall )
    {
        GDALDumpOpenDatasets( stderr );
        GDALDestroyDriverManager();
    }
#endif
}


/************************************************************************/
/*                           AttachMetadata()                           */
/************************************************************************/

static void AttachMetadata( GDALDatasetH hDS, char **papszMetadataOptions )

{
    int nCount = CSLCount(papszMetadataOptions);
    int i;

    for( i = 0; i < nCount; i++ )
    {
        char    *pszKey = NULL;
        const char *pszValue;
        
        pszValue = CPLParseNameValue( papszMetadataOptions[i], &pszKey );
        GDALSetMetadataItem(hDS,pszKey,pszValue,NULL);
        CPLFree( pszKey );
    }

    CSLDestroy( papszMetadataOptions );
}

void GDALTranslate::dump(std::ostream& out)
{
    // gdal_translate -ot
    // TODO: cout << "eOutputType: " << GDALDataType        eOutputType;    // Default: leave unchanged

    if (panBandList)
    {
        out << "panBandList:";
        for (int i = 0; i < nBandCount; ++i)
            out << " " << panBandList[i];
        out << endl;
    }
    out << "bDefBands: " << bDefBands << endl;
    out << "bStrict: " << bStrict << endl;
    out << "nGCPCount: " << nGCPCount << endl;
    // TODO: GDAL_GCP            *pasGCPs;
    // gdal_translate -a_nodata
    out << "bSetNoData: " << bSetNoData << endl;
    out << "dfNoDataReal: " << dfNoDataReal << endl;
    out << "bGotBounds: " << bGotBounds << endl;
    out << "adfULLR:";
    for (int i = 0; i < 4; ++i)
        out << " " << adfULLR[i];
    out << endl;

    if (papszCreateOptions)
        for (int i = 0; papszCreateOptions[i]; ++i)
            out << "papszCreateOptions[" << i << "]: " << papszCreateOptions[i] << endl;
    else
        out << "papszCreateOptions: null" << endl;

    out << "bScale: " << bScale << endl;
    out << "bHaveScaleSrc: " << bHaveScaleSrc << endl;
    out << "dfScaleSrcMin: " << dfScaleSrcMin << endl;
    out << "dfScaleSrcMax: " << dfScaleSrcMax << endl;
    out << "dfScaleDstMin: " << dfScaleDstMin << endl;
    out << "dfScaleDstMax: " << dfScaleDstMax << endl;

    if (papszMetadataOptions)
        for (int i = 0; papszMetadataOptions[i]; ++i)
            out << "papszMetadataOptions[" << i << "]: " << papszMetadataOptions[i] << endl;
    else
        out << "papszMetadataOptions: null" << endl;

    out << "pszOXSize: " << (pszOXSize ? pszOXSize : "null") << endl;
    out << "pszOYSize: " << (pszOYSize ? pszOYSize : "null") << endl;

    out << "anSrcWin:";
    for (int i = 0; i < 4; ++i)
        out << " " << anSrcWin[i];
    out << endl;

    out << "dfULX: " << dfULX << endl;
    out << "dfULY: " << dfULY << endl;
    out << "dfLRX: " << dfLRX << endl;
    out << "dfLRY: " << dfLRY << endl;

    out << "pszOutputSRS: " << (pszOutputSRS ? pszOutputSRS : "null") << endl;
    out << "nRGBExpand: " << nRGBExpand << endl;
}
