#ifndef __MODBUSRTUQUERYMESSAGE_H	
#define __MODBUSRTUQUERYMESSAGE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define N	1204

//RTU模式会用到的数据
typedef struct ModbusRTU
{
	unsigned long TimeOuts;//设置延时时间
	int ID;//从设备地址
	int Function;//功能码
	int Address;//起始地址
	int RegisterQuantity;//寄存器数
	unsigned char Data_0F[N];//0F变更数据数组
	short Data_10[N];//01变更数据数组
	int flag;//用于判断显示
}ModbusRTUQuery;

//从设备地址、功能码、起始地址、寄存器数，0F变更数据数组,01变更数据数组
unsigned int ModbusRTUQueryMessage(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery);

int ModbusInit(char* InitNum);//对初始化数值进行判断转换
void ModbusRTUDataInit(ModbusRTU* ModbusRTUData);//对通信参数进行初始化
unsigned long crc16(unsigned char *p, unsigned int len);//CRC校验
void DataReelect(ModbusRTU* ModbusRTUData);//用于控制台重复选择ID、功能码、数量等
void SpaceIsTrue();//判断回车
#endif /*__MODBUSRTUQUERYMESSAGE_H*/