#pragma once
#include "BaseShm.h"
#include <string.h>

struct NodeSHMInfo
{
	NodeSHMInfo() : status(false), seckeyID(0)
	{
		bzero(clientID, sizeof(clientID));
		bzero(serverID, sizeof(serverID));
		bzero(seckey, sizeof(seckey));
	}
	
	bool status;		// 秘钥状态: true:可用, false:不可用
	int seckeyID;	    // 秘钥的编号
	char clientID[12];	// 客户端ID, 客户端的标识
	char serverID[12];	// 服务器ID, 服务器标识
	char seckey[128];	// 对称加密的秘钥
};

class SecKeyShm : public BaseShm
{
public:
	SecKeyShm(int key, int maxNode);
	SecKeyShm(string pathName, int maxNode);
	~SecKeyShm();

	void shmInit();
	int shmWrite(NodeSHMInfo* pNodeInfo);
	NodeSHMInfo shmRead(int keyID);
	NodeSHMInfo shmRead(string clientID, string serverID);

private:
	int m_maxNode;
};

