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

//�������ܣ��Գ�ʼ����ֵ�����ж�ת��
//����ֵ��-1�����������������ȷ
int ModbusInit(char* InitNum)
{
	string duqu = InitNum;
	int a = 0;
	for (int j = 0; duqu[j] != '\0'; j++)//�ų���123 123�������ַ���
	{
		if (duqu[j] != ' '&&a == 0)
			a++;
		if (duqu[j] == ' '&&a == 1)
			a++;
		if (duqu[j] != ' '&&a == 2)
			return -1;
	}

	int sub = duqu.rfind(" ");//���ҵ���һ�����ǿո���ַ�
	sub++;//�ո����һ���ַ��Ͳ����ǿո�
	int i, num = 0, num1 = 0;
	for (i = sub; duqu[i] != '\0'; i++)
	{
		if (duqu[i] <= '9'&&duqu[i] >= '0' || duqu[sub]== '-')
		{
			num++;
		}
	}
	if (num > 6)//����λ�����󣬷�ֹint�治��
		return -1;
	if ((i - sub) == num)
	{
		if (atoi(InitNum) > 65535) return -1;
		return atoi(InitNum);
	}
	else
		return -1;//���󷵻�ֵΪ-1
}
//�������ܣ���ʾ��ʼ����
static void ModbusRTUDataInitDefault(ModbusRTU* ModbusRTUData)
{
	printf("===========================================================================================\n");
	printf("��ͨ���������Modbus RTUģʽ��վģʽ\n");
	printf("����Ĭ���������£�\n");
	printf("�˿ںţ�COM1\n");
	printf("�����ʣ�9600 Baud\n");
	printf("����λ��8λ\n");
	printf("��żУ��λ����У��\n");
	printf("ֹͣλ��1λ\n");
	printf("===========================================================================================\n");
	printf("\n");//����
	return;
}
//�������ܣ���ʼ��ͨ�ų�ʱʱ��
static void ModbusRTUDataInitTimeout(ModbusRTU* ModbusRTUData)
{
	char str[300];//���ڶ�ȡ����̨����
	printf(">----------------------------------ͨ�ų�ʱʱ��------------------------------------<\n");
	printf("ͨ�ų�ʱʱ�����÷�ΧΪ50--100000����ʱ��λΪ1ms��\n");
	printf("������ͨ�ų�ʱʱ�䣺\n");
	int TimeOuts = 0;
	gets(str);
	TimeOuts = ModbusInit(str);
	while (TimeOuts == -1 || TimeOuts > 100000 || TimeOuts< 50)
	{
		printf("�����������������\n");
		gets(str);
		TimeOuts = ModbusInit(str);
	}
	ModbusRTUData->TimeOuts = TimeOuts;
	printf("ͨ�ų�ʱʱ���������\n");
	return;
}
//�������ܣ���ʼ���豸ID
static void ModbusRTUDataInitID(ModbusRTU* ModbusRTUData)
{
	char str[300];//���ڶ�ȡ����̨����
	printf(">-----------------------------------�豸ID-----------------------------------------<\n");
	printf("�豸ID�����÷�ΧΪ0--247�������豸IDΪ0������㲥��\n");
	printf("�������豸ID��\n");
	int ID = 0;
	gets(str);
	ID = ModbusInit(str);
	while (ID == -1 || ID > 247 || ID<0)
	{
		printf("�����������������\n");
		gets(str);
		ID = ModbusInit(str);
	}
	ModbusRTUData->ID = ID;
	printf("�豸ID�������\n");
	return;
}
//�������ܣ���ʼ��������
static void ModbusRTUDataInitFunCode(ModbusRTU* ModbusRTUData)
{
	char str[300];//���ڶ�ȡ����̨����
	printf(">-----------------------------------������-----------------------------------------<\n");
	printf("��ǰ������ֻ��1��3��15��16���ĸ���������ĸ���ѡ��1������\n");
	printf("�밴ʮ�������빦���룺\n");
	gets(str);
	while (1)
	{
		ModbusRTUData->Function = ModbusInit(str);
		while (ModbusRTUData->Function == -1 || ModbusRTUData->Function != 1 && \
			ModbusRTUData->Function != 3 && ModbusRTUData->Function != 15 && ModbusRTUData->Function != 16)
		{
			printf("�����������������\n");
			gets(str);
			ModbusRTUData->Function = ModbusInit(str);
		}
		if (ModbusRTUData->ID == 0 && (ModbusRTUData->Function == 1 || ModbusRTUData->Function == 3))
		{
			printf("��ǰ����Ĺ����벻֧�ֹ㲥������������\n");
			gets(str);
		}
		else break;
	}
	printf("�������������\n");
	return;
}
//�������ܣ���ʼ����ʼ��ַ
static void ModbusRTUDataInitAddress(ModbusRTU* ModbusRTUData)
{
	char str[300];//���ڶ�ȡ����̨����
	printf(">-----------------------------------��ʼ��ַ---------------------------------------<\n");
	printf("��ַ���뷶ΧΪ0--65535\n");
	printf("��������ʼ��ַ��\n");
	gets(str);
	ModbusRTUData->Address = ModbusInit(str);
	while (ModbusRTUData->Address == -1 || ModbusRTUData->Address < 0 || ModbusRTUData->Address > 65535)
	{
		printf("�����������������\n");
		gets(str);
		ModbusRTUData->Address = ModbusInit(str);
	}
	printf("��ʼ��ַ�������\n");
	//memset(str, 0, sizeof(str));//����ַ���
	return;
}
//�������ܣ���ʼ������
static void InitQuantity(ModbusRTU* ModbusRTUData, int Num)
{
	char str[300];//���ڶ�ȡ����̨����
	printf(">------------------------------------����------------------------------------------<\n");
	printf("���ʵ���Ȧ�Ĵ������������뷶ΧΪ1--%d\n",Num);
	printf("�������ȡ����ĵ�������\n");
	gets(str);
	ModbusRTUData->RegisterQuantity = ModbusInit(str);
	while (ModbusRTUData->RegisterQuantity == -1 || ModbusRTUData->RegisterQuantity < 1 \
		|| ModbusRTUData->RegisterQuantity > Num)
	{
		printf("�����������������\n");
		gets(str);
		ModbusRTUData->RegisterQuantity = ModbusInit(str);
	}
	printf("��ȡ����ĵ������������\n");
	printf(">----------------------------------------------------------------------------------<\n");
}
//�������ܣ����ݹ������ʼ������
static void ModbusRTUDataInitRegisterQuantity(ModbusRTU* ModbusRTUData)
{
	switch (ModbusRTUData->Function)
	{
	case(0x01) : InitQuantity(ModbusRTUData, 2000); break;//01
	case(0x0F) : InitQuantity(ModbusRTUData, 1968); break;//0F
	case(0x03) : InitQuantity(ModbusRTUData, 125); break;//03
	case(0x10) : InitQuantity(ModbusRTUData, 123); break;//10
	default:
		break;
	}
	return;
}
//�������ܣ���д�����Ȧ��Ĵ������г�ʼ��
static void ModbusRTUDataInit0F_10(ModbusRTU* ModbusRTUData)
{
	char str[300];//���ڶ�ȡ����̨����
	//�Թ�����0x0F������Ȧ
	if (ModbusRTUData->Function == 0x0F)//01 03 0F 10
	{
		printf(">------------------------------��Ȧ�ı������--------------------------------------<\n");
		int count = ModbusRTUData->RegisterQuantity / 8;
		if (ModbusRTUData->RegisterQuantity % 8 >= 1) count++;
		printf("������ %d �������޸ĵ���Ȧ�ı������\n", count);
		printf("�������뷶ΧΪ0--255���밴ʮ��������,�ո����\n");
		printf("���磺10 1 2 5 6 8 91 128\n");

		ModbusRTUData->Data_0F[N] = { 0 };//����
		char *p;//�����и�
		while (1)
		{
			int i = 0;
			gets(str);//��������
			p = strtok(str, " ");//��ȡ
			while (p)
			{
				if (ModbusInit(p) > 255 || ModbusInit(p) < 0 || i == count)
				{
					i--;
					break;//�������˳�
				}
				else ModbusRTUData->Data_0F[i] = ModbusInit(p);//��ֵ
				p = strtok(NULL, " ");
				i++;
			}
			if (i == count)	break;
			else printf("�����������������\n");
		}
		printf("��Ȧ�ı�������������\n");
		printf(">---------------------------------------------------------------------------------<\n");
	}
	//�Թ�����0x10��������
	else if (ModbusRTUData->Function == 0x10)
	{
		printf(">--------------------------------���ּĴ����ı������-----------------------------<\n");
		int count = ModbusRTUData->RegisterQuantity;
		printf("������ %d �������޸ĵı��ּĴ����ı������\n", count);
		printf("�������뷶ΧΪ-32768--32767���밴ʮ��������,�ո����\n");
		printf("���磺-10 1 2 5 6 8 91 32767\n");
		ModbusRTUData->Data_10[N] = { 0 };//����
		char *p;//�����и�
		while (1)
		{
			int i = 0;
			gets(str);//��������
			p = strtok(str, " ");//��ȡ
			while (p)
			{
				if (ModbusInit(p) > 32767 || ModbusInit(p) < -32768 || i == count)
				{
					i--;
					break;//�������˳�
				}
				else ModbusRTUData->Data_10[i] = ModbusInit(p);//��ֵ
				p = strtok(NULL, " ");
				i++;
			}
			if (i == count&&p == NULL) break;
			else printf("�����������������\n");
		}
		printf("���ּĴ����ı�������������\n");
		printf(">---------------------------------------------------------------------------------<\n");
	}
	return;
}
//�������ܣ���ͨ�Ž��г�ʼ��
void ModbusRTUDataInit(ModbusRTU* ModbusRTUData)
{
	ModbusRTUDataInitDefault(ModbusRTUData);//Ĭ�Ͻ���
	ModbusRTUDataInitTimeout(ModbusRTUData);//��ʱʱ��
	ModbusRTUDataInitID(ModbusRTUData);//���ʵĴ��豸ID
	ModbusRTUDataInitFunCode(ModbusRTUData);//������
	ModbusRTUDataInitAddress(ModbusRTUData);//��ʼ��ַ
	ModbusRTUDataInitRegisterQuantity(ModbusRTUData);//���ʼĴ�������Ȧ����
	ModbusRTUDataInit0F_10(ModbusRTUData);//�Դ�д�����Ȧ��Ĵ�������д��
	return;
}
//�������ܣ����ڿ���̨�ظ�ѡ��ID�������롢������
void DataReelect(ModbusRTU* ModbusRTUData)
{
	char str[300];//���ڶ�ȡ����̨����
	int Num = 0;
	printf("\n"); printf("\n"); printf("\n");
	printf(">---------------------------------------------------------------------------------<\n");
	printf("��ѡ�����±��ִ����Ӧ������\n");
	printf("1��������������ʵĴ��豸ID\n");
	printf("2���������빦����\n");
	printf("3������������ʼ��ַ\n");
	printf("4��������������ʵļĴ�������Ȧ��������ֵ\n");
	printf("5���˳�ѡ��\n");
	printf(">---------------------------------------------------------------------------------<\n");
	while (Num != 5)
	{
		gets(str);
		Num = ModbusInit(str);
		while (Num == -1 || Num != 1 && Num != 2 \
			&& Num != 3 && Num != 4 && Num != 5)
		{
			printf("�����������������\n");
			gets(str);
			Num = ModbusInit(str);
		}
		switch (Num)
		{
		case(1) :
			ModbusRTUDataInitID(ModbusRTUData); break;//���ʵĴ��豸ID
		case(2) :
			ModbusRTUDataInitFunCode(ModbusRTUData); break;//������
		case(3) :
			ModbusRTUDataInitAddress(ModbusRTUData); break;//��ʼ��ַ
		case(4) :
			ModbusRTUDataInitRegisterQuantity(ModbusRTUData);//���ʼĴ�������Ȧ����
			ModbusRTUDataInit0F_10(ModbusRTUData); //�Դ�д�����Ȧ��Ĵ�������д��
			break;
		case(5) : break;
		default:
			break;
		}
		if (Num != 5)
		{
			printf("\n");
			printf("�����ѡ����ִ����Ӧ������\n");
			printf("\n");
		}
	}
	return;
}
//�������ܣ��жϻس�
void SpaceIsTrue()
{
	printf("�밴�س����ͱ���\n");
	char ch;
	gets(&ch);
	while (ch != '\0')
	{
		printf("�����������������\n");
		gets(&ch);
	}
	return;

}