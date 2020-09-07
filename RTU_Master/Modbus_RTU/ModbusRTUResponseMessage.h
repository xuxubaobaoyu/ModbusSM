#ifndef __MODBUSRTURESPONSEMESSAGE_H
#define __MODBUSRTURESPONSEMESSAGE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "ModbusRTUQueryMessage.h"
using namespace std;


//函数功能：计算从机返回的数据长度
unsigned int ReadBufLength(ModbusRTUQuery* Length);
int DecomposeMessage(unsigned char* WriteBUF, unsigned char* ReadBuf, ModbusRTUQuery* FUN);
#endif/*__MODBUSRTURESPONSEMESSAGE_H*/