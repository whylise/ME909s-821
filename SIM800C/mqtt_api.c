#include "mqtt_api.h"

u8 LightAlarm=0;
u8 UPSAlarm=0;
u8 RPSAlarm=0;
u8 PGSAlarm=0;
u8 IAS01Alarm=0;
u8 IAS02Alarm=0;
u8 IAS03Alarm=0;
u8 IAS04Alarm=0;
u8 IAS05Alarm=0;
u8 IAS06Alarm=0;
u8 IAS07Alarm=0;
u8 IAS08Alarm=0;
u8 IAS09Alarm=0;
u8 IAS10Alarm=0;


void analysis_upgrade(void)
{
		u16 File_Total=0;
		u16 posid1=0,posid2=0,posid3=0;
		u16 updatabit=0,Vernum=0;
		u8 updatabitbuf[4]={0},Vernumbuf[8]={0},Filetotalbuf[8]={0};	
		posid1=HEXsearchforGSMUART_buff(">>>",3);
		posid2=HEXsearchforGSMUART_buff("<<<",3);
		posid3=HEXsearchforGSMUART_buff("@@@",3);
		CoypArraryAtoArraryB_LR(GSMUART_buff,updatabitbuf,1,posid1+2);
		CoypArraryAtoArraryB_LR(GSMUART_buff,Vernumbuf,posid3-posid2-3,posid2+2);
		updatabit=GSMatoi(updatabitbuf);	
		Vernum=GSMatoi(Vernumbuf);	
		posid1=HEXsearchforGSMUART_buff("Total\":",7);
		posid2=EndstrsearchGSMUART_buff("}");	
		CoypArraryAtoArraryB_LR(GSMUART_buff,Filetotalbuf,posid2-posid1-7,posid1+6);
		File_Total=GSMatoi(Filetotalbuf);	
		printf("updatabit:  %d\r\n",updatabit);
		printf("Vernum:  %d\r\n",Vernum);
		printf("File_Total:  %d\r\n",File_Total);
		if(updatabit==0)
		{
				FLASH_Unlock();
				MyEE_WriteWord(Vernum_Add,Vernum);
				MyEE_WriteWord(CheckUpdataAdd,updatabit);
				FLASH_Lock();
		}
		else if(updatabit==9)//暗命令复位
		{
				Soft_SystemReset();
		}
		else if(updatabit==8)//暗命令刷Flash的安装状态
		{
				FLASH_Unlock();
				MyEE_WriteWord(Elec_data_S_ELec_ES_Add,22);
				MyEE_WriteWord(Elec_data_S_ELec_PGS_Add,22);
				MyEE_WriteWord(Stabpow_S_StabPow_Con_S_Add,22);
				MyEE_WriteWord(UPS_power_S_UPS_Con_S_Add,22);
				MyEE_WriteWord(Intel_light_Intel_Light_S_Add,22);
				MyEE_WriteWord(IAS_s0_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s1_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s2_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s3_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s4_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s5_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s6_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s7_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s8_IAS_CS_Add,22);
				MyEE_WriteWord(IAS_s9_IAS_CS_Add,22);
				FLASH_Lock();
				Soft_SystemReset();
		}
		else
		{
				FLASH_Unlock();
				MyEE_WriteWord(Vernum_Add,Vernum);
				MyEE_WriteWord(FileNumAdd,File_Total);
				MyEE_WriteWord(CheckUpdataAdd,updatabit);
				FLASH_Lock();
				Soft_SystemReset();
		}
}

void GSM_Find_upgrade(void)
{
		if(strsearchforGSMUART_buff("upgrade")!=0&&strsearchforGSMUART_buff("firmware")!=0)
		{
				printf("\r\nStart upgrade\r\n");
				analysis_upgrade();
				printf("\r\nEnd upgrade\r\n");
		}
}

void Find_FlagBitforBuff(void)
{
		if(strsearchforGSMUART_buff("/getflag/result")!=0)
		{
				printf("\r\nStart FindFlagBit\r\n");
				Find_FlagBit(0);
				MQTT_Sendpuback();
				printf("\r\nEnd FindFlagBit\r\n");
		}
}

