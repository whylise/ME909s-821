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
//描述:清除GSM串口数组缓存
void clear_ETNbuff(void)
{
		u16 i=0;
		for(i=0;i<MAX_GSMUART;i++)GSMUART_buff[i]=0;
}

//形参要视情况而入
void ETNNetConnect(u8 NetConnectStatus)
{
		#define ResetSystem 0//重启系统
		#define MQTT_Connect_INIT 1//初始化MQTT连接
		#define MQTT_SubscribeTopic 2//订阅MQQTT主题
		#define Net_Finish 3//完成网络初始化
		u8 sign_buf[64]={0},Topic_buf[64]={0},ID_buf[11]={0};
		u8 MQTT_Connect_INIT_status=0,MQTT_SubscribeTopic_status=0;
		u16 once_rand=0;
		ETN_NETstatus=0;
		while(1)
		{
				switch(NetConnectStatus)
				{
					case ResetSystem:
							UART_string_newline("软复位",Debug_Uart);
							delay_ms(10);
					    LED_SystemSelf();
							Soft_SystemReset();
							break;
					case MQTT_Connect_INIT:
							UART_string_newline("连接MQTT",Debug_Uart);
							once_rand=system_rand;//获取此时系统随机数
							strcat((char*)sign_buf,(char*)login_password);//数组连接
							strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
							MD5(sign_buf);//计算MD5
							sprintf((char*)ID_buf,"%d",HTTP_PCB_ID);
							MQTT_Connect_INIT_status=Connect_ETN_MQTT((char*)ID_buf,(char*)U16IntToStr(once_rand),MD5_BUF);
							if(MQTT_Connect_INIT_status==0)
		              NetConnectStatus=ResetSystem;
							else									
									NetConnectStatus=MQTT_SubscribeTopic;
							break;
					case MQTT_SubscribeTopic:
							UART_string_newline("订阅MQTT主题",Debug_Uart);
							sprintf((char*)ID_buf,"%d",HTTP_PCB_ID);  //HTTP_PCB_ID
							strcat((char*)Topic_buf,"client/");//数组连接
							strcat((char*)Topic_buf,(char*)ID_buf);//数组连接
							strcat((char*)Topic_buf,"/#");//数组连接
							MQTT_SubscribeTopic_status=Subscribe_ETN_Topic((char*)Topic_buf,1);
							if(MQTT_SubscribeTopic_status==0)
									NetConnectStatus=ResetSystem;
							else
									NetConnectStatus=Net_Finish;
							break;
					case Net_Finish:
							UART_string_newline("网络初始化完成",Debug_Uart);
							ETN_NETstatus=1;
							return;
				}
				delay_ms(10);
		}	
}

u8 Connect_ETN_MQTT(char *client_id,char *username,char *password)
{
		u8 message_buffer[100]={0},ConnectStatua=0,Connect_flag =0;//MQTT消息包缓存
		u16 MQTTlenbuffer=0;
		MQTTlenbuffer=mqtt_connect_message(message_buffer,client_id,username,password);
		if(MQTTlenbuffer>0)
	  Connect_flag=send(SOCK_TCPC,message_buffer,MQTTlenbuffer);
    if(Connect_flag)
		{
				printf("MQTT连接包发送成功\r\n");
				ConnectStatua=1;	
		}
		return ConnectStatua;
}

u8 Subscribe_ETN_Topic(char* TopicBuffer,char qos)
{
		u8 message_buffer[50]={0},SubscribeTopicstatus=0,Connect_flag = 0;//MQTT消息包缓存
		u16 MQTTlenbuffer=0;
		MQTTlenbuffer=mqtt_subscribe_message(message_buffer,TopicBuffer,qos,1);//订阅test主题
		if(MQTTlenbuffer>0)
	  Connect_flag=send(SOCK_TCPC,message_buffer,MQTTlenbuffer);
		if(Connect_flag)	 
		{
				SubscribeTopicstatus =1;
				printf("订阅主题成功\r\n");
		}	
		return SubscribeTopicstatus;
}

void MQTT_ETN_PING(void)
{
		u8 Sendstatus=0;
		u8 message_buffer[16]={0};//MQTT消息包缓存
		u16 MQTTlenbuffer=0;
		while(Sendstatus==0)
		{
				MQTTlenbuffer=MQTTSerialize_pingreq(message_buffer,16);//心跳包
        Sendstatus =send(SOCK_TCPC,message_buffer,MQTTlenbuffer);
        if(Sendstatus)
						break;
				else 
						ETNNetConnect(0);
		}
		UART_string_newline("心跳包已发送",Debug_Uart);
}

