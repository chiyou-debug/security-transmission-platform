#include "OCCI.h"
#include <iostream>

using namespace std;
using namespace oracle::occi;


OCCI::OCCI()
{
}

bool OCCI::connectDB(string uName, string pWd, string connStr)
{
	m_env = Environment::createEnvironment("utf8", "utf8");
	if (m_env == NULL)
	{
		cout << "初始化数据库环境失败..." << endl;
		return false;
	}
	m_conn = m_env->createConnection(uName, pWd, connStr);
	if (m_conn == NULL)
	{
		cout << "连接数据库失败..." << endl;
		return false;
	}

	return true;
}

int OCCI::getKeyID()
{
	// 创建可以操作sql的对象
	Statement* st = m_conn->createStatement();
	st->setSQL("SELECT * FROM KEYSN");
	//执行一个查询
	ResultSet* res = st->executeQuery();
	// 遍历集合
	int keyID = -1;
	if (res->next())
	{
		keyID = res->getInt(1);
	}

	// 释放资源
	st->closeResultSet(res);
	m_conn->terminateStatement(st);

	return keyID;
}

bool OCCI::updataKeyID(int keyID)
{
	// 创建可以操作sql的对象
	Statement* st = m_conn->createStatement();
	st->setAutoCommit(true);
	string sql = "UPDATE KEYSN SET IKEYSN = " + to_string(keyID);
	st->setSQL(sql);
	//执行一个更新
	int ret = st->executeUpdate();
	// 判断得到的结果
	bool bl = true;
	if (ret <= 0)
	{
		bl = false;
	}

	// 释放资源
	m_conn->terminateStatement(st);

	return bl;
}

bool OCCI::writeSecKey(SecKeyInfo * info)
{
	char buf[1024];
	string data = getCurrentTime();
	sprintf(buf, "INSERT INTO SECKEYINFO VALUES('%s', '%s', %d, TO_DATE('%s', 'yyyy-mm-dd hh24:mi:ss'), %d, '%s')",
		info->clientID, info->serverID, info->seckeyID, data.data(), info->status, info->seckey);
	cout << "insert sql: " << buf << endl;
	// 创建可以操作sql的对象
	Statement* st = m_conn->createStatement();
	st->setAutoCommit(true);
	st->setSQL(buf);
	//执行一个更新
	int ret = st->executeUpdate();
	// 判断得到的结果
	bool bl = true;
	if (ret <= 0)
	{
		bl = false;
	}

	// 释放资源
	m_conn->terminateStatement(st);

	return bl;
}

bool OCCI::isValidClient(string clientID, int state)
{
	// 创建可以操作sql的对象
	Statement* st = m_conn->createStatement();
	string sql = "SELECT * FROM SECNODE WHERE ID='" +
		clientID + "' AND STATE=" + to_string(state);
	cout << "isvalidclient sql: " << sql << endl;
	st->setSQL(sql);
	//执行一个查询
	ResultSet* res = st->executeQuery();
	// 遍历集合
	bool bl = false;
	if (res->next())
	{
		bl = true;
	}

	// 释放资源
	st->closeResultSet(res);
	m_conn->terminateStatement(st);

	return bl;
}


void OCCI::closeDB()
{
	m_env->terminateConnection(m_conn);
	Environment::terminateEnvironment(m_env);
}

OCCI::~OCCI()
{
}

string OCCI::getCurrentTime()
{
	// 得到当前时间
	time_t curTm = time(NULL);
	// 格式化得到的世间
	char buf[128] = { 0 };
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&curTm));
	return string(buf);
}
