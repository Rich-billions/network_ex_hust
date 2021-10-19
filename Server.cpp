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
	this->recvBuf = new char[Config::BUFFERLENGTH]; //��ʼ�����ܻ�����
	memset(this->recvBuf, '\0', Config::BUFFERLENGTH);
	this->sessions = new list<SOCKET>();

}

Server::~Server(void)
{
	//�ͷŽ��ܻ�����
	if (this->recvBuf != NULL) {
		delete this->recvBuf;
		this->recvBuf = NULL;
	}
	//�ر�server socket
	if (this->srvSocket != NULL) {
		closesocket(this->srvSocket);
		this->srvSocket = NULL;
	}
	//�ر����лỰsocket���ͷŻỰ����
	if (this->sessions != NULL) {
		for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++)
			closesocket(*itor); //�رջỰ
		delete this->sessions;  //�ͷŶ���
		this->sessions = NULL;
	}
	WSACleanup(); //����winsock ���л���
}

int Server::WinsockStartup() {
	if (WinsockEnv::Startup() == -1) return -1;	//��ʼ��Winsock
	return 0;
}

//��ʼ��Server����������sockect���󶨵�IP��PORT
int Server::ServerStartup() {
	//���� TCP socket
	this->srvSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (this->srvSocket == INVALID_SOCKET) {
		cout << "Server socket create error !\n";
		WSACleanup();
		return -1;
	}
	cout << "Server socket create ok!\n";
	Config::setConfig();
	//���÷�����IP��ַ�Ͷ˿ں�
	this->srvAddr.sin_family = AF_INET;
	this->srvAddr.sin_port = htons(Config::PORT);
	//this->srvAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//���Զ��ҵ����������ʵ�IP��ַ
	this->srvAddr.sin_addr.S_un.S_addr = inet_addr(Config::SERVERADDRESS.c_str()); //��������һ������IP��ַ�ķ���

	//�� socket to Server's IP and port
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

//��ʼ����,�ȴ��ͻ�����������
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

//����url���÷����ļ���׺��content-type)
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
		//cout << "�ͻ��˷������ݴ���!\n";
		//cout << "�������Ϊ��" << WSAGetLastError() << endl;
		//closesocket(socket);
		return recvMsg;
	}
	if (recvMsg == 0) {
		//closesocket(socket);
		return recvMsg;
	}
	//����Http����
	string url;//�����ֶκ�url�ֶ�
	string requestMessage;//http������
	string answerMessage;//http��Ӧ����
	string statusLine;//��Ӧ���ĳ�ʼ״̬��
	string content_type;//�����ļ�����
	string result;//��������
	requestMessage = requestMessage.assign(this->recvBuf);
	//cout << "�����ģ�\n" << requestMessage;
	size_t endOfLine = requestMessage.find("\n");
	string requestLine = requestMessage.substr(0, endOfLine);//����������ײ���
	size_t url_begin = requestLine.find('/');
	size_t url_end = requestLine.find("HTTP");
	url = requestLine.substr(url_begin, url_end - 1 - url_begin);
	//if (url == "/favicon.ico") return recvMsg;
	SOCKADDR_IN RequestSocket;
	int socketAddrLen = sizeof(RequestSocket);
	getpeername(socket, (struct sockaddr*)&RequestSocket, &socketAddrLen);
	cout << "����ͻ�IP��ַ��" << inet_ntoa(RequestSocket.sin_addr) << endl << "����ͻ��˿ںţ�" << ntohs(RequestSocket.sin_port) << endl;
	cout << "�������ײ��У�" << requestLine.c_str() << endl;//����������ײ���
	url.insert(0, Config::data_path, 1, Config::data_path.size() - 2);
	if (url[url.size() - 1] == '/') url.append("index.html");
	//cout << url << endl;
	FILE* file = fopen(url.c_str(), "rb");
	if (!file) {
		url = "dataBase/404.html";
		file = fopen(url.c_str(), "rb");//��404�ļ�
		statusLine = "HTTP/1.1 404 Not Found";
		result = "�����ļ����ڱ���,�ѷ���404����";
	}
	else {
		statusLine = "HTTP/1.1 200 OK";
		result = "�ѷ��ͱ����ļ���" + url;
	}
	fseek(file, 0, 0);
	fseek(file, 0, 2);//���ļ�ָ���ƶ���ĩβ
	int fileLength = ftell(file);//��ȡ�ļ��ֽڳ���
	fseek(file, 0, 0);//��λ
	this->setContentType(url, content_type);
	answerMessage = statusLine + "\r\n" + "Content-Length: " + to_string(fileLength) + "\r\n" + "Content-Type: " + content_type + "\r\n\r\n";//������Ӧ����
	cout << "����������" << result << endl << "��Ӧ���ģ�\n" << answerMessage << endl;
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
		if (feof(file)) break;//����ļ���������ֹ����
	}
	fclose(file);
	if (this->sendBuf != nullptr) delete[] this->sendBuf;
	return recvMsg;
}

