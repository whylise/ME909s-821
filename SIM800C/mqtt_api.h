#ifndef __MQTT_API_H
#define __MQTT_API_H 	

#include "all.h" 
#include "mqtt_api.h"

#define SendWaitTime 150

unsigned short ReturnPacketid(char *str);
void MQTT_Sendpuback(void);
void MQTT_PING(void);
u8 Subscribe_Topic(char* TopicBuffer,char qos);
u8 Connect_MQTT(char *client_id,char *username,char *password);
u16 mqtt_connect_message(u8 *mqtt_message,char *client_id,char *username,char *password);
u16 mqtt_subscribe_message(u8 *mqtt_message,char *topic,u8 qos,u8 whether);

void MQTT_SendTestUpdataData(void);
void MQTT_SendTestSendData(void);

void MQTT_GSMSendCETData_1(u8 Nmode,u8 GSMTCBbit);


void MQTT_GSMSendDHData_1(u8 Nmode);

void MQTT_GSMSendCETData_IAS_03_06(u8 Nmode,u8 GSMTCBbit);
void MQTT_GSMSendCETData_IAS_07_10(u8 Nmode,u8 GSMTCBbit);
void MQTT_GSMSendCETData_Alarm(u8 Nmode,u8 GSMTCBbit);

void MQTT_SendFlagBitData(void);
u8 MQTT_GetFlagData(void);

void MQTT_GSMSendCETProbeConfig_PGS_ES_IAS01_05(void);
void MQTT_GSMSendCETProbeConfig_IAS06_10(void);
void MQTT_SendPushConfig(void);

void MQTT_GetConfighexData(void);
void Find_FlagBitforBuff(void);

void GSM_Find_upgrade(void);
void analysis_upgrade(void);

#endif

