#include "Modbus_RTU_Init.h"

//�������ֲ�����ʽ��һ�㶼ͨ���ĸ���������ɣ�
//
//��1�� �򿪴���
//��2�� ���ô���
//��3�� ��д����
//��4�� �رմ���


HANDLE InitCOM(char* COM, DWORD Delay)
{
	HANDLE hCom = INVALID_HANDLE_VALUE;//ȫ�ֱ��������ھ��
	hCom = CreateFile(COM, //COM��
		GENERIC_READ | GENERIC_WRITE, //�������д
		0, //��Ϊ���ڲ��ܹ����ò���������Ϊ0
		NULL,
		OPEN_EXISTING,//�򿪶����Ǵ���
		0, //ͬ����ʽ
		NULL);
	if (INVALID_HANDLE_VALUE == hCom)
	{
		return INVALID_HANDLE_VALUE;
	}
	SetupComm(hCom, 4096, 4096);//���û���

	//���ô��ھ������
	DCB dcb;
	GetCommState(hCom, &dcb);//���ô���
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = CBR_9600;//������
	dcb.StopBits = ONESTOPBIT;//ֹͣλ
	dcb.ByteSize = 8;//��Чλ
	dcb.Parity = NOPARITY;//����żУ��
	SetCommState(hCom, &dcb);

	PurgeComm(hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);//��շ��ͽ��ջ���

	COMMTIMEOUTS ct;//�趨����ʱ
	//COMMTIMEOUTS�ṹ�ĳ�Ա���Ժ���Ϊ��λ
	ct.ReadIntervalTimeout = 20;//�������ʱ
	//�����ʱ���ܳ�ʱ�������ǲ���ص�
	ct.ReadTotalTimeoutConstant = Delay;//��ʱ�䳣��
	ct.ReadTotalTimeoutMultiplier = 0;//��ʱ��ϵ��

	ct.WriteTotalTimeoutMultiplier = 1;//дʱ�䳣��
	ct.WriteTotalTimeoutConstant = 1;//дʱ��ϵ��

	SetCommTimeouts(hCom, &ct);//���÷��ͽ��ճ�ʱ

	return hCom;
}

bool ComRead(HANDLE hCom, LPBYTE buf, int &len)
{
	DWORD ReadSize = 0;
	BOOL rtn = FALSE;
	bool flag = false;

	rtn = ReadFile(hCom, (char*)buf, 1024, &ReadSize, NULL);
	len = ReadSize;
	//printf("len = %d\n", len);
	if (rtn == TRUE && len != 0)
	{
		flag = true;
	}
	//PurgeComm������մ��ڵ��������������
	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	return flag;
}

bool ComWrite(HANDLE hCom, LPBYTE buf, int &len)
{
	//�����建���Ŀ���Ƿ�ֹ�һ�û���㷢��ָ����Ϳ�ʼ���ҷ��������ˣ�������յ����ݲ���Ҫ
	PurgeComm(hCom, PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR | PURGE_TXABORT);
	//PurgeComm(hCom, PURGE_TXCLEAR | PURGE_TXABORT);
	BOOL rtn = FALSE;
	DWORD WriteSize = 0;

	rtn = WriteFile(hCom, buf, len, &WriteSize, NULL);
	len = WriteSize;


	return rtn != FALSE;
}

