#!/bin/bash

#
# PublicDecompWT library Linux
#

[ -d COMP -o -f PublicDecompWT.zip ] || {
  cat LICENSE
  exit 1
}

[ -f PublicDecompWT.zip ] && {
  unzip=`which unzip`
  [ "$unzip" = "" ] && {
    echo "No unzip program found in Your path."
    echo "Please unzip PublicDecompWT.zip manually"
    exit 1
  }
  $unzip -a PublicDecompWT.zip
  if [ $? -ne 0 ]
  then
    echo "Error decompressing EUMETSAT sources."
    exit 1
  fi
  mv PublicDecompWT.zip Original_PublicDecompWT.zip
}

[ ! -f pdwt.patched ] && {
    dos2unix=`which dos2unix`
    [ "$dos2unix" != "" ] && {
      find . -name "*.cpp" | xargs $dos2unix
      find . -name "*.h"   | xargs $dos2unix
    }
    patch -p1 < patch_EUMETSAT || {
    echo "Failed to patch EUTMETSAT sources."
    exit 1
  }
  touch pdwt.patched
}

BASE=$PWD

cd $BASE/COMP
if [ $? -ne 0 ]
then
  echo "Error in EUMETSAT Sources."
  exit 1
fi
CPPFLAGS="-O2 -Wall -pedantic -Wno-long-long " make
if [ $? -ne 0 ]
then
  echo "Error Compiling EUMETSAT Sources."
  exit 1
fi

cd $BASE/DISE
if [ $? -ne 0 ]
then
  echo "Error in EUMETSAT Sources."
  exit 1
fi
CPPFLAGS="-O2 -Wall -pedantic -Wno-long-long " make
if [ $? -ne 0 ]
then
  echo "Error Compiling EUMETSAT Sources."
  exit 1
fi

echo "Done"
