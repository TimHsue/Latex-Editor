#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include "log.cpp"
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <thread>

#define SERVER_PORT 8888
#define RQS_PACKAGE 1024
#define RSP_PACKAGE 2048
#define ISNUM(X) (X <= '9' and X >= '0')

char *readFile(const char *path) {
	FILE* file = fopen(path, "r");
	int fileLength = 0;
	
	if (not file) {
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
	
	fclose(file);
	return fileBuff;
}

void writeFile(const char *str, const char *path) {
	FILE* file = fopen(path, "w");
	if (not file) {
		ERR("failed to open file!");
	} else {
		LOG("file opened.");
	}
	
	fputs(str, file);
	fclose(file);
}

void handleGet(std :: string requestHeader) {
	
}

void handlePost(std :: string requestHeader, int client, int threadNumber) {
	int requestPos = 0;
	if ((requestPos = requestHeader.find("?")) < 0) {
		ERR("url format error!");
	}
	
	std :: string request = "";
	
	while (requestHeader[requestPos] != ' ') {
		request += requestHeader[requestPos];
		requestPos++;
	
		if (requestPos > requestHeader.length()) {
			break;
		}
	}
		
	int typePos = 0;
	if ((typePos = requestHeader.find("Content-Type: ")) < 0) {
		ERR("header format error!");
	} 
	
	LOG("POST");
	LOG(request.c_str());
	typePos += 14; // length of "Content-Type: "
	std :: string type = "";
	
	while (requestHeader[typePos] != '\r') {
		type += requestHeader[typePos];
		typePos++;
		
		if (typePos > requestHeader.length()) {
			break;
		}
	} 
	
	LOG(type + "(TYPE)"); 
	
	if (type.compare("file") == 0) {
		int sizePos = 0;
		if ((sizePos = requestHeader.find("dataSize=")) < 0) {
			ERR("header format error!");
		} 
		
		sizePos += 9; // size of "dataSize="
		int dataSize = 0;
		while (ISNUM(requestHeader[sizePos])) {
			dataSize = 10 * dataSize + requestHeader[sizePos] - '0';
			sizePos++;
			
			if (sizePos > requestHeader.length()) {
				break;
			}
		}
		
		LOG(std :: to_string(dataSize) + "(SIZE)");
		
		char *content = new char[dataSize + 1];
		
		if (recv(client, content, dataSize, 0) < 0) {
            ERR("failed to receive file!");
        } else {
        	printf("%s", content);
		}
		
		if (request.compare("?texToHtml") == 0 or
			request.compare("?texToPdf") == 0) {
			LOG("generating tmp file.");
			
			std :: string nowPath = getcwd(NULL, 0);
			
			std :: string baseName = "tmptex";
			baseName += std :: to_string(threadNumber);
			
			std :: string threadPath = "";
			threadPath += nowPath + "/thread";
			threadPath += std :: to_string(threadNumber);
			
			std :: string fileName = baseName + ".tex";
			
			std :: string tmpFile = threadPath + "/" + fileName;
			LOG(tmpFile);
			writeFile(content, tmpFile.c_str());
			
			chdir(threadPath.c_str());
			
			LOG(getcwd(NULL, 0));
			
			std :: string com = threadPath + "/ht-latex ";
			com += "tmptex";
			com += std :: to_string(threadNumber) + ".tex";
			com += " .";
			LOG(com);
			system(com.c_str());
			
			/* 
			com = "";
			com += "cp ";
			com += "tmptex";
			com += to_string(threadNumber) + ".pdf " + threadPath;
			LOG(com);
			system(com.c_str());
			
			com = "";
			com += "cp ";
			com += "tmptex";
			com += to_string(threadNumber) + "-final.html " + threadPath;
			LOG(com);
			system(com.c_str());
			
			com = ""; 
			com += "rm ";
			com += "tmptex";
			com += to_string(threadNumber) + "*";
			LOG(com);
			system(com.c_str());
			*/
			
			std :: string responseFile = "";
			
			if (request.compare("?texToHtml") == 0) {
				responseFile = baseName + ".html";
			} else {
				responseFile = baseName + ".pdf";
			}
			
			LOG(responseFile);
			
			char *fileContent = readFile(responseFile.c_str());
			if (fileContent == NULL) {
				fileContent = "Something goes wrong with rendering latex.";
			}
			int fileLength = strlen(fileContent);
			
			if (send(client, fileContent, fileLength, 0) < 0) {
		        ERR("failed to send response!");
		    } else {
		        LOG("response sent.");
		    }
		    
		    com = "rm ";
		    com += "tmptex";
			com += std :: to_string(threadNumber) + ".*";
			LOG(com);
			system(com.c_str());

		    chdir("..");
		    LOG(getcwd(NULL, 0));
		}
	}
}

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
				std :: string requestHeader = rcvBuff;
				
				if (requestHeader.substr(0, 4).compare("POST") == 0) {
					handlePost(requestHeader, client, 3);
				} else if (requestHeader.substr(0, 3).compare("GET") == 0) {
					handleGet(requestHeader);
				}
				/*
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
                */
                close(client);
            }
        }
    }
}
