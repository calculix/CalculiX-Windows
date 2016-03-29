#####################################################################################
# Name:          README_GE-OSS.txt
# Description:   Installation and usage instructions for CalculiX Windows binary package
# Author:        Cesare Guardino
# Last modified: 26 January 2016
#
# GE CONFIDENTIAL INFORMATION © 2016 General Electric Company - All Rights Reserved
#####################################################################################


*****************************************************************************************************************
*****************************************************************************************************************
***** IMPORTANT: THIS VERSION IS PROVIDED AS-IS AND IS NOT FULLY TESTED OR VALIDATED. PLEASE USE WITH CARE. *****
*****************************************************************************************************************
*****************************************************************************************************************


INSTRUCTIONS ON HOW TO INSTALL AND RUN THE WINDOWS VERSION OF CALCULIX
----------------------------------------------------------------------
1) Unzip the package file to any suitable directory (with no white spaces) on your computer.

2) Optionally, install Gnuplot and ImageMagick if you do not already have them.
   You can download these from:
       http://www.gnuplot.info
       http://www.imagemagick.org
   It is strongly recommended to install these in directories with no white spaces.
   Once installed, create new environment variables called GNUPLOT_HOME and IMAGEMAGICK_HOME
   to point to the installation directory where you installed them. This can be done by
   editing the user-editable settings in the environment configuration:
       call <PATH_TO_CALCULIX>\etc\CalculiXWindowsEnvironment.bat
   where <PATH_TO_CALCULIX> is the full path of the directory where you unzipped the package.
   For example:
       rem =========== USER EDITABLE SETTINGS ===========
       set GNUPLOT_HOME=C:\Programs\gp501-win64-mingw
       set IMAGEMAGICK_HOME=C:\Programs\ImageMagick-6.9.1-2
       rem ==============================================

3) Optionally, create file associations and icons for *.fbd, *.frd, *.inp, *.stl and *.stp files by running the script:
       <PATH_TO_CALCULIX>\etc\setFileAssociations.bat
   Once set, this allows the user to double-click on any of these file types to open them in CGX.
   These file associations can be removed at any time using the script:
       <PATH_TO_CALCULIX>\etc\unsetFileAssociations.bat
   Note that these scripts require administrator privileges on your computer.

4) Start a new CMD (DOS) prompt, and run the following command:
       call <PATH_TO_CALCULIX>\etc\CalculiXWindowsEnvironment.bat
   where <PATH_TO_CALCULIX> is the full path of the directory where you unzipped the package.
   The CalculiX environment is now configured correctly for use within this CMD prompt only.

   Alternatively, create a desktop shortcut to <PATH_TO_CALCULIX>\etc\CalculiXWindowsShell.bat
   When you double-click this shortcut, a new CMD prompt is open with the CalculiX environment automatically set.

5) From the CMD prompts opened in the previous step, you can now run the usual CalculiX applications, for example:
       cd /d <PATH_TO_CASE_DIRECTORY>
       cgx -b foo.fbd       ### Create geometry/mesh etc.
       cgx -c foo.inp       ### Check case setup
       ccx foo              ### Run single-threaded solver
       ccx_MT foo           ### Run multi-threaded solver
       unix2dos foo.frd     ### Required for CGX to correctly read in .frd file
       cgx foo.frd          ### View results


NOTES
-----
1) You can also use CGX to view the results from an OpenFOAM analysis. Just run in the case directory:
       cgx -foam .
