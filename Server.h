#pragma once
#include <winsock2.h>
#include <string>
#include <list>
#include <WS2tcpip.h>
using namespace std;

//服务器

class Server
{
private:
	SOCKET srvSocket;			//服务器socket
	char *recvBuf;				//接受缓冲区
	char *sendBuf;
	fd_set rfds;				//用于检查socket是否有数据到来的的文件描述符，用于socket非阻塞模式下等待网络事件通知（有数据到来）
	fd_set wfds;				//用于检查socket是否可以发送的文件描述符，用于socket非阻塞模式下等待网络事件通知（可以发送数据）
	sockaddr_in srvAddr;		//服务器端IP地址
	int numOfSocketSignaled;						//可读、写、有请求到来的socket总数
	list<SOCKET>* sessions;							//当前的会话socket队列

protected:
	virtual int recvMessage(SOCKET socket);										//从SOCKET s接受消息
	virtual void setContentType(const string& url, string& content_type) const;
	//virtual void sendMessage(SOCKET s, string msg);							//向SOCKET s发送消息
	//virtual void  ReceieveMessageFromClients();								//接受客户端发来的信息
	//virtual int AcceptRequestionFromClient();								//等待客户端连接请求

public:
	Server(void);
	virtual ~Server(void);
	virtual int WinsockStartup();		//初始化Winsock
	virtual int ServerStartup();		//初始化Server，包括创建SOCKET，绑定到IP和PORT
	virtual int ListenStartup();		//开始监听客户端请求
	virtual int Loop();					//循环执行"等待客户端请求"->“向其他客户转发信息”->"等待客户端消息"
};
