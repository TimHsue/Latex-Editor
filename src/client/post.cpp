#include <winsock2.h>
#include <cstdio>
#include <iostream>
#include <log.cpp>

#define RQS_PACKAGE 1024
#define RSP_PACKAGE 2048

int main() {
    SOCKET clientSocket;
    WSADATA wsaData;
    struct sockaddr_in serverAddr;

    char sndBuff[RQS_PACKAGE], rcvBuff[RSP_PACKAGE];

    WSAStartup(MAKEWORD(2, 1), &wsaData);

    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR("failed to start socket!");
    } else {
        LOG("socket started.");
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.S_un.S_addr = inet_addr("106.52.251.85");

    if (connect(clientSocket, (struct sockaddr *)&serverAddr,
                sizeof(serverAddr)) < 0) {
        ERR("failed to connect to socket server!");
    } else {
        LOG("socket connected.");
    }

    memset(rcvBuff, 0, sizeof(rcvBuff));
    memset(sndBuff, 0, sizeof(sndBuff));

    strcat(sndBuff, "POST http://106.52.251.85:8888/ HTTP/1.0\r\n");
    strcat(sndBuff, "Host: 106.52.251.85:8888\r\n");
    strcat(sndBuff, "Connection: keep-alive\r\n");
    strcat(sndBuff, "\r\n");
    LOG(sndBuff);

	int reciveLength = 0;
    if ((reciveLength = send(clientSocket, sndBuff, RQS_PACKAGE, 0)) < 0) {
        ERR("failed to send request!");
    } else {
        LOG("request sent.");
    }

    do {
        if ((reciveLength = recv(clientSocket, rcvBuff, RSP_PACKAGE, 0)) <
            0) {
            ERR("failed to receive response!");
        } else if (reciveLength > 0) {
        	printf("%s", rcvBuff);
        	memset(rcvBuff, 0, sizeof(rcvBuff));
        }
    } while (reciveLength > 0);
    printf("\n");
    
    LOG("all responses read.");
    
    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
