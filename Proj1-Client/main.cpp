#include <iostream>
#include <string>
#include "ClientOP.h"
using namespace std;

int usage();
int main()
{
	ClientOP cop("client.json");
	while (1)
	{
		int sel = usage();
		switch (sel)
		{
		case 1:
			// 秘钥协商
			cop.secKeyConsult();
			break;
		case 2:
			// 秘钥校验
			cop.secKeyCheck();
			break;
		case 3:
			// 秘钥注销
			cop.secKeyCancel();
		case 0:
			// 退出程序
			break;
		default:
			break;
		}
	}
	cout << "客户端退出, bye,byte..." << endl;
	return 0;
}

int usage()
{
	int nSel = -1;
	printf("\n  /*************************************************************/");
	printf("\n  /*************************************************************/");
	printf("\n  /*     1.密钥协商                                            */");
	printf("\n  /*     2.密钥校验                                            */");
	printf("\n  /*     3.密钥注销                                            */");
	printf("\n  /*     4.密钥查看                                            */");
	printf("\n  /*     0.退出系统                                            */");
	printf("\n  /*************************************************************/");
	printf("\n  /*************************************************************/");
	printf("\n\n  选择:");

	scanf("%d", &nSel);
	while (getchar() != '\n');

	return nSel;
}