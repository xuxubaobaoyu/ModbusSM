#include "ModbusRTUQueryMessage.h"

//函数功能：实现CRC校验
unsigned long crc16(unsigned char *p, unsigned int len)
{
	unsigned long wcrc = 0XFFFF;//16位crc寄存器预置
	unsigned char temp;
	unsigned int i = 0, j = 0;//计数
	for (i = 0; i < len; i++)//循环计算每个数据
	{
		temp = *p & 0X00FF;//将八位数据与crc寄存器亦或
		p++;//指针地址增加，指向下个数据
		wcrc ^= temp;//将数据存入crc寄存器
		for (j = 0; j < 8; j++)//循环计算数据的
		{
			if (wcrc & 0X0001)//判断右移出的是不是1，如果是1则与多项式进行异或。
			{
				wcrc >>= 1;//先将数据右移一位
				wcrc ^= 0XA001;//与上面的多项式进行异或
			}
			else//如果不是1，则直接移出
			{
				wcrc >>= 1;//直接移出
			}
		}
	}
	return wcrc;
}
//函数功能：实现对查询报文前6个元素的编写
static void AddressQuantity(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	//从设备地址
	WriteBUF[0] = ModbusRTUQuery->ID;
	//功能码
	WriteBUF[1] = ModbusRTUQuery->Function;
	//起始地址
	WriteBUF[3] = ModbusRTUQuery->Address;//低位
	WriteBUF[2] = ModbusRTUQuery->Address >>8 ;//高位	
	//寄存器数
	WriteBUF[5] = ModbusRTUQuery->RegisterQuantity;//低位
	WriteBUF[4] = ModbusRTUQuery->RegisterQuantity >> 8;//高位
	return;
}

//函数功能：实现01、03码的查询报文构建
static unsigned int ModbusFun01and03(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	AddressQuantity(WriteBUF, ModbusRTUQuery);
	unsigned long buf = crc16(WriteBUF, 6);//计算CRC
	//CRC校验码
	WriteBUF[6] = buf;//CRC的低八位
	WriteBUF[7] = buf >> 8;//CRC的高八位
	return 8;
}

//函数功能：实现0F码的查询报文构建
static unsigned int ModbusFun0F(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	AddressQuantity(WriteBUF, ModbusRTUQuery);
	//字节数
	WriteBUF[6] = ModbusRTUQuery->RegisterQuantity / 8;
	if (ModbusRTUQuery->RegisterQuantity % 8 >= 1) WriteBUF[6]++;
	//变更数据
	int i, j;
	for (i = 7, j = 0; i < (WriteBUF[6] + 7); i++, j++)
		WriteBUF[i] = ModbusRTUQuery->Data_0F[j];
	//CRC校验码
	unsigned long buf = crc16(WriteBUF, WriteBUF[6] + 7);//计算CRC
	//CRC校验码
	WriteBUF[WriteBUF[6] + 7] = buf;//CRC的低八位
	WriteBUF[WriteBUF[6] + 8] = buf >> 8;//CRC的高八位
	return WriteBUF[6] + 9;
}

//函数功能：实现10码的查询报文构建
static unsigned int ModbusFun10(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	AddressQuantity(WriteBUF, ModbusRTUQuery);
	//字节数
	WriteBUF[6] = ModbusRTUQuery->RegisterQuantity * 2;
	//变更数据
	int i, j;
	for (i = 7, j = 0; i < WriteBUF[6] + 7; i += 2, j++)
	{
		WriteBUF[i + 1] = ModbusRTUQuery->Data_10[j];//低位
		WriteBUF[i] = ModbusRTUQuery->Data_10[j] >> 8;//高位
	}
	//CRC校验码
	unsigned long buf = crc16(WriteBUF, WriteBUF[6] + 7);//计算CRC
	//CRC校验码
	WriteBUF[WriteBUF[6] + 7] = buf;//CRC的低八位
	WriteBUF[WriteBUF[6] + 8] = buf >> 8;//CRC的高八位
	return WriteBUF[6] + 9;
}
//函数功能：实现对查询报文的构建
//参数：从设备地址、功能码、起始地址、寄存器数，0F变更数据数组
unsigned int ModbusRTUQueryMessage(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	switch (ModbusRTUQuery->Function)
	{
	case(0x01) ://功能码01
	case(0x03) ://功能码03
		return ModbusFun01and03(WriteBUF, ModbusRTUQuery);
	case(0x0F) ://功能码0F
		return ModbusFun0F(WriteBUF, ModbusRTUQuery);
	case(0x10) ://功能码10
		return ModbusFun10(WriteBUF, ModbusRTUQuery);
	default:
		break;
	}
	return 0;
}


