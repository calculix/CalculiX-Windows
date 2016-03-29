@echo off
rem #####################################################################################
rem Name:          setFileAssociations.bat
rem Description:   Creates CalculiX-specific Windows file type, associations and icons
rem Author:        Cesare Guardino
rem Last modified: 19 January 2016
rem 
rem GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
rem #####################################################################################

set CALCULIX_ETC_DIR=%~dp0
if #%CALCULIX_ETC_DIR:~-1%# == #\#  set CALCULIX_ETC_DIR=%CALCULIX_ETC_DIR:~0,-1%
call "%CALCULIX_ETC_DIR%\CALCULIXWindowsEnvironment.bat"
ftype CALCULIX=
ftype CALCULIX="%CALCULIX_ETC_DIR%\..\etc\runCGX.bat" "%%1" %*
assoc .fbd=CALCULIX
rem assoc .foam=CALCULIX
assoc .frd=CALCULIX
assoc .inp=CALCULIX
assoc .stl=CALCULIX
assoc .stp=CALCULIX
reg add HKCR\CALCULIX\DefaultIcon /t REG_EXPAND_SZ /d \""%CALCULIX_ETC_DIR%\calculix.ico"\" /f

set CALCULIX_ETC_DIR=
pause
