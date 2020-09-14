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
	printf("�����봮�ںţ���COM1\n");
	hCom = InitUSART(ModbusRTUWData.TimeOuts);
	while (1){
		SpaceIsTrue();//���ո�������Ͳ�ѯ����
		//���Ͳ�ѯ����
		int length = ModbusRTUQueryMessage(WriteBUF, &ModbusRTUWData);//������ѯ����
		bool a = ComWrite(hCom, WriteBUF, length);//��������
		if (a == false){//�ж��Ƿ�������ʧ��
			hCom = InputCOM(hCom, &ModbusRTUWData);//���¿���COM�˿�
		}
		else if (ModbusRTUWData.ID != 0){//�ж��Ƿ��ǹ㲥���Դ��ж��費��Ҫ��ȡ����ֵ
			unsigned char ReadBuf[N];//���ڶ�ȡ����
			memset(ReadBuf, 0, N);//����ϴν��յ�������
			int ReSize = 0;//�洢ʵ�ʶ�ȡ�����ֽ���
			bool b = ComRead(hCom, ReadBuf, ReSize);//�˿ںš��洢���顢�����ֽ���//�����ж�	
			if (b) {
				SlaveShow(&ModbusRTUWData, ReSize, WriteBUF, ReadBuf);//��������ʾ��Ӧ����
				ModbusRTUWData.flag = 0;//��0
			}
			else{
				ModbusRTUWData.flag = 1;
				printf("�ȴ�������Ӧ���ĳ�ʱ\n");
			}
			printf("\n"); printf("\n");//����
		}
		DataReelect(&ModbusRTUWData);//�ж��Ƿ�����������ʲ���
	}
	CloseHandle(hCom);//�رվ��
	system("pause");
	return 0;
}