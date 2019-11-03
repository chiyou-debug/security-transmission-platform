#pragma once
#include <string>
#include "SecKeyShm.h"
using namespace std;
class ClientOP
{
public:
	ClientOP(string fileName);
	~ClientOP();

	// 秘钥协商
	void secKeyConsult();
	// 秘钥校验
	void secKeyCheck() {}
	// 秘钥注销
	void secKeyCancel() {}

private:
	string m_ip;
	unsigned short m_port;
	string m_clientID;
	string m_serverID;
	SecKeyShm* m_shm;
};

