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
	SetupComm(hCom, 2048, 2048);//���û���

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
	//��ÿ���ַ��������1.5���ַ���Ϊ��Ч�ַ���9600�������¼����Ϊ1.7�࣬������Ϊ2
	ct.ReadIntervalTimeout = 2;//�������ʱ
	//�����ʱ���ܳ�ʱ�������ǲ���ص�
	ct.ReadTotalTimeoutConstant = 0;//��ʱ��ϵ��
	ct.ReadTotalTimeoutMultiplier = 2000;//��ʱ�䳣��
	ct.WriteTotalTimeoutMultiplier = 0;//дʱ��ϵ��
	ct.WriteTotalTimeoutConstant = 0;//дʱ�䳣��

	SetCommTimeouts(hCom, &ct);//���÷��ͽ��ճ�ʱ

	return hCom;
}

bool ComRead(HANDLE hCom, LPBYTE buf, int &len)
{
	DWORD ReadSize = 0, dwEvent = 0;
	BOOL rtn = FALSE;
	//���ö�ȡ1���ֽ����ݣ��������������ݵ���ʱ����������أ�����ֱ����ʱ
	rtn = ReadFile(hCom, buf, 1, &ReadSize, NULL);
	//����ǳ�ʱrtn=true����ReadSize=0����������ݵ�����ȡһ���ֽ�ReadSize=1
	if (rtn == TRUE && 1 == ReadSize){
		DWORD Error;
		COMSTAT cs = { 0 };
		int ReadLen = 0;
		//��ѯʣ������ֽ�δ��ȡ���洢��cs.cbInQue��
		ClearCommError(hCom, &Error, &cs);
		ReadLen = (cs.cbInQue > len) ? len : cs.cbInQue;
		if (ReadLen > 0){
			//����֮ǰ�ȴ�ʱ�Զ�ȡһ���ֽڣ�����buf+1
			rtn = ReadFile(hCom, buf + 1, ReadLen, &ReadSize, NULL);
			len = 0;
			if (rtn){
				len = ReadLen + 1;
			}
		}
	}
	//PurgeComm������մ��ڵ��������������
	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);
	//return rtn != FALSE;
	return ReadSize>1;
}

bool ComWrite(HANDLE hCom, LPBYTE buf, int &len)
{
	PurgeComm(hCom, PURGE_TXCLEAR | PURGE_TXABORT);
	BOOL rtn = FALSE;
	DWORD WriteSize = 0;

	rtn = WriteFile(hCom, buf, len, &WriteSize, NULL);
	len = WriteSize;

	PurgeComm(hCom, PURGE_RXABORT | PURGE_RXCLEAR);//�����建���Ŀ���Ƿ�ֹ�һ�û���㷢��ָ����Ϳ�ʼ���ҷ��������ˣ�������յ����ݲ���Ҫ
	return rtn != FALSE;
}

