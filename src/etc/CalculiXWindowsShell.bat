@echo off
rem #####################################################################################
rem Name:          CalculiXWindowsShell.bat
rem Description:   Script to start a Windows CMD prompt shell with CalculiX environment
rem Author:        Cesare Guardino
rem Last modified: 22 April 2016
rem 
rem GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
rem #####################################################################################

set CALCULIX_ETC_DIR=%~dp0
if #%CALCULIX_ETC_DIR:~-1%# == #\#  set CALCULIX_ETC_DIR=%CALCULIX_ETC_DIR:~0,-1%
call "%CALCULIX_ETC_DIR%\CALCULIXWindowsEnvironment.bat"
set CALCULIX_ETC_DIR=
mode 160,40
color 81
echo ------------------------------
echo Command shell for CalculiX 2.10
echo ------------------------------
echo/
cmd.exe
