#include "ClientOP.h"
#include <json/json.h>
#include "RequestFactory.h"
#include "RespondFactory.h"
#include "RsaCrypto.h"
#include <fstream>
#include <sstream>
#include "TcpSocket.h"
#include "Hash.h"

using namespace Json;
using namespace std;

ClientOP::ClientOP(string fileName)
{
	// 读参数对应的json的配置文件
	// 1. 使用流对象将文件打开
	ifstream ifs(fileName);
	Reader r;
	Value root;
	r.parse(ifs, root);
	// 2. 通过root将数据取出
	m_ip = root["IP"].asString();
	m_port = root["Port"].asInt();
	m_clientID = root["ClientID"].asString();
	m_serverID = root["ServerID"].asString();
	string key = root["ShmKey"].asString();

	// 实例化共享内存对象
	m_shm = new SecKeyShm(key, 1);
	// 初始化共享内存
	m_shm->shmInit();
}


ClientOP::~ClientOP()
{
	//m_shm->delShm();
	//delete m_shm;
}

void ClientOP::secKeyConsult()
{
	// 1. 准备数据 -> 初始化要发送的数据
	/*
	message RequestMsg
	{
		int32 cmdType = 1;
		bytes clientID = 2; // "hello.pem"
		bytes serverID = 3;
		bytes sign = 4;
		bytes data = 5;
	}
	*/
	// 1. 创建工厂对象
	RequestInfo reqInfo;
	reqInfo.clientID = m_clientID;
	reqInfo.serverID = m_serverID;
	reqInfo.cmd = 1;	// 密钥协商
	// 创建rsa对象生成密钥对
	RsaCrypto crypto;
	crypto.generateRsakey(1024);	// 秘钥长度: 单位bit
	// 将公钥信息从磁盘中读出
	ifstream ifs("public.pem");
#if 0
	ifs.seekg(ios::end);
	int len = ifs.tellg();
	ifs.seekg(ios::beg);
	ifs.read(bu, filesize);
#else
	stringstream sss;
	sss << ifs.rdbuf();
	// 取出数据
	string data = sss.str();
#endif

	// 读出的公钥文件中的数据
	cout << "公钥信息: " << data << endl;

	reqInfo.data = data;	// rsa公钥字符串
	Hash md5(HashType::T_MD5);
	md5.addData(data);
	reqInfo.sign = crypto.rsaSign(md5.result());	// 对共哟啊字符串签名
	CodecFactory* factory = new RequestFactory(&reqInfo);
	// 2. 通过工厂对象的工厂函数创建对应的编解码对象
	Codec* code = factory->createCodec();
	// 3. 最终得到了编码之后的数据
	string encStr = code->encodeMsg();
	delete code;
	delete factory;

	// 4. 网络通信
	// 连接服务器
	TcpSocket* tcp = new TcpSocket;
	int ret = tcp->connectToHost(m_ip, m_port);
	if (ret != 0)
	{
		cout << "连接服务器失败..." << endl;
		delete tcp;
		return;
	}
	// 发送序列化之后的数据给服务器
	tcp->sendMsg(encStr);
	// 等待服务器回复数据
	string sMsg = tcp->recvMsg();

	// 5. 反序列化服务器回复的数据
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
	// 创建解码的工厂类
	factory = new RespondFactory(sMsg);
	code = factory->createCodec();
	RespondMsg* resMsg = (RespondMsg*)code->decodeMsg();
	// 判断服务器回复的状态
	if (resMsg->status() != true)
	{
		cout << "服务器处理秘钥协商请求失败..." << endl;
		tcp->disConnect();
		delete tcp;
		delete factory;
		delete code;
		return;
	}

	// 需要使用私钥解密
	cout << "接收的加密的秘钥: " << resMsg->data() << endl;
	string key = crypto.rsaPriKeyDecrypt(resMsg->data());
	cout << "对称加密的秘钥: " << key << endl;

	// 将协商得到的秘钥写入到共享内存中
	NodeSHMInfo shmInfo;
	strcpy(shmInfo.clientID, m_clientID.data());
	strcpy(shmInfo.serverID, m_serverID.data());
	strcpy(shmInfo.seckey, key.data());
	shmInfo.status = 1;
	shmInfo.seckeyID = resMsg->seckeyid();
	m_shm->shmWrite(&shmInfo);

	// 释放资源
	// delete close
	delete code;
	delete factory;
	// 断开和服务器的连接
	tcp->disConnect();
	delete tcp;
}
