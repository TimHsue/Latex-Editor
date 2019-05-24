#ifndef LOG_CPP
#define LOG_CPP

#include <iostream>
#include <cstdio>
#include <string>
#include <ctime>

#define DEBUG

#ifdef DEBUG
#define OUTSTREAM stdout
#else
FILE* file = fopen("log.out", "a");
#define OUTSTREAM file
#endif  /* DEBUG */

inline void LOG(std :: string msg) {
    time_t timeP;
    time(&timeP);
    char timeF[64];
    struct tm nowTime;
    localtime_r(&timeP, &nowTime);
    sprintf(timeF, "%04d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1,
            nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    fprintf(OUTSTREAM, "\nLOG %s\n%s\n", timeF, msg.c_str());
}

inline void ERR(std :: string msg) {
    time_t timeP;
    time(&timeP);
    char timeF[64];
    struct tm nowTime;
    localtime_r(&timeP, &nowTime);
    sprintf(timeF, "%04d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1,
            nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    fprintf(OUTSTREAM, "\nERROR %s\n%s\n", timeF, msg.c_str());
}

#endif /* LOG_CPP */