void MQTT_ETN_Sendpuback(void)
{
		u16 mqttlen=0,MQTTSendDatastatus=0;
		u8 MQTT_msg[32]={0};//MQTT消息包缓存
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

//mode为0为，无法解析门限值
void Find_ETN_FlagBit(u8 mode)
{
		u16 posflag=0;
		u8 FlagbitBuffer[16]={0};//标志位缓存
		u8 Queue_BUFF=0,FlagBITcnt=99;
		BaseType_t err;
		posflag=strsearchforGSMUART_buff("FlagBit\":\"");//获取关键字位置
		if(posflag)
		{
				clear_FlagBIT();
				CoypArraryAtoArraryB_LR(GSMUART_buff,FlagbitBuffer,16,posflag+10-1);//复制要获取的部分数组
				UART_string_newline(FlagbitBuffer,Debug_Uart);
				//第0位：复位	第1位：自检	第2位：更新数据
				//第3位：修改探测器报警值	第4位：发送当前探测器配置信息
				//第5位：服务器需要PCBA从服务器读取配置信息，然后更新探测器配置
				if(FlagbitBuffer[0]=='1')//服务器请求复位
				{
						FlagBITcnt=0;
						FlagBIT_BUF[0]='1';
						Queue_BUFF=W_DETE_Reset;
						err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//探测器复位
						UART_string_newline("收到ResetSignal!!!",Debug_Uart);
				}		
			
				if(FlagbitBuffer[1]=='1')//服务器请求自检
				{
						FlagBITcnt=1;
						FlagBIT_BUF[1]='1';
						Queue_BUFF=Self_System;
						err=xQueueSendToFront(ASSIT_Message_Queue,&Queue_BUFF,portMAX_DELAY);//系统自检
						//Queue_BUFF=W_DETE_Self;
						//err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//探测器自检
						UART_string_newline("收到SelfSignal!!!",Debug_Uart);
				}
				if(FlagbitBuffer[2]=='1')//服务器请求数据更新
				{
						Queue_BUFF=ETN_MQTT_SendDate;
						err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);//上传数据
						UART_string_newline("收到UpdataSignal!!!",Debug_Uart);
				}
				if(FlagbitBuffer[3]=='1'||FlagbitBuffer[5]=='1')//服务器中的门限修改或配置信息有修改
				{
						FlagBITcnt=3+5;
						FlagBIT_BUF[3]='1';
						FlagBIT_BUF[5]='1';
						if(mode==1)
						{
								//Analyse_Data();//GET配置信息
								//Queue_BUFF=W_DETE_Alarming_Value;
								//err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//修改探测器门限值
								UART_string_newline("收到ConfigSignal!!!",Debug_Uart);
						}
						else if(mode==0)
						{
								Queue_BUFF=ETN_MQTT_GetConfig;
								err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);//先去获取配置信息
								UART_string_newline("先去获取配置信息",Debug_Uart);
						}
				}
				if(FlagbitBuffer[4]=='1')//更新服务器配置
				{
						FlagBITcnt=4;
						FlagBIT_BUF[4]='1';
						Queue_BUFF=ETN_MQTT_UpdataConfig;
						err=xQueueSendToFront(ETN_Message_Queue,&Queue_BUFF,portMAX_DELAY);//修改探测器门限值
						UART_string_newline("收到UpConfigSignal!!!",Debug_Uart);
				}
				if(FlagBITcnt!=99)
				{
						UART_string_newline("发送清除任务",Debug_Uart);
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[650]={0};//MQTT消息包缓存
		char jsonbuf[630]={0};
		char prodatabuf[500]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
				DH_UpDETEdatatoSend();
				ProbeData_DH(prodatabuf);
				
				JsonData_1(jsonbuf,UpdataJsonData(Nmode,1,prodatabuf,once_rand),0);
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
										printf("发送数据成功\r\n");
										break;
								}
						}
						if(i==10)
						{
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
										printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
//				if(Nmode==0)//不报警全部一起发
//				{
						//if(Sys_status.Light_Sys_S== Status_Normal||RandomData == 1)//安装light
						{ProbeData_light(prodatabuf);strcat(prodatabuf,",");}
						//if(Sys_status.UPS_Sys_S== Status_Normal||RandomData == 1)//安装UPS
						{ProbeData_ups(prodatabuf);strcat(prodatabuf,",");}
						//if(Sys_status.StabPow_Sys_S== Status_Normal||RandomData == 1)//安装RPS
						{ProbeData_RPS(prodatabuf);strcat(prodatabuf,",");}
						//if(Sys_status.Main_Sys_S== Status_Normal||RandomData == 1)//安装电力监测系统
						{ProbeData_PGS(prodatabuf);strcat(prodatabuf,",");ProbeData_ES(prodatabuf);strcat(prodatabuf,",");}
						//if(IAS_s[0].IAS_CS== Status_Normal||RandomData == 1)//安装回路1
						{ProbeData_IAS(prodatabuf,0);strcat(prodatabuf,",");}
						//if(IAS_s[1].IAS_CS== Status_Normal||RandomData == 1)//安装回路2
						ProbeData_IAS(prodatabuf,1);
//				}
//				else//报警的时候，不报警的不发
//				{
//						if((CET_OnlineStatus&0x10)==0x10&&LightAlarm==1)//安装light
//						ProbeData_light(prodatabuf);
//						if((CET_OnlineStatus&0x02)==0x02&&UPSAlarm==1)//安装UPS
//						ProbeData_ups(prodatabuf);
//						if((CET_OnlineStatus&0x04)==0x04&&RPSAlarm==1)//安装RPS
//						ProbeData_RPS(prodatabuf);
//						if((CET_OnlineStatus&0x01)==0x01&&PGSAlarm==1)//安装电力监测系统
//						{ProbeData_PGS(prodatabuf);ProbeData_ES(prodatabuf);}
//						if((CET_OnlineStatus_IAS&0x0001)==0x0001&&IAS01Alarm==1)//安装回路1
//						ProbeData_IAS(prodatabuf,0);
//						if((CET_OnlineStatus_IAS&0x0002)==0x0002&&IAS02Alarm==1)//安装回路2
//						ProbeData_IAS(prodatabuf,1);
//				}
				
				JsonData_1(jsonbuf,UpdataJsonData(0,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
//				if(Nmode==0)
//				{
						//if(IAS_s[2].IAS_CS== Status_Normal||RandomData == 1)//安装回路3
						{ProbeData_IAS(prodatabuf,2);strcat(prodatabuf,",");}
						//if(IAS_s[3].IAS_CS== Status_Normal||RandomData == 1)//安装回路4
						{ProbeData_IAS(prodatabuf,3);strcat(prodatabuf,",");}
						//if(IAS_s[4].IAS_CS== Status_Normal||RandomData == 1)//安装回路5
						{ProbeData_IAS(prodatabuf,4);strcat(prodatabuf,",");}
						//if(IAS_s[5].IAS_CS== Status_Normal||RandomData == 1)//安装回路6
						ProbeData_IAS(prodatabuf,5);
//				}
//				else
//				{
//						if((CET_OnlineStatus_IAS&0x0004)==0x0004&&IAS03Alarm==1)//安装回路3
//						ProbeData_IAS(prodatabuf,2);
//						if((CET_OnlineStatus_IAS&0x0008)==0x0008&&IAS04Alarm==1)//安装回路4
//						ProbeData_IAS(prodatabuf,3);
//						if((CET_OnlineStatus_IAS&0x0010)==0x0010&&IAS05Alarm==1)//安装回路5
//						ProbeData_IAS(prodatabuf,4);
//						if((CET_OnlineStatus_IAS&0x0020)==0x0020&&IAS06Alarm==1)//安装回路6
//						ProbeData_IAS(prodatabuf,5);
//				}
					
				JsonData_1(jsonbuf,UpdataJsonData(0,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
//				if(Nmode==0)
//				{
						//if(IAS_s[6].IAS_CS== Status_Normal||RandomData == 1)//安装回路7
						{ProbeData_IAS(prodatabuf,6);strcat(prodatabuf,",");}
						//if(IAS_s[7].IAS_CS== Status_Normal||RandomData == 1)//安装回路8
						{ProbeData_IAS(prodatabuf,7);strcat(prodatabuf,",");}
					///	if(IAS_s[8].IAS_CS== Status_Normal||RandomData == 1)//安装回路9
						{ProbeData_IAS(prodatabuf,8);strcat(prodatabuf,",");}
						//if(IAS_s[9].IAS_CS== Status_Normal||RandomData == 1)//安装回路10
						ProbeData_IAS(prodatabuf,9);
//				}
//				else
//				{
//						if((CET_OnlineStatus_IAS&0x0040)==0x0040&&IAS07Alarm==1)//安装回路7
//						ProbeData_IAS(prodatabuf,6);
//						if((CET_OnlineStatus_IAS&0x0080)==0x0080&&IAS08Alarm==1)//安装回路8
//						ProbeData_IAS(prodatabuf,7);
//						if((CET_OnlineStatus_IAS&0x0100)==0x0100&&IAS09Alarm==1)//安装回路9
//						ProbeData_IAS(prodatabuf,8);
//						if((CET_OnlineStatus_IAS&0x0200)==0x0200&&IAS10Alarm==1)//安装回路10
//						ProbeData_IAS(prodatabuf,9);
//				}
					
				JsonData_1(jsonbuf,UpdataJsonData(0,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[356]={0};//MQTT消息包缓存
		char jsonbuf[300]={0};
		char prodatabuf[200]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
				ProbeData_Alarm(prodatabuf);
				JsonData_1(jsonbuf,UpdataJsonData(Nmode,1,prodatabuf,once_rand),GSMTCBbit);
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		u16 mqttlen=0,i=0;
		u8 MQTTSendDatastatus=0;
		u8 FlagBitMQTT_msg[50]={0};//MQTT消息包缓存
		char jsonbuf[16]={0};
		char Topic_buf[64]={0};
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/Getflag");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
										printf("获取标志位成功\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Find_FlagBit(0);
										break;
								}
								else
								{
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("获取标志位错误!!!",Debug_Uart);
										ETNNetConnect(0);
								}
						}
				}
				else
				{	
					  printf("发送获取配置错误\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		char jsonbuf[16]={0};
		u8 MQTT_msg[50]={0};//MQTT消息包缓存
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/Getconfighex");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
										printf("获取配置成功\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Analyse_ConfigHexData();
										break;
								}
								else
								{
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("获取配置错误!!!",Debug_Uart);
										ETNNetConnect(0);
								}
						}
				}
				else
				{	
					  printf("发送获取配置错误\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[150]={0};//MQTT消息包缓存
		char jsonbuf[100]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
				JsonData_Flagbit(jsonbuf,UpdataJsonDataFlagbit(once_rand));
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
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
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				//strcat((char*)Topic_buf,"/UpdateConfig");//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
										printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
				ProbeConfig_IAS(prodatabuf,5);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,6);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,7);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,8);strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,9);
				
				JsonData_ProbeConfig(jsonbuf,UpdataJsonDataProbeConfig(once_rand,prodatabuf));
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				//strcat((char*)Topic_buf,"/UpdateConfig");//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
										printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[600]={0};//MQTT消息包缓存
		char jsonbuf[550]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		Once_JsonDataPushConfig JsonDataPushConfig;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
				JsonDataPushConfig.SignBuf=MD5_BUF;
				JsonDataPushConfig.OnceBuf=(char*)U16IntToStr(once_rand);
				JsonDataPushConfig.ID_Buf=PCB_ID_STR;
				JsonData_PushConfig(jsonbuf,JsonDataPushConfig);
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
										printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
								printf("发送数据成功\r\n");
								break;
						}
				}
				else
				{	
					  printf("发送数据失败\r\n");
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
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		char jsonbuf[16]={0};
		u8 MQTT_msg[50]={0};//MQTT消息包缓存
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				clear_ETNbuff();
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/Getflag");//数组连接
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
							 printf("检测Puback失败\r\n");
						   Losed_Connect++;
							 if(Losed_Connect<=3)
							 {
									 if(connect(SOCK_TCPC,local_ip,FLahMqttPort))/*socket连接服务器*/ 
									 {
											printf("连接TCPIP成功\r\n");					
									 }
									 else
									 {
											Losed_Connect =0;
											ETNNetConnect(0);
											printf("连接TCPIP失败\r\n");
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
										printf("获取标志位成功\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Find_ETN_FlagBit(0);
										break;
								}
								else
								{
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("获取标志位错误!!!",Debug_Uart);
										ETNNetConnect(0);
								}
						}
				}
				else
				{	
					  printf("发送获取标志位错误\r\n");
						ETNNetConnect(0);
				}
				for(i=0;i<50;i++)MQTT_msg[i]=0;
				for(i=0;i<16;i++)jsonbuf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

