#ifndef _MODBUS_RTU_INIT_H
#define _MODBUS_RTU_INIT_H

#include <iostream>
#include <Windows.h>
#include <process.h>

using namespace std;

HANDLE InitCOM(char* COM, DWORD Delay);
bool ComRead(HANDLE hCom, LPBYTE buf, int &len);
bool ComWrite(HANDLE hCom, LPBYTE buf, int &len);
#endif	/*_MODBUS_RTU_INIT_H*/