#ifndef __MODBUS_TCP_INIT_H
#define __MODBUS_TCP_INIT_H

#include <stdio.h>  
#include <stdlib.h>  
#include <winsock2.h> /*socket通信，系统头文件*/

//该库对应ws2_32.dll，提供了对以下网络相关API的支持，若使用其中的API，
//则应该将ws2_32.lib加入工程（否则需要动态载入ws2_32.dll）。
#pragma comment(lib, "WS2_32.lib")  // 链接到WS2_32.lib 
using namespace std;
void Modbus_TCP_Init(short int prot, unsigned int *buf);
unsigned int Modbus_TCP_Accept(unsigned int SListen);
void Modbus_TCP_Close(unsigned int Listen, unsigned int Client);
#endif /*__MODBUS_TCP_INIT_H*/