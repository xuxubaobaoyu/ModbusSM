#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <process.h>
#include "Modbus_RTU_Init.h"
#include "ModbusRTUQueryMessage.h"
#include "ModbusRTUResponseMessage.h"
using namespace std;
int main()
{
	volatile HANDLE hCom;
	unsigned char WriteBUF[N] = { 0 };//写缓冲
	ModbusRTUQuery ModbusRTUWData;//定义一个存放初始值的结构体
	ModbusRTUDataInit(&ModbusRTUWData);//进行初始化例如ID、Function等
	//初始化串口
	hCom = InitCOM("COM1", ModbusRTUWData.TimeOuts);//端口号、通信超时时间
	while (1)
	{
		SpaceIsTrue();
		//发送查询报文
		int length = ModbusRTUQueryMessage(WriteBUF, &ModbusRTUWData);
		ComWrite(hCom, WriteBUF, length);//发送数据
		if (ModbusRTUWData.ID != 0)//判断是否是广播，以此判断需不需要读取返回值
		{
			BYTE ReadBuf[300];//用于读取缓冲
			int len = ReadBufLength(&ModbusRTUWData);//先计算响应报文的长度,设备ID为0,即广播就不用读取
			bool b = false;
			b = ComRead(hCom, ReadBuf, len);//端口号、存储数组、访问字节数//用于判断
			if (b)
			{
				DecomposeMessage(WriteBUF, ReadBuf, &ModbusRTUWData);
				printf("响应报文如下：\n");
				for (int i = 0; i < len; i++)
					printf("%02X ", ReadBuf[i]);
			}
			else
			{
				printf("等待接收响应报文超时\n");
			}
			printf("\n");//换行
			printf("\n");//换行
			DataReelect(&ModbusRTUWData);//判断是否重新访问
		}
	}
	CloseHandle(hCom);//关闭句柄
	system("pause");
	return 0;
}