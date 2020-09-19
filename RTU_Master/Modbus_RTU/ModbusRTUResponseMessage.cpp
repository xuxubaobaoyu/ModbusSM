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
	if (Code == 0x01){
		printf("从站不支持此功能码\n");
		return;
	}
	else if (Code == 0x02){
		printf("指定的数据地址在从站设备中不存在\n");
		return;
	}
	printf("指定的数据超过范围或者不允许使用\n");		
	return;
}
//函数功能：实现对功能码01和03的解析与判断
static int ModbusRTURead_01and03(unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	int Num = 0;//用于判断响应报文的数据域字节数是否正确
	int cc = WriteBUF[4] * 256 + WriteBUF[5];
	if (WriteBUF[1] == 1)//01码的寄存器数量计算
	{
		Num = (cc+7) / 8;
	}
	else if (WriteBUF[1] == 3)//03码的寄存器数量计算
		Num = cc * 2;
	if (WriteBUF[1] == ReadBuf[1] && Num == ReadBuf[2])//先判断功能码和查询报文期望的数据域字节数是否匹配
	{
		unsigned long CRC = crc16(ReadBuf, Num + 3);//再进行CRC校验
		//CRC低位  高位
		if ((CRC % 256) == ReadBuf[Num + 3] && (CRC >> 8) == ReadBuf[Num + 4])
			return Num + 3;
		else
		{
			printf("响应报文CRC校验未通过错误01\n");
			return 0;
		}
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//判断是否是异常码
	{
		unsigned long CRC = crc16(ReadBuf, 3);//进行CRC校验
		//CRC低位  高位
		if ((CRC % 256) == ReadBuf[3] && (CRC >> 8) == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//对错误进行显示
			//对读取到的数据进行显示
			return 3;
		}
		else
		{
			printf("响应报文CRC校验未通过错误01\n");
			return 0;
		}
	}
	printf("响应报文的功能码或数据域字节数错误\n");
	return 0;
}
//函数功能：实现对功能码0F和10的解析与判断
static int ModbusRTURead_0Fand10(unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	if (WriteBUF[1] == ReadBuf[1])//先判断功能码是否正确
	{
		unsigned long CRC = crc16(ReadBuf, 6);//进行CRC校验
		//CRC低位  高位
		if ((CRC % 256) == ReadBuf[6] && (CRC >> 8) == ReadBuf[7]){
			if (WriteBUF[2] == ReadBuf[2] && WriteBUF[3] == ReadBuf[3] && WriteBUF[4] == ReadBuf[4] && \
				WriteBUF[5] == ReadBuf[5])//再判起始地址，数量
				return 8;
			else
				return 0;
		}
		else{
			printf("响应报文CRC校验未通过错误10\n");
			return 0;
		}
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//判断是否是异常码
	{
		unsigned long CRC = crc16(ReadBuf, 3);//进行CRC校验
		//CRC低位  高位
		if ((CRC % 256) == ReadBuf[3] && (CRC >> 8) == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//对错误进行显示
			return 8;
		}
		else
		{
			printf("响应报文CRC校验未通过错误10\n");
			return 0;
		}
	}
	printf("响应报文的功能码错误\n");
	return 0;
}
//函数功能：解析响应报文
//参数：查询报文、接收报文，
//返回值用于对ReadBuf显示的长度指定
int DecomposeMessage(unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	if (WriteBUF[0] != ReadBuf[0])
	{
		printf("响应报文的设备ID错误\n");
		return 0;
	}
	switch (WriteBUF[1])//根据查询报文的功能码调用不同的函数
	{
	case(0x01) :
	case(0x03) :
			   return ModbusRTURead_01and03(WriteBUF, ReadBuf);
	case(0x0F) :
	case(0x10) :
			   return ModbusRTURead_0Fand10(WriteBUF, ReadBuf);
	default:
		break;
	}
}
//函数功能：显示01和03吗的响应数据
void SlaveData(unsigned char* ReadBuf, ModbusRTUQuery* FUN, int Num)
{
	if (ReadBuf[1] == 0x01){
		printf("读取到的数据：");
		for (int j = 3; j < Num; j++){
			printf("%02X ", ReadBuf[j]);
		}
		printf("\n");
	}
	else if (ReadBuf[1] == 0x03){
		printf("读取到的数据：");
		for (int j = 3; j < Num; j += 2){
			printf("%02X ", ReadBuf[j] * 256 + ReadBuf[j + 1]);
		}
		printf("\n");
	}
	return;
}

//函数功能：解析与显示响应报文
void SlaveShow(ModbusRTUQuery* SlaveS, int ReSize, unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	int len = ReadBufLength(SlaveS);//根据查询报文计算应该读取多少个响应报文的数据
	unsigned char Rbuf[N] = { 0 };
	if (ReSize >= 5){
		/*******************************异常报文***************************************/
		memset(Rbuf, 0, len + 1);//先清空
		for (int i = ReSize - 3, k = 2; k >= 0; i--, k--){
			Rbuf[k] = ReadBuf[i];//取出3字节数据
		}
		unsigned long CRC = crc16(Rbuf, 3);//进行CRC校验
		//CRC低位  高位
		if ((CRC % 256) == ReadBuf[ReSize - 2] && (CRC >> 8) == ReadBuf[ReSize - 1]){
			Rbuf[3] = ReadBuf[ReSize - 2];
			Rbuf[4] = ReadBuf[ReSize - 1];
			printf("响应报文如下：\n");
			for (int i = 0; i < 5; i++){
				printf("%02X ", Rbuf[i]);
			}
			int Num = DecomposeMessage(WriteBUF, Rbuf);//解析响应报文
			SlaveData(Rbuf, SlaveS, Num);//显示01和03码的响应数据
			printf("\n"); printf("\n");//换行
			return;
		}
		/*******************************正常报文***************************************/
		else if (ReSize >= len){//判断实际字节是否有我希望的字节长
			memset(Rbuf, 0, len + 1);//先清空
			for (int i = ReSize - 3, k = len - 3; k >= 0; i--, k--){
				Rbuf[k] = ReadBuf[i];//取出len个字节数据
			}
			unsigned long CRC = crc16(Rbuf, len - 2 );//进行CRC校验
			//CRC低位  高位
			if ((CRC % 256) == ReadBuf[ReSize - 2] && (CRC >> 8) == ReadBuf[ReSize - 1]){
				Rbuf[len - 2] = ReadBuf[ReSize - 2];
				Rbuf[len - 1] = ReadBuf[ReSize - 1];
				printf("响应报文如下：\n");
				for (int i = 0; i < len; i++){
					printf("%02X ", Rbuf[i]);
				}
				int Num = DecomposeMessage(WriteBUF, Rbuf);//解析响应报文
				SlaveData(Rbuf, SlaveS, Num);//显示01和03码的响应数据
				printf("\n"); printf("\n");//换行
				return;
			}
		}
	}
	printf("接收数据与希望的接收数据不一样\n\n");	
	return;
}