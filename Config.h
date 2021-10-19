#pragma once
#include <string>
#include <winsock2.h>
#include <fstream>
#include <iostream>

using namespace std;

//保存配置信息
class Config
{
public:
	static const int MAXCONNECTION;		//最大连接数
	static const int BUFFERLENGTH;		//缓冲区大小
	static string SERVERADDRESS;		//服务器地址
	static int PORT;					//服务器端口
	static const u_long BLOCKMODE;		//SOCKET阻塞模式
	static const string config_path;    //配置文件路径
	static string data_path;			//资源路径（主目录）
	static bool setConfig();			//配置 Web 服务器的监听地址、监听端口和主目录
private:
	Config(void);
	~Config(void);
};