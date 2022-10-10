//
// Copyright 2006, Convergent Mechanical Solutions LLC
// all rights reserved
//

// Modified by Cesare Guardino for building with MinGW-w64, 19 January 2016.
// GE CONFIDENTIAL INFORMATION © 2016 General Electric Company – All Rights Reserved

//#define _WIN32_WINNT 0X0500
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/utsname.h>

int uname(struct utsname *uts) {
    DWORD nSize= 255;
    DWORD sLength;
    char nodename[256];
    SYSTEM_INFO   siSysInfo;    // Struct for hardware info
    OSVERSIONINFO siVerInfo;    // Struct for version info
    GetSystemInfo(&siSysInfo);  // Get hardware OEM

    // Get major and minor number
    ZeroMemory(&siVerInfo, sizeof(OSVERSIONINFO));
    siVerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx((OSVERSIONINFO *) &siVerInfo)){
        fprintf(stderr, "Could not get OS Version info\n");
        exit(1);
    }

    memset(uts, 0, sizeof ( *uts ) );

    switch(siVerInfo.dwPlatformId) {
        case VER_PLATFORM_WIN32s: 
            sprintf(uts->sysname, "Win%d",siVerInfo.dwMajorVersion);
            break;
        case VER_PLATFORM_WIN32_WINDOWS:
            if(siVerInfo.dwMinorVersion==0)
                sprintf(uts->sysname, "Win95");
            else if(siVerInfo.dwMinorVersion==10)  
                sprintf(uts->sysname, "Win98");
            else if(siVerInfo.dwMinorVersion==90)  
                sprintf(uts->sysname, "WinME");
            break;
        case VER_PLATFORM_WIN32_NT:
            if(siVerInfo.dwMajorVersion==5 && siVerInfo.dwMinorVersion==0)
                sprintf(uts->sysname, "Win2k");
            else if(siVerInfo.dwMajorVersion==5 && siVerInfo.dwMinorVersion==1)
                sprintf(uts->sysname, "WinXP");
            else if(siVerInfo.dwMajorVersion<=4)
                sprintf(uts->sysname, "WinNT");
            else if (siVerInfo.dwMajorVersion==5 && siVerInfo.dwMinorVersion==1)
                sprintf(uts->sysname, "Windows XP");
            else if (siVerInfo.dwMajorVersion==6 && siVerInfo.dwMinorVersion==0)
                sprintf(uts->sysname, "Windows Vista");
            else if (siVerInfo.dwMajorVersion==6 && siVerInfo.dwMinorVersion==1)
                sprintf(uts->sysname, "Windows 7");
            else if (siVerInfo.dwMajorVersion==6 && siVerInfo.dwMinorVersion==2)
                sprintf(uts->sysname, "Windows 8");
            else if (siVerInfo.dwMajorVersion==6 && siVerInfo.dwMinorVersion==3)
                sprintf(uts->sysname, "Windows 8.1");
            else if (siVerInfo.dwMajorVersion==10 && siVerInfo.dwMinorVersion==0)
                sprintf(uts->sysname, "Windows 10");
            else //for unknown windows/newest windows version
                sprintf(uts->sysname, "Win%d",siVerInfo.dwMajorVersion);
            nSize = 255;
    }

    char Computer_Name[20];
    DWORD Size = sizeof ( Computer_Name ); 
    if (getenv("COMPUTERNAME") != NULL)
        strcpy(uts->nodename, getenv("COMPUTERNAME"));
    else if (GetComputerName(Computer_Name, &Size)!=0)
        strcpy(uts->nodename, Computer_Name);
    else
        strcpy(uts->nodename, "UNKOWN");

    sprintf(uts->release, "%d",siVerInfo.dwMajorVersion);
    sprintf(uts->machine, "%d",siSysInfo.dwProcessorType);
    sprintf(uts->version, "%d.%d %s %d",
            siVerInfo.dwMajorVersion,
            siVerInfo.dwMinorVersion,
            siVerInfo.szCSDVersion,
            siVerInfo.dwBuildNumber & 0xFFFF);
    return 1;
}
