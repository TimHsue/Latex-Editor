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
#include <map>

#define SERVER_PORT 8888
#define RQS_PACKAGE 256
#define RSP_PACKAGE 256
#define COOKIE_LENGTH 16
#define ISNUM(X) (X <= '9' and X >= '0')

std :: map <std :: string, std :: string> actToVCode;
std :: map <std :: string, int> lastSent;

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
		fputs(str, file);
		fclose(file);
	}
}

void handleGet(std :: string requestHeader) { }




std :: string genVCode() {
	std :: string res = "";
	for (int i = 1; i <= 6; i++) {
		res += (char)(rand() % 10 + '0');
	}
	return res;
}

void sendVCode(std :: string account) {
	// todo: check account is a phone number
	
	auto VCode = genVCode();
	// todo: use api
	
	actToVCode[account] = VCode;
}

bool checkVCode(std :: string account, std :: string VCode) {
	auto it = actToVCode.find(account);
	if (it == actToVCode.end()) return false;
	
	auto sentVCode = actToVCode[account];
	if (sentVCode.compare(VCode) == 0) return true;
	return false;
}


std :: string genCookie() {
	std :: string res = "";
	for (int i = 1; i = 16; i++) {
		int type = rand() % 3;
		if (type == 0) {
			res += (char)(rand() % 10 + '0');
		} else if (type == 1) {
			res += (char)(rand() % 26 + 'a');
		} else if (type == 2) {
			res += (char)(rand() % 26 + 'A');
		}
	}
	return res;
}

std :: string getCookie(std :: string account) {
	// todo: link to mysql
	return genCookie();
}

bool checkPassword(std :: string account, std :: string password) {
	return false;
}

std :: string addAccount(std :: string account, std :: string password) {
	// todo gen cookie at the same time
} 


void handleLogin(std :: string rspHeader, int client, bool login = true, bool rstVCode = false) {
    int dataNumber = 0;
    if (login) dataNumber = 2;
    else if (rstVCode) dataNumber = 1;
    else dataNumber = dataNumber = 3;
    char rcvBuff[RQS_PACKAGE + 1];
    memset(rcvBuff, 0, sizeof(rcvBuff));
    std :: string account, password, vCode, cookie;
    int buffLength;
    
    /* read act length */
    int actPos = rspHeader.find("actSize=") + 8;
    int actLength = 0;
    while (ISNUM(rspHeader[actPos])) {
        actLength = actLength * 10 + rspHeader[actPos] - '0';
        actPos++;

        if (actPos >= rspHeader.length()) break;
    }
    
    /* read act */
    LOG("actLength", actLength);
	if ((buffLength = recv(client, rcvBuff, actLength, 0)) < 0) {
        ERR("failed to receive account!", errno);
        LOG(rcvBuff);
		account = "anonymous";
    } else {
    	rcvBuff[actLength] = 0;
    	account = rcvBuff; 
	}
	LOG(rcvBuff);
    
    
    /* send vcode */
    if ((not login) and (rstVCode)) {
    	LOG("send VCode");
    	sendVCode(account);
    	return;
	}
    
    /* read pwd length*/
    int pwdPos = rspHeader.find("pwdSize=") + 8;
    int pwdLength = 0;
    
    while (ISNUM(rspHeader[pwdPos])) {
        pwdLength = pwdLength * 10 + rspHeader[pwdPos] - '0';
        pwdPos++;

        if (pwdPos >= rspHeader.length()) break;
    }
    
    /* read pwd */
    memset(rcvBuff, 0, sizeof(rcvBuff));
    if ((buffLength = recv(client, rcvBuff, pwdLength, 0)) < 0) {
        ERR("failed to receive password!", buffLength);
		password = "";
    } else {
    	rcvBuff[pwdLength] = 0;
    	password = rcvBuff; 
	}
	LOG(rcvBuff);
	
	if (login) {
		if (account.compare("anonymous") == 0 or password.compare("") == 0) {
			cookie = "network goes wrg";
		} if (checkPassword(account, password)) {
			cookie = getCookie(account);
		} else {
			cookie = "wrong act or pwd";
		}
		
		/* send cookie */
		LOG(cookie);
		if (send(client, cookie.c_str(), COOKIE_LENGTH, 0) < 0) {
	        ERR("failed to send cookie!");
	    } else {
	        LOG("cookie sent.");
	    }
	    return;
	}
	
	
	if ((not login) and (not rstVCode)) {
		/* read vcode */
	    memset(rcvBuff, 0, sizeof(rcvBuff));
	    if ((buffLength = recv(client, rcvBuff, 7, 0)) < 0) {
	        ERR("failed to receive vcode!", buffLength);
			vCode = "";
	    } else {
	    	rcvBuff[7] = 0;
	    	vCode = rcvBuff; 
		}
		LOG(rcvBuff);

		if (checkVCode(account, vCode)) {
			cookie = addAccount(account, password);
		} else {
			cookie = "wrong vrfy codes";
		}
		
		/* send cookie */
		LOG(cookie);
		if (send(client, cookie.c_str(), COOKIE_LENGTH, 0) < 0) {
	        ERR("failed to send cookie!");
	    } else {
	        LOG("cookie sent.");
	    }
	    return;
	}
	
}