unsigned short ReturnPacketid(char *str)
{
		unsigned short PacketidBuf=0;
		u16 pos=0,strlenbuf=0;
		u8 PacketidBuffer[2]={0};
		strlenbuf=strlen(str);
		pos=strsearchforGSMUART_buff((unsigned char *)str);//获取关键字位置
		if(pos!=0)
		{
				CoypArraryAtoArraryB_LR(GSMUART_buff,PacketidBuffer,2,pos+strlenbuf-1);//复制要获取的部分数组
				PacketidBuf=PacketidBuffer[0]<<8|PacketidBuffer[1];
		}
		return PacketidBuf;
}

void MQTT_Sendpuback(void)
{
		u16 mqttlen=0,MQTTSendDatastatus=0;
		u8 MQTT_msg[32]={0};//MQTT消息包缓存
		u16 packetid=0;
		char sendbuff[20];
		while(1)
		{
				packetid=ReturnPacketid("/getconfig/result");
				clear_USART1buff();
				mqttlen=MQTTSerialize_puback(MQTT_msg,32,packetid);//puback						
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msg,mqttlen);
					delay_ms(2000);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						MQTTSendDatastatus=1;
					}
					else
					    MQTTSendDatastatus=0;
				}
				
				if(MQTTSendDatastatus==0)
				{
						NetConnect(2);
				}
				else
				{
						UART_string_newline("AlreadySendPuback",Debug_Uart);
						break;
				}			
		}
}

