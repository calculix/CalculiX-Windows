//
// Copyright 2006, Convergent Mechanical Solutions LLC
// all rights reserved
//

#ifndef _SYS_UTSNAME_H
#define _SYS_UTSNAME_H

#ifdef __cplusplus
extern "C" {
#endif

/**
        dwPlatformID  dwMajorVersion  dwMinorVersion  dwBuildNumber
  95          1             4               0               950
  95 SP1      1             4               0               > 950 && <= 1080
  95 OSR2     1             4               < 10            > 1080
  98          1             4               10              1998
  98 SP1      1             4               10              >1998 && < 2183
  98 SE       1             4               10              >= 2183
  Me          1             4               90              3000
  NT 3.51     2             3               51              1057
  NT          4             2               4               0     1381
  2000        2             5               0               2195
  XP          2             5               1
  CE 1.0      3             1               0
  CE 2.0      3             2               0
  CE 2.1      3             2               1
  CE 3.0      3             3               0
  from http://www.codeproject.com/system/winvertable.asp
*/
struct utsname
{
  char sysname[20];
  char nodename[20];  // for Win95, 98 and ME, this should be MAX_COMPUTERNAME_LENGTH + 1
  char release[20];
  char version[20];
  char machine[20];
};

int uname (struct utsname *);

#ifdef __cplusplus
}
#endif

#endif
