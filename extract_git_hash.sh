#!/bin/bash

# SVN Revision Extractor for Unix systems
# Written by Burlex, 2008/02/20
# Rewritten by Twl, 2010/05/10

hash=`git log | grep commit | cut -f2 -d' ' > lg && read -r hd < lg && echo $hd`
rm -f lg
echo "// This file was automatically generated by the GIT revision extractor." > git_revision.h
echo "// There is no need to modify it." >> git_revision.h
echo "" >> git_revision.h
echo "#ifndef GIT_REVISION_H" >> git_revision.h
echo "#define GIT_REVISION_H" >> git_revision.h
echo "" >> git_revision.h
echo "static const char * BUILD_TAG = \"ASCNHALF-1.0\";" >> git_revision.h
echo "static const char * BUILD_HASH = \""$hash"\";" >> git_revision.h
echo "static int BUILD_REVISION = 200;" >> git_revision.h
echo "static const char * BUILD_USER = \"$USER\";" >> git_revision.h
echo "static const char * BUILD_HOST = \"`hostname -f`\";" >> git_revision.h
echo "" >> git_revision.h
echo "#endif         // GIT_REVISION_H" >> git_revision.h
echo "" >> git_revision.h
mv ./git_revision.h ./src/ascnhalf-shared
echo $hash;
