#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include "log.cpp"
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 8888
#define RQS_PACKAGE 1024
#define RSP_PACKAGE 2048

int main() {
    int serverSocket;
    struct sockaddr_in serverAddr;

    char sndBuff[RSP_PACKAGE], rcvBuff[RQS_PACKAGE];

    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        ERR("failed to start socket!");
    } else {
        LOG("socket started.");
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = PF_INET;
    serverAddr.sin_addr.s_addr = htons(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) <
        0) {
        ERR("failed to bind socket!");
    } else {
        LOG("socket binded.");
    }

    if (listen(serverSocket, 1) < 0) {
        ERR("failed to listen port!");
    } else {
        LOG("listening started.");
    }

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLength = sizeof(clientAddr);
        int client;

        if ((client = accept(serverSocket, (struct sockaddr*)&clientAddr,
                             &clientAddrLength)) < 0) {
            ERR("failed to link to client!");
        } else {
            LOG("clint linked.");

            if (recv(client, rcvBuff, RQS_PACKAGE, 0) < 0) {
                ERR("failed to read requst!");
            } else {
                LOG("request read.");
                printf("%s", rcvBuff);

                memset(sndBuff, 0, sizeof(sndBuff));
                strcat(sndBuff, "HTTP/1.1 200 ok\r\nconnection: close\r\n\r\n");

                if (send(client, sndBuff, strlen(sndBuff), 0) < 0) {
                    ERR("failed to send response!");
                } else {
                    LOG("response sent.");
                    int file = open("hello.html", O_RDONLY);

                    if (file == 0) {
                        ERR("failed to read file!");
                    } else {
                        sendfile(client, file, NULL, RSP_PACKAGE);

                        close(file);
                        close(client);
                    }
                }
            }
        }
    }
}
