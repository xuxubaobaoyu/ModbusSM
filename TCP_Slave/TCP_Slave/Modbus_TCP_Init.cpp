#include "Modbus_TCP_Init.h"


//对WS2_32.dll 初始化
static void Modbus_TCP_Init_WS2_32_dll()
{
	// 初始化WS2_32.dll 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(0);//正常运行程序并退出程序
	}
	return;
}
//创建套接字
static unsigned int Modbus_TCP_Create_Socket()
{
	// 创建套节字 
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//IPv4，SOCK_STREAM，TCP传输协议
	//用来指定套接字使用的地址格式，通常使用AF_INET，即IPv4
	//指定套接字的类型，若是SOCK_DGRAM，则用的是udp不可靠传输，流式socket (SOCK_STREAM)流式套接字提供可靠的、面向连接的通信流;它使用TCP协议，从而保证了数据传输的正确性和顺序性
	//配合type参数使用，指定使用的协议类型（当指定套接字类型后，可以设置为0，因为默认为UDP或TCP）
	if (sListen == INVALID_SOCKET)//判断是否成功创建套节字，是0就创建失败
	{
		return 0;
	}
	return sListen;
}
//绑定套接字
static bool Modbus_TCP_Bind(unsigned int SListen, short int prot)
{
	// 填充sockaddr_in结构 ,是个结构体
	/* struct sockaddr_in {
	short sin_family;  //地址族（指定地址格式） ，设为AF_INET
	u_short	sin_port; //端口号
	struct in_addr sin_addr; //IP地址
	char sin_zero[8]; //空子节，设为空
	} */
	sockaddr_in sin;
	sin.sin_family = AF_INET;//sin_family代表协议族，AF_INET代表TCP/IP协议

	//将整型变量从主机字节顺序转变成网络字节顺序
	sin.sin_port = htons(prot);  //1024 ~ 49151：普通用户注册的端口号
	sin.sin_addr.S_un.S_addr = INADDR_ANY;//默认
	// 绑定这个套节字到一个本地地址 
	//bind（上面创建的socket的套接字，
	if (bind(SListen, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR)//判断是否赋值成功，返回-1就没成功
	{
		return false;
	}
	return true;
}

//阻塞等待客户端连接
unsigned int Modbus_TCP_Accept(unsigned int SListen)
{
	
	// 循环接受客户的连接请求 
	sockaddr_in remoteAddr;//用于存储客户端的协议、端口号、IP
	int nAddrLen = sizeof(remoteAddr);
	unsigned int sClient = 0;

	char szText[] = " TCP Server Demo! \r\n";
	while (sClient == 0)
	{
		// 接受一个新连接 
		//（(SOCKADDR*)&remoteAddr）一个指向sockaddr_in结构的指针，用于获取对方地址
		//如果accpet成功，那么其返回值是由内核自动生成的一个全新的描述字，代表与返回客户的TCP连接。
		//第一个参数为服务器的socket描述字，第二个参数为指向struct sockaddr *的指针，用于返回客户端的协议地址，第三个参数为协议地址的长度
		sClient = accept(SListen, (SOCKADDR*)&remoteAddr, &nAddrLen);

		if (sClient == INVALID_SOCKET)//若接收失败，sClient为0
		{
			return 0;
		}
	}
//#ifdef Debug
	printf("接受到一个客户端连接,IP地址：%s \r\n", inet_ntoa(remoteAddr.sin_addr));//inet_ntoa将网络地址转换成“.”点隔的字符串格式。
//#endif
	return sClient;
}

//对TCP连接初始化
void Modbus_TCP_Init(short int prot, unsigned int *Listen_Client)
{
	Modbus_TCP_Init_WS2_32_dll();//对WS2_32.dll 初始化
	unsigned int Socket_Listen = Modbus_TCP_Create_Socket();//创建套接字
	if (Socket_Listen == 0)//判断套接字是否创建成功
	{
		printf("Failed socket() \n");
		return;
	}
	//绑定套接字
	if (Modbus_TCP_Bind(Socket_Listen, prot) == false)//判断绑定套接字是否成功
	{
		printf("Failed bind() \n");
		return;
	}
	//2指的是，监听队列中允许保持的尚未处理的最大连接数
	//socket()函数创建的socket默认是一个主动类型的，listen函数将socket变为被动类型的，等待客户的连接请求。
	if (listen(Socket_Listen, 2) == SOCKET_ERROR)//监听
	{
		printf("Failed listen() \n");
		return ;
	}
	printf("等待客户端连接。。。\n");

	//返回套接字用于发送和接收
	Listen_Client[0] = Socket_Listen;//
	Listen_Client[1] = Modbus_TCP_Accept(Socket_Listen);//阻塞等待客户端连接
	if (Listen_Client[1] == 0)
	{
		printf("Failed accept()");
		return;
	}		
	return ;
}