#ifndef __MODBUS_ONE_TCP_SLAVE_H
#define __MODBUS_ONE_TCP_SLAVE_H

#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>
using namespace std;


typedef struct ModbusTCPSlave
{
	int port;//网络端口
	int ID;//从设备ID
	int Address;//从设备起始地址
	int Quantity;//从设备寄存器或线圈数量
	unsigned char Local_01_Address[2600];//从设备保持寄存器，用于01功能码的情况
	short Local_03_Address[10002];//从设备，用于03功能码的情况
}ModbusTCPSlave;

int Modbus_One_TCP_Slave(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv);

#endif /*__MODBUS_ONE_TCP_SLAVE_H*/