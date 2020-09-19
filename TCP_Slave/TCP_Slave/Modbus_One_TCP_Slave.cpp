#include "Modbus_One_TCP_Slave.h"

//函数功能：对接收的地址和数量进行整合
static void AddressQuantuty(unsigned char* TCP_Slave, int* SumAddress, int* SQuantity)
{
	*SumAddress = TCP_Slave[8] * 256;//地址高位
	*SumAddress += TCP_Slave[9];//地址低位

	*SQuantity = TCP_Slave[10] * 256;//访问的数量高位
	*SQuantity += TCP_Slave[11];//访问的数量低位
	return;
}
//函数功能；返回异常码
static void CodeNum(unsigned char* TCP_Slave, int Num)
{
	TCP_Slave[5] = 0x03;//后续字节长度

	TCP_Slave[7] += 0x80;//加上0x80
	if (Num == 1){
		TCP_Slave[8] = 0x01;//说明从站设备不支持这个功能码
	}
	else if (Num == 2){
		TCP_Slave[8] = 0x02;//说明地址在从设备中不存在
	}
	else if (Num == 3){
		TCP_Slave[8] = 0x03;//说明访问的是非法数据值
	}
	return;
}
//函数功能：实现对异常码03的检查
static int AbnormalCode03(unsigned char* TCP_Slave, int Code, int sum)
{
	if (sum == TCP_Slave[5])//判断MBAP报文头是否正确
	{
		int SumAddress = 0;
		int SQuantity = 0;
		int Num = 0;//用于存储需要写入的字节数
		AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//计算出查询报文想要写入的字节数
		if (Code == 0x0F)//15
		{
			Num = (SQuantity + 7 ) / 8;
		}
		if (Code == 0x10)//16
		{
			Num = SQuantity * 2;
		}
		if (Num != TCP_Slave[12])//判断查询报文想要的写入的字节数与查询报文
		{
			printf("非法数据值\n");
			CodeNum(TCP_Slave, 3);
			return 9;//返回需要返回给客户端的一帧数据的长度
		}
		else
			return 0;
	}
	printf("MBAP报文头字节数错误\n");
	return 1;
}
//函数功能：对MBAP报文头和功能码进行判断是否正确
static int MBAPCodeIsTrue(unsigned char* TCP_Slave, short int QRecv, unsigned char ID)
{
	if (QRecv != 12 && (TCP_Slave[7] == 0x01 || TCP_Slave[7] == 0x03))//判断长度是否正确,0x01和0x03查询报文长度一定是12
	{
		printf("帧格式错误1\n");
		return 1;
	}
	if (QRecv < 12){
		printf("帧格式错误1\n");
		return 1;
	}
	if (TCP_Slave[2] != 0 || TCP_Slave[3] != 0)//判断MBAP报文头中的协议标识符是否正确
	{
		printf("MBAP报文头的协议标识符错误\n");
		return 1;
	}
	if (TCP_Slave[6] != ID &&TCP_Slave[6] != 0)
	{
		printf("查找的设备号与本机的设备号不匹配\n");
		return 1;
	}
	int sum = TCP_Slave[4] * 256;//用于判断接收到的数据是否正确
	sum += TCP_Slave[5];
	if (sum != (QRecv - 6))
	{
		printf("字节长度与实际长度不一致\n");
		return 1;
	}
	//若查询报文是01 03功能码
	if ((TCP_Slave[7] == 0x01 || TCP_Slave[7] == 0x03) && (sum == 6 && QRecv == 12))//当功能码为01或03时MBAP中的字节长度一定是6
	{
		return 0;//若查询报文正确，则MBAP字节长度一定是6，总长度一定是12
	}
	//查询报文0F功能码
	else if (TCP_Slave[7] == 0x0F)
	{
		return AbnormalCode03(TCP_Slave, 0x0F, sum);
	}
	//查询报文10功能码
	else if (TCP_Slave[7] == 0x10)
	{
		return AbnormalCode03(TCP_Slave, 0x10, sum);
	}

	printf("功能码不匹配\n");
	CodeNum(TCP_Slave, 1);//说明从站设备不支持这个功能码
	return 9;//返回需要返回给客户端的一帧数据的长度
}

