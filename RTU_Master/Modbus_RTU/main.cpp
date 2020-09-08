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
	unsigned char WriteBUF[N] = { 0 };//д����
	ModbusRTUQuery ModbusRTUWData;//����һ����ų�ʼֵ�Ľṹ��
	ModbusRTUDataInit(&ModbusRTUWData);//���г�ʼ������ID��Function��
	//��ʼ������
	hCom = InitCOM("COM1", ModbusRTUWData.TimeOuts);//�˿ںš�ͨ�ų�ʱʱ��
	while (1){
		SpaceIsTrue();
		//���Ͳ�ѯ����
		int length = ModbusRTUQueryMessage(WriteBUF, &ModbusRTUWData);
		ComWrite(hCom, WriteBUF, length);//��������
		if (ModbusRTUWData.ID != 0){//�ж��Ƿ��ǹ㲥���Դ��ж��費��Ҫ��ȡ����ֵ
			BYTE ReadBuf[300];//���ڶ�ȡ����
			int len = ReadBufLength(&ModbusRTUWData);//�ȼ�����Ӧ���ĵĳ���,�豸IDΪ0,���㲥�Ͳ��ö�ȡ
			bool b = false;
			b = ComRead(hCom, ReadBuf, len);//�˿ںš��洢���顢�����ֽ���//�����ж�
			if (b){
				DecomposeMessage(WriteBUF, ReadBuf, &ModbusRTUWData);
				printf("��Ӧ�������£�\n");
				for (int i = 0; i < len; i++)
					printf("%02X ", ReadBuf[i]);
			}
			else{
				printf("�ȴ�������Ӧ���ĳ�ʱ\n");
			}
			printf("\n");printf("\n");//����
			DataReelect(&ModbusRTUWData);//�ж��Ƿ�����������ʲ���
		}
	}
	CloseHandle(hCom);//�رվ��
	system("pause");
	return 0;
}