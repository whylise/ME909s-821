#include "sim800c.h"

u8 SimInsertGsmStatus=0;//SIM卡是否插入,(gsm模块返回的状态)0:没有 1:有

u8 Gsm_init_Status=0x10;//重关闭GSM电源开始
u8 gprs_rex=0;
u8 g_gsm_status=0;//GSM 状态

void GSMConnect_INIT(void)
{
		#define INFO_GSM_POWER_OFF  0x10//关闭电源
		#define INFO_GSM_POWER_ON   0x11//打开电源
		#define INFO_CHECK_AT       0x12//检查AT指令是否正常
	    
		#define READ_SIM_STATUS     0x13//检查SIM卡
		#define NET_Select			0x14//选择网络
		#define SET_GSM_CERG        0x15//等待CERG
		#define SET_GSM_PARA        0x16//设置基本配置
		#define GSM_INIT_GPRS       0x17//初始化GPRS
		#define GSM_INIT_FINISH     0x18//初始化成功
	  u8 check_AT_error=0,pos=0,error_creg=0;//检测AT通信错误计数
		while(1)
		{
				delay_ms(100);
				switch(Gsm_init_Status)
				{
						case INFO_GSM_POWER_OFF://关闭电源
									if(RunMode==1)UART_string_newline("INFO_GSM_POWER_OFF",Debug_Uart);
									GSM_POWER_OFF();
									Gsm_init_Status=INFO_GSM_POWER_ON;
									break;
						case INFO_GSM_POWER_ON://打开电源
									if(RunMode==1)UART_string_newline("INFO_GSM_POWER_ON",Debug_Uart);
									GSM_POWER_ON();
									Gsm_init_Status=INFO_CHECK_AT;
									g_gsm_status=GSM_OFFLINE;
									break;
						case INFO_CHECK_AT://检查AT指令是否正常
									if(RunMode==1)UART_string_newline("INFO_CHECK_AT",Debug_Uart);
									if(check_ATcommand())
									{
											UART_string_newline("ATE0",SIM_Uart);
	                                        delay_ms(500);
										    if(SetMe909SleepTime())
											{
											 Gsm_init_Status=READ_SIM_STATUS;
											}
											else
											{
											  delay_ms(500);
											  check_AT_error++;//累计错误次数											
											if(check_AT_error>=2)//重复检查AT指令3次，若三次都失败，重启GSM电源
											{
													check_AT_error=0;
													Gsm_init_Status=INFO_GSM_POWER_OFF;//重新启动GSM模块
											}											}											
									}
									else 
									{
											delay_ms(500);
											check_AT_error++;//累计错误次数
									}
									if(check_AT_error>=2)//重复检查AT指令3次，若三次都失败，重启GSM电源
									{
											check_AT_error=0;
											Gsm_init_Status=INFO_GSM_POWER_OFF;//重新启动GSM模块
									}
									break;
						case READ_SIM_STATUS://检查SIM卡
									if(RunMode==1)UART_string_newline("READ_SIM_STATUS",Debug_Uart);
									if(checkSIMsertStatus())
									{
											SimInsertGsmStatus=1;//有卡
											Gsm_init_Status=NET_Select;
									}
									else//没卡，一直循环，直至拨电重启
									{
											SimInsertGsmStatus=0;
											LCDChangeNeed=1;
										    Gsm_init_Status=INFO_GSM_POWER_OFF;
									}
									break;
						case 	NET_Select://选择网络
									Gsm_init_Status=SET_GSM_CERG;
									break;
						case SET_GSM_CERG://等待CERG
								    if(RunMode==1)UART_string_newline("WaitCGREG",Debug_Uart);
						            delay_ms(5000);
									if(WaitCGREG())
									{
											Gsm_init_Status=SET_GSM_PARA;
									}
									else
									{
										    Gsm_init_Status=INFO_GSM_POWER_OFF;
									}							        
									break;
						case SET_GSM_PARA://设置基本配置
									Gsm_init_Status=GSM_INIT_GPRS;
									break;
						case GSM_INIT_GPRS://初始化IPINIT
									if(RunMode==1) UART_string_newline("GSM_INIT_GPRS",Debug_Uart);				         
  								    if(IPINIT())
									{
										if(CheckIPINIT())
											Gsm_init_Status=GSM_INIT_FINISH;
										else
											Gsm_init_Status=INFO_GSM_POWER_OFF;
									}
									else
									{
										    Gsm_init_Status=INFO_GSM_POWER_OFF;
									}							        
									break;
						case GSM_INIT_FINISH:
						{
								if(RunMode==1)UART_string_newline("GSM_INIT_FINISH",Debug_Uart);
								delay_ms(200);
								return;//进入开机前的延时4s,为了以后可以正常连接网络
						}		
				}
		}
}


