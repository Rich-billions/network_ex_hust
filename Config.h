#pragma once
#include <string>
#include <winsock2.h>
#include <fstream>
#include <iostream>

using namespace std;

//����������Ϣ
class Config
{
public:
	static const int MAXCONNECTION;		//���������
	static const int BUFFERLENGTH;		//��������С
	static string SERVERADDRESS;		//��������ַ
	static int PORT;					//�������˿�
	static const u_long BLOCKMODE;		//SOCKET����ģʽ
	static const string config_path;    //�����ļ�·��
	static string data_path;			//��Դ·������Ŀ¼��
	static bool setConfig();			//���� Web �������ļ�����ַ�������˿ں���Ŀ¼
private:
	Config(void);
	~Config(void);
};