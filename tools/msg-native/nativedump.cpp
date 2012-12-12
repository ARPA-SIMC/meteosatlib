//---------------------------------------------------------------------------
//
//  File        :   nativedump.cpp
//  Description :   Dump MSG Native format image in ASCII format
//  Project     :   Lamma 2005
//  Author      :   Graziano Giuliani (Lamma Regione Toscana)
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
#include <iostream>
#include <msat/msg-native/MSG_native.h>

int main(int argc, char **argv)
{
  MSG_native native;

  if (argc < 2) return 1;
  if (! native.open(argv[1])) return 1;

  native.read( );

  std::cout << native;

  native.close( );
  return 0;
}
