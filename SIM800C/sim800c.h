#ifndef __SIM800C_H__
#define __SIM800C_H__ 	

#include "all.h" 
#include "sim800c.h"

//GSM��״̬
#define GSM_OFFLINE 0//gsmģ��Ͽ�������
#define GSM_FINDING 1//������
#define GSM_ONLINE  2//gsmģ���Ѿ�ע�����磬������������״̬

void GSMConnect_INIT(void);
u8 checkSIMsertStatus(void);
u8 check_ATcommand(void);
u8 CIPRXGET_Command(void);
u8 Return_Signal(void);
u8 set_GSMconfiguration(void);
u8 gsm_gprs_init(void);
u8 GetSimAttachGprs(void);
u8 WriteApnName(u8 *name);
u8 StartApnGprs(void);
u8 IsGetDnsIp(void);
u8 http_init(void);
u8 GetCurrGprsStatus(void);
void GetDNS(void);
u8 TCPConnect(char* IP_Add,char* Port_Add);
void NetConnect(u8 TCPIP_INIT);
u8 SetMe909SleepTime(void);	
u8 ResetMe909(void);
u8 WaitCGREG(void);
u8 IPINIT(void);
u8 CheckIPINIT(void);
	
#endif