void handlePost(std :: string requestHeader, int client, int threadNumber) {
	int requestPos = 0;
	if ((requestPos = requestHeader.find("?")) < 0) {
		ERR("url format error!");
	}
	
	std :: string request = "";
	
	while (requestHeader[requestPos] != ' ' and requestHeader[requestPos] != '&') {
		request += requestHeader[requestPos];
		requestPos++;
	
		if (requestPos > requestHeader.length()) {
			break;
		}
	}

	if (request.compare("?login") == 0) {
		handleLogin(requestHeader, client);
		return ;
	} else if (request.compare("?signin") == 0) {
		handleLogin(requestHeader, client, false);
		return ;
	} else if (request.compare("?vcode") == 0) {
		handleLogin(requestHeader, client, false, true);
		return ;
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
		
		char *content = new char[dataSize + 27];

		if (recv(client, content, dataSize, 0) < 0) {
            ERR("failed to receive file!");
        } else {
        	content[dataSize] = 0;
		}
		
		strcpy(content + dataSize, "lack of end");
		content[dataSize + 11] = '\n';
		content[dataSize + 12] = '\\';
		strcpy(content + dataSize + 13, "end{document}");
		content[dataSize + 26] = 0;
		
		LOG(content); 
		
		
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
			
			std :: string responseFile = "";
			
			if (request.compare("?texToHtml") == 0) {
				responseFile = baseName + ".html";
			} else {
				responseFile = baseName + ".pdf";
			}
			
			LOG(responseFile);
			
			char *fileContent = readFile(responseFile.c_str());
			char *cssContent = NULL;
			int fileLength, cssLength;
			if (fileContent == NULL) {
				fileContent = "Something goes wrong with rendering latex.";
			} else {
				std :: string cssName = baseName + ".css";
				cssContent = readFile(cssName.c_str());
			}
			
			std :: string rspHeader = "";
			fileLength = strlen(fileContent);
			
			if (cssContent == NULL) {
				rspHeader = "dataNumber=1\r\n";
				rspHeader += "fileSize=";
				rspHeader += std :: to_string(fileLength);
			} else {
				rspHeader = "dataNumber=2\r\n";
				rspHeader += "fileSize=";
				rspHeader += std :: to_string(fileLength);
				rspHeader += "\r\n";
				
				cssLength = strlen(cssContent);
				rspHeader += "cssSize=";
				rspHeader += std :: to_string(cssLength);
				rspHeader += "\r\n";
			}
			
			while(rspHeader.length() < RSP_PACKAGE) rspHeader += ' ';
			
			if (send(client, rspHeader.c_str(), RSP_PACKAGE, 0) < 0) {
		        ERR("failed to send response header!");
		    } else {
		        LOG("response header sent.");
		    }
			
			char rcvBuff[RSP_PACKAGE + 1];
   			memset(rcvBuff, 0, sizeof(rcvBuff));
   			
			for (int i = 0; i < fileLength; i += RSP_PACKAGE) {
		    	int sendSize = RSP_PACKAGE;
		    	if (i + RSP_PACKAGE > fileLength) {
		    		sendSize = fileLength - i;
				}
				
				strncpy(rcvBuff, fileContent + i, sendSize);
				rcvBuff[sendSize] = 0;
				LOG(rcvBuff);
				
		    	if (send(client, rcvBuff, sendSize, 0) < 0) {
			        ERR("failed to receive css!");
			    }
			}
		    LOG("file sent.");
		
		    
		    if (cssContent != NULL) {
			    for (int i = 0; i < cssLength; i += RSP_PACKAGE) {
			    	int sendSize = RSP_PACKAGE;
			    	if (i + RSP_PACKAGE > cssLength) {
			    		sendSize = cssLength - i;
					}
					
					strncpy(rcvBuff, cssContent + i, sendSize);
					rcvBuff[sendSize] = 0;
					LOG(rcvBuff);
					
			    	if (send(client, rcvBuff, sendSize, 0) < 0) {
				        ERR("failed to receive css!");
				    }
				}
			    LOG("css sent.");
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
	srand((unsigned)time(NULL));
    int serverSocket;
    struct sockaddr_in serverAddr;

    char sndBuff[RSP_PACKAGE + 1], rcvBuff[RQS_PACKAGE + 1];

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
	
	int recvTimeout = 3 * 1000;
	int sendTimeout = 3 * 1000; 
	
	setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, (char *)&recvTimeout ,sizeof(int));
	setsockopt(serverSocket, SOL_SOCKET, SO_SNDTIMEO, (char *)&sendTimeout ,sizeof(int));
	
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLength = sizeof(clientAddr);
        int client;

        if ((client = accept(serverSocket, (struct sockaddr*)&clientAddr,
                             &clientAddrLength)) < 0) {
            ERR("failed to link to client!");
        } else {
            LOG("clint linked.");
			
			int length = 0;
            if ((length = recv(client, rcvBuff, RQS_PACKAGE, 0)) < 0) {
                ERR("failed to read requst!");
            } else {
                LOG("request read."); 
                rcvBuff[RQS_PACKAGE] = 0;
                LOG(std :: to_string(length));
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
                Sleep(2); // waiting for buffer
                close(client);
            }
        }
    }
}
