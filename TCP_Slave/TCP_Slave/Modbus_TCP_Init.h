#ifndef __MODBUS_TCP_INIT_H
#define __MODBUS_TCP_INIT_H

#include <stdio.h>  
#include <stdlib.h>  
#include <winsock2.h> /*socketͨ�ţ�ϵͳͷ�ļ�*/

//�ÿ��Ӧws2_32.dll���ṩ�˶������������API��֧�֣���ʹ�����е�API��
//��Ӧ�ý�ws2_32.lib���빤�̣�������Ҫ��̬����ws2_32.dll����
#pragma comment(lib, "WS2_32.lib")  // ���ӵ�WS2_32.lib 

void Modbus_TCP_Init(short int prot, unsigned int *buf);
unsigned int Modbus_TCP_Accept(unsigned int SListen);
void Modbus_TCP_Close(unsigned int Listen, unsigned int Client);
#endif /*__MODBUS_TCP_INIT_H*/