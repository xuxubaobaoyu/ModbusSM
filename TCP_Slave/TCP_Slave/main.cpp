#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>
#include <winsock2.h> /*socket通信，系统头文件*/
#include "Modbus_One_TCP_Slave.h"
#include "Modbus_TCP_Init.h"
#include "TCPParameterInit.h"
using namespace std;
#define		N		300//接收数组的长度

int main()
{
	ModbusTCPSlave TCPSlave;//定义初始化结构体
	/*----------------------------------------------------------*/
	/*功能码01的待读取、功能码0F待写入的数组*/
	TCPSlave.Local_01_Address[0] = { 0x80 };
	TCPSlave.Local_01_Address[1] = { 0x00 };

	/*功能码03的待读取、功能码10待写入的数组*/
	TCPSlave.Local_03_Address[1] = { 6 };
	/*----------------------------------------------------------*/
	//初始化参数
	ParameterInit(&TCPSlave);
	unsigned int Listen_Client[2] = { 0 };//Listen_Client[0]存储的套接字, Listen_Client[1]存储的阻塞等待客户端
	//初始化TCP连接
	Modbus_TCP_Init(TCPSlave.port, Listen_Client);
	while (1)
	{
		// 从客户端接收数据 
		char buff_char[N];
		int nRecv = recv(Listen_Client[1], buff_char, N, 0);
		if (nRecv <= 0){
			printf(" 客户端断开连接\n");
			closesocket(Listen_Client[1]);//关闭同客户端的连接
			Listen_Client[1] = Modbus_TCP_Accept(Listen_Client[0]);//阻塞等待客户端连接
		}
		else if (nRecv == SOCKET_ERROR){
			printf(" 网络错误\n");
			break;
		}
		else if (nRecv > 0){
			unsigned char buff_unchar[N];
			memset(buff_unchar, -1 ,N);
			printf(" 接收到数据：\n");
			for (int i = 0; i < nRecv; i++){
				buff_unchar[i] = buff_char[i];//将有符号转化为无符号
				printf("%02X  ", buff_unchar[i]);
			}
			printf("\n");
			int buf = Modbus_One_TCP_Slave(buff_unchar, &TCPSlave, nRecv);
			//转换
			for (int i = 0; i < buf; i++)
				buff_char[i] = buff_unchar[i];
			// 向客户端发送数据 
			send(Listen_Client[1], buff_char, buf, 0);
		}
	}
	// 关闭监听套节字
	closesocket(Listen_Client[0]);
	system("pause");
	return 0;
}