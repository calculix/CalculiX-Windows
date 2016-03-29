@echo off
rem #####################################################################################
rem Name:          CalculiXWindowsEnvironment.bat
rem Description:   Environment setup script for use with Windows CMD prompt shells
rem Author:        Cesare Guardino
rem Last modified: 19 January 2016
rem 
rem GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
rem #####################################################################################

if defined CALCULIX_ENV_SET goto :eof

rem =========== USER EDITABLE SETTINGS ===========
rem set GNUPLOT_HOME=C:\Programs\gp501-win64-mingw
rem set IMAGEMAGICK_HOME=C:\Programs\ImageMagick-6.9.1-2
rem ==============================================

if defined GNUPLOT_HOME set PATH=%GNUPLOT_HOME%\bin;%PATH%
if defined IMAGEMAGICK_HOME set PATH=%IMAGEMAGICK_HOME%;%PATH%
set PATH=%~dp0..\bin;%~dp0..\utils;%PATH%
set PRINTF_EXPONENT_DIGITS=2
set CCX_NPROC_STIFFNESS=1
set CALCULIX_ENV_SET=1
