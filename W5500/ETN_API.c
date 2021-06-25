#include "ETN_api.h" 

void ETN_Find_FlagBitforBuff(void)
{
		if(strsearchforGSMUART_buff("/getflag/result")!=0)
		{
				printf("\r\nStart FindFlagBit\r\n");
				Find_ETN_FlagBit(0);
				MQTT_ETN_Sendpuback();
				printf("\r\nEnd FindFlagBit\r\n");
		}
}

//*************************************************
//����:���GSM�������黺��
void clear_ETNbuff(void)
{
		u16 i=0;
		for(i=0;i<MAX_GSMUART;i++)GSMUART_buff[i]=0;
}

//�β�Ҫ���������
void ETNNetConnect(u8 NetConnectStatus)
{
		#define ResetSystem 0//����ϵͳ
		#define MQTT_Connect_INIT 1//��ʼ��MQTT����
		#define MQTT_SubscribeTopic 2//����MQQTT����
		#define Net_Finish 3//��������ʼ��
		u8 sign_buf[64]={0},Topic_buf[64]={0},ID_buf[11]={0};
		u8 MQTT_Connect_INIT_status=0,MQTT_SubscribeTopic_status=0;
		u16 once_rand=0;
		ETN_NETstatus=0;
		while(1)
		{
				switch(NetConnectStatus)
				{
					case ResetSystem:
							UART_string_newline("��λ",Debug_Uart);
							delay_ms(10);
					    LED_SystemSelf();
							Soft_SystemReset();
							break;
					case MQTT_Connect_INIT:
							UART_string_newline("����MQTT",Debug_Uart);
							once_rand=system_rand;//��ȡ��ʱϵͳ�����
							strcat((char*)sign_buf,(char*)login_password);//��������
							strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
							MD5(sign_buf);//����MD5
							sprintf((char*)ID_buf,"%d",HTTP_PCB_ID);
							MQTT_Connect_INIT_status=Connect_ETN_MQTT((char*)ID_buf,(char*)U16IntToStr(once_rand),MD5_BUF);
							if(MQTT_Connect_INIT_status==0)
		              NetConnectStatus=ResetSystem;
							else									
									NetConnectStatus=MQTT_SubscribeTopic;
							break;
					case MQTT_SubscribeTopic:
							UART_string_newline("����MQTT����",Debug_Uart);
							sprintf((char*)ID_buf,"%d",HTTP_PCB_ID);  //HTTP_PCB_ID
							strcat((char*)Topic_buf,"client/");//��������
							strcat((char*)Topic_buf,(char*)ID_buf);//��������
							strcat((char*)Topic_buf,"/#");//��������
							MQTT_SubscribeTopic_status=Subscribe_ETN_Topic((char*)Topic_buf,1);
							if(MQTT_SubscribeTopic_status==0)
									NetConnectStatus=ResetSystem;
							else
									NetConnectStatus=Net_Finish;
							break;
					case Net_Finish:
							UART_string_newline("�����ʼ�����",Debug_Uart);
							ETN_NETstatus=1;
							return;
				}
				delay_ms(10);
		}	
}

u8 Connect_ETN_MQTT(char *client_id,char *username,char *password)
{
		u8 message_buffer[100]={0},ConnectStatua=0,Connect_flag =0;//MQTT��Ϣ������
		u16 MQTTlenbuffer=0;
		MQTTlenbuffer=mqtt_connect_message(message_buffer,client_id,username,password);
		if(MQTTlenbuffer>0)
	  Connect_flag=send(SOCK_TCPC,message_buffer,MQTTlenbuffer);
    if(Connect_flag)
		{
				printf("MQTT���Ӱ����ͳɹ�\r\n");
				ConnectStatua=1;	
		}
		return ConnectStatua;
}

u8 Subscribe_ETN_Topic(char* TopicBuffer,char qos)
{
		u8 message_buffer[50]={0},SubscribeTopicstatus=0,Connect_flag = 0;//MQTT��Ϣ������
		u16 MQTTlenbuffer=0;
		MQTTlenbuffer=mqtt_subscribe_message(message_buffer,TopicBuffer,qos,1);//����test����
		if(MQTTlenbuffer>0)
	  Connect_flag=send(SOCK_TCPC,message_buffer,MQTTlenbuffer);
		if(Connect_flag)	 
		{
				SubscribeTopicstatus =1;
				printf("��������ɹ�\r\n");
		}	
		return SubscribeTopicstatus;
}