//*************************************************
//描述:发送AT到GSM模块,检测AT是否正常
//返回: 返回0失败 1成功
u8 check_ATcommand(void)
{
		ME909WAKE=1;
		clear_USART1buff();
	    delay_ms(500);
		UART_string_newline("AT^CURC=0",SIM_Uart);
		delay_ms(500);
		if(strsearch(GSMUART_buff,"OK")!=0)
				return 1;
	    else
				return 0;
}

//*************************************************
//描述:设置GSM模块的TCPIP为手动模块,必须在GPRS初始化前完成设置
//返回: 返回0失败 1成功
u8 CIPRXGET_Command(void)
{
		clear_USART1buff();
		UART_string_newline("AT+CIPRXGET=1",SIM_Uart);
		delay_ms(100);
		clear_USART1buff();
		UART_string_newline("AT+CIPRXGET?",SIM_Uart);
		delay_ms(100);
		if(strsearch(GSMUART_buff,"+CIPRXGET: 1")!=0)
				return 1;
		else
				return 0;
}

//*************************************************
//描述:检查SIM卡是否插上
//返回: 返回1有卡 0没卡
u8 checkSIMsertStatus(void)
{
		u8 try_cnt=0,try_flag=0;
		for(try_cnt=0;try_cnt<3;try_cnt++)
		{
				clear_USART1buff();
				UART_string_newline("AT^CARDMODE",SIM_Uart);
				delay_ms(1000);
			    if(strsearch(GSMUART_buff,"^CARDMODE: 1"))
				{
						try_flag=1;
						clear_USART1buff();
						
				}					
				else  if(strsearch(GSMUART_buff,"^CARDMODE: 2"))
				{
						try_flag=1;
						clear_USART1buff();
				}
				else
				{
					try_flag=0;
				    clear_USART1buff();
				}
		}
		return try_flag;
}

u8 Return_Signal(void)
{
		u16 Signal_Buf=0,pos=0;
	    u8  GsmSignal=0;
	    u16 posflag=0;
		clear_USART1buff();
	    UART_string_newline("AT^HCSQ?",SIM_Uart);//关闭回显
	    delay_ms(600);
	    posflag=strsearchforGSMUART_buff("LTE:");//获取关键字位置 
	    GsmSignal= GSMUART_buff[posflag+4];
		return GsmSignal;
}

//*************************************************
//描述:设置GSM模块配置
//返回: 返回0成功
u8 set_GSMconfiguration(void)
{
		u8 try_cnt=0;
		u8 returnflag=0;
		clear_USART1buff();
		for(try_cnt=0;try_cnt<3;try_cnt++)
		{
				UART_string_newline("ATE0V1",SIM_Uart);//关闭回显
				delay_ms(100);
				if(!strsearch(GSMUART_buff,"OK"))
					returnflag=1;
				else
					break;
		}		
		clear_USART1buff();
		for(try_cnt=0;try_cnt<3;try_cnt++)
		{
				UART_string_newline("AT+CLTS=1",SIM_Uart);//设定可以搜索网络时间
				delay_ms(2000);
				if(!strsearch(GSMUART_buff,"OK"))
					returnflag=4;	
				else
					break;
		}
		clear_USART1buff();
		return returnflag;
}

