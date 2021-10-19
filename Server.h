#pragma once
#include <winsock2.h>
#include <string>
#include <list>
#include <WS2tcpip.h>
using namespace std;

//������

class Server
{
private:
	SOCKET srvSocket;			//������socket
	char *recvBuf;				//���ܻ�����
	char *sendBuf;
	fd_set rfds;				//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�������ݵ�����
	fd_set wfds;				//���ڼ��socket�Ƿ���Է��͵��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�����Է������ݣ�
	sockaddr_in srvAddr;		//��������IP��ַ
	int numOfSocketSignaled;						//�ɶ���д������������socket����
	list<SOCKET>* sessions;							//��ǰ�ĻỰsocket����

protected:
	virtual int recvMessage(SOCKET socket);										//��SOCKET s������Ϣ
	virtual void setContentType(const string& url, string& content_type) const;
	//virtual void sendMessage(SOCKET s, string msg);							//��SOCKET s������Ϣ
	//virtual void  ReceieveMessageFromClients();								//���ܿͻ��˷�������Ϣ
	//virtual int AcceptRequestionFromClient();								//�ȴ��ͻ�����������

public:
	Server(void);
	virtual ~Server(void);
	virtual int WinsockStartup();		//��ʼ��Winsock
	virtual int ServerStartup();		//��ʼ��Server����������SOCKET���󶨵�IP��PORT
	virtual int ListenStartup();		//��ʼ�����ͻ�������
	virtual int Loop();					//ѭ��ִ��"�ȴ��ͻ�������"->���������ͻ�ת����Ϣ��->"�ȴ��ͻ�����Ϣ"
};