//函数功能：判断访问的起始地址和数量是否和本地匹配
//参数：本地地址和数量
//返回值：9和0                                            
static int AddressIsTrue(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni)
{
	int SumAddress = 0;//用判断本地地址和查询报文的地址
	int SQuantity = 0;//用判断本地寄存器数量和查询报文的寄存器数量
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);

	int add = SumAddress - ParameterIni->Address;
	if (SumAddress < ParameterIni->Address)//判断地址是否在从设备中不存在
	{
		printf("地址从设备中不存在\n");
		CodeNum(TCP_Slave, 2);//说明地址在从设备中不存在
		return 9;//返回需要返回给客户端的一帧数据的长度
	}

	if ((add + SQuantity) > ParameterIni->Quantity)//判断是不是非法数据值，即指定的数据超过范围或者不允许使用
	{
		printf("指定的数据超过范围\n");
		CodeNum(TCP_Slave, 2);//说明地址在从设备中不存在
		return 9;//返回需要返回给客户端的一帧数据的长度
	}
	int sum = 9999;//可以访问的线圈寄存器的最大地址
	if (SQuantity == 0 || SumAddress > 9999)//当访问数量为0或者访问地址超过，错误
	{
		CodeNum(TCP_Slave, 3);//说明访问指定的数据超过范围
		return 9;//返回需要返回给客户端的一帧数据的长度
	}
	if (SQuantity > 2000 && TCP_Slave[7] == 1){//01
		CodeNum(TCP_Slave, 3);//说明访问指定的数据超过范围
		return 9;//返回需要返回给客户端的一帧数据的长度
	}
	else if (SQuantity > 1968 && TCP_Slave[7] == 15){//15
		CodeNum(TCP_Slave, 3);//说明访问指定的数据超过范围
		return 9;//返回需要返回给客户端的一帧数据的长度
	}
	else if (SQuantity > 125 && TCP_Slave[7] == 3){//03
		CodeNum(TCP_Slave, 3);//说明访问指定的数据超过范围
		return 9;//返回需要返回给客户端的一帧数据的长度
	}
	else if (SQuantity > 123 && TCP_Slave[7] == 16){//16
		CodeNum(TCP_Slave, 3);//说明访问指定的数据超过范围
		return 9;//返回需要返回给客户端的一帧数据的长度
	}
	return 0;
}

//用控制台存放的时候按照1个字节来输入的
//函数功能：实现对01码的功能				  
static int TCP_ID_01(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni)
{            /*当功能码是01时本地控制台输入地址（0--65535）和数量（1--20000）*/
	//ID==0代表为广播，但功能码01不支持广播
	if (TCP_Slave[6] == 0)
	{
		printf("功能码01不支持广播\n");
		return 0;//返回需要返回给客户端的一帧数据的长度
	}
	else
	{
		//判断查询报文中的地址和数量是否和本地匹配
		if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;

		//因为是采用控制台输入的，所以按字节来读取
		int SumAddress = 0;//用于查找存储查找的起始地址
		int SQuantity = 0;//用于查找存储的数量
		AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//对地址和数量进行整合

		//先对响应报文的数量进行处理
		int sum = (SQuantity + 7) / 8;//直接

		TCP_Slave[5] = sum + 3;//MBAP中的字节长度
		TCP_Slave[8] = sum;//数据域字节数

		/*从指定位置读取本地寄存器数据*/
		SumAddress -= ParameterIni->Address;
		int num = SumAddress / 8;//用作本地寄存器数组中的访问下标，指定那个元素

		int rem = SumAddress % 8;//指定那个元素的第几位开始读取	
		unsigned char bb = ParameterIni->Local_01_Address[num];

		for (int i = 9; i < sum + 10; i++)//对需要存储的内存进行清零
			TCP_Slave[i] = 0x00;
		unsigned char huo[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };//用来读取每一位的数据 
		for (int i = 8, rem_begin = rem; rem < SQuantity + rem_begin; rem++)
		{
			if (rem % 8 == 0 && i != 8) 
				bb = ParameterIni->Local_01_Address[++num];
			if ((rem - rem_begin) % 8 == 0) 
				i++;
			//解析
			unsigned char buf = bb & huo[rem % 8];
			if (buf == huo[rem % 8])//难道字节长度不一样，虽存储的数值一样大，但是就是不相等？
				TCP_Slave[i] |= huo[(rem - rem_begin) % 8];
		}
		return sum + 9;//+9为MABP加上功能码的长度和本身
	}
}

//用控制台存放的时候按照1个字节来输入的
//函数功能：实现对03码的功能				  
static int TCP_ID_03(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni)
{												//输入范围为-32768到32767，有符号
	//ID==0代表为广播，但功能码03不支持广播
	if (TCP_Slave[6] == 0)
	{
		printf("功能码03不支持广播\n");
		return 0;//返回需要返回给客户端的一帧数据的长度
	}
	//判断查询报文中的地址和数量是否和本地匹配
	if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;

	//因为是采用控制台输入的，所以按字节来读取
	int SumAddress = 0;//用于查找存储查找的起始地址
	int SQuantity = 0;//用于查找存储的数量
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//对地址和数量进行整合

	//先对响应报文的数量进行处理
	int sum = SQuantity * 2;

	TCP_Slave[5] = sum + 3;//MABP中的字节长度
	TCP_Slave[8] = sum;//数据域字节数
	int buf = SumAddress - ParameterIni->Address;
	for (int i = buf, j = 9; i <= SQuantity + buf; i++, j += 2)
	{
		TCP_Slave[j + 1] = ParameterIni->Local_03_Address[i];//存放低8位
		TCP_Slave[j] = ParameterIni->Local_03_Address[i] >> 8;//存放高8位	
	}
	return sum + 9;//+9为MABP加上功能码的长度和本身
}

