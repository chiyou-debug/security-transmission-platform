#pragma once
#include <string>
using namespace std;
class Interface
{
public:
	Interface(string fileName);
	~Interface();

	// 加密
	string aesEncrypt(string data);
	// 解密
	string aesDecrypt(string data);

private:
	string m_key;
};

