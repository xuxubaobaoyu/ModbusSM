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
	if (Code == 0x01){
		printf("��վ��֧�ִ˹�����\n");
		return;
	}
	else if (Code == 0x02){
		printf("ָ�������ݵ�ַ�ڴ�վ�豸�в�����\n");
		return;
	}
	printf("ָ�������ݳ�����Χ���߲�����ʹ��\n");		
	return;
}
//�������ܣ�ʵ�ֶԹ�����01��03�Ľ������ж�
static int ModbusRTURead_01and03(unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	int Num = 0;//�����ж���Ӧ���ĵ��������ֽ����Ƿ���ȷ
	int cc = WriteBUF[4] * 256 + WriteBUF[5];
	if (WriteBUF[1] == 1)//01��ļĴ�����������
	{
		Num = (cc+7) / 8;
	}
	else if (WriteBUF[1] == 3)//03��ļĴ�����������
		Num = cc * 2;
	if (WriteBUF[1] == ReadBuf[1] && Num == ReadBuf[2])//���жϹ�����Ͳ�ѯ�����������������ֽ����Ƿ�ƥ��
	{
		unsigned long CRC = crc16(ReadBuf, Num + 3);//�ٽ���CRCУ��
		//CRC��λ  ��λ
		if ((CRC % 256) == ReadBuf[Num + 3] && (CRC >> 8) == ReadBuf[Num + 4])
			return Num + 3;
		else
		{
			printf("��Ӧ����CRCУ��δͨ������01\n");
			return 0;
		}
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//�ж��Ƿ����쳣��
	{
		unsigned long CRC = crc16(ReadBuf, 3);//����CRCУ��
		//CRC��λ  ��λ
		if ((CRC % 256) == ReadBuf[3] && (CRC >> 8) == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//�Դ��������ʾ
			//�Զ�ȡ�������ݽ�����ʾ
			return 3;
		}
		else
		{
			printf("��Ӧ����CRCУ��δͨ������01\n");
			return 0;
		}
	}
	printf("��Ӧ���ĵĹ�������������ֽ�������\n");
	return 0;
}
//�������ܣ�ʵ�ֶԹ�����0F��10�Ľ������ж�
static int ModbusRTURead_0Fand10(unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	if (WriteBUF[1] == ReadBuf[1])//���жϹ������Ƿ���ȷ
	{
		unsigned long CRC = crc16(ReadBuf, 6);//����CRCУ��
		//CRC��λ  ��λ
		if ((CRC % 256) == ReadBuf[6] && (CRC >> 8) == ReadBuf[7]){
			if (WriteBUF[2] == ReadBuf[2] && WriteBUF[3] == ReadBuf[3] && WriteBUF[4] == ReadBuf[4] && \
				WriteBUF[5] == ReadBuf[5])//������ʼ��ַ������
				return 8;
			else
				return 0;
		}
		else{
			printf("��Ӧ����CRCУ��δͨ������10\n");
			return 0;
		}
	}
	else if (WriteBUF[1] + 0x80 == ReadBuf[1])//�ж��Ƿ����쳣��
	{
		unsigned long CRC = crc16(ReadBuf, 3);//����CRCУ��
		//CRC��λ  ��λ
		if ((CRC % 256) == ReadBuf[3] && (CRC >> 8) == ReadBuf[4])
		{
			ErrorCode(ReadBuf[2]);//�Դ��������ʾ
			return 8;
		}
		else
		{
			printf("��Ӧ����CRCУ��δͨ������10\n");
			return 0;
		}
	}
	printf("��Ӧ���ĵĹ��������\n");
	return 0;
}
//�������ܣ�������Ӧ����
//��������ѯ���ġ����ձ��ģ�
//����ֵ���ڶ�ReadBuf��ʾ�ĳ���ָ��
int DecomposeMessage(unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	if (WriteBUF[0] != ReadBuf[0])
	{
		printf("��Ӧ���ĵ��豸ID����\n");
		return 0;
	}
	switch (WriteBUF[1])//���ݲ�ѯ���ĵĹ�������ò�ͬ�ĺ���
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
//�������ܣ���ʾ01��03�����Ӧ����
void SlaveData(unsigned char* ReadBuf, ModbusRTUQuery* FUN, int Num)
{
	if (ReadBuf[1] == 0x01){
		printf("��ȡ�������ݣ�");
		for (int j = 3; j < Num; j++){
			printf("%02X ", ReadBuf[j]);
		}
		printf("\n");
	}
	else if (ReadBuf[1] == 0x03){
		printf("��ȡ�������ݣ�");
		for (int j = 3; j < Num; j += 2){
			printf("%02X ", ReadBuf[j] * 256 + ReadBuf[j + 1]);
		}
		printf("\n");
	}
	return;
}

//�������ܣ���������ʾ��Ӧ����
void SlaveShow(ModbusRTUQuery* SlaveS, int ReSize, unsigned char* WriteBUF, unsigned char* ReadBuf)
{
	int len = ReadBufLength(SlaveS);//���ݲ�ѯ���ļ���Ӧ�ö�ȡ���ٸ���Ӧ���ĵ�����
	unsigned char Rbuf[N] = { 0 };
	if (ReSize >= 5){
		/*******************************�쳣����***************************************/
		memset(Rbuf, 0, len + 1);//�����
		for (int i = ReSize - 3, k = 2; k >= 0; i--, k--){
			Rbuf[k] = ReadBuf[i];//ȡ��3�ֽ�����
		}
		unsigned long CRC = crc16(Rbuf, 3);//����CRCУ��
		//CRC��λ  ��λ
		if ((CRC % 256) == ReadBuf[ReSize - 2] && (CRC >> 8) == ReadBuf[ReSize - 1]){
			Rbuf[3] = ReadBuf[ReSize - 2];
			Rbuf[4] = ReadBuf[ReSize - 1];
			printf("��Ӧ�������£�\n");
			for (int i = 0; i < 5; i++){
				printf("%02X ", Rbuf[i]);
			}
			int Num = DecomposeMessage(WriteBUF, Rbuf);//������Ӧ����
			SlaveData(Rbuf, SlaveS, Num);//��ʾ01��03�����Ӧ����
			printf("\n"); printf("\n");//����
			return;
		}
		/*******************************��������***************************************/
		else if (ReSize >= len){//�ж�ʵ���ֽ��Ƿ�����ϣ�����ֽڳ�
			memset(Rbuf, 0, len + 1);//�����
			for (int i = ReSize - 3, k = len - 3; k >= 0; i--, k--){
				Rbuf[k] = ReadBuf[i];//ȡ��len���ֽ�����
			}
			unsigned long CRC = crc16(Rbuf, len - 2 );//����CRCУ��
			//CRC��λ  ��λ
			if ((CRC % 256) == ReadBuf[ReSize - 2] && (CRC >> 8) == ReadBuf[ReSize - 1]){
				Rbuf[len - 2] = ReadBuf[ReSize - 2];
				Rbuf[len - 1] = ReadBuf[ReSize - 1];
				printf("��Ӧ�������£�\n");
				for (int i = 0; i < len; i++){
					printf("%02X ", Rbuf[i]);
				}
				int Num = DecomposeMessage(WriteBUF, Rbuf);//������Ӧ����
				SlaveData(Rbuf, SlaveS, Num);//��ʾ01��03�����Ӧ����
				printf("\n"); printf("\n");//����
				return;
			}
		}
	}
	printf("����������ϣ���Ľ������ݲ�һ��\n\n");	
	return;
}