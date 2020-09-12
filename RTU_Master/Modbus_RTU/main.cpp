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
	if (hCom == INVALID_HANDLE_VALUE)
	{
		printf("���ڴ�ʧ��\n");
		system("pause");
		return 0;
	}
	while (1){
		SpaceIsTrue();
		//���Ͳ�ѯ����
		int length = ModbusRTUQueryMessage(WriteBUF, &ModbusRTUWData);
		bool a = ComWrite(hCom, WriteBUF, length);//��������
		if (a == false){
			printf("��������ʧ��\n");
			continue;
		}
		if (ModbusRTUWData.ID != 0){//�ж��Ƿ��ǹ㲥���Դ��ж��費��Ҫ��ȡ����ֵ
			unsigned char ReadBuf[N];//���ڶ�ȡ����
			memset(ReadBuf, 0, N);//����ϴν��յ�������
			
			int ReSize = 0;//�洢ʵ�ʶ�ȡ�����ֽ���
			bool b = false;
			b = ComRead(hCom, ReadBuf, ReSize);//�˿ںš��洢���顢�����ֽ���//�����ж�
			int len = ReadBufLength(&ModbusRTUWData);//���ݲ�ѯ���ļ���Ӧ�ö�ȡ���ٸ���Ӧ���ĵ�����
			if (b){
				for (int j = 0; j < ReSize; j++)
				{
					printf("��Ӧ�������£�\n");
					for (int i = 0; i < len; i++){

						printf("%02X ", ReadBuf[i]);
					}
						
					int Num = DecomposeMessage(WriteBUF, ReadBuf, &ModbusRTUWData);//������Ӧ����
					SlaveData(ReadBuf, &ModbusRTUWData, Num);//��ʾ01��03�����Ӧ����
					printf("\n"); printf("\n");//����
				}
			}
			else{
				printf("�ȴ�������Ӧ���ĳ�ʱ\n");
			}
			printf("\n");printf("\n");//����
		}
		DataReelect(&ModbusRTUWData);//�ж��Ƿ�����������ʲ���
	}
	CloseHandle(hCom);//�رվ��
	system("pause");
	return 0;
}