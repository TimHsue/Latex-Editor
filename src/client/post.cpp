#ifndef POST_CPP
#define POST_CPP

#include <Winsock2.h>
#include <cstdio>
#include <iostream>
#include <log.cpp>

#define RQS_PACKAGE 1024
#define RSP_PACKAGE 2048
#define BOUNDARY "TPafZ#C3T"
#define ISNUM(X) (X <= '9' && X >= '0')

class Response {
public:
    int havsCss;
    std :: string html;
    std :: string css;

    Response() {}
    Response(std :: string html_, std :: string css_) {
        html = html_;
        css = css_;
        havsCss = true;
    }
    Response(std :: string html_) {
        html = html_;
        havsCss = false;
    }
};


char *readFile(const char *path) {
    FILE* file = fopen(path, "r");
    int fileLength = 0;

    if (! file) {
        ERR("failed to open file!");
        return NULL;
    } else {
        LOG("file opened.");
    }

    fseek(file, 0, SEEK_END);
    fileLength = ftell(file);
    rewind(file);

    char *fileBuff = new char[fileLength + 1];
    memset(fileBuff, 0, sizeof(fileBuff));
    fread(fileBuff, sizeof(char), fileLength, file);
    fileBuff[fileLength] = 0;

    fclose(file);
    return fileBuff;
}

Response handleResponse(std :: string rspHeader, SOCKET client) {
    LOG(rspHeader);
    char *cssChar, *htmlChar;
    std :: string html;

    int numberPos = 0, dataNumber = 0;
    numberPos = rspHeader.find("dataNumber=") + 11;
    while (ISNUM(rspHeader[numberPos])) {
        dataNumber = dataNumber * 10 + rspHeader[numberPos] - '0';
        numberPos++;

        if (numberPos > rspHeader.length()) {
            break;
        }
    }

    int fileSizePos = 0, fileSize = 0;
    fileSizePos = rspHeader.find("fileSize=") + 9;
    while (ISNUM(rspHeader[fileSizePos])) {
        fileSize = fileSize * 10 + rspHeader[fileSizePos] - '0';
        fileSizePos++;

        if (fileSizePos > rspHeader.length()) {
            break;
        }
    }

    int cssSizePos = 0, cssSize = 0;
    if (dataNumber == 2) {
        cssSizePos = rspHeader.find("cssSize=") + 8;
        while (ISNUM(rspHeader[cssSizePos])) {
            cssSize = cssSize * 10 + rspHeader[cssSizePos] - '0';
            cssSizePos++;

            if (cssSizePos > rspHeader.length()) {
                break;
            }
        }
    }

    LOG("header handled.");
    LOG(std :: to_string(dataNumber));
    LOG(std :: to_string(fileSize));
    LOG(std :: to_string(cssSize));

    htmlChar = new char[fileSize + 1];
    if (recv(client, htmlChar, fileSize, 0) < 0) {
        ERR("failed to receive html!");
        return Response("error");
    } else {
        htmlChar[fileSize] = 0;
    }
    html = htmlChar;

    if (dataNumber == 2) {
        cssChar = new char[cssSize + 1];
        if (recv(client, cssChar, cssSize, 0) < 0) {
            ERR("failed to receive css!");
            return Response("error");
        } else {
            cssChar[cssSize] = 0;
        }

        int headPos = html.find("<head>");
        while (html[headPos] != '\n') {
            headPos++;
            if (headPos > html.length()) break;
        }

        std :: string front = html.substr(0, headPos + 1);
        std :: string css = cssChar;
        std :: string behind = html.substr(headPos + 2);
        html = front + "<style>" + css + "</style>" + behind;
    }

    LOG("all response read.");

    ::closesocket(client);
    WSACleanup();

    return Response(html);
}


Response postFile(char *addr, int serverPort, char *target,
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
        return Response("error");
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
        return Response("error");
    } else {
        LOG("socket connected.");
    }

    std :: string tmpContent;
    if (isFile == true) {
        tmpContent = readFile(content);
    } else {
        tmpContent = content;
    }

    const char *fileContent = tmpContent.c_str();

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
        return Response("error");
    } else {
        LOG("request sent.");
    }

    LOG(fileContent);

    if ((reciveLength = send(clientSocket, fileContent, fileLength, 0)) < 0) {
        ERR("failed to send request body!");
        return Response("error");
    } else {
        LOG("request sent.");
    }

    if ((reciveLength = recv(clientSocket, rcvBuff, RSP_PACKAGE, 0)) < 0) {
        ERR("failed to receive response!");
        return Response("error");
    } else if (reciveLength > 0) {
        return handleResponse(rcvBuff, clientSocket);
    }

    return Response("error");
}

#endif
