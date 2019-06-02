#ifndef POST_H
#define POST_H
#include <Winsock2.h>
#include <cstdio>
#include <iostream>
#include <log.cpp>

#define RQS_PACKAGE 256
#define RSP_PACKAGE 256
#define BOUNDARY "TPafZ#C3T"
#define COOKIE_LENGTH 16
#define BASIC_COOKIE "NOLOGIN"
#define ISNUM(X) (X <= '9' && X >= '0')

class Response {
public:
    int havsCss;
    std :: string html;
    std :: string css;

    Response();
    Response(std :: string html_, std :: string css_);
    Response(std :: string html_);
};

char *readFile(const char *path);

Response handleResponse(std :: string rspHeader, SOCKET client);

std :: string postLog(char *addr, int serverPort, std :: string account,
                      std :: string password, bool isSignin = false,
                      bool rqsVCode = false, int vCode = 0);

Response postFile(char *addr, int serverPort, char *target,
                       char *content, bool isFile = true);


#endif // POST_H
