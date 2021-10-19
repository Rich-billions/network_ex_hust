#include "Config.h"

using namespace std;

Config::Config(void)
{
}

Config::~Config(void)
{
}

const int Config::MAXCONNECTION = 5;				//���������5
const int Config::BUFFERLENGTH = 4096;				//��������С4096�ֽ�
const u_long Config::BLOCKMODE = 1;					//SOCKETΪ������ģʽ
const string Config::config_path = "dataBase/Config.ini"; //�����ļ�·��
string Config::SERVERADDRESS = "";
int Config::PORT = 0;
string Config::data_path = "/dataBase";

bool Config::setConfig() {
	ifstream config_filePtr(Config::config_path,ifstream::binary);			//open the file of config in binary
	if (!config_filePtr.is_open()) {
		cout << "�������ļ�ʧ��" << endl;
		return false;
	}
	::getline(config_filePtr, Config::data_path);	//��ȡ��Դ·��
	::getline(config_filePtr, Config::SERVERADDRESS);//��ȡ��������ַ
	string temp;
	::getline(config_filePtr, temp);
	Config::PORT = stoi(temp);						//��ȡ�������˿ں�
	config_filePtr.close();
	cout << Config::PORT;
	return true;
}