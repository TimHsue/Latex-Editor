#ifndef LOG_CPP
#define LOG_CPP

#include <iostream>
#include <cstdio>
#include <string>
#include <ctime>
#include <QDebug>

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
    char timeF[32];
    struct tm nowTime;
    localtime_s(&nowTime, &timeP);
    sprintf(timeF, "%04d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1,
            nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    char output[128];
    sprintf(output, "\nLOG %s\n%s\n", timeF, msg.c_str());
    fprintf(OUTSTREAM, "\nLOG %s\n%s\n", timeF, msg.c_str());
#ifdef DEBUG
    qDebug(output);
#endif
}

inline void ERR(std :: string msg) {
    time_t timeP;
    time(&timeP);
    char timeF[64];
    struct tm nowTime;
    localtime_s(&nowTime, &timeP);
    sprintf(timeF, "%04d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1,
            nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
    char output[128];
    sprintf(output, "\nLOG %s\n%s\n", timeF, msg.c_str());
    fprintf(OUTSTREAM, "\nERROR %s\n%s\n", timeF, msg.c_str());
#ifdef DEBUG
    qDebug(output);
#endif
}

#endif /* LOG_CPP */
