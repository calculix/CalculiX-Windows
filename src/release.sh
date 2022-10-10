#!/bin/sh
#####################################################################################
# Name:          release.sh
# Description:   CalculiX binary package creation script for MINGW
# Author:        Cesare Guardino
# Last modified: 10 October 2022
#
# GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
#####################################################################################

cd ${0%/*} || exit 1    # run from this directory

download() {
    file=$1
    url=$2

    if [ -f $CALCULIX_HOME/downloads/$file ] ; then
        echo "Using already existing file $CALCULIX_HOME/downloads/$file"
    else
        wget --no-check-certificate $url -O $CALCULIX_HOME/downloads/$file
    fi
}

mkchk() {
    dir=$1

    if [ ! -d $dir ] ; then
        mkdir $dir
    fi
}

create_package() {
    PACKAGE_DIR=$1
    TYPE=$2

    echo ""

    PACKAGE_FILE=$PACKAGE_DIR.$TYPE
    if [ -f $PACKAGE_FILE ] ; then
        echo "Removing previous $PACKAGE_FILE ..."
        rm -f $PACKAGE_FILE
    fi

    echo "Creating $PACKAGE_FILE ..."
    7z -t$TYPE a $PACKAGE_FILE $PACKAGE_DIR
    
    if [ -f $PACKAGE_FILE ] ; then
        echo "Successfully created $PACKAGE_FILE"
    else
        echo "Failed to create $PACKAGE_FILE"
    fi
}

get_args() {
    if [ $1 ] ; then
        CGX_VERSION=$1
    else
        echo "*** ERROR: Please specify the version of CalculiX CGX (pre/post) to build (eg. 2.10)"
        exit 1
    fi

    if [ $2 ] ; then
        CCX_VERSION=$2
    else
        echo "*** ERROR: Please specify the version of CalculiX CCX (solver) to build (eg. 2.10)"
        exit 1
    fi

    if [ $3 ] ; then
        TAG=$3
        echo "*** WARNING: Using tag '$TAG' instead of default"
    else
        TAG=GE-OSS
    fi
}

echo
echo "=========== CALCULIX STAND-ALONE PACKAGE CREATION SCRIPT FOR WINDOWS ==========="

ARCH="x64"

CALCULIX_HOME=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
get_args $*

RELEASE_DIR=$CALCULIX_HOME/releasePackages
if [ ! -d $RELEASE_DIR ] ; then
    echo "Creating $RELEASE_DIR ..."
    mkdir $RELEASE_DIR
fi

PACKAGE_DIR=$RELEASE_DIR/CalculiX-$TAG-$CCX_VERSION-win-$ARCH
if [ -d $PACKAGE_DIR ] ; then
    echo "Removing previous $PACKAGE_DIR ..."
    rm -rf $PACKAGE_DIR
fi
echo "Creating $PACKAGE_DIR ..."
mkdir $PACKAGE_DIR

echo "Download missing packages ..."
mkchk $CALCULIX_HOME/downloads
download ccx_${CCX_VERSION}.pdf http://www.dhondt.de/ccx_${CCX_VERSION}.pdf
download cgx_${CGX_VERSION}.pdf http://www.dhondt.de/cgx_${CGX_VERSION}.pdf
download cgx_${CGX_VERSION}.exa.tar.bz2 http://www.dhondt.de/cgx_${CGX_VERSION}.exa.tar.bz2
download coreutils-5.97-3-msys-1.0.13-src.tar.lzma http://sourceforge.net/projects/mingw/files/MSYS/Base/coreutils/coreutils-5.97-3/coreutils-5.97-3-msys-1.0.13-src.tar.lzma/download
download dos2unix-7.2.3-1-msys-1.0.18-src.tar.lzma http://sourceforge.net/projects/mingw/files/MSYS/Extension/dos2unix/dos2unix-7.2.3-1/dos2unix-7.2.3-1-msys-1.0.18-src.tar.lzma/download

echo "Copying files to package directory ..."
cp -rp $CALCULIX_HOME/$ARCH/install $PACKAGE_DIR/bin
cp -rp $CALCULIX_HOME/etc $PACKAGE_DIR
rm -f $PACKAGE_DIR/etc/bashrc.mingw
mkdir $PACKAGE_DIR/doc
cp -p $CALCULIX_HOME/doc/OSS_NOTICE.pdf $PACKAGE_DIR/OSS_NOTICE.pdf
cp -p $CALCULIX_HOME/doc/README_$TAG.txt $PACKAGE_DIR/README.txt
cp -p $CALCULIX_HOME/downloads/ccx_${CCX_VERSION}.pdf $PACKAGE_DIR/doc
cp -p $CALCULIX_HOME/downloads/cgx_${CGX_VERSION}.pdf $PACKAGE_DIR/doc
cp -p $CALCULIX_HOME/downloads/cgx_${CGX_VERSION}.exa.tar.bz2 $PACKAGE_DIR/doc

echo "Copying required MSYS files ..."
mkdir $PACKAGE_DIR/utils
cp -p $CALCULIX_HOME/doc/LICENSE_MINGW.txt $PACKAGE_DIR/utils
cp -p $MSYS_HOME/share/licenses/dos2unix/LICENSE $PACKAGE_DIR/utils/LICENSE_DOS2UNIX.txt
cp -p $MSYS_HOME/bin/msys-2.0.dll $PACKAGE_DIR/utils
cp -p $MSYS_HOME/bin/msys-iconv-2.dll $PACKAGE_DIR/utils
cp -p $MSYS_HOME/bin/msys-intl-8.dll $PACKAGE_DIR/utils
cp -p $MSYS_HOME/bin/rm.exe $PACKAGE_DIR/utils
cp -p $MSYS_HOME/bin/tail.exe $PACKAGE_DIR/utils
cp -p $MSYS_HOME/bin/tee.exe $PACKAGE_DIR/utils
cp -p $MSYS_HOME/bin/unix2dos.exe $PACKAGE_DIR/utils
cp -p $CALCULIX_HOME/downloads/*.lzma $PACKAGE_DIR/utils

echo "Adding packaging information ..."
PACKAGE_INFO_LOG=$PACKAGE_DIR/etc/packageInfo.log
touch $PACKAGE_INFO_LOG
echo "Package creation date : " `date` >> $PACKAGE_INFO_LOG
echo "CGX                   :  $CGX_VERSION" >> $PACKAGE_INFO_LOG
echo "CCX                   :  $CCX_VERSION" >> $PACKAGE_INFO_LOG
unix2dos $PACKAGE_INFO_LOG

echo "Creating archives ..."
create_package $PACKAGE_DIR zip
create_package $PACKAGE_DIR 7z

echo "Removing package directory ..."
rm -rf $PACKAGE_DIR

echo "All done!"
