#ifndef __ETN_API_H
#define __ETN_API_H 	

#include "all.h" 

#define ETNSendWaitTime 10

void clear_ETNbuff(void);
void ETNNetConnect(u8 NetConnectStatus);
u8 Connect_ETN_MQTT(char *client_id,char *username,char *password);
u8 Subscribe_ETN_Topic(char* TopicBuffer,char qos);
void MQTT_ETN_PING(void);

void MQTT_ETN_Sendpuback(void);
void Find_ETN_FlagBit(u8 mode);

void DH_MQTT_ETN_SendDTETData(char Nmode);

void CET_MQTT_ETN_SendDTETData_1(char Nmode,u8 GSMTCBbit);
void CET_MQTT_ETN_SendDTETData_IAS_03_06(char Nmode,u8 GSMTCBbit);
void CET_MQTT_ETN_SendDTETData_IAS_IAS_07_10(char Nmode,u8 GSMTCBbit);
void CET_MQTT_ETN_SendDTETData_IAS_Alarm(char Nmode,u8 GSMTCBbit);
void CET_MQTT_ETN_SendFlagBitData(void);
void CET_MQTT_ETN_SendProbeConfig_PGS_ES_IAS01_05(void);
void CET_MQTT_ETN_SendProbeConfig_IAS06_10(void);

void CET_MQTT_ETN_SendPushConfig(void);
void CET_ETN_MQTT_GetConfighexData(void);
void CET_ENT_MQTT_GetFlagData(void);
void ETN_Find_FlagBitforBuff(void);
void CET_ETN_MQTT_GetFlagData(void);

#endif