//********************************************
//描述:初始化gprs功能
//函数:0:所有项目通过 1:该SIM卡没有依附gprs功能 2:写入APN名失败   3:APN接入场景失败 
u8 GPRS_Init_State=1;
u8 gsm_gprs_init(void)
{
		#define  ATTACH_GPRS  			  1//判断是否有依附着gprs功能
		#define  WRITE_APN_NAME			  2//写入APN名
		#define  START_CONNECT_GPRS		3//启动通过APN接入GPRS
		u8 res=0,GPRS_INIT_ERROR_CNT=0;
		while(1)
		{
				delay_ms(100);
				switch(GPRS_Init_State)
				{
					case ATTACH_GPRS://判断是否有依附着gprs功能
								res=GetSimAttachGprs();
								if(res)GPRS_Init_State=WRITE_APN_NAME;
								else
								{
										GPRS_INIT_ERROR_CNT++;
										if(GPRS_INIT_ERROR_CNT>5)
										{
												GPRS_INIT_ERROR_CNT=0;
												return 1;
										}
								}
								break;
					case WRITE_APN_NAME://写入APN名
								if(WriteApnName("CMNET"))
								{
										if(WriteApnName("CMNET"))return 2;//第一次写失败，重写
										else
												GPRS_Init_State=START_CONNECT_GPRS;
								}
								else
										GPRS_Init_State=START_CONNECT_GPRS;
								break;
					case START_CONNECT_GPRS://启动通过APN接入GPRS
								if(StartApnGprs())return 3;										
								else 
								{
											if(http_init()==0)return 0;//初始化http											
											else
												return 3;
								} 
				}
		}
}

//************************************************************************
//描述: 判断当前sim卡有没有依附gprs功能
//返回: 1:有依附  0错误
u8 GetSimAttachGprs(void)
{
		UART_string_newline("AT+CGATT=1",SIM_Uart);//写入之前必须shut gprs
		delay_ms(2000);
    clear_USART1buff();
		UART_string_newline("AT+CGATT?",SIM_Uart);//读取依附
    delay_ms(100);
		if(strsearch(GSMUART_buff,"+CGATT"))
		{
				if(strsearch(GSMUART_buff,"0"))	
				{
						clear_USART1buff();
						UART_string_newline("AT+CIPSHUT",SIM_Uart);//写入之前必须shut gprs
						delay_ms(500);
						return 0;
				}
				else if(strsearch(GSMUART_buff,"1"))
				{
						return 1;
				}
				else
						return 0;
		}	
		else
				return 0;//没有响应
}

//************************************
//描述:写入APN接入名称
//参数:name:写入的名称
//返回:0:成功写入 1:失败
u8 WriteApnName(u8 *name)
{
		u16 pos = 0;
    clear_USART1buff();
		UART_string_newline("AT+CIPSHUT",SIM_Uart);//写入之前必须shut gprs
    delay_ms(500);
    pos=strsearch(GSMUART_buff,"OK");   	
		if(!pos)return 1;//不能关闭GPRS系统
    clear_USART1buff();
		UART_string("AT+CSTT=",SIM_Uart);			//写入
		UART_string("\x22",SIM_Uart);
		UART_string(name,SIM_Uart);
		UART_string_newline("\x22,\x22\x22,\x22\x22",SIM_Uart);
    delay_ms(1000);
    pos=strsearch(GSMUART_buff,"OK");  	
		if(pos)return 0;
		else return 1;
}

//**********************************************
//描述:通过APN接入GPRS ,执行以后必须确保GPRS处于在网状态
//返回:	 0:接入成功   1:接入失败
u8 StartApnGprs(void)
{
		u16 pos=0;
		u8 res=0;
		u8 times=0;
		startapn:
		while(times < 3)//最多执行三次 APN接入过程,否则放弃
		{
				times++;
				clear_USART1buff();
				res=GetCurrGprsStatus();	
				switch(res)
				{
						case 255://这个是获取状态失败
						case 9:
						default:
										clear_USART1buff();
										UART_string_newline("AT+CIPSHUT",SIM_Uart);//写入之前必须shut gprs
										delay_ms(200);
										pos=strsearch(GSMUART_buff,"OK");
										if(!pos)
										{
												goto startapn;				
										}
						case 0:	//最原始的初始化状态，			
										clear_USART1buff();
										UART_string_newline("AT+CSTT",SIM_Uart);//启动APN
										delay_ms(100);
										pos=strsearch(GSMUART_buff,"OK"); 
										if(!pos)
										{
												goto startapn;				
										}		
						case 1:
										clear_USART1buff();
										UART_string_newline("AT+CIICR",SIM_Uart);//激活场景
										delay_ms(1000);
										pos=strsearch(GSMUART_buff,"OK");
										if(!pos)
										{
												goto startapn;				
										}
						case 2://什么都不需要做
						case 3:	
						case 4:  
						case 8:		
										clear_USART1buff();
										UART_string_newline("AT+CIFSR",SIM_Uart);//获取本地IP
										delay_ms(1000);		
										goto leave;
						case 5:
						case 6:
										clear_USART1buff();
										UART_string_newline("AT+CIPCLOSE",SIM_Uart);//close tcp connect
										delay_ms(1000);
										pos=strsearch(GSMUART_buff,"OK");
										if(!pos)
										{
												goto startapn;				
										}
										break;
				}
		}
		leave:
		if(IsGetDnsIp())return 1;
		else	return 0;
}

