#include "ModbusRTUQueryMessage.h"

//�������ܣ�ʵ��CRCУ��
unsigned long crc16(unsigned char *p, unsigned int len)
{
	unsigned long wcrc = 0XFFFF;//16λcrc�Ĵ���Ԥ��
	unsigned char temp;
	unsigned int i = 0, j = 0;//����
	for (i = 0; i < len; i++)//ѭ������ÿ������
	{
		temp = *p & 0X00FF;//����λ������crc�Ĵ������
		p++;//ָ���ַ���ӣ�ָ���¸�����
		wcrc ^= temp;//�����ݴ���crc�Ĵ���
		for (j = 0; j < 8; j++)//ѭ���������ݵ�
		{
			if (wcrc & 0X0001)//�ж����Ƴ����ǲ���1�������1�������ʽ�������
			{
				wcrc >>= 1;//�Ƚ���������һλ
				wcrc ^= 0XA001;//������Ķ���ʽ�������
			}
			else//�������1����ֱ���Ƴ�
			{
				wcrc >>= 1;//ֱ���Ƴ�
			}
		}
	}
	return wcrc;
}
//�������ܣ�ʵ�ֶԲ�ѯ����ǰ6��Ԫ�صı�д
static void AddressQuantity(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	//���豸��ַ
	WriteBUF[0] = ModbusRTUQuery->ID;
	//������
	WriteBUF[1] = ModbusRTUQuery->Function;
	//��ʼ��ַ
	WriteBUF[3] = ModbusRTUQuery->Address;//��λ
	WriteBUF[2] = ModbusRTUQuery->Address >>8 ;//��λ	
	//�Ĵ�����
	WriteBUF[5] = ModbusRTUQuery->RegisterQuantity;//��λ
	WriteBUF[4] = ModbusRTUQuery->RegisterQuantity >> 8;//��λ
	return;
}

//�������ܣ�ʵ��01��03��Ĳ�ѯ���Ĺ���
static unsigned int ModbusFun01and03(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	AddressQuantity(WriteBUF, ModbusRTUQuery);
	unsigned long buf = crc16(WriteBUF, 6);//����CRC
	//CRCУ����
	WriteBUF[6] = buf;//CRC�ĵͰ�λ
	WriteBUF[7] = buf >> 8;//CRC�ĸ߰�λ
	return 8;
}

//�������ܣ�ʵ��0F��Ĳ�ѯ���Ĺ���
static unsigned int ModbusFun0F(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	AddressQuantity(WriteBUF, ModbusRTUQuery);
	//�ֽ���
	WriteBUF[6] = ModbusRTUQuery->RegisterQuantity / 8;
	if (ModbusRTUQuery->RegisterQuantity % 8 >= 1) WriteBUF[6]++;
	//�������
	int i, j;
	for (i = 7, j = 0; i < (WriteBUF[6] + 7); i++, j++)
		WriteBUF[i] = ModbusRTUQuery->Data_0F[j];
	//CRCУ����
	unsigned long buf = crc16(WriteBUF, WriteBUF[6] + 7);//����CRC
	//CRCУ����
	WriteBUF[WriteBUF[6] + 7] = buf;//CRC�ĵͰ�λ
	WriteBUF[WriteBUF[6] + 8] = buf >> 8;//CRC�ĸ߰�λ
	return WriteBUF[6] + 9;
}

//�������ܣ�ʵ��10��Ĳ�ѯ���Ĺ���
static unsigned int ModbusFun10(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	AddressQuantity(WriteBUF, ModbusRTUQuery);
	//�ֽ���
	WriteBUF[6] = ModbusRTUQuery->RegisterQuantity * 2;
	//�������
	int i, j;
	for (i = 7, j = 0; i < WriteBUF[6] + 7; i += 2, j++)
	{
		WriteBUF[i + 1] = ModbusRTUQuery->Data_10[j];//��λ
		WriteBUF[i] = ModbusRTUQuery->Data_10[j] >> 8;//��λ
	}
	//CRCУ����
	unsigned long buf = crc16(WriteBUF, WriteBUF[6] + 7);//����CRC
	//CRCУ����
	WriteBUF[WriteBUF[6] + 7] = buf;//CRC�ĵͰ�λ
	WriteBUF[WriteBUF[6] + 8] = buf >> 8;//CRC�ĸ߰�λ
	return WriteBUF[6] + 9;
}
//�������ܣ�ʵ�ֶԲ�ѯ���ĵĹ���
//���������豸��ַ�������롢��ʼ��ַ���Ĵ�������0F�����������
unsigned int ModbusRTUQueryMessage(unsigned char* WriteBUF, ModbusRTU* ModbusRTUQuery)
{
	switch (ModbusRTUQuery->Function)
	{
	case(0x01) ://������01
	case(0x03) ://������03
		return ModbusFun01and03(WriteBUF, ModbusRTUQuery);
	case(0x0F) ://������0F
		return ModbusFun0F(WriteBUF, ModbusRTUQuery);
	case(0x10) ://������10
		return ModbusFun10(WriteBUF, ModbusRTUQuery);
	default:
		break;
	}
	return 0;
}


