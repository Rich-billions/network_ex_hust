#include "Config.h"

using namespace std;

Config::Config(void)
{
}

Config::~Config(void)
{
}

const int Config::MAXCONNECTION = 5;				//最大连接数5
const int Config::BUFFERLENGTH = 4096;				//缓冲区大小4096字节
const u_long Config::BLOCKMODE = 1;					//SOCKET为非阻塞模式
const string Config::config_path = "dataBase/Config.ini"; //配置文件路径
string Config::SERVERADDRESS = "";
int Config::PORT = 0;
string Config::data_path = "/dataBase";

bool Config::setConfig() {
	ifstream config_filePtr(Config::config_path,ifstream::binary);			//open the file of config in binary
	if (!config_filePtr.is_open()) {
		cout << "打开配置文件失败" << endl;
		return false;
	}
	::getline(config_filePtr, Config::data_path);	//读取资源路径
	::getline(config_filePtr, Config::SERVERADDRESS);//读取服务器地址
	string temp;
	::getline(config_filePtr, temp);
	Config::PORT = stoi(temp);						//读取服务器端口号
	config_filePtr.close();
	cout << Config::PORT;
	return true;
}