//*************************
//描述: 判断是否成功获取DNS地址
//返回:	0:成功  1:获取失败
u8 IsGetDnsIp(void)
{
		u16 pos = 0;
    clear_USART1buff();
		UART_string_newline("AT+CDNSCFG?",SIM_Uart);//获取DNS地址
		delay_ms(500);
		pos=strsearch(GSMUART_buff,"PrimaryDns:");
		if(pos)
		{
				pos=strsearch(GSMUART_buff,"PrimaryDns: 0.0.0.0");
				if(pos)return 1;	
				else	return 0;
		}
		else return 1;	
}

//**************************************************************
//描述:http初始化函数
//参数:bearer init
//返回:	0:成功   others:失败 
u8 http_init(void)
{
		clear_USART1buff();
		UART_string_newline("AT+SAPBR=3,1,\"Contype\",\"GPRS\"",SIM_Uart);//配置承载场景1
		delay_ms(500);
		clear_USART1buff();
		UART_string_newline("AT+SAPBR=3,1,\"APN\",\"CMNET\"",SIM_Uart);//配置承载场景1
		delay_ms(500);
		clear_USART1buff();
		UART_string_newline("AT+SAPBR=1,1",SIM_Uart);//激活一个GPRS上下文
		delay_ms(500);
		clear_USART1buff();
		UART_string_newline("AT+SAPBR=2,1",SIM_Uart);//查询GPRS上下文
		delay_ms(500);  
		if(!strsearch(GSMUART_buff,"0.0.0.0"))return 0;//判断是否激活成功
		return 1;
}

//**************************************************************
//描述:获取目前的GPRS状态
//返回: 255:获取失败 0: 初始化状态	 1:启动任务	 2:配置场景	 	
//返回: 3:接受场景配置  4:	获得本地IP地址	 5:连接中
//返回:6: 连接成功 7 :正在关闭 8:已经关闭 9:场景被释放
u8 GetCurrGprsStatus(void)
{
		u16 pos = 0;
    clear_USART1buff();
		UART_string_newline("AT+CIPSTATUS",SIM_Uart);
    delay_ms(1000);
    pos=strsearch(GSMUART_buff,"IP INITIAL");  
		if(pos)return 0;	
    pos=strsearch(GSMUART_buff,"IP START");  
		if(pos)return 1; 
    pos=strsearch(GSMUART_buff,"IP CONFIG");  
		if(pos)return 2;
    pos=strsearch(GSMUART_buff,"IP GPRSACT");  
		if(pos)return 3; 
    pos=strsearch(GSMUART_buff,"IP STATUS");  
		if(pos)return 4;	
    pos=strsearch(GSMUART_buff,"CONNECTING");  
		if(pos)return 5; 
    pos=strsearch(GSMUART_buff,"CONNECT OK");  
		if(pos)return 6; 
    pos=strsearch(GSMUART_buff,"CLOSING");  
		if(pos)return 7; 
    pos=strsearch(GSMUART_buff,"CLOSED");  
		if(pos)return 8; 
		pos=strsearch(GSMUART_buff,"PDP DEACT");  
		if(pos)return 9; 
		return 255;
}