void MQTT_ETN_PING(void)
{
		u8 Sendstatus=0;
		u8 message_buffer[16]={0};//MQTT��Ϣ������
		u16 MQTTlenbuffer=0;
		while(Sendstatus==0)
		{
				MQTTlenbuffer=MQTTSerialize_pingreq(message_buffer,16);//������
        Sendstatus =send(SOCK_TCPC,message_buffer,MQTTlenbuffer);
        if(Sendstatus)
						break;
				else 
						ETNNetConnect(0);
		}
		UART_string_newline("�������ѷ���",Debug_Uart);
}

void MQTT_ETN_Sendpuback(void)
{
		u16 mqttlen=0,MQTTSendDatastatus=0;
		u8 MQTT_msg[32]={0};//MQTT��Ϣ������
		u16 packetid=0;
		clear_ETNbuff();
		while(1)
		{
				packetid=ReturnPacketid("/getconfig/result");
				mqttlen=MQTTSerialize_puback(MQTT_msg,32,packetid);//puback	
				MQTTSendDatastatus =send(SOCK_TCPC,MQTT_msg,mqttlen);
        if(MQTTSendDatastatus)
						break;
				else 
						ETNNetConnect(0);
		}
		UART_string_newline("AlreadySendPuback",Debug_Uart);
}

//modeΪ0Ϊ���޷���������ֵ
void Find_ETN_FlagBit(u8 mode)
{
		u16 posflag=0;
		u8 FlagbitBuffer[16]={0};//��־λ����
		u8 Queue_BUFF=0,FlagBITcnt=99;
		BaseType_t err;
		posflag=strsearchforGSMUART_buff("FlagBit\":\"");//��ȡ�ؼ���λ��
		if(posflag)
		{
				clear_FlagBIT();
				CoypArraryAtoArraryB_LR(GSMUART_buff,FlagbitBuffer,16,posflag+10-1);//����Ҫ��ȡ�Ĳ�������
				UART_string_newline(FlagbitBuffer,Debug_Uart);
				//��0λ����λ	��1λ���Լ�	��2λ����������
				//��3λ���޸�̽��������ֵ	��4λ�����͵�ǰ̽����������Ϣ
				//��5λ����������ҪPCBA�ӷ�������ȡ������Ϣ��Ȼ�����̽��������
				if(FlagbitBuffer[0]=='1')//����������λ
				{
						FlagBITcnt=0;
						FlagBIT_BUF[0]='1';
						Queue_BUFF=W_DETE_Reset;
						err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//̽������λ
						UART_string_newline("�յ�ResetSignal!!!",Debug_Uart);
				}		
			
				if(FlagbitBuffer[1]=='1')//�����������Լ�
				{
						FlagBITcnt=1;
						FlagBIT_BUF[1]='1';
						Queue_BUFF=Self_System;
						err=xQueueSendToFront(ASSIT_Message_Queue,&Queue_BUFF,portMAX_DELAY);//ϵͳ�Լ�
						//Queue_BUFF=W_DETE_Self;
						//err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//̽�����Լ�
						UART_string_newline("�յ�SelfSignal!!!",Debug_Uart);
				}
				if(FlagbitBuffer[2]=='1')//�������������ݸ���
				{
						Queue_BUFF=ETN_MQTT_SendDate;
						err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);//�ϴ�����
						UART_string_newline("�յ�UpdataSignal!!!",Debug_Uart);
				}
				if(FlagbitBuffer[3]=='1'||FlagbitBuffer[5]=='1')//�������е������޸Ļ�������Ϣ���޸�
				{
						FlagBITcnt=3+5;
						FlagBIT_BUF[3]='1';
						FlagBIT_BUF[5]='1';
						if(mode==1)
						{
								//Analyse_Data();//GET������Ϣ
								//Queue_BUFF=W_DETE_Alarming_Value;
								//err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//�޸�̽��������ֵ
								UART_string_newline("�յ�ConfigSignal!!!",Debug_Uart);
						}
						else if(mode==0)
						{
								Queue_BUFF=ETN_MQTT_GetConfig;
								err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);//��ȥ��ȡ������Ϣ
								UART_string_newline("��ȥ��ȡ������Ϣ",Debug_Uart);
						}
				}
				if(FlagbitBuffer[4]=='1')//���·���������
				{
						FlagBITcnt=4;
						FlagBIT_BUF[4]='1';
						Queue_BUFF=ETN_MQTT_UpdataConfig;
						err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);//�޸�̽��������ֵ
						UART_string_newline("�յ�UpConfigSignal!!!",Debug_Uart);
				}
				if(FlagBITcnt!=99)
				{
						UART_string_newline("�����������",Debug_Uart);
						Queue_BUFF=ETN_MQTT_ClearFlagBit;
						err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);
				}
		}

}

