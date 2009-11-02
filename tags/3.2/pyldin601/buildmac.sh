#!/bin/sh

if [ "x$ARCH" = "x" ]; then
    ARCH=`uname -m`
    case $ARCH in
	i*86)
	    ARCH=i386
	    ;;
    esac
fi

VERSION=3.2rev`unset LC_ALL LANG ; svn info | grep Revision | cut -f2 -d' '`

TMPINST=/tmp/pyldin601_${VERSION}_osx_${ARCH}

make -f Makefile.OSX EXTRA_CFLAGS="-I/opt/local/include" $1

make -f Makefile.OSX EXTRA_CFLAGS="-I/opt/local/include" DESTDIR=$TMPINST install

cd $TMPINST && zip -r9 $TMPINST.zip .

rm -rf $TMPINST
