@echo off
rem #####################################################################################
rem Name:          unsetFileAssociations.bat
rem Description:   Removes CalculiX-specific Windows file type, associations and icons
rem Author:        Cesare Guardino
rem Last modified: 19 January 2016
rem 
rem GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
rem #####################################################################################

ftype CALCULIX=
assoc .fbd=
rem assoc .foam=
assoc .frd=
assoc .inp=
assoc .stl=
assoc .stp=
pause
