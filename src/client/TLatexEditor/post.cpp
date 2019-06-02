#ifndef POST_CPP
#define POST_CPP

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
    std :: string html, css;

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

        if (fileSizePos >= rspHeader.length()) {
            break;
        }
    }

    int cssSizePos = 0, cssSize = 0;
    if (dataNumber == 2) {
        cssSizePos = rspHeader.find("cssSize=") + 8;
        while (ISNUM(rspHeader[cssSizePos])) {
            cssSize = cssSize * 10 + rspHeader[cssSizePos] - '0';
            cssSizePos++;

            if (cssSizePos >= rspHeader.length()) {
                break;
            }
        }
    }

    LOG("header handled.");
    LOG(std :: to_string(dataNumber));
    LOG(std :: to_string(fileSize));
    LOG(std :: to_string(cssSize));


    char rcvBuff[RSP_PACKAGE + 1];
    memset(rcvBuff, 0, sizeof(rcvBuff));

    html = "";
    css = "";
    int reciveLength;
    htmlChar = new char[fileSize + 1];

    for (int i = 0; i < fileSize; i += RSP_PACKAGE) {
        int receiveSize = RSP_PACKAGE;
        if (i + RSP_PACKAGE > fileSize) {
            receiveSize = fileSize - i;
        }

        if ((reciveLength = recv(client, rcvBuff, receiveSize, 0)) < 0) {
            ERR("failed to receive html!");
            return Response("error");
        } else {
            rcvBuff[receiveSize] = 0;
            for (int j = 0; j < receiveSize; j++) html += rcvBuff[j];
            memset(rcvBuff, 0, sizeof(rcvBuff));
        }
        Sleep(80);
    }

    qDebug() << html.length();
    qDebug() << fileSize;

    if (html.length() == fileSize) {
        int htmlEnd = html.find("</html>");
        if (htmlEnd > 0) html = html.substr(0, htmlEnd + 7);
    }


    if (dataNumber == 2) {
        for (int i = 0; i < cssSize; i += RSP_PACKAGE) {
            int receiveSize = RSP_PACKAGE;
            if (i + RSP_PACKAGE > cssSize) {
                receiveSize = cssSize - i;
            }

            if ((reciveLength = recv(client, rcvBuff, receiveSize, 0)) < 0) {
                ERR("failed to receive css!");
                return Response("error");
            } else {
                rcvBuff[receiveSize] = 0;
                css += rcvBuff;
                memset(rcvBuff, 0, sizeof(rcvBuff));
            }
        }

        int cssBegin = css.find("start css.sty") - 3;
        if (cssBegin > 0) css = css.substr(cssBegin);

        int headPos = html.find("<head>");
        if (headPos < 0) return Response(html);
        while (html[headPos] != '\n') {
            headPos++;
            if (headPos >= html.length()) {
                return Response(html);
            }
        }

        std :: string front = html.substr(0, headPos + 1);
        std :: string behind = html.substr(headPos + 2);
        html = front + "<style>" + css + "</style>" + behind;
    }

    LOG("all response read.");

    closesocket(client);
    WSACleanup();
    if (dataNumber == 2) return Response(html, css);
    else return Response(html);
}

Response postFile(char *addr, int serverPort, const char *target,
                  char *content, bool isFile = true,
                  std :: string pCookie = "", std :: string name = "") {
    SOCKET clientSocket;
    WSADATA wsaData;
    struct sockaddr_in serverAddr;

    std :: string header;
    char sndBuff[RQS_PACKAGE + 1], rcvBuff[RSP_PACKAGE + 1];
    memset(sndBuff, 0, sizeof(sndBuff));
    memset(rcvBuff, 0, sizeof(rcvBuff));

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

    if (pCookie.length() > 1) {
        header += "cookie=";
        header += pCookie + "\r\n";
    }

    if (name.length() > 1) {
        header += "name=";
        header += name + "\r\n";
    }

    header += "dataSize=";
    header += std :: to_string(fileLength);
    header += "\r\n";

    LOG(header);

    while (header.length() < RQS_PACKAGE) header += " ";

    int reciveLength = 0;
    if ((reciveLength = send(clientSocket, header.c_str(), RQS_PACKAGE, 0)) < 0) {
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
        if (pCookie.length() >= 1) {
            return Response(rcvBuff);
        } else {
            return handleResponse(rcvBuff, clientSocket);
        }
    }

    return Response("error");
}

#endif /* POST_CPP */
