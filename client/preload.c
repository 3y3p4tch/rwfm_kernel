#include <stdio.h>
#include <dlfcn.h>
#include "underlying_functions.h"
#include "rule_engine.h"

void debuglog(char *log) {
    int logfd = underlying_open("/tmp/preload.log", 02000|02);
    underlying_write(logfd, log, strlen(log));
    close(logfd);
}