int Server::Loop() {
	u_long blockMode = Config::BLOCKMODE;//��srvSock��Ϊ������ģʽ�Լ����ͻ���������
	int rtn;

	if ((rtn = ioctlsocket(this->srvSocket, FIONBIO, &blockMode) == SOCKET_ERROR)) { //FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		cout << "ioctlsocket() failed with error!\n";
		return -1;
	}
	cout << "ioctlsocket() for server socket ok!Waiting for client connection and data\n";

	SOCKET newSession;	//�Ựsocket
	SOCKET newSession_2;
	bool first_connection = true;
	sockaddr_in clientAddr;		//�ͻ���IP��ַ
	int nAddrLen = sizeof(clientAddr);
	while (true) {
		//����ļ�����������
		FD_ZERO(&this->rfds);
		FD_ZERO(&this->wfds);

		FD_SET(this->srvSocket, &this->rfds);		//�ѷ������׽��ּ��뵽�ɶ��ļ�������������
		
		//�ѵ�ǰ�ĻỰsocket���뵽rfds,�ȴ��û����ݵĵ���;�ӵ�wfds���ȴ�socket�ɷ�������
		//if (!first_connection) {
			//FD_SET(newSession, &this->rfds);
			//FD_SET(newSession, &this->wfds);
		//}

		//�ѵ�ǰ�ĻỰsocket���뵽rfds,�ȴ��û����ݵĵ���;�ӵ�wfds���ȴ�socket�ɷ�������
		for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++) {
			FD_SET(*itor, &rfds);
			FD_SET(*itor, &wfds);
		}

		//�ȴ��û�����������û����ݵ�����Ựsocket�ɷ�������
		if ((this->numOfSocketSignaled = select(0, &this->rfds, &this->wfds, NULL, NULL)) == SOCKET_ERROR) { //select���������пɶ����д��socket��������������rtn��.���һ�������趨�ȴ�ʱ�䣬��ΪNULL��Ϊ����ģʽ
			cout << "select() failed with error!\n";
			cout << WSAGetLastError();
			return -1;
		}
		//���ȼ���Ƿ��пͻ��������ӵ���
		if (this->numOfSocketSignaled > 0) {
			if (FD_ISSET(this->srvSocket, &this->rfds)) {
				this->numOfSocketSignaled--;
				//�����Ựsocket
				newSession = accept(this->srvSocket, (LPSOCKADDR) & (clientAddr), &nAddrLen);
				//cout << ++times << endl;
				if (newSession == INVALID_SOCKET) {
					cout << "Server accept connection request error!\n";
					return -1;
				}
				//cout << "New client connection request arrived and new session created\n";

				//���µĻỰsocket��Ϊ������ģʽ��
				if (ioctlsocket(newSession, FIONBIO, &blockMode) == SOCKET_ERROR) {
					cout << "ioctlsocket() for new session failed with error!\n";
					return -1;
				}
				//���Ựsocket�����ļ�����������
				//FD_SET(newSession, &this->rfds);
				this->sessions->push_back(newSession);
				//FD_SET(newSession, &this->wfds);
				first_connection = false;
			}
		}

		if (this->numOfSocketSignaled > 0) {
			//�����Ự�б��е�����socket������Ƿ������ݵ���
			for (list<SOCKET>::iterator itor = this->sessions->begin(); itor != this->sessions->end(); itor++) {
				if (FD_ISSET(*itor, &rfds)) {  //ĳ�Ựsocket�����ݵ���
					//��������
					this->recvMessage(*itor);
				}
			}//end for
		}

		//����Ƿ����û����ݵ������߻Ựsocket���Է�������
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