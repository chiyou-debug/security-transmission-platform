#include "ServerOP.h"
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <json/json.h>
#include <unistd.h>
#include "Hash.h"

using namespace std;
using namespace Json;



ServerOP::ServerOP(string fileName)
{
	// 解析json文件数据到内存中
	ifstream ifs(fileName);
	Reader r;
	Value root;
	r.parse(ifs, root);
	// 2. 通过root将数据取出
	// 监听的端口
	m_port = root["Port"].asInt();
	// 当前服务器端的ID
	m_serverID = root["ServerID"].asString();

	// 数据库的连接信息
	string DBUser = root["DBUser"].asString();
	string DBpwd = root["DBPasswd"].asString();
	string DBHost = root["DBHost"].asString();
	unsigned short port = root["DBPort"].asInt();
	string DBName = root["DBName"].asString();

	// 共享内存的信息
	string key = root["ShmKey"].asString();
	int maxNode = root["ShmMaxNode"].asInt();
	// 创建共享内存对象
	m_shm = new SecKeyShm(key, maxNode);
	m_shm->shmInit();

	// 连接数据库
	// "192....:1521/orcl"
	string connStr = DBHost + ":" + to_string(port) + "/" + DBName;
	cout << "连接串: " << connStr << endl;
	bool bl = m_oci.connectDB(DBUser, DBpwd, connStr);
	cout << "数据库连接状态: " << bl << endl;
}


ServerOP::~ServerOP()
{
	m_server->closefd();
	delete m_server;
}

void ServerOP::startServer()
{
	m_server = new TcpServer;
	// 设置监听
	cout << "监听的端口: " << m_port << endl;
	m_server->setListen(m_port);
	// 等待并接受客户端的连接 -> 持续的动作
	while (1)
	{
		TcpSocket* tcp = m_server->acceptConn();
		if (tcp == NULL)
		{
			cout << "接受客户端连接失败, 重试..." << endl;
			continue;
		}

		cout << "和客户端的连接成功建立..." << endl;
		// 通信 -> 交给子线程
		// 创建子线程
		pthread_t tid;
		pthread_create(&tid, NULL, hardwork, this);
		// tcp存储到容器中
		m_list.insert(make_pair(tid, tcp));
	}
}

// 子线程的处理动作, 和客户端通信
void * ServerOP::working(void * arg)
{
	return nullptr;
}

string ServerOP::secKeyConsult(RequestMsg* msg)
{
	// 已经拿了客户端原始数据 -> msg对象中
	// 将接收到的公钥写入到文件中 -> public.pem
	ofstream ofs("public.pem");
	ofs << msg->data();
	ofs.close();

	cout << "公钥文件信息: " << msg->data() << endl;

	RespondInfo info;
	info.clientID = msg->clientid();
	info.serverID = m_serverID;

	// 校验客户端的身份
	bool flag  = m_oci.isValidClient(msg->clientid());
	cout << "校验客户端身份: " << flag << endl;

	// 对公钥的签名进行校验
	RsaCrypto crypto("public.pem", false);
	Hash md5(HashType::T_MD5);
	md5.addData(msg->data());
	bool bl = crypto.rsaVerify(md5.result(), msg->sign());
	if (bl == false || flag == false)
	{
		cout << "签名校验失败..." << endl;
		info.status = false;
	}
	else
	{
		cout << "签名校验成功..." << endl;
		info.status = true;
		// 准备数据
		info.seckeyID = 123;	// 需要数据库
		string myKey = getRandKey(L16);
		cout << "对称加密的秘钥-明文: " << myKey << endl;
		// 使用客户端传递过来的公钥加密
		// 对称加密的秘钥的密文
		info.data = crypto.rsaPubKeyEncrypt(myKey);

		// 将生成的信息的秘钥信息写入数据库中
		SecKeyInfo keyInfo;
		int keyID = m_oci.getKeyID();
		strcpy(keyInfo.clientID, msg->clientid().data());
		strcpy(keyInfo.serverID, m_serverID.data());
		strcpy(keyInfo.seckey, myKey.data());
		keyInfo.status = 1;
		keyInfo.seckeyID = keyID;
		int r = m_oci.writeSecKey(&keyInfo);
		cout << "写秘钥信息: " << r << endl;
		if (r)
		{
			m_oci.updataKeyID(keyID + 1);
		}

		// 将新的秘钥信息写入到共享内存中
		m_shm->shmWrite(&keyInfo);
	}

	// 序列化数据, 并发送给客户端
	// 准备回复数据, 需要先初始化回复的数据

	/*
	message RespondMsg
	{
		bool status = 1;
		int32 seckeyID = 2;
		bytes clientID = 3;
		bytes serverID = 4;
		bytes data = 5;
	}
	*/
	// 序列化
	CodecFactory* factory = new RespondFactory(&info);
	Codec* c = factory->createCodec();
	string resMsg = c->encodeMsg();
	
	// 释放资源
	delete c;
	delete factory;

	return resMsg;
}

string ServerOP::getRandKey(KeyLen len)
{
	srand(time(NULL));
	// 小写字母, 大写字母, 数字, 特殊字符
	// 特殊字符
	char *buf = "~!@#$%^&*()_+=-}{[]";
	char cc;
	string ret = string();
	int flag = 0;
	for (int i = 0; i < len; ++i)
	{
		flag = rand() % 4;	// 随机指定是那种类型的字符
		switch (flag)
		{
		case 0:	// 小写字母
			cc = rand() % 26 + 'a';
			break;
		case 1:	// 大写字母
			cc = rand() % 26 + 'A';
			break;
		case 2:	// 数字
			cc = rand() % 10 + '0';
			break;
		case 3:	// 特殊字符
			cc = buf[rand() % strlen(buf)];
			break;
		}
		ret.append(1, cc);
	}
	return ret;
}

void * hardwork(void * arg)
{
	sleep(1);
	// 当前线程的ID
	pthread_t tid = pthread_self();
	ServerOP* sop = (ServerOP*)arg;
	// 得到了通信的对象
	TcpSocket* tcp = sop->m_list[tid];
	// 接收数据
	string cliMsg = tcp->recvMsg();
	// 将接收的数据解码
	CodecFactory* factory = new RequestFactory(cliMsg);
	Codec* c = factory->createCodec();
	// 解码
	RequestMsg * reqMsg = (RequestMsg *)c->decodeMsg();
	// 根据cmdtype判断要处理的动作

	string data = string();

	cout << "cmdtype: " << reqMsg->cmdtype() << endl;
	cout << "public key: " << reqMsg->data() << endl;
	cout << "sign data: " << reqMsg->sign() << endl;
	switch (reqMsg->cmdtype())
	{
	case 1:
		// 秘钥的协商
		data = sop->secKeyConsult(reqMsg);
		break;
	case 2:
		break;
	case 3:
		break;
	}

	// 服务器序列化的数据发送给客户端
	tcp->sendMsg(data);

	// 释放资源
	tcp->disConnect();
	delete c;
	delete factory;
	// 将tcp键值对从map中删除
	sop->m_list.erase(tid);
	delete tcp;

	return nullptr;
}