u8 MQTT_GetFlagData(void)
{
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		u16 mqttlen=0,i=0;
		u8 MQTTSendDatastatus=0;
		u8 FlagBitMQTT_msg[50]={0};//MQTT消息包缓存
		MQTTString topicString = MQTTString_initializer;
		char jsonbuf[16]={0};
		char Topic_buf[64]={0};
		char sendbuff[20];
		while(1)
		{
			
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/Getflag");//数组连接
				topicString.cstring = Topic_buf;
				mqttlen=MQTTSerialize_publish(FlagBitMQTT_msg, 50, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)FlagBitMQTT_msg,mqttlen);
					UART3_BUF_SEND((char *)FlagBitMQTT_msg,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    MQTTSendDatastatus=1;
									if(strsearchforGSMUART_buff("FlagBit")!=0)
									{
											printf("获取标记位成功\r\n");
											UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
											Find_FlagBit(0);
										    return 1;
											break;
									}
									else
									{
											UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
											UART_string_newline("获取标志位错误!!!",Debug_Uart);
											NetConnect(2);
										    return 0;
									}
										
							}
							else
							{
								    MQTTSendDatastatus=0;
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									UART_string_newline("发送获取标志位错误!!!",Debug_Uart);
									NetConnect(2);
								    return 0;
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					MQTTSendDatastatus=0;
				}	
				if(MQTTSendDatastatus==0)
				{
						NetConnect(2);
				}
				for(i=0;i<16;i++)jsonbuf[i]=0;
				for(i=0;i<50;i++)FlagBitMQTT_msg[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_GetConfighexData(void)
{
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		u16 mqttlen=0,i=0;
		u8 MQTTSendDatastatus=0;
		u8 MQTT_msg[50]={0};//MQTT消息包缓存
		char sendbuff[20];
		MQTTString topicString = MQTTString_initializer;
		char jsonbuf[16]={0};
		char Topic_buf[64]={0};
		while(1)
		{
			
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/Getconfighex");//数组连接
				topicString.cstring = Topic_buf;			
				mqttlen=MQTTSerialize_publish(MQTT_msg, 50, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msg,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msg,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								  
								if(strsearchforGSMUART_buff("result")!=0)
								{
									    MQTTSendDatastatus=1;
										printf("获取配置成功\r\n");
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										Analyse_ConfigHexData();
										Find_FlagBitforBuff();
										break;
								}
								else
								{
									    MQTTSendDatastatus=0;
										UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
										UART_string_newline("获取配置错误!!!",Debug_Uart);
								}
							}
							else
							{
								    MQTTSendDatastatus=0;
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									UART_string_newline("发送获取配置错误!!!",Debug_Uart);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					MQTTSendDatastatus=0;
				}	
				if(MQTTSendDatastatus==0)
				{
						NetConnect(2);
				}
				for(i=0;i<16;i++)jsonbuf[i]=0;
				for(i=0;i<50;i++)MQTT_msg[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_GSMSendDHData_1(u8 Nmode)
{
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
				UART_string_newline("AT+CIPSEND",SIM_Uart);
				delay_ms(500);
				clear_USART1buff();
				UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
				UART_Sendchar(0X1A,SIM_Uart);
				test_status=Find_StringforGSMUART_buff("SEND OK",50,0);
				if(test_status==0)
				{
						NetConnect(2);
				}
				else
				{
						delay_ms(1000);
						clear_USART1buff();
						UART_string_newline("AT+CIPRXGET=2,1000",SIM_Uart);
						delay_ms(100);
						if(strsearchforGSMUART_buff(packetidbuf)!=0)
						{	
								UART_string_newline("AlreadySendData!!!",Debug_Uart);
							  break;
						}
						else
						{
								UART_string_newline("AlreadySendData错误!!!",Debug_Uart);
								NetConnect(2);
						}
				}
				for(i=0;i<630;i++)jsonbuf[i]=0;
				for(i=0;i<650;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_SendFlagBitData(void)
{
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		u16 mqttlen=0;
		u8 i=0,test_status=0;
		char sign_buf[56]={0};
		char Topic_buf[64]={0};
		u8 FlagBitMQTT_msg[150]={0};//MQTT消息包缓存
		char jsonbuf[100]={0};
		u16 once_rand=0;
		char sendbuff[20];
		while(1)
		{
			
				MQTTString topicString = MQTTString_initializer;
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
			
				JsonData_Flagbit(jsonbuf,UpdataJsonDataFlagbit(once_rand));
			
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				//strcat((char*)Topic_buf,"/updateconfig");//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
				topicString.cstring = Topic_buf;
				mqttlen=MQTTSerialize_publish(FlagBitMQTT_msg, 150, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(500);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)FlagBitMQTT_msg,mqttlen);
					UART3_BUF_SEND((char *)FlagBitMQTT_msg,mqttlen);
					delay_ms(1000);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendData!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendData错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<100;i++)jsonbuf[i]=0;
				for(i=0;i<150;i++)FlagBitMQTT_msg[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_SendPushConfig(void)
{
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		u16 mqttlen=0;
		u8 i=0,test_status=0;
		char sign_buf[56]={0};
		char Topic_buf[64]={0};
		u8 PushConfigMQTT_msg[600]={0};//MQTT消息包缓存
		char jsonbuf[550]={0};
		u16 once_rand=0;
		char sendbuff[20];
		Once_JsonDataPushConfig JsonDataPushConfig;
		while(1)
		{
			
				MQTTString topicString = MQTTString_initializer;
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
				mqttlen=MQTTSerialize_publish(PushConfigMQTT_msg, 600, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)PushConfigMQTT_msg,mqttlen);
					UART3_BUF_SEND((char *)PushConfigMQTT_msg,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<550;i++)jsonbuf[i]=0;
				for(i=0;i<600;i++)PushConfigMQTT_msg[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}


void MQTT_GSMSendCETData_Alarm(u8 Nmode,u8 GSMTCBbit)
{
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[356]={0};//MQTT消息包缓存
		char jsonbuf[300]={0};
		char prodatabuf[200]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		char sendbuff[20];
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
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
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1300, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<356;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<300;i++)jsonbuf[i]=0;
				for(i=0;i<200;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_GSMSendCETData_1(u8 Nmode,u8 GSMTCBbit)
{
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		char sendbuff[20];
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
//				if(Nmode==0)//不报警全部一起发
//				{
					//	if(Sys_status.Light_Sys_S== Status_Normal||RandomData == 1)//安装light
						{ProbeData_light(prodatabuf);strcat(prodatabuf,",");}
					//	if(Sys_status.UPS_Sys_S== Status_Normal||RandomData == 1)//安装UPS
						{ProbeData_ups(prodatabuf);strcat(prodatabuf,",");}
					//	if(Sys_status.StabPow_Sys_S== Status_Normal||RandomData == 1)//安装RPS
						{ProbeData_RPS(prodatabuf);strcat(prodatabuf,",");}
					//	if(Sys_status.Main_Sys_S== Status_Normal||RandomData == 1)//安装电力监测系统
						{ProbeData_PGS(prodatabuf);strcat(prodatabuf,",");ProbeData_ES(prodatabuf);strcat(prodatabuf,",");}
					//	if(IAS_s[0].IAS_CS== Status_Normal||RandomData == 1)//安装回路1
						{ProbeData_IAS(prodatabuf,0);strcat(prodatabuf,",");}
					//	if(IAS_s[1].IAS_CS== Status_Normal||RandomData == 1)//安装回路2
						{ProbeData_IAS(prodatabuf,1);}
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
								clear_USART1buff();
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_GSMSendCETData_IAS_03_06(u8 Nmode,u8 GSMTCBbit)
{
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		char sendbuff[20];
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
//				if(Nmode==0)
//				{
					//	if(IAS_s[2].IAS_CS== Status_Normal||RandomData == 1)//安装回路3
						{ProbeData_IAS(prodatabuf,2);strcat(prodatabuf,",");}
					//	if(IAS_s[3].IAS_CS== Status_Normal||RandomData == 1)//安装回路4
						{ProbeData_IAS(prodatabuf,3);strcat(prodatabuf,",");}
					//	if(IAS_s[4].IAS_CS== Status_Normal||RandomData == 1)//安装回路5
						{ProbeData_IAS(prodatabuf,4);strcat(prodatabuf,",");}
					//	if(IAS_s[5].IAS_CS== Status_Normal||RandomData == 1)//安装回路6
						{ProbeData_IAS(prodatabuf,5);}
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
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_GSMSendCETData_IAS_07_10(u8 Nmode,u8 GSMTCBbit)
{
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1300]={0};//MQTT消息包缓存
		char jsonbuf[1250]={0};
		char prodatabuf[1150]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		char sendbuff[20];
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
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
						//if(IAS_s[8].IAS_CS== Status_Normal||RandomData == 1)//安装回路9
						{ProbeData_IAS(prodatabuf,8);strcat(prodatabuf,",");}
						//if(IAS_s[9].IAS_CS== Status_Normal||RandomData == 1)//安装回路10
						{ProbeData_IAS(prodatabuf,9);}
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
				clear_USART1buff();				
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<1300;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1250;i++)jsonbuf[i]=0;
				for(i=0;i<1150;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}


void MQTT_GSMSendCETProbeConfig_PGS_ES_IAS01_05(void)
{
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1024]={0};//MQTT消息包缓存
		char jsonbuf[1000]={0};
		char prodatabuf[900]={0};
		char sign_buf[56]={0};
		char sendbuff[20];
		u16 once_rand=0;
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
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
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1024, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<1024;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1000;i++)jsonbuf[i]=0;
				for(i=0;i<900;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

void MQTT_GSMSendCETProbeConfig_IAS06_10(void)
{
		u16 mqttlen=0,i=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 MQTT_msgBuffer[1024]={0};//MQTT消息包缓存
		char jsonbuf[1000]={0};
		char prodatabuf[900]={0};
		char sign_buf[56]={0};
		u16 once_rand=0;
		char sendbuff[20];
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				once_rand=system_rand;//获取此时系统随机数
				strcat((char*)sign_buf,(char*)login_password);//数组连接
				strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
				MD5(sign_buf);//计算MD5
				
				ProbeConfig_IAS(prodatabuf,5);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,6);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,7);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,8);
				strcat(prodatabuf,",");
				ProbeConfig_IAS(prodatabuf,9);
				
				JsonData_ProbeConfig(jsonbuf,UpdataJsonDataProbeConfig(once_rand,prodatabuf));
				
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				//strcat((char*)Topic_buf,"/UpdateConfig");//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
				topicString.cstring = Topic_buf;
				mqttlen=MQTTSerialize_publish(MQTT_msgBuffer, 1024, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				clear_USART1buff();
				sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",mqttlen);
				UART_string_newline(sendbuff,SIM_Uart);
				UART_string_newline(sendbuff,3);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"OK")!=0)
				{
					clear_USART1buff();
					UART1_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					UART3_BUF_SEND((char *)MQTT_msgBuffer,mqttlen);
					delay_ms(1500);
					if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
					{
						delay_ms(1000);
						if(strsearch(GSMUART_buff,"^IPDATA")!=0)
					    {	
							if(strsearchforGSMUART_buff(packetidbuf)!=0)
							{	
								    test_status=1;
									UART_string_newline("AlreadySendUpdateConfig!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									Find_FlagBitforBuff();
								    break;
							}
							else
							{
								    test_status=0;
									UART_string_newline("AlreadySendUpdateConfig错误!!!",Debug_Uart);
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnect(2);
						     }
				        }
							
				    }			
				}
				else 
				{
					clear_USART1buff();
					test_status=0;
				}	
				if(test_status==0)
				{
						NetConnect(2);
				}
				for(i=0;i<1024;i++)MQTT_msgBuffer[i]=0;
				for(i=0;i<1000;i++)jsonbuf[i]=0;
				for(i=0;i<900;i++)prodatabuf[i]=0;
				for(i=0;i<56;i++)sign_buf[i]=0;
				for(i=0;i<64;i++)Topic_buf[i]=0;
		}
}

//测试使用，新协议要改
void MQTT_SendTestSendData(void)
{
		u16 mqttlen=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 TestMQTT_msg[400]={0};//MQTT消息包缓存
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				char jsonbuf[400]="{\"sign\":\"319BE00289BB964C365D2D67A05E0F49\",\"once\":\"88\",\"ID\":4,\"Status\":0,\"Signal\":99,\"ProbeData\":[{\"PID\":4001,\"S\":0,\"L\":10,\"LS\":0,\"T1\":20,\"T2\":20,\"T3\":20,\"T4\":20,\"TS1\":0,\"TS2\":0,\"TS3\":0,\"TS4\":0,\"CA\":10,\"CB\":10,\"CC\":10,\"CSA\":0,\"CSB\":0,\"CSC\":0}]}";
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/push");//数组连接
				topicString.cstring = Topic_buf;
				mqttlen=MQTTSerialize_publish(TestMQTT_msg, 400, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				UART_string_newline("AT+CIPSEND",SIM_Uart);
				delay_ms(500);
				clear_USART1buff();
				UART1_BUF_SEND((char *)TestMQTT_msg,mqttlen);
				UART_Sendchar(0X1A,SIM_Uart);
				test_status=Find_StringforGSMUART_buff("SEND OK",50,0);
				if(test_status==0)
				{
						NetConnect(2);
				}
				else
				{
						delay_ms(1000);
						clear_USART1buff();
						UART_string_newline("AT+CIPRXGET=2,1000",SIM_Uart);
						delay_ms(100);
						if(strsearchforGSMUART_buff(packetidbuf)!=0)
						{	
								UART_string_newline("AlreadySendData!!!",Debug_Uart);
							  break;
						}
						else
						{
								UART_string_newline("AlreadySendData错误!!!",Debug_Uart);
								NetConnect(2);
						}
				}
		}
}

//测试使用，新协议要改
void MQTT_SendTestUpdataData(void)
{
		u16 mqttlen=0;
		u8 test_status=0;
		u8 packetidbuf[]={0x02,0x33,0x33};//根据packet来更改
		char Topic_buf[64]={0};
		u8 TestMQTT_msg[400]={0};//MQTT消息包缓存
		while(1)
		{
				MQTTString topicString = MQTTString_initializer;
				char jsonbuf[400]="{\"Sign\":\"fb08220679837413ff8758f53e5c4fae\",\"once\":\"171202040201\",\"ID\":4,\"ProbeConfig\":[{\"PID\":4001,\"LS\":0,\"TS1\":0,\"TS2\":0,\"TS3\":0,\"TS4\":0,\"CSA\":0,\"CSB\":0,\"CSC\":0}]}";
				strcat((char*)Topic_buf,"data/");//数组连接
				strcat((char*)Topic_buf,PCB_ID_STR);//数组连接
				strcat((char*)Topic_buf,"/UpdateConfig");//数组连接
				topicString.cstring = Topic_buf;
				mqttlen=MQTTSerialize_publish(TestMQTT_msg, 400, 0, 1, 0, 13107, topicString, (u8 *)jsonbuf, strlen((const char *)jsonbuf)); /* 2 */
				UART_string_newline("AT+CIPSEND",SIM_Uart);
				delay_ms(500);
				clear_USART1buff();
				UART1_BUF_SEND((char *)TestMQTT_msg,mqttlen);
				UART_Sendchar(0X1A,SIM_Uart);
				test_status=Find_StringforGSMUART_buff("SEND OK",50,0);
				if(test_status==0)
				{
						NetConnect(2);
				}
				else
				{
						delay_ms(1000);
						clear_USART1buff();
						UART_string_newline("AT+CIPRXGET=2,1000",SIM_Uart);
						delay_ms(100);
						if(strsearchforGSMUART_buff(packetidbuf)!=0)
						{	
								UART_string_newline("AlreadySendData!!!",Debug_Uart);
							  break;
						}
						else
						{
								UART_string_newline("AlreadySendData错误!!!",Debug_Uart);
								NetConnect(2);
						}
				}
		}
}

void MQTT_PING(void)
{
		u8 Sendstatus=0;
		u8 message_buffer[50]={0};//MQTT消息包缓存
		char sendbuff[20];
		u16 MQTTlenbuffer=0;
		while(Sendstatus==0)
		{
		    MQTTlenbuffer=MQTTSerialize_pingreq(message_buffer,50);//心跳包							
			clear_USART1buff();
			sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",MQTTlenbuffer);
			UART_string_newline(sendbuff,SIM_Uart);
			UART_string_newline(sendbuff,3);
			delay_ms(2000);
			if(strsearch(GSMUART_buff,"OK")!=0)
			{
				clear_USART1buff();
				UART1_BUF_SEND((char *)message_buffer,MQTTlenbuffer);
				delay_ms(2000);
				if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
				{
					clear_USART1buff();
					UART_string_newline("AT^IPLISTEN=\"TCP\",1821",SIM_Uart);
					delay_ms(1000);			
					if((strsearch(GSMUART_buff,"OK")!=0)||(strsearch(GSMUART_buff,"1008")!=0))	
					{
						clear_USART1buff();
						Sendstatus=1;
					}	
					else 
					{
						clear_USART1buff();
						Sendstatus=0;
					}
						
			   }			
			}
			else 
			{
				clear_USART1buff();
				Sendstatus=0;
			}
			if(Sendstatus==0)
			{
					NetConnect(2);
			}
			else
					break;
			}
		UART_string_newline("心跳包已发送",Debug_Uart);
}

u8 Subscribe_Topic(char* TopicBuffer,char qos)
{
		u8 message_buffer[70]={0},SubscribeTopicstatus=0;//MQTT消息包缓存
		char sendbuff[30];
//		u16 MQTTlenbuffer=0;
//		MQTTlenbuffer=mqtt_subscribe_message(message_buffer,TopicBuffer,qos,1);//订阅test主题
//		UART_string_newline("AT+CIPSEND",SIM_Uart);
//		delay_ms(500);
//		clear_USART1buff();
//		UART1_BUF_SEND((char *)message_buffer,MQTTlenbuffer);
//		UART_Sendchar(0X1A,SIM_Uart);
//		Find_StringforGSMUART_buff("SEND OK",50,1);
//		clear_USART1buff();
//		UART_string_newline("AT+CIPRXGET=2,1000",SIM_Uart);
//		SubscribeTopicstatus=Find_StringforGSMUART_buff("OK",50,0);
//		return SubscribeTopicstatus;
		u16 MQTTlenbuffer=0;
		MQTTlenbuffer=mqtt_subscribe_message(message_buffer,TopicBuffer,qos,1);;
		clear_USART1buff();
		sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",MQTTlenbuffer);
        UART_string_newline(sendbuff,SIM_Uart);
		UART_string_newline(sendbuff,3);
		delay_ms(2000);
		if(strsearch(GSMUART_buff,"OK")!=0)
		{
			clear_USART1buff();
			UART1_BUF_SEND((char *)message_buffer,MQTTlenbuffer);
			delay_ms(2000);
			if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
			{
				clear_USART1buff();
				UART_string_newline("AT^IPLISTEN=\"TCP\",1821",SIM_Uart);
				delay_ms(1000);			
				if((strsearch(GSMUART_buff,"OK")!=0)||(strsearch(GSMUART_buff,"1008")!=0))			
				{
					clear_USART1buff();
					return 1;
				}	
				else 
				{
					clear_USART1buff();
					return 0;
				}
					
		   }			
		}
		else 
		{
			clear_USART1buff();
			return 0;
		}	
}

u8 Connect_MQTT(char *client_id,char *username,char *password)
{
		char *sendbuff;
		u8 message_buffer[100]={0};	
	    //u8 sendbuff[200]={0};
//		u8 message_buffer[100]={0},ConnectStatua=0;//MQTT消息包缓存
//		u16 MQTTlenbuffer=0;
//		MQTTlenbuffer=mqtt_connect_message(message_buffer,client_id,username,password);
//		UART_string_newline("AT+CIPSEND",SIM_Uart);
//		delay_ms(500);
//		clear_USART1buff();
//		UART1_BUF_SEND((char *)message_buffer,MQTTlenbuffer);
//		UART_Sendchar(0X1A,SIM_Uart);
//		Find_StringforGSMUART_buff("SEND OK",50,1);
//		clear_USART1buff();
//		UART_string_newline("AT+CIPRXGET=2,1000",SIM_Uart);
//		ConnectStatua=Find_StringforGSMUART_buff("OK",10,0);
//		return ConnectStatua;
		//u8 message_buffer[100]={0};
		u16 MQTTlenbuffer=0;
 		MQTTlenbuffer=mqtt_connect_message(message_buffer,client_id,username,password);
		clear_USART1buff();
		sprintf(sendbuff,"AT^IPSENDEX=1,2,%d",MQTTlenbuffer);
        UART_string_newline(sendbuff,SIM_Uart);
		UART_string_newline(sendbuff,3);
		delay_ms(2000);
		if(strsearch(GSMUART_buff,"OK")!=0)
		{
			clear_USART1buff();
			UART1_BUF_SEND((char *)message_buffer,MQTTlenbuffer);
			delay_ms(2000);
			if(strsearch(GSMUART_buff,"^IPSENDEX: 1")!=0)
			{
				clear_USART1buff();
				UART_string_newline("AT^IPLISTEN=\"TCP\",1821",SIM_Uart);
				delay_ms(1000);			
				if((strsearch(GSMUART_buff,"OK")!=0)||(strsearch(GSMUART_buff,"1008")!=0))		
				{
					clear_USART1buff();
					return 1;
				}	
				else 
				{
					clear_USART1buff();
					return 0;
				}
					
		   }			
		}
		else 
		{
			clear_USART1buff();
			return 0;
		}	
}

//构建MQTT连接包 
u16 mqtt_connect_message(u8 *mqtt_message,char *client_id,char *username,char *password)
{
		u16 client_id_length = strlen(client_id);
		u16 username_length = strlen(username);
		u16 password_length = strlen(password);
		u16 packetLen;
		u16 i,baseIndex;
		
		packetLen = 12 + 2 + client_id_length;
		if(username_length > 0)
				packetLen = packetLen + 2 + username_length;
		if(password_length > 0)
				packetLen = packetLen+ 2 + password_length;	
		mqtt_message[0] = 16;				//0x10 // MQTT Message Type CONNECT
		mqtt_message[1] = packetLen - 2;	//剩余长度(不包括固定头部)
		baseIndex = 2;
		if(packetLen > 127)
		{
				mqtt_message[2] = 1;			//packetLen/127;    
				baseIndex = 3;
		}
		mqtt_message[baseIndex++] = 0;		// Protocol Name Length MSB    
		mqtt_message[baseIndex++] = 4;		// Protocol Name Length LSB    
		mqtt_message[baseIndex++] = 77;		// ASCII Code for M    
		mqtt_message[baseIndex++] = 81;		// ASCII Code for Q    
		mqtt_message[baseIndex++] = 84;		// ASCII Code for T    
		mqtt_message[baseIndex++] = 84;		// ASCII Code for T    
		mqtt_message[baseIndex++] = 4;		// MQTT Protocol version = 4    
		mqtt_message[baseIndex++] = 0xC2;		// conn flags 0xC2
		mqtt_message[baseIndex++] = 0;		// Keep-alive Time Length MSB    
		mqtt_message[baseIndex++] = 60;		// Keep-alive Time Length LSB    
		mqtt_message[baseIndex++] = (0xff00&client_id_length)>>8;// Client ID length MSB    
		mqtt_message[baseIndex++] = 0xff&client_id_length;	// Client ID length LSB    

		// Client ID
		for(i = 0; i < client_id_length; i++)
		{
				mqtt_message[baseIndex + i] = client_id[i];    
		}
		baseIndex = baseIndex+client_id_length;		
		if(username_length > 0)
		{
				//username    
				mqtt_message[baseIndex++] = (0xff00&username_length)>>8;//username length MSB    
				mqtt_message[baseIndex++] = 0xff&username_length;	//username length LSB    
				for(i = 0; i < username_length ; i++)
				{
						mqtt_message[baseIndex + i] = username[i];    
				}
				baseIndex = baseIndex + username_length;
		}
			
		if(password_length > 0)
		{
				//password    
				mqtt_message[baseIndex++] = (0xff00&password_length)>>8;//password length MSB    
				mqtt_message[baseIndex++] = 0xff&password_length;	//password length LSB    
				for(i = 0; i < password_length ; i++)
				{
						mqtt_message[baseIndex + i] = password[i];    
				}
				baseIndex += password_length; 
		}	
		return baseIndex;    
}

//构建MQTT订阅请求/取消订阅包
//whether=1,订阅; whether=0,取消订阅
u16 mqtt_subscribe_message(u8 *mqtt_message,char *topic,u8 qos,u8 whether)
{    
		u16 topic_len = strlen(topic);
		u16 i,index = 0;
		static u16 id=0;
		
		id++;
		if(whether)
				mqtt_message[index++] = 130;				//0x82 //消息类型和标志 SUBSCRIBE 订阅
		else
				mqtt_message[index++] = 162;				//0xA2 取消订阅
		mqtt_message[index++] = topic_len + 5;			//剩余长度(不包括固定头部)
		mqtt_message[index++] = (0xff00&id)>>8;			//消息标识符
		mqtt_message[index++] = 0xff&id;				//消息标识符
		mqtt_message[index++] = (0xff00&topic_len)>>8;	//主题长度(高位在前,低位在后)
		mqtt_message[index++] = 0xff&topic_len;			//主题长度 
		
		for (i = 0;i < topic_len; i++)
		{
				mqtt_message[index + i] = topic[i];
		}
		index += topic_len;
		if(whether)
		{
				mqtt_message[index] = qos;//QoS级别
				index++;
		}
		return index;
}

