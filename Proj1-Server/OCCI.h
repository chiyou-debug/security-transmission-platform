#pragma once
#include <string>
#include <occi.h>
#include "SecKeyInfo.h"
using namespace std;

class OCCI
{
public:
	OCCI();
	// 连接数据库
	// ip:prot/orcl
	bool connectDB(string uName, string pWd, string connStr);
	// 操作的API
	// 得到秘钥的keyID
	int getKeyID();
	// 更新keyID
	bool updataKeyID(int keyID);
	// 添加秘钥
	bool writeSecKey(SecKeyInfo* info);
	// 判断客户端节点是否有效可用
	bool isValidClient(string clientID, int state = 1);
	// 断开连接的接口
	void closeDB();
	~OCCI();

private:
	string getCurrentTime();

private:
	oracle::occi::Environment* m_env;
	oracle::occi::Connection* m_conn;
};

