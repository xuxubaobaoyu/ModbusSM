#include <stdio.h>  
#include <stdlib.h>  
#include <iostream>
#include <winsock2.h> /*socketͨ�ţ�ϵͳͷ�ļ�*/
#include "Modbus_One_TCP_Slave.h"
#include "Modbus_TCP_Init.h"
using namespace std;
#define		N		300//��������ĳ���

int main()
{
	ModbusTCPSlave TCPSlave;//�����ʼ���ṹ��
	/*----------------------------------------------------------*/
	/*������01�Ĵ���ȡ��������0F��д�������*/
	TCPSlave.Local_01_Address[0] = { 0x00 };
	TCPSlave.Local_01_Address[1] = { 0x00 };

	/*������03�Ĵ���ȡ��������10��д�������*/
	TCPSlave.Local_03_Address[1] = { 0 };
	/*----------------------------------------------------------*/
	//��ʼ������
	ModbusTCPParameterInit(&TCPSlave);
	unsigned int Listen_Client[2] = { 0 };//Listen_Client[0]�洢���׽���, Listen_Client[1]�洢�������ȴ��ͻ���
	//��ʼ��TCP����
	Modbus_TCP_Init(TCPSlave.port, Listen_Client);
	while (1)
	{
		// �ӿͻ��˽������� 
		char buff_char[N];
		int nRecv = recv(Listen_Client[1], buff_char, N, 0);
		if (nRecv <= 0)
		{
			printf(" �ͻ��˶Ͽ�����\n");
			closesocket(Listen_Client[1]);//�ر�ͬ�ͻ��˵�����
			Listen_Client[1] = Modbus_TCP_Accept(Listen_Client[0]);//�����ȴ��ͻ�������
		}
		else if (nRecv == SOCKET_ERROR)
		{
			printf(" �������\n");
			break;
		}
		else if (nRecv > 0)
		{
			unsigned char buff_unchar[N];
			memset(buff_unchar, -1 ,N);
			printf(" ���յ����ݣ�\n");
			for (int i = 0; i < nRecv; i++)
			{
				buff_unchar[i] = buff_char[i];//���з���ת��Ϊ�޷���
				printf("%02X  ", buff_unchar[i]);
			}
			printf("\n");
			int buf = Modbus_One_TCP_Slave(buff_unchar, &TCPSlave, nRecv);
			//printf("%d ", TCPSlave.Local_0F_Address[0]);
			//ת��
			for (int i = 0; i < buf; i++)
				buff_char[i] = buff_unchar[i];
			// ��ͻ��˷������� 
			send(Listen_Client[1], buff_char, buf, 0);
		}
	}
	// �رռ����׽���
	closesocket(Listen_Client[0]);
	system("pause");
	return 0;
}