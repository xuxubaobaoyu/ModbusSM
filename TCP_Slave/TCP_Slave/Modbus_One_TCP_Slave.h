#ifndef __MODBUS_ONE_TCP_SLAVE_H
#define __MODBUS_ONE_TCP_SLAVE_H

#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>
using namespace std;


typedef struct ModbusTCPSlave
{
	int port;//����˿�
	int ID;//���豸ID
	int Address;//���豸��ʼ��ַ
	int Quantity;//���豸�Ĵ�������Ȧ����
	unsigned char Local_01_Address[2600];//���豸���ּĴ���������01����������
	short Local_03_Address[10000];//���豸������03����������
}ModbusTCPSlave;

int Modbus_One_TCP_Slave(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv);
int ParameterIsTrue(char* InitNum);
void ModbusTCPParameterInit(ModbusTCPSlave* Parameter);
#endif /*__MODBUS_ONE_TCP_SLAVE_H*/