u8 DNS_Buffer[20]={0};
void GetDNS(void)
{
		u8 URL_LEN=0,i=0;
		u8 URLtoDNSbuffer[100]={0};
		u16 posid1=0,posid2=0;
		URL_LEN=MyEE_ReadWord(HTTP_URL_LenAdd);
		for(i=0;i<URL_LEN;i++)URLtoDNSbuffer[i]=MyEE_ReadWord(HTTP_URL_Add+i);//读取FLASH的正常工作域名
		if(URLtoDNSbuffer[0]=='h'&&URLtoDNSbuffer[1]=='t'&&URLtoDNSbuffer[2]=='t'&&URLtoDNSbuffer[3]=='p')
		{
				//需要将http://去掉
				for(i=0;i<100;i++)
				{
						if(i<URL_LEN-7)
								URLtoDNSbuffer[i]=URLtoDNSbuffer[i+7];
						else 
								URLtoDNSbuffer[i]=0x00;
				}
		}
		clear_USART1buff();
		UART_string("AT+CDNSGIP =\"",SIM_Uart);
		UART_string(URLtoDNSbuffer,SIM_Uart);
		UART_string_newline("\"",SIM_Uart);
		Find_StringforGSMUART_buff("+CDNSGIP: 1",100,1);
		
		posid1=strsearch(GSMUART_buff,"\",\"");//获取关键字位置
		posid2=EndstrsearchGSMUART_buff("\"\x0d\x0a");//获取关键字位置
		CoypArraryAtoArraryB_LR(GSMUART_buff,DNS_Buffer,posid2-posid1-3,posid1+3-1);//复制要获取的部分数组
		
//		//调试使用sync4
//		if(Way_of_Proj == 1)
//		{
//				for(i=0;i<20;i++)DNS_Buffer[i]=0;//120.79.171.8 
//				DNS_Buffer[0]=0x31;DNS_Buffer[1]=0x32;DNS_Buffer[2]=0x30;
//				DNS_Buffer[3]=0x2E;DNS_Buffer[4]=0x37;DNS_Buffer[5]=0x39;
//				DNS_Buffer[6]=0x2E;DNS_Buffer[7]=0x31;DNS_Buffer[8]=0x37;
//				DNS_Buffer[9]=0x31;DNS_Buffer[10]=0x2E;DNS_Buffer[11]=0x38;
//    }
		
		UART_string("DNS:",Debug_Uart);
		UART_string_newline(DNS_Buffer,Debug_Uart);
}

u8 TCPConnect(char* IP_Add,char* Port_Add)
{
    clear_USART1buff();
	UART_string_newline("AT^IPOPEN=1,\"TCP\",\"47.106.255.185\",1821",SIM_Uart);
	delay_ms(5000);
	if(strsearch(GSMUART_buff,"OK")!=0)
			return 1;
	else if(strsearch(GSMUART_buff,"1004")!=0)
		    return 1;
	else
			return 0;	
}

