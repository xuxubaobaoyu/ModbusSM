#ifndef __MODBUSRTUQUERYMESSAGE_H	
#define __MODBUSRTUQUERYMESSAGE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

#define N	1204

//RTUģʽ���õ�������
typedef struct ModbusRTU
{
	unsigned long TimeOuts;//������ʱʱ��
	int ID;//���豸��ַ
	int Function;//������
	int Address;//��ʼ��ַ
	int RegisterQuantity;//�Ĵ�����
	unsigned char Data_0F[N];//0F�����������
	short Data_10[N];//01�����������
}ModbusRTUQuery;

//���豸��ַ�������롢��ʼ��ַ���Ĵ�������0F�����������,01�����������
unsigned int ModbusRTUQueryMessage(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery);

int ModbusInit(char* InitNum);//�Գ�ʼ����ֵ�����ж�ת��
void ModbusRTUDataInit(ModbusRTU* ModbusRTUData);//��ͨ�Ų������г�ʼ��
unsigned long crc16(unsigned char *p, unsigned int len);//CRCУ��
void DataReelect(ModbusRTU* ModbusRTUData);//���ڿ���̨�ظ�ѡ��ID�������롢������
void SpaceIsTrue();//�жϻس�
#endif /*__MODBUSRTUQUERYMESSAGE_H*/