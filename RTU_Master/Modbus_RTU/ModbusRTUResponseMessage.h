#ifndef __MODBUSRTURESPONSEMESSAGE_H
#define __MODBUSRTURESPONSEMESSAGE_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "ModbusRTUQueryMessage.h"
using namespace std;


//�������ܣ�����ӻ����ص����ݳ���
unsigned int ReadBufLength(ModbusRTUQuery* Length);
int DecomposeMessage(unsigned char* WriteBUF, unsigned char* ReadBuf, ModbusRTUQuery* FUN);
void SlaveData(unsigned char* ReadBuf, ModbusRTUQuery* FUN, int Num);
#endif/*__MODBUSRTURESPONSEMESSAGE_H*/