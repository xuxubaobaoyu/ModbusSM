#include <winsock2.h> /*socket通信，系统头文件*/
#include <stdlib.h>  
#include <conio.h>  
#include <stdio.h>  




class CInitSock
{
public:
	CInitSock(BYTE minorVer = 2, BYTE majorVer = 2)
	{
		// 初始化WS2_32.dll 
		WSADATA wsaData;
		WORD sockVersion = MAKEWORD(minorVer, majorVer);
		if (::WSAStartup(sockVersion, &wsaData) != 0)
		{
			exit(0);
		}
	}
	~CInitSock()
	{
		::WSACleanup();
	}
};