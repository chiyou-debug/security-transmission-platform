#include <cstdio>
#include "TcpSocket.h"
#include <string.h>
#include <iostream>
#include <unistd.h>
#include "Interface.h"

using namespace std;

int main()
{
	// 创建套接字
	TcpSocket socket;
	// 连接服务器
	cout << "开始连接...";
	socket.connectToHost("127.0.0.1", 8888);
	cout << "连接成功...";

	// 创建接口对象
	Interface inter("config.json");
	// 通信
	while (1)
	{
		string sendmsg = "hello server ...";
		sendmsg = inter.aesEncrypt(sendmsg);
		cout << "发送数据" << sendmsg << endl;
		socket.sendMsg(sendmsg);
		// 接收数据
		int recvLen = -1;
		string recvMsg = socket.recvMsg();
		// 数据解密
		recvMsg = inter.aesDecrypt(recvMsg);
		cout << "recvmsg: " << recvMsg << endl;
		sleep(1);
	}
    return 0;
}