#!/bin/bash -ue

#
# This script manages the PublicDecompWT library
#

# Find the name of the patch file for a PublicDecompWT version
findpatch() {
	VER="$1"
	# First try to pick from a database of known, working versions
	while read cand patch
	do
		if [ $VER = $cand ]
		then
			echo $patch
			return 0
		fi
	done << EOT
2.02 patch_EUMETSAT_2.02
2.04 patch_EUMETSAT_2.04
EOT
	# If all is unknown, use a default
	echo "PublicDecompWT version $VER is unknown: I will try the newest" >&2
	echo "patch that I have, and hope that it will work" >&2
	echo patch_EUMETSAT_2.04
	return 1
}

# If the COMP or DISE directories are missing, we regenerate them
if [ ! -d COMP -o ! -d DISE ]
then
  rm -f pdwt.patched
fi

# If we need to regenerate the library but we do not have the source zipfile,
# we stop here
if [ ! -f pdwt.patched -a ! -f PublicDecompWT.zip ]
then
  cat LICENSE
  exit 1
fi

# Regenerate the patched sources if needed
if [ ! -f pdwt.patched ]
then
  if ! which unzip > /dev/null
  then
    echo "no unzip program found." >&2
    exit 1
  fi
  if ! which dos2unix > /dev/null
  then
    echo "no dos2unix program found." >&2
    exit 1
  fi

  # Delete old files
  rm -rf COMP DISE

  # Extract the PublicDecompWT version
  PDVER=`unzip -Z1 PublicDecompWT.zip | grep '^[0-9]' | head -1 | cut -f1 -d/`
  PATCHFILE=`findpatch $PDVER`

  # Extract the new files
  if ! unzip -a PublicDecompWT.zip "$PDVER/COMP/*" "$PDVER/DISE/*"
  then
    echo "Cannot decompress EUMETSAT sources." >&2
    exit 1
  fi

  # Get what we want out of the versioned directory
  mv $PDVER/{COMP,DISE} .
  rmdir $PDVER

  # Fix newline encoding
  find COMP DISE -name "*.cpp" -or -name "*.h" -or -name "Makefile" | xargs dos2unix

  # Apply our patch
  if ! patch -p1 < $PATCHFILE
  then
    echo "Failed to patch EUTMETSAT sources." >&2
    exit 1
  fi

  touch pdwt.patched
fi

if ! (
  cd COMP
  CPPFLAGS="-O2 -Wall -pedantic -Wno-long-long -fPIC " make
  cd ../DISE
  CPPFLAGS="-O2 -Wall -pedantic -Wno-long-long -fPIC " make
)
then
  echo "Error Compiling EUMETSAT Sources." >&2
  exit 1
fi

echo "Done"
