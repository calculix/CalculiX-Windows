#!/bin/sh
#####################################################################################
# Name:          clean.sh
# Description:   Removes previous build, downloads and output directories
# Author:        Cesare Guardino
# Last modified: 19 January 2016
#
# GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
#####################################################################################

# {{{ DEFINE UTILITY FUNCTIONS
remove_dir() {
    dir=$1

    rm -rf $dir > /dev/null 2>&1
}
# }}}

# {{{ DEFINE PROCESS FUNCTIONS
start() {
    echo "======================== CALCULIX WINDOWS MINGW-W64 CLEAN SCRIPT ========================"
}

initialise() {
    echo ""

    BUILD_HOME=`pwd`
    ARCH="x64"

    BUILD_DIR=$BUILD_HOME/$ARCH/build
    INSTALL_DIR=$BUILD_HOME/$ARCH/install
    OUT_DIR=$BUILD_HOME/$ARCH/output
}

cleanup() {
    echo ""
    echo "Removing previous builds ..."

    remove_dir $BUILD_DIR
    remove_dir $INSTALL_DIR
    remove_dir $OUT_DIR
    remove_dir $BUILD_HOME/downloads
}

finish() {
    echo ""
    echo "All done!"
}
# }}}

# {{{ MAIN EXECUTION
cd ${0%/*} || exit 1    # run from this directory
start
initialise
cleanup
finish
# }}}
