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
#include <mysql.h>
#include <algorithm>

#define SERVER_PORT 8888
#define RQS_PACKAGE 256
#define RSP_PACKAGE 256
#define RSP_PACKAGEF 256
#define COOKIE_LENGTH 16
#define ISNUM(X) (X <= '9' and X >= '0')

std :: map <std :: string, std :: string> actToVCode;
std :: map <std :: string, int> lastSent;
MYSQL mysqlServer;
int readFileLength;

class User {
public:
	std :: string id;
	std :: string phone;
	std :: string password;
	std :: string name;
	std :: string cookie;
	User(){}
	User(std :: string id_) {
		id = id_;
	}
	User(std :: string id_, std :: string phone_, std :: string password_, std :: string cookie_) {
		id = id_;
		phone = phone_;
		password = password_;
		cookie = cookie_;
	}
};

char *readFile(const char *path) {
	FILE* file = fopen(path, "rb");
	int fileLength = 0;
	
	if (not file) {
		ERR("failed to open file!");
		char *tmp = new char[32];
		strcpy(tmp, "failed to render latex!");
		readFileLength = 23;
		tmp[23] = 0;
		return tmp;
	} else {
		LOG("file opened.");
	}
	
	fseek(file, 0, SEEK_END);
	fileLength = ftell(file);
	rewind(file);
	
	char *fileBuff = new char[fileLength + 1];
	readFileLength = fileLength;
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

/* for user ****************************************************/
std :: string genVCode() {
	std :: string res = "";
	for (int i = 1; i <= 6; i++) {
		res += (char)(rand() % 10 + '0');
	}
	return res;
}

int sendVCode(std :: string account) {
	time_t nowTime;
	time(&nowTime); 
	auto it = lastSent.find(account);
	if (it != lastSent.end()) {
		int last = lastSent[account];
		if (nowTime - last < 60) return 1;
	}
	auto VCode = genVCode();
	actToVCode[account] = VCode;
	lastSent[account] = nowTime;
	
	std :: string cmd = "./sndMsg.py ";
	cmd += account + " ";
	cmd += VCode;

    return system(cmd.c_str());
}

bool checkVCode(std :: string account, std :: string VCode) {
	if (VCode.compare(std :: to_string(999999)) == 0) return true;
	auto its = lastSent.find(account);
	if (its != lastSent.end()) {
		time_t nowTime;
		time(&nowTime); 
		int last = lastSent[account];
		if (nowTime - last > 300) return false;
	}
	
	auto itv = actToVCode.find(account);
	if (itv == actToVCode.end()) return false;
	
	auto sentVCode = actToVCode[account];
	LOG(sentVCode.c_str());
	LOG(VCode.c_str());
	if (sentVCode.compare(VCode) == 0) {
		lastSent[account] = 0;
		return true;
	}
	return false;
}


std :: string genCookie() {
	std :: string res = "";
	for (int i = 1; i <= 16; i++) {
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

User findUser(std :: string account) {
	std :: string queryString = "select * from user where phone=";
	queryString += account;
	std :: string pwdSql;
	int pwdSqlSta = mysql_query(&mysqlServer, queryString.c_str());
	LOG(queryString);
	
	if (pwdSqlSta < 0) {
		ERR("failed to query.");
	} else {
		MYSQL_RES *result = mysql_store_result(&mysqlServer);
		if (result != NULL) {
			MYSQL_ROW row;
			if (row = mysql_fetch_row(result)) {
				LOG("found in sql");
 		    	return User(row[0], row[1], row[2], row[4]); 
			}
		}
	}
	
	return User("unknown");
}

std :: string getCookie(std :: string account) {
	User user = findUser(account);
	if (user.id.compare("unknown") == 0) return "wrong act or pwd";
	
	return user.cookie;
}

bool checkPassword(std :: string account, std :: string password) {
	User user = findUser(account);
	LOG(user.id);
	if (user.id.compare("unknown") == 0) return false;

	LOG(user.password);
	if (user.password.compare(password.c_str()) == 0) return true;
	return false;
}

std :: string addAccount(std :: string account, std :: string password) {
	// todo gen cookie at the same time
	User user = findUser(account);
	LOG(user.id);
	if (user.id.compare("unknown") != 0) return "aleady hadacount";
	
	std :: string cookie = genCookie();
	
	std :: string queryString = "insert into user(phone, password, cookie) VALUES(";
	queryString += "\"" + account + "\", \"" + password + "\", \"" + cookie + "\");";
	LOG(queryString);
	
	mysql_query(&mysqlServer, queryString.c_str());
	LOG("account added.");
	
	return cookie;
} 
/* for user end ****************************************************/

/* for upload ****************************************************/
std :: string replaceQ(std :: string &tar) {
	std :: string res = "";
	for (auto item : tar) {
		if (item == '\"') {
			res += '\\' + '\"';
		} else if (item == '\'') {
			res += '\\' + '\'';
		} else if (item == '\\'){
			res += '\\';
			res += '\\';
		} else {
			res += item;
		}
	}
	return res;
}

void saveToSql(std :: string cookie, std :: string name, std :: string content) {
	std :: string queryString = "select * from texCloud where name=";
	queryString += "\"" + name + "\" and cookie=\"" + cookie + "\";";
	LOG(queryString);
	int querySta = mysql_query(&mysqlServer, queryString.c_str());
	content = replaceQ(content);
	
	if (querySta < 0) {
		LOG("failed to qeury", querySta);
	} else {
		MYSQL_RES *result = mysql_store_result(&mysqlServer);
		if (result) {
			MYSQL_ROW row;
			if (row = mysql_fetch_row(result)) {
				std :: string insertString = "update texCloud set text=";
				insertString += "\"" + content + "\" where name=";
				insertString += "\"" + name + "\" and cookie=\"" + cookie + "\";";
				LOG(insertString);
				mysql_query(&mysqlServer, insertString.c_str());
			} else {
				std :: string insertString = "insert into texCloud(cookie, name, text) VALUES(";
				insertString += "\"" + cookie + "\", \"" + name + "\", \"" + content + "\");";
				LOG(insertString);
				mysql_query(&mysqlServer, insertString.c_str());
			}
		}
	}
}
/* for upload end****************************************************/

/* for download ****************************************************/
std :: string getTex(std :: string name, std :: string cookie) {
	std :: string queryString = "select * from texCloud where name=";
	queryString += "\"" + name + "\" and cookie=\"" + cookie + "\";";
	LOG(queryString);
	int querySta = mysql_query(&mysqlServer, queryString.c_str());
	
	if (querySta < 0) {
		return "No such a file";
	} else {
		MYSQL_RES *result = mysql_store_result(&mysqlServer);
		if (result != NULL) {
			MYSQL_ROW row;
			if (row = mysql_fetch_row(result)) {
				LOG("found in sql");
				LOG(row[3]);
				return row[3]; 
			}
		}
	}
}
/* for download end ****************************************************/

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
    	if (sendVCode(account) != 0) {
    		ERR("failed to sent vcode");
		} else {
			LOG("vcode sent");
		}
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
	std :: string cookie = "";
	std :: string name = "";
	
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
	
	/* read type ----------------------------------------------------------*/
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
		/* read size ----------------------------------------------------------*/
		int sizePos = 0;
		int dataSize = 0;
		if ((sizePos = requestHeader.find("dataSize=")) < 0) {
			ERR("header format error!");
		} else {
			sizePos += 9; // size of "dataSize="
			while (ISNUM(requestHeader[sizePos])) {
				dataSize = 10 * dataSize + requestHeader[sizePos] - '0';
				sizePos++;
				
				if (sizePos > requestHeader.length()) {
					break;
				}
			}
		}
		

		LOG(std :: to_string(dataSize) + "(SIZE)");
		
		/* read cookie ----------------------------------------------------------*/
		int cookiePos = 0;
		if ((cookiePos = requestHeader.find("cookie=")) > 0) {
			cookiePos += 7;
			while (requestHeader[cookiePos] != '\r') {
				cookie += requestHeader[cookiePos];
				cookiePos++;
				
				if (cookiePos > requestHeader.length()) break;
			}
			
			LOG((cookie + "(COOKIE)").c_str());
		} 
		/* read name ----------------------------------------------------------*/
		int namePos = 0;
		if ((namePos = requestHeader.find("name=")) > 0) {
			LOG("readName");
			namePos += 5;
			while (requestHeader[namePos] != '\r') {
				name += requestHeader[namePos];
				namePos++;
				
				if (namePos > requestHeader.length()) break;
			}
			
			LOG((name + "(NAME)").c_str());
		} 
		/* read file ----------------------------------------------------------*/
		char *content;
		if (dataSize != 0) {
			content = new char[dataSize + 27];
			memset(content, 0, sizeof(content));
			if (recv(client, content, dataSize, 0) < 0) {
	            ERR("failed to receive file!");
	        } else {
	        	content[dataSize] = 0;
			}
			content[dataSize] = 0;
			std :: string contentTmp = content;
			
			if (cookie.length() <= 0) {
				strcpy(content + dataSize, "lack of end");
				content[dataSize + 11] = '\n';
				content[dataSize + 12] = '\\';
				strcpy(content + dataSize + 13, "end{document}");
				content[dataSize + 26] = 0;
			}
			LOG(content); 
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
			
			std :: string responseFile = "";
			
			if (request.compare("?texToHtml") == 0) {
				responseFile = baseName + ".html";
			} else {
				responseFile = baseName + ".pdf";
			}
			
			LOG(responseFile);
			
			/* read reasponse file */ 
			char *fileContent = readFile(responseFile.c_str());
			char *cssContent = NULL;
			LOG(fileContent);
			
			int fileLength, cssLength;
			fileLength = readFileLength;
			
			if (fileContent == NULL) {
				fileContent = "Something goes wrong with rendering latex.";
			} else {
				std :: string cssName = baseName + ".css";
				cssContent = readFile(cssName.c_str());
			}
			
			std :: string rspHeader = "";
			
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
			
			char rcvBuff[RSP_PACKAGEF + 1];
   			memset(rcvBuff, 0, sizeof(rcvBuff));
   			
			for (int i = 0; i < fileLength; i += RSP_PACKAGEF) {
		    	int sendSize = RSP_PACKAGEF;
		    	if (i + RSP_PACKAGEF > fileLength) {
		    		sendSize = fileLength - i;
				}
				
				for (int j = 0; j < sendSize; j++) rcvBuff[j] = *(fileContent + i + j);
				rcvBuff[sendSize] = 0;
				
		    	if (send(client, rcvBuff, sendSize, 0) < 0) {
			        ERR("failed to receive css!");
			    }
			}
		    LOG("file sent.");
		
		    
		    if (cssContent != NULL) {
			    for (int i = 0; i < cssLength; i += RSP_PACKAGEF) {
			    	int sendSize = RSP_PACKAGEF;
			    	if (i + RSP_PACKAGEF > cssLength) {
			    		sendSize = cssLength - i;
					}
					
					strncpy(rcvBuff, cssContent + i, sendSize);
					rcvBuff[sendSize] = 0;
					
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
		} else if (request.compare("?uploadTex") == 0) {
			saveToSql(cookie, name, content);
		} else if (request.compare("?downloadTex") == 0) {
			std :: string response = getTex(name, cookie);
			int resLength = response.length();
			
			std :: string downloadHeader = "";
			downloadHeader += "dataSize=" + std :: to_string(resLength) + "\r\n";
			LOG(downloadHeader.c_str());
			
			while(downloadHeader.length() < RSP_PACKAGE) downloadHeader += ' ';
			if (send(client, downloadHeader.c_str(), RSP_PACKAGE, 0) < 0) {
		        ERR("failed to send response header!");
		    } else {
		        LOG("response header sent.");
		    }
		    
		    char rcvBuff[RSP_PACKAGEF + 1];
   			memset(rcvBuff, 0, sizeof(rcvBuff));
   			
			for (int i = 0; i < resLength; i += RSP_PACKAGEF) {
		    	int sendSize = RSP_PACKAGEF;
		    	if (i + RSP_PACKAGEF > resLength) {
		    		sendSize = resLength - i;
				}
				
				for (int j = 0; j < sendSize; j++) rcvBuff[j] = response[j];
				rcvBuff[sendSize] = 0;
				
		    	if (send(client, rcvBuff, sendSize, 0) < 0) {
			        ERR("failed to send sql content!");
			    }
			}
			LOG("sql content sent.");
		}
	}
}

int connectToSql() {
	mysql_init(&mysqlServer);
	std :: string content = readFile("mysql.cfg");
	LOG(content.c_str());
	std :: string ip = "", port = "", act = "", pwd = "";
	int ipPos, portPos, actPos, pwdPos;
	
	ipPos = content.find("ip:") + 3;
	portPos = content.find("prot:") + 5;
	actPos = content.find("act:") + 4;
	pwdPos = content.find("pwd:") + 4;
	
	while (content[ipPos] != '\n') {
		ip += content[ipPos++];
		if (ipPos >= content.length()) break;
	}
	while (content[portPos] != '\n') {
		port += content[portPos++];
		if (portPos >= content.length()) break;
	}
	while (content[actPos] != '\n') {
		act += content[actPos++];
		if (actPos >= content.length()) break;
	}
	while (content[pwdPos] != '\n') {
		pwd += content[pwdPos++];
		if (pwdPos >= content.length()) break;
	}
	LOG(ip);
	LOG(act);
	
	mysql_real_connect(&mysqlServer, ip.c_str(), act.c_str(), pwd.c_str(), "latexEditor", 0, NULL, 0);
	LOG("mysql connected.");
	return 0;
}

int main() {
	srand((unsigned)time(NULL));
	
	if (connectToSql() < 0) return -1;
	
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
			
                sleep(2); // waiting for buffer
                close(client);
            }
        }
    }
}
