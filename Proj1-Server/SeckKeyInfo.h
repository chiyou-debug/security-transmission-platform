#pragma once
class SeckKeyInfo
{
public:
	SeckKeyInfo();
	~SeckKeyInfo();

	bool status;		// 秘钥状态: true:可用, false:不可用
	int seckeyID;	    // 秘钥的编号
	char clientID[12];	// 客户端ID, 客户端的标识
	char serverID[12];	// 服务器ID, 服务器标识
	char seckey[128];	// 对称加密的秘钥

};

struct NodeSHMInfo
{
};

