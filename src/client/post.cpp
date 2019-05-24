#ifndef POST_CPP
#define POST_CPP

#include <Winsock2.h>
#include <cstdio>
#include <iostream>
#include <log.cpp>

#define RQS_PACKAGE 1024
#define RSP_PACKAGE 2048
#define BOUNDARY "TPafZ#C3T"

char *readFile(const char *path) {
    FILE* file = fopen(path, "r");
    int fileLength = 0;

    if (! file) {
        ERR("failed to open file!");
    } else {
        LOG("file opened.");
    }

    fseek(file, 0, SEEK_END);
    fileLength = ftell(file);
    rewind(file);

    char *fileBuff = new char[fileLength + 1];
    memset(fileBuff, 0, sizeof(fileBuff));
    fread(fileBuff, sizeof(char), fileLength, file);

    fclose(file);
    return fileBuff;
}

void postFile(char *addr, int serverPort, char *target,
              char *content, bool isFile = true) {
    SOCKET clientSocket;
    WSADATA wsaData;
    struct sockaddr_in serverAddr;

    std :: string header;
    char sndBuff[RQS_PACKAGE], rcvBuff[RSP_PACKAGE];
    memset(&sndBuff, 0, sizeof(sndBuff));
    memset(&rcvBuff, 0, sizeof(rcvBuff));

    WSAStartup(MAKEWORD(2, 1), &wsaData);

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR("failed to start socket!");
    } else {
        LOG("socket started.");
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = ::htons(serverPort);
    serverAddr.sin_addr.S_un.S_addr = inet_addr(addr);

    if (::connect(clientSocket, (struct sockaddr *)&serverAddr,
                sizeof(serverAddr)) < 0) {
        ERR("failed to connect to socket server!");
    } else {
        LOG("socket connected.");
    }

    char *fileContent;
    if (isFile == true) {
        fileContent = readFile(content);
    } else {
        fileContent = content;
    }

    int fileLength = strlen(fileContent);

    header = "";
    header += "POST http://";
    header += addr;
    header += ":";
    header += std :: to_string(serverPort);
    header += "?";
    header += target;
    header += " HTTP/1.0\r\n";

    header += "Host: ";
    header += addr;
    header += ":";
    header += std :: to_string(serverPort);
    header += "\r\n";

    header += "Connection: keep-alive\r\n";

    header += "Accept: */*\r\n";

    header += "Content-Type: file\r\n";
    header += "\r\n";

    header += "dataSize=";
    header += std :: to_string(fileLength);
    header += "\r\n";

    LOG(header);

    int reciveLength = 0;
    if ((reciveLength = send(clientSocket, header.c_str(), header.length(), 0)) < 0) {
        ERR("failed to send request header!");
    } else {
        LOG("request sent.");
    }

    if ((reciveLength = send(clientSocket, fileContent, fileLength, 0)) < 0) {
        ERR("failed to send request body!");
    } else {
        LOG("request sent.");
    }

    do {
        FILE* file = fopen("tmp.html", "w");
        if ((reciveLength = recv(clientSocket, rcvBuff, RSP_PACKAGE, 0)) <
            0) {
            ERR("failed to receive response!");
        } else if (reciveLength > 0) {
            printf("%s", rcvBuff);
            fputs(rcvBuff, file);
            memset(rcvBuff, 0, sizeof(rcvBuff));
        }
    } while (reciveLength > 0);
    printf("\n");

    LOG("all responses read.");

    ::closesocket(clientSocket);
    WSACleanup();
}

#endif

int main() {
	char ip[] = "106.52.251.85";
    char type[] = "texToHtml";
    char transfer[] = "test";
    postFile(ip, 8888, type, transfer, false);
}
