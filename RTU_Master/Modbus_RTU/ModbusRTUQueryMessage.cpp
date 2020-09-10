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

//函数功能：对初始化数值进行判断转换
//返回值：-1代表错误，整数代表正确
int ModbusInit(char* InitNum)
{
	string duqu = InitNum;
	int a = 0;
	for (int j = 0; duqu[j] != '\0'; j++)//排除例123 123这样的字符串
	{
		if (duqu[j] != ' '&&a == 0)
			a++;
		if (duqu[j] == ' '&&a == 1)
			a++;
		if (duqu[j] != ' '&&a == 2)
			return -1;
	}

	int sub = duqu.rfind(" ");//查找到第一个不是空格的字符
	sub++;//空格的下一个字符就不再是空格
	int i, num = 0, num1 = 0;
	for (i = sub; duqu[i] != '\0'; i++)
	{
		if (duqu[i] <= '9'&&duqu[i] >= '0' || duqu[sub]== '-')
		{
			num++;
		}
	}
	if (num > 6)//输入位数过大，防止int存不下
		return -1;
	if ((i - sub) == num)
	{
		if (atoi(InitNum) > 65535) return -1;
		return atoi(InitNum);
	}
	else
		return -1;//错误返回值为-1
}
//函数功能：显示初始界面
static void ModbusRTUDataInitDefault(ModbusRTU* ModbusRTUData)
{
	printf("===========================================================================================\n");
	printf("本通信软件用于Modbus RTU模式主站模式\n");
	printf("串口默认设置如下：\n");
	printf("端口号：COM\n");
	printf("波特率：9600 Baud\n");
	printf("数据位：8位\n");
	printf("奇偶校验位：无校验\n");
	printf("停止位：1位\n");
	printf("===========================================================================================\n");
	printf("\n");//换行
	return;
}
//函数功能：初始化通信超时时间
static void ModbusRTUDataInitTimeout(ModbusRTU* ModbusRTUData)
{
	char str[300];//用于读取控制台输入
	printf(">----------------------------------通信超时时间------------------------------------<\n");
	printf("通信超时时间设置范围为50--100000（延时单位为1ms）\n");
	printf("请输入通信超时时间：\n");
	int TimeOuts = 0;
	gets(str);
	TimeOuts = ModbusInit(str);
	while (TimeOuts == -1 || TimeOuts > 100000 || TimeOuts< 50)
	{
		printf("输入错误请重新输入\n");
		gets(str);
		TimeOuts = ModbusInit(str);
	}
	ModbusRTUData->TimeOuts = TimeOuts;
	printf("通信超时时间输入完成\n");
	return;
}
//函数功能：初始化设备ID
static void ModbusRTUDataInitID(ModbusRTU* ModbusRTUData)
{
	char str[300];//用于读取控制台输入
	printf(">-----------------------------------设备ID-----------------------------------------<\n");
	printf("设备ID的设置范围为0--247（输入设备ID为0，代表广播）\n");
	printf("请输入设备ID：\n");
	int ID = 0;
	gets(str);
	ID = ModbusInit(str);
	while (ID == -1 || ID > 247 || ID<0)
	{
		printf("输入错误请重新输入\n");
		gets(str);
		ID = ModbusInit(str);
	}
	ModbusRTUData->ID = ID;
	printf("设备ID输入完成\n");
	return;
}
//函数功能：初始化功能码
static void ModbusRTUDataInitFunCode(ModbusRTU* ModbusRTUData)
{
	char str[300];//用于读取控制台输入
	printf(">-----------------------------------功能码-----------------------------------------<\n");
	printf("当前功能码只有1，3，15，16这四个，请从这四个中选择1个输入\n");
	printf("请按十进制输入功能码：\n");
	gets(str);
	while (1)
	{
		ModbusRTUData->Function = ModbusInit(str);
		while (ModbusRTUData->Function == -1 || ModbusRTUData->Function != 1 && \
			ModbusRTUData->Function != 3 && ModbusRTUData->Function != 15 && ModbusRTUData->Function != 16)
		{
			printf("输入错误请重新输入\n");
			gets(str);
			ModbusRTUData->Function = ModbusInit(str);
		}
		if (ModbusRTUData->ID == 0 && (ModbusRTUData->Function == 1 || ModbusRTUData->Function == 3))
		{
			printf("当前输入的功能码不支持广播，请重新输入\n");
			gets(str);
		}
		else break;
	}
	printf("功能码输入完成\n");
	return;
}
//函数功能：初始化起始地址
static void ModbusRTUDataInitAddress(ModbusRTU* ModbusRTUData)
{
	char str[300];//用于读取控制台输入
	printf(">-----------------------------------起始地址---------------------------------------<\n");
	printf("地址输入范围为0--65535\n");
	printf("请输入起始地址：\n");
	gets(str);
	ModbusRTUData->Address = ModbusInit(str);
	while (ModbusRTUData->Address == -1 || ModbusRTUData->Address < 0 || ModbusRTUData->Address > 65535)
	{
		printf("输入错误请重新输入\n");
		gets(str);
		ModbusRTUData->Address = ModbusInit(str);
	}
	printf("起始地址输入完成\n");
	//memset(str, 0, sizeof(str));//清空字符串
	return;
}
//函数功能：初始化数量
static void InitQuantity(ModbusRTU* ModbusRTUData, int Num)
{
	char str[300];//用于读取控制台输入
	printf(">------------------------------------数量------------------------------------------<\n");
	printf("访问的线圈寄存器的数量输入范围为1--%d\n",Num);
	printf("请输入读取或更改的数量：\n");
	gets(str);
	ModbusRTUData->RegisterQuantity = ModbusInit(str);
	while (ModbusRTUData->RegisterQuantity == -1 || ModbusRTUData->RegisterQuantity < 1 \
		|| ModbusRTUData->RegisterQuantity > Num)
	{
		printf("输入错误请重新输入\n");
		gets(str);
		ModbusRTUData->RegisterQuantity = ModbusInit(str);
	}
	printf("读取或更改的数量输入完成\n");
	printf(">----------------------------------------------------------------------------------<\n");
}
//函数功能：根据功能码初始化数量
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
//函数功能：对写入的线圈或寄存器进行初始化
static void ModbusRTUDataInit0F_10(ModbusRTU* ModbusRTUData)
{
	char str[1024];//用于读取控制台输入
	//对功能码0x0F输入线圈
	if (ModbusRTUData->Function == 0x0F)//01 03 0F 10
	{
		printf(">------------------------------线圈的变更数据--------------------------------------<\n");
		int count = ModbusRTUData->RegisterQuantity / 8;
		if (ModbusRTUData->RegisterQuantity % 8 >= 1) count++;
		printf("请输入 %d 个你想修改的线圈的变更数据\n", count);
		printf("数量输入范围为0--255，请按十进制输入,空格隔开\n");
		printf("例如：10 1 2 5 6 8 91 128\n");

		ModbusRTUData->Data_0F[N] = { 0 };//清零
		char *p;//用于切割
		while (1)
		{
			int i = 0;
			gets(str);//输入数据
			p = strtok(str, " ");//截取
			while (p)
			{
				if (ModbusInit(p) > 255 || ModbusInit(p) < 0 || i == count)
				{
					i--;
					break;//非正常退出
				}
				else ModbusRTUData->Data_0F[i] = ModbusInit(p);//赋值
				p = strtok(NULL, " ");
				i++;
			}
			if (i == count)	break;
			else printf("输入错误请重新输入\n");
		}
		printf("线圈的变更数据输入完成\n");
		printf(">---------------------------------------------------------------------------------<\n");
	}
	//对功能码0x10输入数据
	else if (ModbusRTUData->Function == 0x10)
	{
		printf(">--------------------------------保持寄存器的变更数据-----------------------------<\n");
		int count = ModbusRTUData->RegisterQuantity;
		printf("请输入 %d 个你想修改的保持寄存器的变更数据\n", count);
		printf("数量输入范围为-32768--32767，请按十进制输入,空格隔开\n");
		printf("例如：-10 1 2 5 6 8 91 32767\n");
		ModbusRTUData->Data_10[N] = { 0 };//清零
		char *p;//用于切割
		while (1)
		{
			int i = 0;
			gets(str);//输入数据
			p = strtok(str, " ");//截取
			while (p)
			{
				if (ModbusInit(p) > 32767 || ModbusInit(p) < -32768 || i == count)
				{
					i--;
					break;//非正常退出
				}
				else ModbusRTUData->Data_10[i] = ModbusInit(p);//赋值
				p = strtok(NULL, " ");
				i++;
			}
			if (i == count&&p == NULL) break;
			else printf("输入错误请重新输入\n");
		}
		printf("保持寄存器的变更数据输入完成\n");
		printf(">---------------------------------------------------------------------------------<\n");
	}
	return;
}
//函数功能：对通信进行初始化
void ModbusRTUDataInit(ModbusRTU* ModbusRTUData)
{
	ModbusRTUDataInitDefault(ModbusRTUData);//默认界面
	ModbusRTUDataInitTimeout(ModbusRTUData);//超时时间
	ModbusRTUDataInitID(ModbusRTUData);//访问的从设备ID
	ModbusRTUDataInitFunCode(ModbusRTUData);//功能码
	ModbusRTUDataInitAddress(ModbusRTUData);//起始地址
	ModbusRTUDataInitRegisterQuantity(ModbusRTUData);//访问寄存器或线圈数量
	ModbusRTUDataInit0F_10(ModbusRTUData);//对待写入的线圈或寄存器进行写入
	return;
}
//函数功能；用于控制台重复选择ID、功能码、数量等
void DataReelect(ModbusRTU* ModbusRTUData)
{
	char str[300];//用于读取控制台输入
	int Num = 0;
	printf("\n"); printf("\n"); printf("\n");
	printf(">---------------------------------------------------------------------------------<\n");
	printf("请选择以下编号执行相应操作：\n");
	printf("1、重新输入待访问的从设备ID\n");
	printf("2、重新输入功能码\n");
	printf("3、重新输入起始地址\n");
	printf("4、重新输入待访问的寄存器或线圈数量和数值\n");
	printf("5、退出选择\n");
	printf(">---------------------------------------------------------------------------------<\n");
	while (Num != 5)
	{
		gets(str);
		Num = ModbusInit(str);
		while (Num == -1 || Num != 1 && Num != 2 \
			&& Num != 3 && Num != 4 && Num != 5)
		{
			printf("输入错误请重新输入\n");
			gets(str);
			Num = ModbusInit(str);
		}
		switch (Num)
		{
		case(1) :
			ModbusRTUDataInitID(ModbusRTUData); break;//访问的从设备ID
		case(2) :
			ModbusRTUDataInitFunCode(ModbusRTUData); break;//功能码
		case(3) :
			ModbusRTUDataInitAddress(ModbusRTUData); break;//起始地址
		case(4) :
			ModbusRTUDataInitRegisterQuantity(ModbusRTUData);//访问寄存器或线圈数量
			ModbusRTUDataInit0F_10(ModbusRTUData); //对待写入的线圈或寄存器进行写入
			break;
		case(5) : break;
		default:
			break;
		}
		if (Num != 5)
		{
			printf("\n");
			printf("请继续选择编号执行相应操作：\n");
			printf("\n");
		}
	}
	return;
}
//函数功能：判断回车
void SpaceIsTrue()
{
	printf("请按回车发送报文\n");
	char ch;
	gets(&ch);
	while (ch != '\0')
	{
		printf("输入错误，请重新输入\n");
		gets(&ch);
	}
	return;

}