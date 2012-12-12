#ifndef MSAT_GDALTRANSLATE_H
#define MSAT_GDALTRANSLATE_H

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

#include <gdal/gdal_priv.h>
#include <iosfwd>

struct GDALTranslate
{
    // gdal_translate -ot
    GDALDataType        eOutputType;    // Default: leave unchanged

    // gdal_translate -b
    int                 *panBandList;   // Array of bands for reordering (default leave unchanged)
    int                 nBandCount;     // Number of bands (default leave unchanged)
    int                 bDefBands;      // TRUE if the bands are NOT reordered

    // gdal_translate -not_strict/-strict
    int                 bStrict;        // Default: not strict

    // gdal_translate -gcp
    int                 nGCPCount;
    GDAL_GCP            *pasGCPs;
    
    // gdal_translate -a_nodata
    int                 bSetNoData;
    double              dfNoDataReal;
    
    // gdal_translate -a_ullr
    int                 bGotBounds;
    double              adfULLR[4];

    // gdal_translate -co
    char                **papszCreateOptions;

    // gdal_translate -scale
    int                 bScale, bHaveScaleSrc;
    double              dfScaleSrcMin, dfScaleSrcMax;
    double              dfScaleDstMin, dfScaleDstMax;

    // gdal_translate -mo
    char                **papszMetadataOptions;

    // gdal_translate -outsize
    const char          *pszOXSize, *pszOYSize;

    // gdal_translate -srcwin
    int                 anSrcWin[4];

    // gdal_translate -projwin
    double              dfULX, dfULY, dfLRX, dfLRY;

    // gdal_translate -a_srs
    char                *pszOutputSRS;

    // gdal_translate -expand (rgb=3, rgba=4)
    int                 nRGBExpand;

    GDALTranslate();
    ~GDALTranslate();

    /**
     * Create a translated virtual dataset according to how the structure
     * has been configured
     */
    GDALDataset* translate(GDALDatasetH hDataset);

    void dump(std::ostream& out);
};

#endif
