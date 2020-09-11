#include "Modbus_One_TCP_Slave.h"

//�������ܣ��Խ��յĵ�ַ��������������
static void AddressQuantuty(unsigned char* TCP_Slave, int* SumAddress, int* SQuantity)
{
	*SumAddress = TCP_Slave[8] * 256;//��ַ��λ
	*SumAddress += TCP_Slave[9];//��ַ��λ

	*SQuantity = TCP_Slave[10] * 256;//���ʵ�������λ
	*SQuantity += TCP_Slave[11];//���ʵ�������λ
	return;
}
//�������ܣ������쳣��
static void CodeNum(unsigned char* TCP_Slave, int Num)
{
	TCP_Slave[5] = 0x03;//�����ֽڳ���

	TCP_Slave[7] += 0x80;//����0x80
	if (Num == 1){
		TCP_Slave[8] = 0x01;//˵����վ�豸��֧�����������
	}
	else if (Num == 2){
		TCP_Slave[8] = 0x02;//˵����ַ�ڴ��豸�в�����
	}
	else if (Num == 3){
		TCP_Slave[8] = 0x03;//˵�����ʵ��ǷǷ�����ֵ
	}
	return;
}
//�������ܣ�ʵ�ֶ��쳣��03�ļ��
static int AbnormalCode03(unsigned char* TCP_Slave, int Code, int sum)
{
	if (sum == TCP_Slave[5])//�ж�MBAP����ͷ�Ƿ���ȷ
	{
		int SumAddress = 0;
		int SQuantity = 0;
		int Num = 0;//���ڴ洢��Ҫд����ֽ���
		AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//�������ѯ������Ҫд����ֽ���
		if (Code == 0x0F)//15
		{
			Num = SQuantity / 8;
			if (SQuantity % 8) Num++;
		}
		if (Code == 0x10)//16
		{
			Num = SQuantity * 2;
		}
		if (Num != TCP_Slave[12])//�жϲ�ѯ������Ҫ��д����ֽ������ѯ����
		{
			printf("�Ƿ�����ֵ\n");
			CodeNum(TCP_Slave, 3);
			return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
		}
		else
			return 0;
	}
	printf("MBAP����ͷ�ֽ�������\n");
	return 1;
}
//�������ܣ���MBAP����ͷ�͹���������ж��Ƿ���ȷ
static int MBAPCodeIsTrue(unsigned char* TCP_Slave, short int QRecv, unsigned char ID)
{
	if (QRecv != 12 && (TCP_Slave[7] == 0x01 || TCP_Slave[7] == 0x03))//�жϳ����Ƿ���ȷ,0x01��0x03��ѯ���ĳ���һ����12
	{
		printf("֡��ʽ����1\n");
		return 1;
	}
	if (QRecv < 12){
		printf("֡��ʽ����1\n");
		return 1;
	}
	if (TCP_Slave[2] != 0 || TCP_Slave[3] != 0)//�ж�MBAP����ͷ�е�Э���ʶ���Ƿ���ȷ
	{
		printf("MBAP����ͷ��Э���ʶ������\n");
		return 1;
	}
	if (TCP_Slave[6] != ID &&TCP_Slave[6] != 0)
	{
		printf("���ҵ��豸���뱾�����豸�Ų�ƥ��\n");
		return 1;
	}
	int sum = TCP_Slave[4] * 256;//�����жϽ��յ��������Ƿ���ȷ
	sum += TCP_Slave[5];
	//����ѯ������01 03������
	if (TCP_Slave[7] == 0x01 || TCP_Slave[7] == 0x03)//��������Ϊ01��03ʱMBAP�е��ֽڳ���һ����6
	{
		if (sum == 6 && QRecv == 12) return 0;//����ѯ������ȷ����MBAP�ֽڳ���һ����6���ܳ���һ����12
		else
		{
			printf("��ѯ���ĵ�֡��ʽ����2\n");
			return 1;
		}
	}
	//��ѯ����0F������
	else if (TCP_Slave[7] == 0x0F)
	{
		return AbnormalCode03(TCP_Slave, 0x0F, sum);
	}
	//��ѯ����10������
	else if (TCP_Slave[7] == 0x10)
	{
		return AbnormalCode03(TCP_Slave, 0x10, sum);
	}

	printf("�����벻ƥ��\n");
	CodeNum(TCP_Slave, 1);//˵����վ�豸��֧�����������
	return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
}

