//---------------------------------------------------------------------------
//
//  File        :   OpenMTP_info
//  Description :   Print Meteosat OpenMTP file header
//  Project     :   -
//  Author      :   Deneys S. Maartens
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

#include <config.h>

#include <cstdio>
#include <cmath>
#include <cstdlib>

// OpenMTP format interface

#include <msat/openmtp/OpenMTP.h>
#include <libgen.h>

static bool
OpenMTP_info( char *inpath )
{
  OpenMTP omtp;
  omtp.open(inpath);
  std::cout << omtp;
  return true;
}

//---------------------------------------------------------------------------

int
main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cout << "Usage : "
      << basename(argv[0])
      << " OpenMTP_filename\n";

    return EXIT_FAILURE;
  }

  if (!strcmp(argv[1], "-V"))
  {
    std::cout << argv[0] << " " << PACKAGE_STRING << std::endl;
    return 0;
  }

  if (!OpenMTP_info(argv[1]))
  {
    std::cerr << "Cannot get OpenMPT info from " << argv[1] << std::endl;
    return 1;
  }


  return EXIT_SUCCESS;
}
//---------------------------------------------------------------------------
