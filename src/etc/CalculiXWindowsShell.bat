@echo off
rem #####################################################################################
rem Name:          CalculiXWindowsShell.bat
rem Description:   Script to start a Windows CMD prompt shell with CalculiX environment
rem Author:        Cesare Guardino
rem Last modified: 11 September 2024
rem 
rem GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
rem #####################################################################################

set CALCULIX_ETC_DIR=%~dp0
if #%CALCULIX_ETC_DIR:~-1%# == #\#  set CALCULIX_ETC_DIR=%CALCULIX_ETC_DIR:~0,-1%
call "%CALCULIX_ETC_DIR%\CALCULIXWindowsEnvironment.bat"
set CALCULIX_ETC_DIR=
rem mode 160,40
rem color 30
echo -------------------------------
echo Command shell for CalculiX 2.22
echo -------------------------------
echo/
cd /d %USERPROFILE%
cmd.exe
