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
	printf("请输入串口号，例COM1\n");
	hCom = InitUSART(ModbusRTUWData.TimeOuts);
	while (1){
		SpaceIsTrue();//按空格继续发送查询报文
		//发送查询报文
		int length = ModbusRTUQueryMessage(WriteBUF, &ModbusRTUWData);//构建查询报文
		bool a = ComWrite(hCom, WriteBUF, length);//发送数据
		if (a == false){//判断是否发送数据失败
			hCom = InputCOM(hCom, &ModbusRTUWData);//重新开启COM端口
		}
		else if (ModbusRTUWData.ID != 0){//判断是否是广播，以此判断需不需要读取返回值
			unsigned char ReadBuf[N];//用于读取缓冲
			memset(ReadBuf, 0, N);//清空上次接收到的数据
			int ReSize = 0;//存储实际读取到的字节数
			bool b = ComRead(hCom, ReadBuf, ReSize);//端口号、存储数组、访问字节数//用于判断	
			if (b) {
				SlaveShow(&ModbusRTUWData, ReSize, WriteBUF, ReadBuf);//解析与显示响应报文
				ModbusRTUWData.flag = 0;//清0
			}
			else{
				ModbusRTUWData.flag = 1;
				printf("等待接收响应报文超时\n");
			}
			printf("\n"); printf("\n");//换行
		}
		DataReelect(&ModbusRTUWData);//判断是否重新输入访问参数
	}
	CloseHandle(hCom);//关闭句柄
	system("pause");
	return 0;
}