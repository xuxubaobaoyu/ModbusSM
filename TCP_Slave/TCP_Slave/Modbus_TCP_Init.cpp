#include "Modbus_TCP_Init.h"


//��WS2_32.dll ��ʼ��
static void Modbus_TCP_Init_WS2_32_dll()
{
	// ��ʼ��WS2_32.dll 
	WSADATA wsaData;
	WORD sockVersion = MAKEWORD(2, 2);
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		exit(0);//�������г����˳�����
	}
	return;
}
//�����׽���
static unsigned int Modbus_TCP_Create_Socket()
{
	// �����׽��� 
	SOCKET sListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);//IPv4��SOCK_STREAM��TCP����Э��
	//����ָ���׽���ʹ�õĵ�ַ��ʽ��ͨ��ʹ��AF_INET����IPv4
	//ָ���׽��ֵ����ͣ�����SOCK_DGRAM�����õ���udp���ɿ����䣬��ʽsocket (SOCK_STREAM)��ʽ�׽����ṩ�ɿ��ġ��������ӵ�ͨ����;��ʹ��TCPЭ�飬�Ӷ���֤�����ݴ������ȷ�Ժ�˳����
	//���type����ʹ�ã�ָ��ʹ�õ�Э�����ͣ���ָ���׽������ͺ󣬿�������Ϊ0����ΪĬ��ΪUDP��TCP��
	if (sListen == INVALID_SOCKET)//�ж��Ƿ�ɹ������׽��֣���0�ʹ���ʧ��
	{
		return 0;
	}
	return sListen;
}
//���׽���
static bool Modbus_TCP_Bind(unsigned int SListen, short int prot)
{
	// ���sockaddr_in�ṹ ,�Ǹ��ṹ��
	/* struct sockaddr_in {
	short sin_family;  //��ַ�壨ָ����ַ��ʽ�� ����ΪAF_INET
	u_short	sin_port; //�˿ں�
	struct in_addr sin_addr; //IP��ַ
	char sin_zero[8]; //���ӽڣ���Ϊ��
	} */
	sockaddr_in sin;
	sin.sin_family = AF_INET;//sin_family����Э���壬AF_INET����TCP/IPЭ��

	//�����ͱ����������ֽ�˳��ת��������ֽ�˳��
	sin.sin_port = htons(prot);  //1024 ~ 49151����ͨ�û�ע��Ķ˿ں�
	sin.sin_addr.S_un.S_addr = INADDR_ANY;//Ĭ��
	// ������׽��ֵ�һ�����ص�ַ 
	//bind�����洴����socket���׽��֣�
	if (bind(SListen, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR)//�ж��Ƿ�ֵ�ɹ�������-1��û�ɹ�
	{
		return false;
	}
	return true;
}

//�����ȴ��ͻ�������
unsigned int Modbus_TCP_Accept(unsigned int SListen)
{
	
	// ѭ�����ܿͻ����������� 
	sockaddr_in remoteAddr;//���ڴ洢�ͻ��˵�Э�顢�˿ںš�IP
	int nAddrLen = sizeof(remoteAddr);
	unsigned int sClient = 0;

	char szText[] = " TCP Server Demo! \r\n";
	while (sClient == 0)
	{
		// ����һ�������� 
		//��(SOCKADDR*)&remoteAddr��һ��ָ��sockaddr_in�ṹ��ָ�룬���ڻ�ȡ�Է���ַ
		//���accpet�ɹ�����ô�䷵��ֵ�����ں��Զ����ɵ�һ��ȫ�µ������֣������뷵�ؿͻ���TCP���ӡ�
		//��һ������Ϊ��������socket�����֣��ڶ�������Ϊָ��struct sockaddr *��ָ�룬���ڷ��ؿͻ��˵�Э���ַ������������ΪЭ���ַ�ĳ���
		sClient = accept(SListen, (SOCKADDR*)&remoteAddr, &nAddrLen);

		if (sClient == INVALID_SOCKET)//������ʧ�ܣ�sClientΪ0
		{
			return 0;
		}
	}
//#ifdef Debug
	printf("���ܵ�һ���ͻ�������,IP��ַ��%s \r\n", inet_ntoa(remoteAddr.sin_addr));//inet_ntoa�������ַת���ɡ�.��������ַ�����ʽ��
//#endif
	return sClient;
}

//��TCP���ӳ�ʼ��
void Modbus_TCP_Init(short int prot, unsigned int *Listen_Client)
{
	Modbus_TCP_Init_WS2_32_dll();//��WS2_32.dll ��ʼ��
	unsigned int Socket_Listen = Modbus_TCP_Create_Socket();//�����׽���
	if (Socket_Listen == 0)//�ж��׽����Ƿ񴴽��ɹ�
	{
		printf("Failed socket() \n");
		return;
	}
	//���׽���
	if (Modbus_TCP_Bind(Socket_Listen, prot) == false)//�жϰ��׽����Ƿ�ɹ�
	{
		printf("Failed bind() \n");
		return;
	}
	//2ָ���ǣ����������������ֵ���δ��������������
	//socket()����������socketĬ����һ���������͵ģ�listen������socket��Ϊ�������͵ģ��ȴ��ͻ�����������
	if (listen(Socket_Listen, 2) == SOCKET_ERROR)//����
	{
		printf("Failed listen() \n");
		return ;
	}
	printf("�ȴ��ͻ������ӡ�����\n");

	//�����׽������ڷ��ͺͽ���
	Listen_Client[0] = Socket_Listen;//
	Listen_Client[1] = Modbus_TCP_Accept(Socket_Listen);//�����ȴ��ͻ�������
	if (Listen_Client[1] == 0)
	{
		printf("Failed accept()");
		return;
	}		
	return ;
}