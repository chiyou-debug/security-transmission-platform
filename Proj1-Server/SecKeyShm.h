#pragma once
#include "BaseShm.h"
#include <string.h>
#include "SecKeyInfo.h"

class SecKeyShm : public BaseShm
{
public:
	SecKeyShm(int key, int maxNode);
	SecKeyShm(string pathName, int maxNode);
	~SecKeyShm();

	void shmInit();
	int shmWrite(SecKeyInfo* pNodeInfo);
	SecKeyInfo shmRead(int keyID);
	SecKeyInfo shmRead(string clientID, string serverID);

private:
	int m_maxNode;
};