void DH_MQTT_ETN_SendDTETData(char Nmode)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[650]={0};//MQTT��Ϣ������
		char jsonbuf[630]={0};
		char prodatabuf[500]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
				DH_UpDETEdatatoSend();
				ProbeData_DH(prodatabuf);
				
				JsonData_1(jsonbuf,UpdataJsonData(Nmode,1,prodatabuf,once_rand),0);
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 650, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
			  UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<10;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										printf("�������ݳɹ�\r\n");
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
										printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				break;
		}
}

void CET_MQTT_ETN_SendDTETData_1(char Nmode,u8 GSMTCBbit)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT��Ϣ������
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
//				if(Nmode==0)//������ȫ��һ��
//				{
						//if(Sys_status.Light_Sys_S== Status_Normal||RandomData == 1)//��װlight
						{ProbeData_light(prodatabuf);strcat(prodatabuf,",");}
						//if(Sys_status.UPS_Sys_S== Status_Normal||RandomData == 1)//��װUPS
						{ProbeData_ups(prodatabuf);strcat(prodatabuf,",");}
						//if(Sys_status.StabPow_Sys_S== Status_Normal||RandomData == 1)//��װRPS
						{ProbeData_RPS(prodatabuf);strcat(prodatabuf,",");}
						//if(Sys_status.Main_Sys_S== Status_Normal||RandomData == 1)//��װ�������ϵͳ
						{ProbeData_PGS(prodatabuf);strcat(prodatabuf,",");ProbeData_ES(prodatabuf);strcat(prodatabuf,",");}
						//if(IAS_s[0].IAS_CS== Status_Normal||RandomData == 1)//��װ��·1
						{ProbeData_IAS(prodatabuf,0);strcat(prodatabuf,",");}
						//if(IAS_s[1].IAS_CS== Status_Normal||RandomData == 1)//��װ��·2
						ProbeData_IAS(prodatabuf,1);
