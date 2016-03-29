@echo off
rem #####################################################################################
rem Name:          runCGX.bat
rem Description:   Startup script for CalculiX CCX used by file associations
rem Author:        Cesare Guardino
rem Last modified: 19 January 2016
rem 
rem GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
rem #####################################################################################

set CALCULIX_BIN_DIR=%~dp0
if #%CALCULIX_BIN_DIR:~-1%# == #\#  set CALCULIX_BIN_DIR=%CALCULIX_BIN_DIR:~0,-1%
call "%CALCULIX_BIN_DIR%\CALCULIXWindowsEnvironment.bat"
set CALCULIX_BIN_DIR=
call "%~dp0..\etc\CALCULIXWindowsEnvironment.bat"

:start
  cls
  set oldcwd=%cd%
  for %%F in ("%~1") do set dirname=%%~dpF
  if #%dirname:~-1%# == #\#  set dirname=%dirname:~0,-1%
  cd /d "%dirname%"
  if  %~x1==.inp goto ccxc
  goto end

:ccxc
  ccx_MT.exe %~n1
  goto end

:end
  cd /d "%oldcwd%"
  set oldcwd=
  set dirname=
  pause
