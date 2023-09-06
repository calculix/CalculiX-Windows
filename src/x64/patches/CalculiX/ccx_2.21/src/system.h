#ifndef SYSTEM_H
#define SYSTEM_H

#include <stdlib.h>  /* for atoi and getenv */

int sysconf(int numProcessors) {
    char* envsys = getenv("NUMBER_OF_PROCESSORS");

    if (envsys) {
        numProcessors = atoi(envsys);
    }

    return numProcessors;
}

#endif