//				}
//				else//������ʱ�򣬲������Ĳ���
//				{
//						if((CET_OnlineStatus&0x10)==0x10&&LightAlarm==1)//��װlight
//						ProbeData_light(prodatabuf);
//						if((CET_OnlineStatus&0x02)==0x02&&UPSAlarm==1)//��װUPS
//						ProbeData_ups(prodatabuf);
//						if((CET_OnlineStatus&0x04)==0x04&&RPSAlarm==1)//��װRPS
//						ProbeData_RPS(prodatabuf);
//						if((CET_OnlineStatus&0x01)==0x01&&PGSAlarm==1)//��װ�������ϵͳ
//						{ProbeData_PGS(prodatabuf);ProbeData_ES(prodatabuf);}
//						if((CET_OnlineStatus_IAS&0x0001)==0x0001&&IAS01Alarm==1)//��װ��·1
//						ProbeData_IAS(prodatabuf,0);
//						if((CET_OnlineStatus_IAS&0x0002)==0x0002&&IAS02Alarm==1)//��װ��·2
//						ProbeData_IAS(prodatabuf,1);
//				}
				
				JsonData_1(jsonbuf,UpdataJsonData(0,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1300, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendDTETData_IAS_03_06(char Nmode,u8 GSMTCBbit)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT��Ϣ������
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
//				if(Nmode==0)
//				{
						//if(IAS_s[2].IAS_CS== Status_Normal||RandomData == 1)//��װ��·3
						{ProbeData_IAS(prodatabuf,2);strcat(prodatabuf,",");}
						//if(IAS_s[3].IAS_CS== Status_Normal||RandomData == 1)//��װ��·4
						{ProbeData_IAS(prodatabuf,3);strcat(prodatabuf,",");}
						//if(IAS_s[4].IAS_CS== Status_Normal||RandomData == 1)//��װ��·5
						{ProbeData_IAS(prodatabuf,4);strcat(prodatabuf,",");}
						//if(IAS_s[5].IAS_CS== Status_Normal||RandomData == 1)//��װ��·6
						ProbeData_IAS(prodatabuf,5);
//				}
//				else
//				{
//						if((CET_OnlineStatus_IAS&0x0004)==0x0004&&IAS03Alarm==1)//��װ��·3
//						ProbeData_IAS(prodatabuf,2);
//						if((CET_OnlineStatus_IAS&0x0008)==0x0008&&IAS04Alarm==1)//��װ��·4
//						ProbeData_IAS(prodatabuf,3);
//						if((CET_OnlineStatus_IAS&0x0010)==0x0010&&IAS05Alarm==1)//��װ��·5
//						ProbeData_IAS(prodatabuf,4);
//						if((CET_OnlineStatus_IAS&0x0020)==0x0020&&IAS06Alarm==1)//��װ��·6
//						ProbeData_IAS(prodatabuf,5);
//				}
					
				JsonData_1(jsonbuf,UpdataJsonData(0,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1300, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendDTETData_IAS_IAS_07_10(char Nmode,u8 GSMTCBbit)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT��Ϣ������
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
//				if(Nmode==0)
//				{
						//if(IAS_s[6].IAS_CS== Status_Normal||RandomData == 1)//��װ��·7
						{ProbeData_IAS(prodatabuf,6);strcat(prodatabuf,",");}
						//if(IAS_s[7].IAS_CS== Status_Normal||RandomData == 1)//��װ��·8
						{ProbeData_IAS(prodatabuf,7);strcat(prodatabuf,",");}
					///	if(IAS_s[8].IAS_CS== Status_Normal||RandomData == 1)//��װ��·9
						{ProbeData_IAS(prodatabuf,8);strcat(prodatabuf,",");}
						//if(IAS_s[9].IAS_CS== Status_Normal||RandomData == 1)//��װ��·10
						ProbeData_IAS(prodatabuf,9);
//				}
//				else
//				{
//						if((CET_OnlineStatus_IAS&0x0040)==0x0040&&IAS07Alarm==1)//��װ��·7
//						ProbeData_IAS(prodatabuf,6);
//						if((CET_OnlineStatus_IAS&0x0080)==0x0080&&IAS08Alarm==1)//��װ��·8
//						ProbeData_IAS(prodatabuf,7);
//						if((CET_OnlineStatus_IAS&0x0100)==0x0100&&IAS09Alarm==1)//��װ��·9
//						ProbeData_IAS(prodatabuf,8);
//						if((CET_OnlineStatus_IAS&0x0200)==0x0200&&IAS10Alarm==1)//��װ��·10
//						ProbeData_IAS(prodatabuf,9);
//				}
					
				JsonData_1(jsonbuf,UpdataJsonData(0,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1300, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendDTETData_IAS_Alarm(char Nmode,u8 GSMTCBbit)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[356]={0};//MQTT��Ϣ������
		char jsonbuf[300]={0};
		char prodatabuf[200]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
				ProbeData_Alarm(prodatabuf);
				JsonData_1(jsonbuf,UpdataJsonData(Nmode,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 356, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<356;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<300;i++)jsonbuf[i]=0;
				for(i=0;i<200;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_ENT_MQTT_GetFlagData(void)
{
		static u8 Losed_Connect= 0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		u16 mqttlen=0,i=0;
		u8 MQTTSendDatastatus=0;
		u8 FlagBitMQTT_msg[50]={0};//MQTT��Ϣ������
		char jsonbuf[16]={0};
		char Topic_buf[64]={0};
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/Getflag");//��������
				topicString.cstring = Topic_buf;
				mqttlen=MQTTSerialize_publish(FlagBitMQTT_msg, 50, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				MQTTSendDatastatus =send(SOCK_TCPC,FlagBitMQTT_msg,mqttlen);
				if(MQTTSendDatastatus)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)break;
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								if(strsearchforGSMUART_buff("FlagBit")!=0)
								{
										printf("��ȡ��־λ�ɹ�\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Find_FlagBit(0);
										break;
								}
								else
								{
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("��ȡ��־λ����!!!",Debug_Uart);
										ETNNetConnect(0);
								}
						}
				}
				else
				{	
					  printf("���ͻ�ȡ���ô���\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<50;i++)FlagBitMQTT_msg[i]=0;
				for(i=0;i<16;i++)jsonbuf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_ETN_MQTT_GetConfighexData(void)
{
		static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		char jsonbuf[16]={0};
		u8 MQTT_msg[50]={0};//MQTT��Ϣ������
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/Getconfighex");//��������
				topicString.cstring = Topic_buf;
			
				mqttlen=MQTTSerialize_publish(MQTT_msg, 50, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				test_status =send(SOCK_TCPC,MQTT_msg,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								if(strsearchforGSMUART_buff("result")!=0)
								{
										printf("��ȡ���óɹ�\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Analyse_ConfigHexData();
										break;
								}
								else
								{
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("��ȡ���ô���!!!",Debug_Uart);
										ETNNetConnect(0);
								}
						}
				}
				else
				{	
					  printf("���ͻ�ȡ���ô���\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<50;i++)MQTT_msg[i]=0;
				for(i=0;i<16;i++)jsonbuf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendFlagBitData(void)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[150]={0};//MQTT��Ϣ������
		char jsonbuf[100]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
				JsonData_Flagbit(jsonbuf,UpdataJsonDataFlagbit(once_rand));
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 150, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<150;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<100;i++)jsonbuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendProbeConfig_PGS_ES_IAS01_05(void)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT��Ϣ������
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
				ProbeConfig_PGS_ES(prodatabuf);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,0);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,1);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,2);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,3);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,4);
				
				JsonData_ProbeConfig(jsonbuf,UpdataJsonDataProbeConfig(once_rand,prodatabuf));
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				//strcat((char*)Topic_buf,"/UpdateConfig");//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1300, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
										printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendProbeConfig_IAS06_10(void)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT��Ϣ������
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
				ProbeConfig_IAS(prodatabuf,5);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,6);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,7);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,8);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,9);
				
				JsonData_ProbeConfig(jsonbuf,UpdataJsonDataProbeConfig(once_rand,prodatabuf));
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				//strcat((char*)Topic_buf,"/UpdateConfig");//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1300, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
										printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_MQTT_ETN_SendPushConfig(void)
{
	  static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[600]={0};//MQTT��Ϣ������
		char jsonbuf[550]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		Once_JsonDataPushConfig JsonDataPushConfig;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//��ȡ��ʱϵͳ�����
				strcat((char*)sign_buf,(char*)login_password);//��������
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
				MD5(sign_buf);//����MD5
				
				JsonDataPushConfig.SignBuf=MD5_BUF;
				JsonDataPushConfig.OnceBuf=(char*)U16IntToStr(once_rand);
				JsonDataPushConfig.ID_Buf=PCB_ID_STR;
				JsonData_PushConfig(jsonbuf,JsonDataPushConfig);
				
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/push");//��������
				topicString.cstring = Topic_buf;		
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 600, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
        UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				test_status =send(SOCK_TCPC,MQTT_msgBuffer,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										ETN_Find_FlagBitforBuff();	
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
										printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								printf("�������ݳɹ�\r\n");
								break;
						}
				}
				else
				{	
					  printf("��������ʧ��\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<600;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<550;i++)jsonbuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void CET_ETN_MQTT_GetFlagData(void)
{
		static u8 Losed_Connect= 0;
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//����packet������
		char Topic_buf[64]={0};
		char jsonbuf[16]={0};
		u8 MQTT_msg[50]={0};//MQTT��Ϣ������
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				strcat((char*)Topic_buf,"data/");//��������
				strcat((char*)Topic_buf,PCB_ID_STR);//��������
				strcat((char*)Topic_buf,"/Getflag");//��������
				topicString.cstring = Topic_buf;
			
				mqttlen=MQTTSerialize_publish(MQTT_msg, 50, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				test_status =send(SOCK_TCPC,MQTT_msg,mqttlen);
				if(test_status)	
				{			
						for(i=0;i<ETNSendWaitTime;i++)
						{
								delay_ms(100);
								GetRecv();
								if(strsearchforGSMUART_buff(packetidbuf)!=0)
								{
										break;
								}
						}
						if(i==10)
						{
							 printf("���Pubackʧ��\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket���ӷ�����*/ 
									 {
											printf("����TCPIP�ɹ�\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("����TCPIPʧ��\r\n");
								 	 }
							 }
							 else
							 {
								  Losed_Connect=0;
							 	  ETNNetConnect(0);
							 }
						}
						else
						{
								if(strsearchforGSMUART_buff("result")!=0)
								{
										printf("��ȡ��־λ�ɹ�\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Find_ETN_FlagBit(0);
										break;
								}
								else
								{
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("��ȡ��־λ����!!!",Debug_Uart);
										ETNNetConnect(0);
								}
						}
				}
				else
				{	
					  printf("���ͻ�ȡ��־λ����\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<50;i++)MQTT_msg[i]=0;
				for(i=0;i<16;i++)jsonbuf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

