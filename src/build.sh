#!/bin/sh
#####################################################################################
# Name:          build.sh
# Description:   CalculiX build script for MINGW
# Author:        Cesare Guardino
# Last modified: 22 April 2016
#
# GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
#####################################################################################

# {{{ DEFINE UTILITY FUNCTIONS
download() {
    file=$1
    url=$2

    if [ -f $BUILD_HOME/downloads/$file ] ; then
        echo "Using already existing file $BUILD_HOME/downloads/$file"
    else
        wget --no-check-certificate $url -O $BUILD_HOME/downloads/$file
    fi
}

extract() {
    file=$1
    program=$2

    cp -p $BUILD_HOME/downloads/$file .
    package=`basename $file`
    if [ "$program" = "7zip" ] ; then
        "$ZIP_EXE" x $package
    else
        $program -cd $package | tar xvf -
    fi
    rm $package
}

unzip_dir() {
    dir=$1

    mkdir $dir
    cd $dir
    extract $dir.zip 7zip
    cd ..
}

patch() {
    dir=$1

    cp -rp $BUILD_HOME/$ARCH/patches/$dir .
}

mkchk() {
    dir=$1

    if [ ! -d $dir ] ; then
        mkdir $dir
    fi
}

mkdel() {
    dir=$1

    rm -rf $dir > /dev/null 2>&1
    mkdir $dir
}
# }}}

# {{{ DEFINE PROCESS FUNCTIONS
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
        RUN_TESTS=1
        if [ $3 != "test" ] ; then
            echo "*** ERROR: Invalid 3rd argument specified (did you mean test)?"
            exit 1
        fi
    fi
}

start() {
    echo "======================== CALCULIX FOR WINDOWS BUILD SCRIPT ========================"
}

initialise() {
    echo ""

    if [ ! "$MINGW_HOME" ] ; then
        echo "*** ERROR: MINGW_HOME environment variable not specified."
        exit 1
    else
        echo "Using MINGW_HOME=$MINGW_HOME"
    fi

    BUILD_HOME=`pwd`
    ZIP_EXE="7z.exe"
    ARCH="x64"

    BUILD_DIR=$BUILD_HOME/$ARCH/build
    INSTALL_DIR=$BUILD_HOME/$ARCH/install
    OUT_DIR=$BUILD_HOME/$ARCH/output

    mkchk $BUILD_HOME/downloads

    echo ""
    echo "All stdout/stderr output is redirected to the directory $OUT_DIR"
    echo "All builds occur in the directory $BUILD_DIR"
    echo "The script will install the completed builds in the directory $INSTALL_DIR"
}

cleanup() {
    echo ""
    echo "Removing previous builds ..."

    mkdel $BUILD_DIR
    mkdel $INSTALL_DIR
    mkdel $OUT_DIR
}

build_library() {
    PACKAGE=$1

    echo "- Building $PACKAGE ..."
    LOG_FILE=$OUT_DIR/$PACKAGE.log
    cd $BUILD_DIR

    case $PACKAGE in

        pthreads-w32-2-9-1-release)
            download $PACKAGE.zip ftp://sourceware.org/pub/pthreads-win32/pthreads-w32-2-9-1-release.zip > $LOG_FILE 2>&1
            unzip_dir $PACKAGE >> $LOG_FILE 2>&1
            patch $PACKAGE
            ;;

        SPOOLES.2.2)
            download spooles.2.2.tgz http://netlib.sandia.gov/linalg/spooles/spooles.2.2.tgz > $LOG_FILE 2>&1
            mkdir $PACKAGE
            cd $PACKAGE
            extract "spooles.2.2.tgz" gzip >> $LOG_FILE 2>&1
            cd ..
            patch $PACKAGE
            cd $PACKAGE
            make lib >> $LOG_FILE 2>&1
            ;;

        ARPACK)
            download arpack96.tar.gz http://www.caam.rice.edu/software/ARPACK/SRC/arpack96.tar.gz > $LOG_FILE 2>&1
            download arpack96_patch.tar.gz http://www.caam.rice.edu/software/ARPACK/SRC/patch.tar.gz >> $LOG_FILE 2>&1
            extract "arpack96.tar.gz" gzip >> $LOG_FILE 2>&1
            extract "arpack96_patch.tar.gz" gzip >> $LOG_FILE 2>&1
            patch $PACKAGE
            cd $PACKAGE
            export ARPACK_HOME=$BUILD_DIR/$PACKAGE    ### NOTE: ARmake.inc uses ARPACK_HOME environment variable to find out where to build sources
            make all >> $LOG_FILE 2>&1
            ;;

        glut-3.7.6-bin)
            download $PACKAGE-32and64.zip http://www.ece.lsu.edu/xinli/OpenGL/glut-3.7.6-bin-32and64.zip > $LOG_FILE 2>&1
            unzip_dir $PACKAGE-32and64 >> $LOG_FILE 2>&1
            cd $PACKAGE-32and64
            mv $PACKAGE GL                    ### NOTE: required because CGX sources need to include "GL/glut.h"
            cd GL
            if [ $ARCH = "x86" ] ; then
                cp -p glut32.lib libglut32.a  ### NOTE: required because of naming convention of including libraries
            else
                cp -p glut64.lib libglut64.a  ### NOTE: required because of naming convention of including libraries
            fi
            ;;

        CalculiX)
            CCX_PACKAGE_NAME=ccx_$CCX_VERSION.src.tar.bz2
            CGX_PACKAGE_NAME=cgx_$CGX_VERSION.all.tar.bz2
            download $CCX_PACKAGE_NAME http://www.dhondt.de/$CCX_PACKAGE_NAME > $LOG_FILE 2>&1
            download $CGX_PACKAGE_NAME http://www.dhondt.de/$CGX_PACKAGE_NAME >> $LOG_FILE 2>&1

            echo "### Extracting ..." >> $LOG_FILE 2>&1
            extract "$CCX_PACKAGE_NAME" bzip2 >> $LOG_FILE 2>&1
            extract "$CGX_PACKAGE_NAME" bzip2 >> $LOG_FILE 2>&1
            echo "### Patching ..." >> $LOG_FILE 2>&1
            patch CalculiX

            echo "### Building CalculiX CCX (sequential version) ..." >> $LOG_FILE 2>&1
            cd $BUILD_DIR/CalculiX/ccx_$CCX_VERSION/src
            make >> $LOG_FILE 2>&1
            echo "### Building CalculiX CCX (multi-threaded version) ..." >> $LOG_FILE 2>&1
            rm *.o *.a > /dev/null 2>&1
            make -f Makefile_MT >> $LOG_FILE 2>&1

            echo "- Building CalculiX CGX ..." >> $LOG_FILE 2>&1
            cd $BUILD_DIR/CalculiX/cgx_$CGX_VERSION/src
            make >> $LOG_FILE 2>&1
            ;;

        *)
            echo "*** ERROR: Unknown package '$PACKAGE'"
            exit 1
            ;;
    esac
}

