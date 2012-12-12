//---------------------------------------------------------------------------
//
//  File        :   xritdump.cpp
//  Description :   Dump the contents and structure of an xRIT dataset
//  Author      :   Enrico Zini (for ARPA SIM Emilia Romagna)
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

#include <config.h>

#include <msat/xrit/fileaccess.h>
#include <msat/xrit/dataaccess.h>
#include <msat/hrit/MSG_HRIT.h>

#include <stdint.h>
#include <set>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cstdio>

#include <getopt.h>

using namespace std;
using namespace msat::xrit;

void do_help(const char* argv0, ostream& out)
{
        out << "Usage: " << argv0 << " [options] file(s)..." << endl << endl
            << "Dump contents and structure of an xRIT dataset." << endl << endl
            << "Options are:" << endl
            << "  --help           Print this help message" << endl;
}

void do_dump(const char* name)
{
        FileAccess fa(name);
        DataAccess da;

        MSG_data pro;
        MSG_data epi;
        MSG_header header;

        da.scan(fa, pro, epi, header);

        cout << "Columns: " << da.columns << endl
             << "Lines: " << da.lines << endl
             << "Segments: " << da.segnames.size() << endl
             << "Pixels per segment: " << da.npixperseg << endl
             << "Scanlines per segment: " << da.seglines << endl
             << "East is: " << (da.swapX ? "left" : "right") << endl
             << "North is: " << (da.swapY ? "down" : "up") << endl
             << "HRV: " << (da.hrv ? "yes" : "no") << endl;

        if (da.hrv)
        {
                cout << "LowerEastColumnActual: " << da.LowerEastColumnActual << endl
                     << "LowerSouthLineActual: " << da.LowerSouthLineActual << endl
                     << "LowerWestColumnActual: " << da.LowerWestColumnActual << endl
                     << "LowerNorthLineActual: " << da.LowerNorthLineActual << endl
                     << "UpperEastColumnActual: " << da.UpperEastColumnActual << endl
                     << "UpperSouthLineActual: " << da.UpperSouthLineActual << endl
                     << "UpperWestColumnActual: " << da.UpperWestColumnActual << endl
                     << "UpperNorthLineActual: " << da.UpperNorthLineActual << endl;
        } else {
                cout << "SouthLineActual: " << da.SouthLineActual << endl
                     << "WestColumnActual: " << da.WestColumnActual << endl;
        }

        string aachars = " .,-:=$#";
        MSG_SAMPLE compmin = 0xffff, compmax = 0;
        for (size_t i = 0; i < da.segnames.size(); ++i)
        {
                cout << "Segment " << i << ": ";
                MSG_data* d = da.segment(i);
                MSG_SAMPLE min = 0xffff, max = 0;
                if (!d)
                {
                        cout << "missing." << endl;
                        continue;
                } else {
                        for (size_t i = 0; i < da.npixperseg; ++i)
                        {
                                if (d->image->data[i] < min) min = d->image->data[i];
                                if (d->image->data[i] > max) max = d->image->data[i];
                        }
                        cout << "min " << min << " max " << max << endl;
                }
                if (min < compmin) compmin = min;
                if (max > compmax) compmax = max;
                // Size of the ascii-art rendition of the segment
                size_t charx = 50, chary = 3;
                for (size_t y = 0; y < chary; ++y)
                {
                        cout << setw(2) << setfill('0') << i << " ";
                        for (size_t x = 0; x < charx; ++x)
                        {
                                size_t px = x * da.columns / charx;
                                size_t py = y * da.seglines / chary;
                                MSG_SAMPLE s = d->image->data[py * da.columns + px];
                                char c = aachars[(s-min) * aachars.size() / (max-min)];
                                cout << c;
                        }
                        cout << endl;
                }
                // TODO: dump segment information
                // TODO: optionally dump segment as a JPG/PNG file
        }

        cout << "Composite image:" << endl;
        size_t comp_sx, comp_sy;
        if (da.hrv)
                comp_sx = comp_sy = 11136;
        else
                comp_sx = comp_sy = 3712;
        // Size of the ascii-art rendition of the segment
        size_t charx = 50, chary = 24;
        for (size_t y = 0; y < chary; ++y)
        {
                size_t py = y * comp_sy / chary;
                MSG_SAMPLE buf[comp_sx];
                bzero(buf, comp_sx * sizeof(MSG_SAMPLE));
                da.line_read(py, buf);
                for (size_t x = 0; x < charx; ++x)
                {
                        size_t px = x * comp_sx / charx;
                        MSG_SAMPLE s = buf[px];
                        char c = aachars[(s-compmin) * aachars.size() / (compmax-compmin)];
                        cout << c;
                }
                cout << endl;
        }
        // TODO: dump segment information
        // TODO: optionally dump segment as a JPG/PNG file

        cout << endl;
}

int main( int argc, char* argv[] )
{
        static struct option longopts[] = {
                { "help", 0, NULL, 'H' },
//                { "area", 1, 0, 'a' },
//                { "pixels", 0, 0, 'p' },
                { 0, 0, 0, 0 },
        };

        bool done = false;
        while (!done) {
                int c = getopt_long(argc, argv, "i:", longopts, (int*)0);
                switch (c) {
                        case 'H': // --help
                                do_help(argv[0], cout);
                                return 0;
#if 0
                        case 'a':
                                if (sscanf(optarg, "%d,%d,%d,%d", &ax,&ay,&aw,&ah) != 4)
                                {
                                        cerr << "Area value should be in the format x,y,width,height" << endl;
                                        do_help(argv[0], cerr);
                                        return 1;
                                }
                                break;
                        case 'p':
                                fromlatlon = false;
                                break;
#endif
                        case -1:
                                done = true;
                                break;
                        default:
                                cerr << "Error parsing commandline." << endl;
                                do_help(argv[0], cerr);
                                return 1;
                }
        }

        if (optind == argc)
        {
                do_help(argv[0], cerr);
                return 1;
        }

        // Read the images
        try
        {
                for (int i = optind; i < argc; ++i)
                        do_dump(argv[i]);
        }
        catch (std::exception& e)
        {
                cerr << e.what() << endl;
                return 1;
        }

        return 0;
}

// vim:set ts=2 sw=2:
