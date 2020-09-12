#include "Modbus_RTU_Init.h"

//无论那种操作方式，一般都通过四个步骤来完成：
//
//（1） 打开串口
//（2） 配置串口
//（3） 读写串口
//（4） 关闭串口


static HANDLE InitCOM(char* COM, DWORD Delay)
{
	HANDLE hCom = INVALID_HANDLE_VALUE;//全局变量，串口句柄
	hCom = CreateFile(COM, //COM口
		GENERIC_READ | GENERIC_WRITE, //允许读和写
		0, //因为串口不能共享，该参数必须置为0
		NULL,
		OPEN_EXISTING,//打开而不是创建
		0, //同步方式
		NULL);
	if (INVALID_HANDLE_VALUE == hCom)
	{
		return INVALID_HANDLE_VALUE;
	}
	SetupComm(hCom, 4096, 4096);//设置缓存

	//配置串口具体参数
	DCB dcb;
	GetCommState(hCom, &dcb);//设置串口
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = CBR_9600;//波特率
	dcb.StopBits = ONESTOPBIT;//停止位
	dcb.ByteSize = 8;//有效位
	dcb.Parity = NOPARITY;//无奇偶校验
	SetCommState(hCom, &dcb);

	PurgeComm(hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);//清空发送接收缓存

	COMMTIMEOUTS ct;//设定读超时
	//COMMTIMEOUTS结构的成员都以毫秒为单位
	ct.ReadIntervalTimeout = 20;//读间隔超时
	//间隔超时和总超时的设置是不相关的
	ct.ReadTotalTimeoutConstant = Delay;//读时间常量
	ct.ReadTotalTimeoutMultiplier = 0;//读时间系数

	ct.WriteTotalTimeoutMultiplier = 1;//写时间常量
	ct.WriteTotalTimeoutConstant = 1;//写时间系数

	SetCommTimeouts(hCom, &ct);//设置发送接收超时

	return hCom;
}

bool ComRead(HANDLE hCom, LPBYTE buf, int &len)
{
	DWORD ReadSize = 0;
	BOOL rtn = FALSE;
	bool flag = false;

	rtn = ReadFile(hCom, (char*)buf, 1024, &ReadSize, NULL);
	len = ReadSize;
	//printf("len = %d\n", len);
	if (rtn == TRUE && len != 0)
	{
		flag = true;
	}
	//PurgeComm函数清空串口的接收缓冲区
	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	return flag;
}

bool ComWrite(HANDLE hCom, LPBYTE buf, int &len)
{
	//这里清缓存的目的是防止我还没给你发送指令你就开始给我发送数据了，这个接收的数据不能要
	PurgeComm(hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
	//PurgeComm(hCom, PURGE_TXCLEAR | PURGE_TXABORT);
	BOOL rtn = FALSE;
	DWORD WriteSize = 0;

	rtn = WriteFile(hCom, buf, len, &WriteSize, NULL);
	len = WriteSize;

	return rtn != FALSE;
}

//函数功能：初始化串口
HANDLE InitUSART(DWORD Delay)
{
	volatile HANDLE hCom;
	char COM[300] = { 0 };
	gets(COM);
	hCom = InitCOM(COM, Delay);//端口号、通信超时时间
	while (hCom == INVALID_HANDLE_VALUE){
		printf("串口打开失败，请重新输入串口\n");
		gets(COM);
		hCom = InitCOM(COM, Delay);//端口号、通信超时时间
	}
	printf("===========================================================================================\n");
	return hCom;
}

//函数功能：重新重新输入端口号
void InputCOM(void* hCom, ModbusRTUQuery* Delay)
{
	printf("发送数据失败\n");
	CloseHandle(hCom);//关闭句柄
	printf("请重新输入COM\n");
	hCom = InitUSART(Delay->TimeOuts);
	return;
}