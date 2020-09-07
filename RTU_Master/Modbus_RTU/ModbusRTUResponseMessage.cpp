#include "ModbusRTUResponseMessage.h"

//�������ܣ�����ӻ����ص����ݳ���
unsigned int ReadBufLength(ModbusRTUQuery* Length)
{
	//���豸IDΪ0,���㲥�Ͳ��ö�ȡ
	if (Length->ID == 0)
		return 0;
	//����01�����Ӧ���ĵĳ���
	if (Length->Function == 1)
	{
		unsigned int count = Length->RegisterQuantity / 8;
		if (Length->RegisterQuantity % 8) count++;
		return count + 5;//�������ֽ���+3����ַ�������롢�ֽ�������ռ���ֽڣ�+2�ֽڵ�CRC��
	}
	//����03�����Ӧ���ĵĳ���
	else if (Length->Function == 3)
	{
		unsigned int count = Length->RegisterQuantity * 2;
		return count + 5;//�������ֽ���+3����ַ�������롢�ֽ�������ռ���ֽڣ�+2�ֽڵ�CRC��
	}
	//����0F���10����Ӧ���ĵĳ���
	else if (Length->Function == 15 || Length->Function == 16)
		return 8;
	return 0;
}
//�������ܣ����쳣�룬������������ʾ
static void ErrorCode(int Code)
{
	switch (Code)
	{
	case 0x01://�Ƿ�������
		printf("��վ��֧�ִ˹�����\n");
		return;
	case 0x02://�Ƿ����ݵ�ַ
		printf("ָ�������ݵ�ַ�ڴ�վ�豸�в�����\n");
		return;
	case 0x03://�Ƿ�����ֵ
		printf("ָ�������ݳ�����Χ���߲�����ʹ��\n");
		return;
	case 0x04://��վ�豸����
		printf("��վ�豸������Ӧ�Ĺ����У�����δ֪����\n");
		return;
	default:
		break;
	}
}
//�������ܣ�ʵ�ֶԹ�����01��03�Ľ������ж�
static int ModbusRTURead_01and03(unsigned char* WriteBUF, unsigned char* ReadBuf, ModbusRTUQuery* FUN)
{
	int Num = 0;//�����ж���Ӧ���ĵ��������ֽ����Ƿ���ȷ
	if (FUN->Function == 1)//01��ļĴ�����������
	{
		Num = FUN->RegisterQuantity / 8;
		if (FUN->RegisterQuantity % 8) Num++;
	}
	else if (FUN->Function == 3)//03��ļĴ�����������
		Num = FUN->RegisterQuantity * 2;

	if (WriteBUF[1] == ReadBuf[1] && Num == ReadBuf[2])//���жϹ�����Ͳ�ѯ�����������������ֽ����Ƿ�ƥ��
	{
		unsigned long CRC = crc16(ReadBuf, Num + 3);//�ٽ���CRCУ��
		//CRC��λ  ��λ
		if (CRC % 256 == ReadBuf[Num + 3] && CRC / 256 == ReadBuf[Num + 4])
			return Num + 3;
		else
		{
			printf("��Ӧ����CRCУ��δͨ������\n");
			return 0;
		}
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//�ж��Ƿ����쳣��
	{
		unsigned long CRC = crc16(ReadBuf, 3);//����CRCУ��
		//CRC��λ  ��λ
		if (CRC % 256 == ReadBuf[3] && CRC / 256 == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//�Դ��������ʾ
			//�Զ�ȡ�������ݽ�����ʾ
			return 3;
		}
		else
		{
			printf("��Ӧ����CRCУ��δͨ������\n");
			return 0;
		}
	}
	printf("��Ӧ���ĵĹ�������������ֽ�������\n");
	return 0;
}
//�������ܣ�ʵ�ֶԹ�����0F��10�Ľ������ж�
static int ModbusRTURead_0Fand10(unsigned char* WriteBUF, unsigned char* ReadBuf, ModbusRTUQuery* FUN)
{
	if (WriteBUF[1] == ReadBuf[1])//���жϹ������Ƿ���ȷ
	{
		unsigned long CRC = crc16(ReadBuf, 6);//����CRCУ��
		//CRC��λ  ��λ
		if (CRC % 256 == ReadBuf[6] && CRC / 256 == ReadBuf[7])
			return 8;
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//�ж��Ƿ����쳣��
	{
		unsigned long CRC = crc16(ReadBuf, 3);//����CRCУ��
		//CRC��λ  ��λ
		if (CRC % 256 == ReadBuf[3] && CRC / 256 == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//�Դ��������ʾ
			return 8;
		}
		else
		{
			printf("��Ӧ����CRCУ��δͨ������\n");
			return 0;
		}
	}
	printf("��Ӧ���ĵĹ��������\n");
	return 0;
}
//�������ܣ�������Ӧ����
//��������ѯ���ġ����ձ��ģ�
//����ֵ���ڶ�ReadBuf��ʾ�ĳ���ָ��
int DecomposeMessage(unsigned char* WriteBUF,unsigned char* ReadBuf, ModbusRTUQuery* FUN)
{
	if (WriteBUF[0] != ReadBuf[0])
	{
		printf("��Ӧ���ĵ��豸ID����\n");
		return 0;
	}
	switch (FUN->Function)//���ݲ�ѯ���ĵĹ�������ò�ͬ�ĺ���
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