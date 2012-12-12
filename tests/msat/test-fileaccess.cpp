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
#include <msat/xrit/fileaccess.h>

using namespace msat;

namespace tut {

#define TESTDATA_LINEAR    DATA_DIR "/H:MSG2:VIS006:200807150900"
#define TESTDATA_HRV       DATA_DIR "/H:MSG1:HRV:200611141200"
#define TESTDATA_NONLINEAR DATA_DIR "/H:MSG1:IR_039:200611130800"

struct fileaccess_shar
{
        fileaccess_shar()
        {
        }

        ~fileaccess_shar()
        {
        }
};
TESTGRP(fileaccess);

// Test the isXRIT function
template<> template<>
void to::test<1>()
{
    // Shortened
    gen_ensure(xrit::isValid(TESTDATA_LINEAR));
    gen_ensure(xrit::isValid(TESTDATA_HRV));
    gen_ensure(xrit::isValid(TESTDATA_NONLINEAR));
    gen_ensure(!xrit::isValid(DATA_DIR "/H:MSG1:HRV"));
    gen_ensure(!xrit::isValid(DATA_DIR));

    // Segment filename
    gen_ensure(xrit::isValid("/foo/bar/H-000-MSG1__-MSG1________-IR_039___-000001___-200611130800-C_"));
}

// Test parser with shortened names
template<> template<>
void to::test<2>()
{
    xrit::FileAccess fa;
    fa.parse(TESTDATA_LINEAR);
    gen_ensure_equals(fa.directory, DATA_DIR);
    gen_ensure_equals(fa.resolution, "H");
    gen_ensure_equals(fa.productid1, "MSG2");
    gen_ensure_equals(fa.productid2, "VIS006");
    gen_ensure_equals(fa.timing, "200807150900");
}

// Test parser with segment filenames
template<> template<>
void to::test<3>()
{
    xrit::FileAccess fa;
    fa.parse("/foo/bar/H-000-MSG1__-MSG1________-IR_039___-000001___-200611130800-C_");
    gen_ensure_equals(fa.directory, "/foo/bar");
    gen_ensure_equals(fa.resolution, "H");
    gen_ensure_equals(fa.productid1, "MSG1");
    gen_ensure_equals(fa.productid2, "IR_039");
    gen_ensure_equals(fa.timing, "200611130800");
}

// Test parser with alternate channel
template<> template<>
void to::test<4>()
{
    xrit::FileAccess fa1;
    fa1.parse(TESTDATA_LINEAR);

    xrit::FileAccess fa;
    fa.parse(fa1, "PIPPO");

    gen_ensure_equals(fa.directory, DATA_DIR);
    gen_ensure_equals(fa.resolution, "H");
    gen_ensure_equals(fa.productid1, "MSG2");
    gen_ensure_equals(fa.productid2, "PIPPO");
    gen_ensure_equals(fa.timing, "200807150900");
}

}

/* vim:set ts=4 sw=4: */
