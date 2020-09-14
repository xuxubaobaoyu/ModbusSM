#ifndef _MODBUS_RTU_INIT_H
#define _MODBUS_RTU_INIT_H

#include <iostream>
#include <Windows.h>
#include <process.h>
#include "ModbusRTUQueryMessage.h"
using namespace std;


HANDLE InitUSART(DWORD Delay);
bool ComRead(HANDLE hCom, LPBYTE buf, int &len);
bool ComWrite(HANDLE hCom, LPBYTE buf, int &len);
HANDLE InputCOM(void* hCom, ModbusRTUQuery* Delay);
#endif	/*_MODBUS_RTU_INIT_H*/