build_libraries() {
    echo ""
    echo "Building libraries ..."
    build_library pthreads-w32-2-9-1-release
    build_library SPOOLES.2.2
    build_library ARPACK
    build_library glut-3.7.6-bin
    build_library CalculiX
}

create_dirs() {
    echo ""
    echo "Checking for build directories and creating them if required ..."

    mkchk $BUILD_DIR
    mkchk $INSTALL_DIR
    mkchk $OUT_DIR
}

run_tests() {
    echo ""
    echo "Testing ..."

    LOG_FILE=$OUT_DIR/CalculiX_CCX_test.log

    cd $BUILD_DIR

    CCX_TEST_PACKAGE_NAME=ccx_$CCX_VERSION.test.tar.bz2
    download $CCX_TEST_PACKAGE_NAME http://www.dhondt.de/$CCX_TEST_PACKAGE_NAME > $LOG_FILE 2>&1
    echo "### Extracting ..." >> $LOG_FILE 2>&1
    extract "$CCX_TEST_PACKAGE_NAME" bzip2 >> $LOG_FILE 2>&1
    echo "### Patching ..." >> $LOG_FILE 2>&1
    patch CalculiX

    echo "### Running CalculiX CCX tests ..." >> $LOG_FILE 2>&1
    cd $BUILD_DIR/CalculiX/ccx_$CCX_VERSION/test
    export PRINTF_EXPONENT_DIGITS=2         ### NOTE: Enable 2 digits in exponents (default is 3). See http://sourceforge.net/project/shownotes.php?release_id=24832
    export CCX_NPROC_STIFFNESS=1            ### NOTE: Required to avoid convergence issues when running multi-threaded (see CCX documentation).
    ./compare >> $LOG_FILE 2>&1
}

install() {
    echo ""
    echo "Installing ..."

    cd $INSTALL_DIR
    cp -p $BUILD_HOME/doc/LICENSE.txt .
    cp -p $BUILD_DIR/CalculiX/ccx_$CCX_VERSION/src/ccx_$CCX_VERSION ccx.exe
    cp -p $BUILD_DIR/CalculiX/ccx_$CCX_VERSION/src/ccx_${CCX_VERSION}_MT ccx_MT.exe
    cp -p $BUILD_DIR/CalculiX/cgx_$CGX_VERSION/src/cgx.exe cgx.exe
    cp -p $MINGW_HOME/bin/libgfortran-3.dll .
    cp -p $MINGW_HOME/bin/libgomp-1.dll .
    cp -p $MINGW_HOME/bin/libquadmath-0.dll .
    cp -p $MINGW_HOME/bin/libstdc++-6.dll .
    cp -p $MINGW_HOME/bin/libwinpthread-1.dll .
    if [ $ARCH = "x86" ] ; then
        cp -p $BUILD_DIR/glut-3.7.6-bin-32and64/GL/glut32.dll .
        cp -p $MINGW_HOME/bin/libgcc_s_sjlj-1.dll .
    else
        cp -p $BUILD_DIR/glut-3.7.6-bin-32and64/GL/glut64.dll .
        cp -p $MINGW_HOME/bin/libgcc_s_seh-1.dll .
        cp -p $BUILD_DIR/pthreads-w32-2-9-1-release/Pre-built.2/dll/x64/pthreadGC2.dll .
    fi

    echo "Adding build information ..."
    BUILD_INFO_LOG=buildInfo.log
    touch $BUILD_INFO_LOG
    echo "Build date : " `date` >> $BUILD_INFO_LOG
    echo "CGX        :  $CGX_VERSION" >> $BUILD_INFO_LOG
    echo "CCX        :  $CCX_VERSION" >> $BUILD_INFO_LOG
    echo "G++        : " `(g++ --version 2>&1) 2> /dev/null | head -1` >> $BUILD_INFO_LOG
    echo "GCC        : " `(gcc --version 2>&1) 2> /dev/null | head -1` >> $BUILD_INFO_LOG
    echo "GFortran   : " `(gfortran --version 2>&1) 2> /dev/null | head -1`  >> $BUILD_INFO_LOG
    unix2dos $BUILD_INFO_LOG
}

finish() {
    echo ""
    echo "All done!"
}
# }}}

# {{{ MAIN EXECUTION
cd ${0%/*} || exit 1    # run from this directory
start
get_args $*
initialise
if [ $RUN_TESTS ] ; then
    create_dirs
    run_tests
else
    cleanup
    build_libraries
    install
fi
finish
# }}}
