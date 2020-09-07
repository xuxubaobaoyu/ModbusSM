#include "ModbusRTUResponseMessage.h"

//函数功能：计算从机返回的数据长度
unsigned int ReadBufLength(ModbusRTUQuery* Length)
{
	//若设备ID为0,即广播就不用读取
	if (Length->ID == 0)
		return 0;
	//计算01码的响应报文的长度
	if (Length->Function == 1)
	{
		unsigned int count = Length->RegisterQuantity / 8;
		if (Length->RegisterQuantity % 8) count++;
		return count + 5;//数据域字节数+3（地址、功能码、字节数本身占的字节）+2字节的CRC码
	}
	//计算03码的响应报文的长度
	else if (Length->Function == 3)
	{
		unsigned int count = Length->RegisterQuantity * 2;
		return count + 5;//数据域字节数+3（地址、功能码、字节数本身占的字节）+2字节的CRC码
	}
	//计算0F码和10的响应报文的长度
	else if (Length->Function == 15 || Length->Function == 16)
		return 8;
	return 0;
}
//函数功能：对异常码，具体错误进行显示
static void ErrorCode(int Code)
{
	switch (Code)
	{
	case 0x01://非法功能码
		printf("从站不支持此功能码\n");
		return;
	case 0x02://非法数据地址
		printf("指定的数据地址在从站设备中不存在\n");
		return;
	case 0x03://非法数据值
		printf("指定的数据超过范围或者不允许使用\n");
		return;
	case 0x04://从站设备故障
		printf("从站设备处理响应的过程中，出现未知错误\n");
		return;
	default:
		break;
	}
}
//函数功能：实现对功能码01和03的解析与判断
static int ModbusRTURead_01and03(unsigned char* WriteBUF, unsigned char* ReadBuf, ModbusRTUQuery* FUN)
{
	int Num = 0;//用于判断响应报文的数据域字节数是否正确
	if (FUN->Function == 1)//01码的寄存器数量计算
	{
		Num = FUN->RegisterQuantity / 8;
		if (FUN->RegisterQuantity % 8) Num++;
	}
	else if (FUN->Function == 3)//03码的寄存器数量计算
		Num = FUN->RegisterQuantity * 2;

	if (WriteBUF[1] == ReadBuf[1] && Num == ReadBuf[2])//先判断功能码和查询报文期望的数据域字节数是否匹配
	{
		unsigned long CRC = crc16(ReadBuf, Num + 3);//再进行CRC校验
		//CRC低位  高位
		if (CRC % 256 == ReadBuf[Num + 3] && CRC / 256 == ReadBuf[Num + 4])
			return Num + 3;
		else
		{
			printf("响应报文CRC校验未通过错误\n");
			return 0;
		}
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//判断是否是异常码
	{
		unsigned long CRC = crc16(ReadBuf, 3);//进行CRC校验
		//CRC低位  高位
		if (CRC % 256 == ReadBuf[3] && CRC / 256 == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//对错误进行显示
			//对读取到的数据进行显示
			return 3;
		}
		else
		{
			printf("响应报文CRC校验未通过错误\n");
			return 0;
		}
	}
	printf("响应报文的功能码或数据域字节数错误\n");
	return 0;
}
//函数功能：实现对功能码0F和10的解析与判断
static int ModbusRTURead_0Fand10(unsigned char* WriteBUF, unsigned char* ReadBuf, ModbusRTUQuery* FUN)
{
	if (WriteBUF[1] == ReadBuf[1])//先判断功能码是否正确
	{
		unsigned long CRC = crc16(ReadBuf, 6);//进行CRC校验
		//CRC低位  高位
		if (CRC % 256 == ReadBuf[6] && CRC / 256 == ReadBuf[7])
			return 8;
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//判断是否是异常码
	{
		unsigned long CRC = crc16(ReadBuf, 3);//进行CRC校验
		//CRC低位  高位
		if (CRC % 256 == ReadBuf[3] && CRC / 256 == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//对错误进行显示
			return 8;
		}
		else
		{
			printf("响应报文CRC校验未通过错误\n");
			return 0;
		}
	}
	printf("响应报文的功能码错误\n");
	return 0;
}
//函数功能：解析响应报文
//参数：查询报文、接收报文，
//返回值用于对ReadBuf显示的长度指定
int DecomposeMessage(unsigned char* WriteBUF,unsigned char* ReadBuf, ModbusRTUQuery* FUN)
{
	if (WriteBUF[0] != ReadBuf[0])
	{
		printf("响应报文的设备ID错误\n");
		return 0;
	}
	switch (FUN->Function)//根据查询报文的功能码调用不同的函数
	{
	case(0x01) :		
	case(0x03) :
			   return ModbusRTURead_01and03(WriteBUF, ReadBuf, FUN);
	case(0x0F) :
	case(0x10) :
			   return ModbusRTURead_0Fand10(WriteBUF, ReadBuf, FUN);
	default:
		break;
	}
	return 0;
}