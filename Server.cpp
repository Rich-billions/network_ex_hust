#include "Server.h"
#include "Config.h"
#include "winsockEnv.h"
#include <iostream>
#include <string>

#pragma comment(lib,"ws2_32.lib")
#pragma warning(disable:4996)

using namespace std;

Server::Server(void)
{
	this->recvBuf = new char[Config::BUFFERLENGTH]; //初始化接受缓冲区
	memset(this->recvBuf, '\0', Config::BUFFERLENGTH);
	this->sessions = new list<SOCKET>();

}

Server::~Server(void)
{
	//释放接受缓冲区
	if (this->recvBuf != NULL) {
		delete this->recvBuf;
		this->recvBuf = NULL;
	}
	//关闭server socket
	if (this->srvSocket != NULL) {
		closesocket(this->srvSocket);
		this->srvSocket = NULL;
	}
	//关闭所有会话socket并释放会话队列
	if (this->sessions != NULL) {
		for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++)
			closesocket(*itor); //关闭会话
		delete this->sessions;  //释放队列
		this->sessions = NULL;
	}
	WSACleanup(); //清理winsock 运行环境
}

int Server::WinsockStartup() {
	if (WinsockEnv::Startup() == -1) return -1;	//初始化Winsock
	return 0;
}

//初始化Server，包括创建sockect，绑定到IP和PORT
int Server::ServerStartup() {
	//创建 TCP socket
	this->srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->srvSocket == INVALID_SOCKET) {
		cout << "Server socket create error !\n";
		WSACleanup();
		return -1;
	}
	cout << "Server socket create ok!\n";
	Config::setConfig();
	//设置服务器IP地址和端口号
	this->srvAddr.sin_family = AF_INET;
	this->srvAddr.sin_port = htons(Config::PORT);
	//this->srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//会自动找到服务器合适的IP地址
	this->srvAddr.sin_addr.S_un.S_addr = inet_addr(Config::SERVERADDRESS.c_str()); //这是另外一种设置IP地址的方法

	//绑定 socket to Server's IP and port
	int rtn = bind(this->srvSocket, (LPSOCKADDR) & (this->srvAddr), sizeof(this->srvAddr));
	if (rtn == SOCKET_ERROR) {
		cout << "Server socket bind error!\n";
		closesocket(this->srvSocket);
		WSACleanup();
		return -1;
	}

	cout << "Server socket bind ok!\n";
	return 0;
}

//开始监听,等待客户的连接请求
int Server::ListenStartup() {
	int rtn = listen(this->srvSocket, Config::MAXCONNECTION);
	if (rtn == SOCKET_ERROR) {
		cout << "Server socket listen error!\n";
		closesocket(this->srvSocket);
		WSACleanup();
		return -1;
	}

	cout << "Server socket listen ok!\n";
	return 0;
}

//根据url设置发送文件后缀（content-type)
void Server::setContentType (const string& url, string& content_type) const {
	size_t back_beginPos = url.find('.');
	string backStr = url.substr(back_beginPos);
	if (backStr == ".html" || backStr == ".htm") content_type = "text/html";
	else if (backStr == ".jpg" || backStr == ".jpeg") content_type = "image/jpeg";
	else if (backStr == ".png") content_type = "image/png";
	else if (backStr == ".pdf") content_type = "application/pdf";
	else if (backStr == ".mp4") content_type = "video/mpeg4";
	else if (backStr == ".ico") content_type = "application/x-ico";
}

int Server::recvMessage(SOCKET socket) {
	memset(this->recvBuf, '\0', Config::BUFFERLENGTH);
	int recvMsg = recv(socket, this->recvBuf, Config::BUFFERLENGTH, 0);
	if (recvMsg == SOCKET_ERROR) {
		//cout << "客户端发送数据错误!\n";
		//cout << "错误代码为：" << WSAGetLastError() << endl;
		//closesocket(socket);
		return recvMsg;
	}
	if (recvMsg == 0) {
		//closesocket(socket);
		return recvMsg;
	}
	//接受Http报文
	string url;//方法字段和url字段
	string requestMessage;//http请求报文
	string answerMessage;//http响应报文
	string statusLine;//响应报文初始状态行
	string content_type;//请求文件类型
	string result;//请求处理结果
	requestMessage = requestMessage.assign(this->recvBuf);
	//cout << "请求报文：\n" << requestMessage;
	size_t endOfLine = requestMessage.find("\n");
	string requestLine = requestMessage.substr(0, endOfLine);//获得请求报文首部行
	size_t url_begin = requestLine.find('/');
	size_t url_end = requestLine.find("HTTP");
	url = requestLine.substr(url_begin, url_end - 1 - url_begin);
	//if (url == "/favicon.ico") return recvMsg;
	SOCKADDR_IN RequestSocket;
	int socketAddrLen = sizeof(RequestSocket);
	getpeername(socket, (struct sockaddr*)&RequestSocket, &socketAddrLen);
	cout << "请求客户IP地址：" << inet_ntoa(RequestSocket.sin_addr) << endl << "请求客户端口号：" << ntohs(RequestSocket.sin_port) << endl;
	cout << "请求报文首部行：" << requestLine.c_str() << endl;//输出请求报文首部行
	url.insert(0, Config::data_path, 1, Config::data_path.size() - 2);
	if (url[url.size() - 1] == '/') url.append("index.html");
	//cout << url << endl;
	FILE* file = fopen(url.c_str(), "rb");
	if (!file) {
		url = "dataBase/404.html";
		file = fopen(url.c_str(), "rb");//打开404文件
		statusLine = "HTTP/1.1 404 Not Found";
		result = "请求文件不在本地,已返回404错误";
	}
	else {
		statusLine = "HTTP/1.1 200 OK";
		result = "已发送本地文件：" + url;
	}
	fseek(file, 0, 0);
	fseek(file, 0, 2);//将文件指针移动到末尾
	int fileLength = ftell(file);//获取文件字节长度
	fseek(file, 0, 0);//归位
	this->setContentType(url, content_type);
	answerMessage = statusLine + "\r\n" + "Content-Length: " + to_string(fileLength) + "\r\n" + "Content-Type: " + content_type + "\r\n\r\n";//构造响应报文
	cout << "请求处理结果：" << result << endl << "响应报文：\n" << answerMessage << endl;
	if (send(socket, answerMessage.c_str(), answerMessage.size(), 0) == SOCKET_ERROR) {
		cout << "send1 ERROR!\n";
	}
	this->sendBuf = new char[Config::BUFFERLENGTH];
	int t = 0;
	while (true) {
		memset(this->sendBuf, 0, Config::BUFFERLENGTH);
		int file_real_Length = fread(this->sendBuf, 1, Config::BUFFERLENGTH, file);
		if (send(socket, this->sendBuf, file_real_Length, 0) == SOCKET_ERROR) {
			cout << "send2 ERROR!\n";
		}
		if (feof(file)) break;//如果文件结束，终止传输
	}
	fclose(file);
	if (this->sendBuf != nullptr) delete[] this->sendBuf;
	return recvMsg;
}

