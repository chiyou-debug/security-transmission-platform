#include "Interface.h"
#include <json/json.h>
#include <fstream>
#include "AesCrypto.h"
#include "SecKeyShm.h"

using namespace Json;
using namespace std;

Interface::Interface(string fileName)
{
	ifstream ifs(fileName);
	Reader r;
	Value root;
	r.parse(ifs, root);
	string key = root["KeyID"].asString();
	int maxNode = root["MaxNode"].asInt();
	string cliID = root["ClientID"].asString();
	string serID = root["ServerID"].asString();
	// 打开共享内存
	SecKeyShm shm(key, maxNode);
	// 读共享内存
	NodeSHMInfo shmInfo = shm.shmRead(cliID, serID);
	m_key = shmInfo.seckey;
}


Interface::~Interface()
{
}

string Interface::aesEncrypt(string data)
{
	AesCrypto aes(m_key);
	return aes.aesCBCEncrypt(data);
}

string Interface::aesDecrypt(string data)
{
	AesCrypto aes(m_key);
	return aes.aesCBCDecrypt(data);
}
