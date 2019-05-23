#ifndef LOG_CPP
#define LOG_CPP

#include <iostream>
#include <cstdio>
#include <string>
#include <ctime>
using namespace std;

#define DEBUG
#ifdef DEBUG
#define OUTSTREAM stdout
#else
FILE* file = fopen("log.out", "a");
#define OUTSTREAM file
#endif  /* DEBUG */

inline void LOG(string msg) {
	time_t timeP;
	time(&timeP);
	char timeF[64];
	strftime(timeF, sizeof(timeF), "%Y-%m-%d %H:%M:%S", 
		localtime(&timeP));
	fprintf(OUTSTREAM, "\nLOG %s:\n%s\n", timeF, msg.c_str());
}

inline void ERR(string msg) {
	time_t timeP;
	time(&timeP);
	char timeF[64];
	strftime(timeF, sizeof(timeF), "%Y-%m-%d %H:%M:%S", 
		localtime(&timeP));
	fprintf(OUTSTREAM, "\nERROR %s:\n%s\n", timeF, msg.c_str());
	exit(-1);
} 

#endif /* LOG_CPP */

