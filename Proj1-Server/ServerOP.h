#pragma once
#include <map>
#include "TcpServer.h"
#include "Message.pb.h"
#include "OCCI.h"
#include "SecKeyShm.h"
class ServerOP
{
public:
	enum KeyLen {L16 = 16,L24=24, L32=32};
	ServerOP(string fileName);
	~ServerOP();

	// 启动服务器程序
	void startServer();
	// 子线程工作函数
	static void* working(void* arg);
	friend void* hardwork(void* arg);

	// 秘钥协商
	string secKeyConsult(RequestMsg* msg);

private:
	string getRandKey(KeyLen len);

private:
	OCCI m_oci;	// 数据库操作对象
	unsigned short m_port;
	string m_serverID;
	TcpServer* m_server;
	map<pthread_t, TcpSocket*> m_list;
	SecKeyShm *m_shm;
};

void* hardwork(void* arg);

