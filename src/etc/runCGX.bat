@echo off
rem #####################################################################################
rem Name:          runCGX.bat
rem Description:   Startup script for CalculiX CGX used by file associations
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
  if  %~x1==.inp goto cgxc
  if  %~x1==.fbd goto cgxb
  if  %~x1==.foam goto cgxfoam
  if  %~x1==.frd goto cgx
  if  %~x1==.stl goto cgxstl
  if  %~x1==.stp goto cgxstp
  goto end

:cgxc
  cgx.exe -c %1
  goto end

:cgxb
  cgx.exe -b %1
  goto end

:cgxfoam
  cgx.exe -foam %dirname%
  goto end

:cgx
  cgx.exe -read %1
  goto end

:cgxstl
  cgx.exe -stl %1
  goto end

:cgxstp
  cgx.exe -step %1
  goto end

:end
  cd /d "%oldcwd%"
  set oldcwd=
  set dirname=
