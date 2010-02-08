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
#include <msat/xrit/dataaccess.h>
#include <msat/xrit/fileaccess.h>
#include <hrit/MSG_HRIT.h>

using namespace msat::xrit;

namespace tut {

#define TESTDATA_LINEAR    DATA_DIR "/H:MSG2:VIS006:200807150900"
#define TESTDATA_HRV       DATA_DIR "/H:MSG1:HRV:200611141200"
#define TESTDATA_NONLINEAR DATA_DIR "/H:MSG1:IR_039:200611130800"

struct dataaccess_shar
{
        dataaccess_shar()
        {
        }

        ~dataaccess_shar()
        {
        }
};
TESTGRP(dataaccess);

template<> template<>
void to::test<1>()
{
        FileAccess fa(TESTDATA_LINEAR);
        DataAccess da;

        MSG_data pro;
        MSG_data epi;
        MSG_header header;
        da.scan(fa, pro, epi, header);

        gen_ensure_equals(da.npixperseg, 3712u * 123u);
        gen_ensure_equals(da.seglines, 123u);
        gen_ensure_equals(da.swapX, true);
        gen_ensure_equals(da.swapY, true);
        gen_ensure_equals(da.hrv, false);
        gen_ensure_equals(da.segnames.size(), 1u);
        gen_ensure_equals(da.columns, 3712u);
        gen_ensure_equals(da.lines, 3712u);
        gen_ensure_equals(da.SouthLineActual, 1u);
        gen_ensure_equals(da.WestColumnActual, 1u);

        gen_ensure_equals(da.line_start(   0), 0u);
        gen_ensure_equals(da.line_start(1856), 0u);
        gen_ensure_equals(da.line_start(3712), 0u);
        gen_ensure_equals(da.line_start(9999), 0u);

        gen_ensure(da.segment(0) != NULL);

        MSG_SAMPLE buf[3712];
        da.line_read(0, buf);
}

template<> template<>
void to::test<2>()
{
        FileAccess fa(TESTDATA_NONLINEAR);
        DataAccess da;

        MSG_data pro;
        MSG_data epi;
        MSG_header header;
        da.scan(fa, pro, epi, header);

        gen_ensure_equals(da.npixperseg, 3712u * 123u);
        gen_ensure_equals(da.seglines, 123u);
        gen_ensure_equals(da.swapX, true);
        gen_ensure_equals(da.swapY, true);
        gen_ensure_equals(da.hrv, false);
        gen_ensure_equals(da.segnames.size(), 1u);
        gen_ensure_equals(da.columns, 3712u);
        gen_ensure_equals(da.lines, 3712u);
        gen_ensure_equals(da.SouthLineActual, 1u);
        gen_ensure_equals(da.WestColumnActual, 1u);

        gen_ensure_equals(da.line_start(   0), 0u);
        gen_ensure_equals(da.line_start(1856), 0u);
        gen_ensure_equals(da.line_start(3712), 0u);
        gen_ensure_equals(da.line_start(9999), 0u);

        gen_ensure(da.segment(0) != NULL);

        MSG_SAMPLE buf[3712];
        da.line_read(0, buf);
}

template<> template<>
void to::test<3>()
{
        FileAccess fa(TESTDATA_HRV);
        DataAccess da;

        MSG_data pro;
        MSG_data epi;
        MSG_header header;
        da.scan(fa, pro, epi, header);

        gen_ensure_equals(da.npixperseg, 11136u * 123u);
        gen_ensure_equals(da.seglines, 123u);
        gen_ensure_equals(da.swapX, true);
        gen_ensure_equals(da.swapY, true);
        gen_ensure_equals(da.hrv, true);
        gen_ensure_equals(da.segnames.size(), 1u);
        gen_ensure_equals(da.columns, 11136u);
        gen_ensure_equals(da.lines, 11136u);

        gen_ensure_equals(da.LowerEastColumnActual, 1u);
        gen_ensure_equals(da.LowerSouthLineActual, 1u);
        gen_ensure_equals(da.LowerWestColumnActual, 5568u);
        gen_ensure_equals(da.LowerNorthLineActual, 8064u);
        gen_ensure_equals(da.UpperEastColumnActual, 2064u);
        gen_ensure_equals(da.UpperSouthLineActual, 8065u);
        gen_ensure_equals(da.UpperWestColumnActual, 7631u);
        gen_ensure_equals(da.UpperNorthLineActual, 11136u);

        gen_ensure_equals(da.line_start(   0), 0u);
        gen_ensure_equals(da.line_start(1856), 0u);
        gen_ensure_equals(da.line_start(3712), 0u);
        gen_ensure_equals(da.line_start(9999), 0u);

        gen_ensure(da.segment(0) != NULL);

        MSG_SAMPLE buf[3712];
        da.line_read(0, buf);
}

}

/* vim:set ts=4 sw=4: */
