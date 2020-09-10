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
	hCom = InitCOM("COM3", ModbusRTUWData.TimeOuts);//端口号、通信超时时间
	if (hCom == INVALID_HANDLE_VALUE)
	{
		printf("串口打开失败\n");
		system("pause");
		return 0;
	}
	while (1){
		SpaceIsTrue();
		//发送查询报文
		int length = ModbusRTUQueryMessage(WriteBUF, &ModbusRTUWData);
		ComWrite(hCom, WriteBUF, length);//发送数据
		if (ModbusRTUWData.ID != 0){//判断是否是广播，以此判断需不需要读取返回值
			unsigned char ReadBuf[N];//用于读取缓冲
			memset(ReadBuf, 0, N);
			int len = N;
			bool b = false;
			b = ComRead(hCom, ReadBuf, len);//端口号、存储数组、访问字节数//用于判断
			if (b){
				printf("响应报文如下：\n");
				for (int i = 0; i < len; i++)
					printf("%02X ", ReadBuf[i]);
				int Num = DecomposeMessage(WriteBUF, ReadBuf, &ModbusRTUWData);//解析响应报文
				SlaveData(ReadBuf, &ModbusRTUWData, Num);//显示01和03吗的响应数据
			}
			else{
				printf("等待接收响应报文超时\n");
			}
			printf("\n");printf("\n");//换行
			DataReelect(&ModbusRTUWData);//判断是否重新输入访问参数
		}
	}
	CloseHandle(hCom);//关闭句柄
	system("pause");
	return 0;
}