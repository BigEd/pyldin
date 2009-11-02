#!/bin/bash


if [ "x$ARCH" = "x" ]; then
    ARCH=`dpkg-architecture -qDEB_BUILD_ARCH`
fi

VERSION=3.2rev`unset LC_ALL LANG ; svn info | grep Revision | cut -f2 -d' '`

TMPINST=/tmp/pyldin601_${VERSION}_${ARCH}

make $@

make DESTDIR=${TMPINST} install

tar jcvf /tmp/pyldin601_${VERSION}_${ARCH}.tar.bz2 --group 0 --owner 0 -C ${TMPINST} .

mkdir ${TMPINST}/DEBIAN

cat > ${TMPINST}/DEBIAN/control << EOF
Package: pyldin601
Version: $VERSION
Section: games
Priority: optional
Architecture: $ARCH
Depends: libsdl1.2debian, libsdl-image1.2, libjpeg62, libpng12-0
Maintainer: Alexander Chukov <sash@pdaXrom.org>
Description: Pyldin601 emulator
EOF

dpkg-deb --build ${TMPINST}

rm -rf ${TMPINST}
