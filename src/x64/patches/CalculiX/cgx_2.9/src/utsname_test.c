/*
Name           : utsname_test.c
Description    : Simple test code for Windows version of uname function for building with MinGW-w64.
Author         : Cesare Guardino
Last modified  : 19 January 2016
Notes          : Partly derived from code in cgx.c.

GE CONFIDENTIAL INFORMATION © 2016 General Electric Company – All Rights Reserved
*/

#include <stdio.h>
#include <sys/utsname.h>

int main() {
    struct utsname  cursys[1];
    uname (cursys);
    printf ("on a %s machine, ", cursys->sysname);
    printf ("nodename %s, ", cursys->nodename);
    printf ("release %s, ", cursys->release);
    printf ("version %s, ", cursys->version);
    printf ("machine %s \n", cursys->machine);
}