//形参要视情况而入
void NetConnect(u8 NetConnectStatus)
{
		#define ResetSystem 0//重启系统
		#define GPRS_INIT 1//初始化GPRS
		#define TCPIP_INIT 2//初始化TCPIP
		#define MQTT_Connect_INIT 3//初始化MQTT连接
		#define MQTT_SubscribeTopic 4//订阅MQQTT主题
		#define Net_Finish 5//完成网络初始化
		u8 sign_buf[64]={0},Topic_buf[64]={0},ID_buf[11]={0},i=0;
		u8 GPRS_INIT_status=0,TCPIP_INIT_status=0,MQTT_Connect_INIT_status=0,MQTT_SubscribeTopic_status1=0,MQTT_SubscribeTopic_status2=0;
		u16 once_rand=0;
		u8 TCPIP_Connect_ErrorCnt=0;
		u8 ConnectMqttErr=0;
		while(1)
		{
				switch(NetConnectStatus)
				{
					case ResetSystem:
							UART_string_newline("软复位",Debug_Uart);
							Soft_SystemReset();
							break;
					case GPRS_INIT:
							UART_string_newline("初始化GPRS",Debug_Uart);
							GPRS_INIT_status=0;
							GPRS_INIT_status=gsm_gprs_init();
							if(GPRS_INIT_status!=0)
									NetConnectStatus=ResetSystem;
							else if(GPRS_INIT_status==0)
									NetConnectStatus=TCPIP_INIT;
							break;
					case TCPIP_INIT:
									UART_string_newline("连接TCPIP",Debug_Uart);
									TCPIP_INIT_status=TCPConnect((char*)DNS_Buffer,(char*)U16IntToStr(FLahMqttPort));
									UART3_BUF_SEND((char *)GSMUART_buff,MAX_GSMUART);
									NetConnectStatus=MQTT_Connect_INIT;
							break;
					case MQTT_Connect_INIT:
						    delay_ms(3000);
							UART_string_newline("连接MQTT",Debug_Uart);
							once_rand=system_rand;//获取此时系统随机数
							strcat((char*)sign_buf,(char*)login_password);//数组连接
							strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
							MD5(sign_buf);//计算MD5
							sprintf((char*)ID_buf,"%d",HTTP_PCB_ID);					
							MQTT_Connect_INIT_status=Connect_MQTT((char*)ID_buf,(char*)U16IntToStr(once_rand),MD5_BUF);
							if(MQTT_Connect_INIT_status==0)
							{
								ConnectMqttErr++;
								NetConnectStatus=TCPIP_INIT;								
							}
							else									
									NetConnectStatus=MQTT_SubscribeTopic;
							if(ConnectMqttErr>5)
								NetConnectStatus=ResetSystem;
							break;
					case MQTT_SubscribeTopic:
							UART_string_newline("订阅MQTT主题",Debug_Uart);
							sprintf((char*)ID_buf,"%d",HTTP_PCB_ID);
							strcat((char*)Topic_buf,"client/");//数组连接
							strcat((char*)Topic_buf,(char*)ID_buf);//数组连接
							strcat((char*)Topic_buf,"/#");//数组连接
							MQTT_SubscribeTopic_status1=Subscribe_Topic((char*)Topic_buf,1);
							for(i=0;i<64;i++)Topic_buf[i]=0;
							strcat((char*)Topic_buf,"firmware/");//数组连接
							strcat((char*)Topic_buf,(char*)ID_buf);//数组连接
							strcat((char*)Topic_buf,"/#");//数组连接
							MQTT_SubscribeTopic_status2=Subscribe_Topic((char*)Topic_buf,1);
							if(MQTT_SubscribeTopic_status1==0&&MQTT_SubscribeTopic_status2==0)
									NetConnectStatus=MQTT_Connect_INIT;
							else
									NetConnectStatus=Net_Finish;
							break;
					case Net_Finish:
							UART_string_newline("网络初始化完成",Debug_Uart);
							GSM_NETstatus=1;
							return;
				}
				delay_ms(10);
		}
		
}
u8 SetMe909SleepTime(void)
{
	clear_USART1buff();
	UART_string_newline("AT^SLEEPCFG=1,3600",SIM_Uart);
	delay_ms(2000);
	if(strsearch(GSMUART_buff,"OK")!=0)
			return 1;
	else
			return 0;	

}
u8 ResetMe909(void)
{
	clear_USART1buff();
	UART_string_newline("AT^RESET",SIM_Uart);
	delay_ms(500);
	if(strsearch(GSMUART_buff,"OK")!=0)
			return 1;
	else
			return 0;	

}

u8 SetSimNet(void)
{
    clear_USART1buff();
	UART_string_newline("AT^SYSCFGEX=\"03\",3FFFFFFF,1,2,7FFFFFFFFFFFFFFF,,",SIM_Uart);
	delay_ms(500);
	if(strsearch(GSMUART_buff,"OK")!=0)
			return 1;
	else
			return 0;	
		
}

u8 WaitCGREG(void)
{
    clear_USART1buff();
	UART_string_newline("AT+CGREG?",SIM_Uart);
	delay_ms(500);
	if(strsearch(GSMUART_buff,"+CGREG: 0,1")!=0)
			return 1;
	else
			return 0;		
}

u8 IPINIT(void)
{
    clear_USART1buff();
	UART_string_newline("AT^IPINIT=\"GSM\"",SIM_Uart);
	delay_ms(8000);
	if(strsearch(GSMUART_buff,"OK")!=0)
			return 1;
	else
			return 0;		
}

u8 CheckIPINIT(void)
{
    clear_USART1buff();
	UART_string_newline("AT^IPINIT?",SIM_Uart);
	delay_ms(500);
	if(strsearch(GSMUART_buff,"^IPINIT: 1")!=0)
			return 1;
	else
			return 0;		
}
