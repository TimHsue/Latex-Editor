#ifndef LOG_CPP
#define LOG_CPP

#include <iostream>
#include <cstdio>
#include <string>
#include <ctime>

#define DEBUG

inline void LOG(std :: string msg, int code = -1) {
    time_t timeP;
    time(&timeP);
    char timeF[64];
    struct tm nowTime;
    localtime_r(&timeP, &nowTime);
    sprintf(timeF, "%04d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1,
            nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
#ifdef DEBUG
	fprintf(stdout, "\nLOG %s\n%s with code: %d\n", timeF, msg.c_str(), code);
#else
	FILE* file = fopen("log.out", "a");
	fprintf(file, "\nLOG %s\n%s with code: %d\n", timeF, msg.c_str(), code);
	fclose(file);
#endif  /* DEBUG */
    
}

inline void ERR(std :: string msg, int code =-1) {
    time_t timeP;
    time(&timeP);
    char timeF[64];
    struct tm nowTime;
    localtime_r(&timeP, &nowTime);
    sprintf(timeF, "%04d-%02d-%02d %02d:%02d:%02d", nowTime.tm_year + 1900, nowTime.tm_mon + 1,
            nowTime.tm_mday, nowTime.tm_hour, nowTime.tm_min, nowTime.tm_sec);
#ifdef DEBUG
	fprintf(stdout, "\nERROR %s\n%s with code: %d\n", timeF, msg.c_str(), code);
#else
	FILE* file = fopen("log.out", "a");
	fprintf(file, "\nERROR %s\n%s with code: %d\n", timeF, msg.c_str(), code);
	fclose(file);
#endif  /* DEBUG */
}

#endif /* LOG_CPP */