//用控制台存放的时候按照1个字节来输入的
//函数功能：实现对0F码的功能				 
static int TCP_ID_0F(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	//判断查询报文中的地址和数量是否和本地匹配
	if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;

	//因为是采用控制台输入的，所以按字节来读取
	int SumAddress = 0;//用于查找存储查找的起始地址
	int SQuantity = 0;//用于查找存储的数量
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//对地址和数量进行整合

	SumAddress -= ParameterIni->Address;//计算出差值
	int num = SumAddress / 8;//商代表从第几个数据开始写入

	unsigned char huo[8] = { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };//用来读取每一位的数据
	unsigned char yu[8] = { 0xFE, 0xFD, 0xFB, 0xF7, 0xEF, 0xDF, 0xBF, 0x7F };//用来与每位数据
	for (int i = 0, j = num; i < SQuantity; i++)
	{
		if ((SumAddress + i) % 8 == 0 && i != 0) 
			j++;//移动写入数据的下标
		unsigned char buf = TCP_Slave[(i / 8) + 13] & huo[i % 8];//从查询报文中提取数据，同时判断是写入的是0还是1
		if (buf == huo[i % 8])
			ParameterIni->Local_01_Address[j] |= huo[(SumAddress + i) % 8];//写1
		else 
			ParameterIni->Local_01_Address[j] &= yu[(SumAddress + i) % 8];//写0
	}

	//ID==0代表为广播，功能码0F支持广播，所以返回值有两种
	if (TCP_Slave[6] == 0) return 0;//广播不用返回响应报文，所以返回长度为0

	TCP_Slave[5] = 0x06;//后续字节长度更改且一定为6
	return 12;//功能码0F响应报文长度一定为12
}

//用控制台存放的时候按照1个字节来输入的
//函数功能：实现对10码的功能				   
static int TCP_ID_10(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	//判断查询报文中的地址和数量是否和本地匹配
	if (AddressIsTrue(TCP_Slave, ParameterIni) == 9) return 9;
	//判断变更字节数是否匹配
	if (QRecv - 13 != TCP_Slave[12] || TCP_Slave[12] == 0)//字节数为0
	{
		printf("数据帧格式错误10\n");
		return 0;
	}

	//因为是采用控制台输入的，所以按字节来读取
	int SumAddress = 0;//用于查找存储查找的起始地址
	int SQuantity = 0;//用于查找存储的数量
	AddressQuantuty(TCP_Slave, &SumAddress, &SQuantity);//对地址和数量进行整合

	SumAddress -= ParameterIni->Address;//计算出差值

	for (int i = 13, j = SumAddress, k = 13; k < 13 + SQuantity; k++, i += 2, j++)//13为变更数据的第一个字节
	{
		ParameterIni->Local_03_Address[j] = TCP_Slave[i] * 256 + TCP_Slave[i + 1];
	}
	//ID==0代表为广播，功能码0F支持广播，所以返回值有两种
	if (TCP_Slave[6] == 0) return 0;//广播不用返回响应报文，所以返回长度为0

	TCP_Slave[5] = 0x06;//后续字节长度更改且一定为6
	return 12;//功能码0F响应报文长度一定为12
}

//函数功能：实现按功能码调用不同的函数
static int FunctionCode(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	switch (TCP_Slave[7])
	{
	case 0x01://读取线圈输出状态
		return	TCP_ID_01(TCP_Slave, ParameterIni);
	case 0x0F://写多个线圈
		return TCP_ID_0F(TCP_Slave, ParameterIni, QRecv);
	case 0x03://读取保持寄存器值
		return	TCP_ID_03(TCP_Slave, ParameterIni);
	case 0x10://写多个保持寄存器
		return TCP_ID_10(TCP_Slave, ParameterIni, QRecv);
	}
}

//函数功能：完成对接收到的数据的解析
//参数：接收到的数据，设备ID，功能码，本地01码寄存器存储数组，本地03码，本地0F码，本地10码，本地地址，Recv函数返回数量，本地的存储数量
int Modbus_One_TCP_Slave(unsigned char* TCP_Slave, ModbusTCPSlave* ParameterIni, short QRecv)
{
	int buf = MBAPCodeIsTrue(TCP_Slave, QRecv, ParameterIni->ID);//对MBAP报文头与功能码进行判断是否正确
	if (buf == 9) return 9;//返回9是功能码或访问数量错误
	if (buf == 1) return 0;//其他错误
		return FunctionCode(TCP_Slave, ParameterIni, QRecv);
}