//�������ܣ��жϷ��ʵ���ʼ��ַ�������Ƿ�ͱ���ƥ��
//���������ص�ַ������
//����ֵ��9��0                                            
static int AddressIsTrue(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni)
{
	int SumAddress = 0;//���жϱ��ص�ַ�Ͳ�ѯ���ĵĵ�ַ
	int SQuantity = 0;//���жϱ��ؼĴ��������Ͳ�ѯ���ĵļĴ�������
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);

	int add = SumAddress - ParameterIni->Address;
	if (SumAddress < ParameterIni->Address)//�жϵ�ַ�Ƿ��ڴ��豸�в�����
	{
		printf("��ַ���豸�в�����\n");
		CodeNum(TCP_Slave, 2);//˵����ַ�ڴ��豸�в�����
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}

	if ((add + SQuantity) > ParameterIni->Quantity)//�ж��ǲ��ǷǷ�����ֵ����ָ�������ݳ�����Χ���߲�����ʹ��
	{
		printf("ָ�������ݳ�����Χ\n");
		CodeNum(TCP_Slave, 2);//˵����ַ�ڴ��豸�в�����
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	int sum = 9999;//���Է��ʵ���Ȧ�Ĵ���������ַ
	if (SQuantity == 0 || SumAddress > 9999)//����������Ϊ0���߷��ʵ�ַ����������
	{
		CodeNum(TCP_Slave, 3);//˵������ָ�������ݳ�����Χ
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	if (SQuantity > 2000 && TCP_Slave[7] == 1){//01
		CodeNum(TCP_Slave, 3);//˵������ָ�������ݳ�����Χ
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	else if (SQuantity > 1968 && TCP_Slave[7] == 15){//15
		CodeNum(TCP_Slave, 3);//˵������ָ�������ݳ�����Χ
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	else if (SQuantity > 125 && TCP_Slave[7] == 3){//03
		CodeNum(TCP_Slave, 3);//˵������ָ�������ݳ�����Χ
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	else if (SQuantity > 123 && TCP_Slave[7] == 16){//16
		CodeNum(TCP_Slave, 3);//˵������ָ�������ݳ�����Χ
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	return 0;
}

//�ÿ���̨��ŵ�ʱ����1���ֽ��������
//�������ܣ�ʵ�ֶ�01��Ĺ���				  
static int TCP_ID_01(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni)
{            /*����������01ʱ���ؿ���̨�����ַ��0--65535����������1--20000��*/
	//ID==0����Ϊ�㲥����������01��֧�ֹ㲥
	if (TCP_Slave[6] == 0)
	{
		printf("������01��֧�ֹ㲥\n");
		CodeNum(TCP_Slave, 1);//��վ�в�����ʹ��
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	else
	{
		//�жϲ�ѯ�����еĵ�ַ�������Ƿ�ͱ���ƥ��
		if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;

		//��Ϊ�ǲ��ÿ���̨����ģ����԰��ֽ�����ȡ
		int SumAddress = 0;//���ڲ��Ҵ洢���ҵ���ʼ��ַ
		int SQuantity = 0;//���ڲ��Ҵ洢������
		AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//�Ե�ַ��������������

		//�ȶ���Ӧ���ĵ��������д���
		int sum = SQuantity / 8;//ֱ��
		if ((SQuantity % 8) >= 1) sum++;

		TCP_Slave[5] = sum + 3;//MBAP�е��ֽڳ���
		TCP_Slave[8] = sum;//�������ֽ���

		/*��ָ��λ�ö�ȡ���ؼĴ�������*/
		SumAddress -= ParameterIni->Address;
		int num = SumAddress / 8;//�������ؼĴ��������еķ����±ָ꣬���Ǹ�Ԫ��
		int rem = SumAddress % 8;//ָ���Ǹ�Ԫ�صĵڼ�λ��ʼ��ȡ	
		unsigned char bb = ParameterIni->Local_01_Address[num];

		for (int i = 9; i < sum + 10; i++)//����Ҫ�洢���ڴ��������
			TCP_Slave[i] = 0x00;
		unsigned char huo[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };//������ȡÿһλ������ 
		for (int i = 8, rem_begin = rem; rem < SQuantity + rem_begin; rem++)
		{
			if (rem % 8 == 0 && i != 8) bb = ParameterIni->Local_01_Address[++num];
			if ((rem - rem_begin) % 8 == 0) i++;
			//����
			unsigned char buf = bb & huo[rem % 8];
			if (buf == huo[rem % 8])//�ѵ��ֽڳ��Ȳ�һ������洢����ֵһ���󣬵��Ǿ��ǲ���ȣ�
				TCP_Slave[i] |= huo[(rem - rem_begin) % 8];
		}
		return sum + 9;//+9ΪMABP���Ϲ�����ĳ��Ⱥͱ���
	}
}

//�ÿ���̨��ŵ�ʱ����1���ֽ��������
//�������ܣ�ʵ�ֶ�03��Ĺ���				  
static int TCP_ID_03(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni)
{												//���뷶ΧΪ-32768��32767���з���
	//ID==0����Ϊ�㲥����������03��֧�ֹ㲥
	if (TCP_Slave[6] == 0)
	{
		printf("������01��֧�ֹ㲥\n");
		CodeNum(TCP_Slave, 1);//��վ�в�����ʹ��
		return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
	}
	//�жϲ�ѯ�����еĵ�ַ�������Ƿ�ͱ���ƥ��
	if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;

	//��Ϊ�ǲ��ÿ���̨����ģ����԰��ֽ�����ȡ
	int SumAddress = 0;//���ڲ��Ҵ洢���ҵ���ʼ��ַ
	int SQuantity = 0;//���ڲ��Ҵ洢������
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//�Ե�ַ��������������

	//�ȶ���Ӧ���ĵ��������д���
	int sum = SQuantity * 2;

	TCP_Slave[5] = sum + 3;//MABP�е��ֽڳ���
	TCP_Slave[8] = sum;//�������ֽ���
	int buf = SumAddress - ParameterIni->Address;
	for (int i = buf, j = 9; i <= SQuantity + buf; i++, j += 2)
	{
		TCP_Slave[j + 1] = ParameterIni->Local_03_Address[i];//��ŵ�8λ
		TCP_Slave[j] = ParameterIni->Local_03_Address[i] >> 8;//��Ÿ�8λ	
	}
	return sum + 9;//+9ΪMABP���Ϲ�����ĳ��Ⱥͱ���
}

//�ÿ���̨��ŵ�ʱ����1���ֽ��������
//�������ܣ�ʵ�ֶ�0F��Ĺ���				 
static int TCP_ID_0F(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	//�жϲ�ѯ�����еĵ�ַ�������Ƿ�ͱ���ƥ��
	if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;
	//�жϱ���ֽ����Ƿ�ƥ��
	if (QRecv - 13 != TCP_Slave[12] || TCP_Slave[12] == 0)//�ֽ���Ϊ0
	{
		printf("����֡��ʽ����0F\n");
		return 0;
	}

	//��Ϊ�ǲ��ÿ���̨����ģ����԰��ֽ�����ȡ
	int SumAddress = 0;//���ڲ��Ҵ洢���ҵ���ʼ��ַ
	int SQuantity = 0;//���ڲ��Ҵ洢������
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//�Ե�ַ��������������

	SumAddress -= ParameterIni->Address;//�������ֵ
	int num = SumAddress / 8;//�̴���ӵڼ������ݿ�ʼд��

	unsigned char huo[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };//������ȡÿһλ������
	unsigned char yu[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };//������ÿλ����
	for (int i = 0, j = num; i < SQuantity; i++)
	{
		if ((SumAddress + i) % 8 == 0 && i != 0) j++;//�ƶ�д�����ݵ��±�
		unsigned char buf = TCP_Slave[(i / 8) + 13] & huo[i % 8];//�Ӳ�ѯ��������ȡ���ݣ�ͬʱ�ж���д�����0����1
		if (buf == huo[i % 8]) ParameterIni->Local_01_Address[j] |= huo[(SumAddress + i) % 8];//д1
		else ParameterIni->Local_01_Address[j] &= yu[(SumAddress + i) % 8];//д0
	}

	//ID==0����Ϊ�㲥��������0F֧�ֹ㲥�����Է���ֵ������
	if (TCP_Slave[6] == 0) return 0;//�㲥���÷�����Ӧ���ģ����Է��س���Ϊ0

	TCP_Slave[5] = 0x06;//�����ֽڳ��ȸ�����һ��Ϊ6
	return 12;//������0F��Ӧ���ĳ���һ��Ϊ12
}

//�ÿ���̨��ŵ�ʱ����1���ֽ��������
//�������ܣ�ʵ�ֶ�10��Ĺ���				   
static int TCP_ID_10(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	//�жϲ�ѯ�����еĵ�ַ�������Ƿ�ͱ���ƥ��
	if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;
	//�жϱ���ֽ����Ƿ�ƥ��
	if (QRecv - 13 != TCP_Slave[12] || TCP_Slave[12] == 0)//�ֽ���Ϊ0
	{
		printf("����֡��ʽ����10\n");
		return 0;
	}

	//��Ϊ�ǲ��ÿ���̨����ģ����԰��ֽ�����ȡ
	int SumAddress = 0;//���ڲ��Ҵ洢���ҵ���ʼ��ַ
	int SQuantity = 0;//���ڲ��Ҵ洢������
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//�Ե�ַ��������������

	SumAddress -= ParameterIni->Address;//�������ֵ

	for (int i = 13, j = SumAddress, k = 13; k < 13 + SQuantity; k++, i += 2, j++)//13Ϊ������ݵĵ�һ���ֽ�
	{
		ParameterIni->Local_03_Address[j] = TCP_Slave[i] * 256 + TCP_Slave[i + 1];
	}
	//ID==0����Ϊ�㲥��������0F֧�ֹ㲥�����Է���ֵ������
	if (TCP_Slave[6] == 0) return 0;//�㲥���÷�����Ӧ���ģ����Է��س���Ϊ0

	TCP_Slave[5] = 0x06;//�����ֽڳ��ȸ�����һ��Ϊ6
	return 12;//������0F��Ӧ���ĳ���һ��Ϊ12
}

//�������ܣ�ʵ�ְ���������ò�ͬ�ĺ���
static int FunctionCode(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	switch (TCP_Slave[7])
	{
	case 0x01://��ȡ��Ȧ���״̬
		return	TCP_ID_01(TCP_Slave, ParameterIni);
	case 0x0F://д�����Ȧ
		return TCP_ID_0F(TCP_Slave, ParameterIni, QRecv);
	case 0x03://��ȡ���ּĴ���ֵ
		return	TCP_ID_03(TCP_Slave, ParameterIni);
	case 0x10://д������ּĴ���
		return TCP_ID_10(TCP_Slave, ParameterIni, QRecv);
	default:
		break;
	}
	printf("�����벻ƥ��\n");
	CodeNum(TCP_Slave, 1);//��վ�в�����ʹ��
	return 9;//������Ҫ���ظ��ͻ��˵�һ֡���ݵĳ���
}

//�������ܣ���ɶԽ��յ������ݵĽ���
//���������յ������ݣ��豸ID�������룬����01��Ĵ����洢���飬����03�룬����0F�룬����10�룬���ص�ַ��Recv�����������������صĴ洢����
int Modbus_One_TCP_Slave(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	int buf = MBAPCodeIsTrue(TCP_Slave, QRecv, ParameterIni->ID);//��MBAP����ͷ�빦��������ж��Ƿ���ȷ
	if (buf == 9) return 9;//����9�ǹ�����������������
	if (buf == 1) return 0;//��������
	if (buf == 0)
		return FunctionCode(TCP_Slave, ParameterIni, QRecv);
}

//�������ܣ��Գ�ʼ����ֵ�����ж�ת��
int ParameterIsTrue(char* InitNum)
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
		if (duqu[i] <= '9'&&duqu[i] >= '0')
		{
			num++;

		}
	}
	if (num > 5)//����λ�����󣬷�ֹint�治��
		return -1;
	if ((i - sub) == num)
	{
		if (atoi(InitNum) > 65535) return -1;
		return atoi(InitNum);
	}
	else
		return -1;//���󷵻�ֵΪ-1
}

//�������ܣ���ʼ���˿ں�
static void ModbusTCPParameterInit_Port(ModbusTCPSlave* Port)
{
	/*�������ݵ�����*/
	char str[500];
	printf(">----------------------------------------------------------------------------<\n");
	printf("���뷶Χ0--65535�����п�����Щ�˿ڱ�ռ���޷�ʹ�ã���������ΪModbusЭ���Ĭ�϶˿�502\n");
	printf("������������˿ںţ�\n");
	gets(str);
	Port->port = ParameterIsTrue(str);
	while (Port->port == -1 || Port->port > 65535 || Port->port < 0)
	{
		printf("�����������������\n");
		gets(str);
		Port->port = ParameterIsTrue(str);
	}
	printf("�������˿��������\n");
	return;
}
//�������ܣ���ʼ�����豸ID
static void ModbusTCPParameterInit_ID(ModbusTCPSlave* ID)
{
	/*�������ݵ�����*/
	char str[500];
	printf(">----------------------------------------------------------------------------<\n");
	printf("�豸ID�����÷�ΧΪ1--247\n");
	printf("�������豸ID��\n");
	gets(str);
	ID->ID = ParameterIsTrue(str);
	while (ID->ID == -1 || ID->ID > 247 || ID->ID < 1)
	{
		printf("�����������������\n");
		gets(str);
		ID->ID = ParameterIsTrue(str);
	}
	printf("�豸ID�������\n");
	return;
}

//�������ܣ����õ�ǰ���豸����ʼ��ַ
static void ModbusTCPParameterInit_Address(ModbusTCPSlave* Address)
{
	/*�������ݵ�����*/
	char str[500];
	printf(">----------------------------------------------------------------------------<\n");
	printf("���豸��Ȧ�Ĵ����ͱ��ּĴ�����ʼ��ַ���뷶ΧΪ0--65535\n");
	printf("����������ʵ���ʼ��ַ��\n");
	gets(str);
	Address->Address = ParameterIsTrue(str);
	while (Address->Address == -1 || Address->Address < 0 || Address->Address > 65535)
	{
		printf("�����������������\n");
		gets(str);
		Address->Address = ParameterIsTrue(str);
	}
	printf("��ʼ��ַ�������\n");
	//memset(str, 0, sizeof(str));//����ַ���
	return;
}

//�������ܣ����õ�ǰ���豸�Ĵ����ʼĴ�������Ȧ����
static void ModbusTCPParameterInit_Quantity(ModbusTCPSlave* Quantity)
{
	/*�������ݵ�����*/
	char str[500];
	printf(">----------------------------------------------------------------------------<\n");
	printf("��Ȧ�Ĵ����ͱ��ּĴ����������뷶ΧΪ1--10000\n");
	printf("����������ʵĶ�ȡ��д��������\n");
	gets(str);
	Quantity->Quantity = ParameterIsTrue(str);
	while (Quantity->Quantity == -1 || Quantity->Quantity < 1 || Quantity->Quantity > 10000)
	{
		printf("�����������������\n");
		gets(str);
		Quantity->Quantity = ParameterIsTrue(str);
	}
	printf("��ȡ�����������\n");
	printf(">----------------------------------------------------------------------------<\n");
	//memset(str, 0, sizeof(str));//����ַ���
	return;
}
//�������ܣ�ͨ������̨��ʼ�����豸�Ĳ���
void ModbusTCPParameterInit(ModbusTCPSlave* Parameter)
{
	ModbusTCPParameterInit_Port(Parameter);//����˵Ķ˿ں�
	ModbusTCPParameterInit_ID(Parameter);//����˵��豸ID
	ModbusTCPParameterInit_Address(Parameter);//���豸����ʼ��ַ
	ModbusTCPParameterInit_Quantity(Parameter);//�����ʼĴ�������Ȧ����
	return;
}