int Server::Loop() {
	u_long blockMode = Config::BLOCKMODE;//将srvSock设为非阻塞模式以监听客户连接请求
	int rtn;

	if ((rtn = ioctlsocket(this->srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO：允许或禁止套接口s的非阻塞模式。
		cout << "ioctlsocket() failed with error!\n";
		return -1;
	}
	cout << "ioctlsocket() for server socket ok!Waiting for client connection and data\n";

	SOCKET newSession;	//会话socket
	SOCKET newSession_2;
	bool first_connection = true;
	sockaddr_in clientAddr;		//客户端IP地址
	int nAddrLen = sizeof(clientAddr);
	while (true) {
		//清空文件描述符集合
		FD_ZERO(&this->rfds);
		FD_ZERO(&this->wfds);

		FD_SET(this->srvSocket, &this->rfds);		//把服务器套接字加入到可读文件描述符集合中
		
		//把当前的会话socket加入到rfds,等待用户数据的到来;加到wfds，等待socket可发送数据
		//if (!first_connection) {
			//FD_SET(newSession, &this->rfds);
			//FD_SET(newSession, &this->wfds);
		//}

		//把当前的会话socket加入到rfds,等待用户数据的到来;加到wfds，等待socket可发送数据
		for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++) {
			FD_SET(*itor, &rfds);
			FD_SET(*itor, &wfds);
		}

		//等待用户连接请求或用户数据到来或会话socket可发送数据
		if ((this->numOfSocketSignaled = select(0, &this->rfds, &this->wfds, NULL, NULL)) == SOCKET_ERROR) { //select函数返回有可读或可写的socket的总数，保存在rtn里.最后一个参数设定等待时间，如为NULL则为阻塞模式
			cout << "select() failed with error!\n";
			cout << WSAGetLastError();
			return -1;
		}
		//首先检查是否有客户请求连接到来
		if (this->numOfSocketSignaled > 0) {
			if (FD_ISSET(this->srvSocket, &this->rfds)) {
				this->numOfSocketSignaled--;
				//产生会话socket
				newSession = accept(this->srvSocket, (LPSOCKADDR) & (clientAddr), &nAddrLen);
				//cout << ++times << endl;
				if (newSession == INVALID_SOCKET) {
					cout << "Server accept connection request error!\n";
					return -1;
				}
				//cout << "New client connection request arrived and new session created\n";

				//将新的会话socket设为非阻塞模式，
				if (ioctlsocket(newSession, FIONBIO, &blockMode) == SOCKET_ERROR) {
					cout << "ioctlsocket() for new session failed with error!\n";
					return -1;
				}
				//将会话socket加入文件描述符集中
				//FD_SET(newSession, &this->rfds);
				this->sessions->push_back(newSession);
				//FD_SET(newSession, &this->wfds);
				first_connection = false;
			}
		}

		if (this->numOfSocketSignaled > 0) {
			//遍历会话列表中的所有socket，检查是否有数据到来
			for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++) {
				if (FD_ISSET(*itor, &rfds)) {  //某会话socket有数据到来
					//接受数据
					this->recvMessage(*itor);
				}
			}//end for
		}

		//检查是否有用户数据到来或者会话socket可以发送数据
		//if (this->numOfSocketSignaled > 0) {
			//if (FD_ISSET(newSession, &this->rfds)) {
				//int rtn_2 = this->recvMessage(newSession);
				//if (rtn_2 <= 0) {
					//if (!this->sessions->empty()) this->sessions->pop_back();
					//if(!this->sessions->empty()) newSession = this->sessions->back();
					//first_connection = true;
					//closesocket(newSession);
				//}
			//}
		//}
	}
	return 0;
}