#include "http_api.h"

const u8 login_password[]="antenicantenic888";//登录账户密码

Json_Register SIM800C_Register;
u8 cdkey_length=0;//CDKEY长度

u8 cdkeybuffer[18]={0};//CDKEY缓存
u32 HTTP_PCB_ID=0;//从服务获取的注册ID
u8 EncryptMode=0;
u16 FLahMqttPort=0;
u8 FlashURL[100]="http://pilot.antenic.com";
char PCB_ID_STR[11]={0};

//***********************
//描述:POSTCDKEY
//返回:0:成功 1:失败
u8 POST_HTTP_CDKEY_MOTHOD(void)
{
		u8 sign_buf[64]={0},URL_BUF[50]={0};
		u16 poscd1=0,poscd2=0,URL_LEN=0,i=0,once_rand=0,length_buffer=0;
		clear_USART1buff();
		UART_string_newline("AT+SAPBR=2,1",SIM_Uart);//查询GPRS上下文是否正常
		delay_ms(500); 
		if(!strsearch(GSMUART_buff,"0.0.0.0"))
		{	
				clear_USART1buff();
				UART_string_newline("AT+CGSN",SIM_Uart);//查询序列号
				delay_ms(50);
				if(GSMUART_buff[0]==0x0d&&GSMUART_buff[1]==0x0a)poscd1=3;
				poscd2=strsearch(GSMUART_buff,"\x0d\x0a\x0d\x0aOK\x0d\x0a");
				if(poscd1!=0&&poscd2!=0)
				{	
						CoypArraryAtoArraryB_LR(GSMUART_buff,cdkeybuffer,poscd2-poscd1,poscd1-1);//将序列号复制到cdkeybuffer
						cdkey_length=poscd2-poscd1;
						once_rand=system_rand;//获取此时系统随机数
						strcat((char*)sign_buf,(char*)login_password);//数组连接
						strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//数组连接
						MD5(sign_buf);//计算MD5
						URL_LEN=MyEE_ReadWord(HTTP_SCURL_LenAdd);//读取FLASH的生产工作域名的长度
						for(i=0;i<URL_LEN;i++)URL_BUF[i]=MyEE_ReadWord(HTTP_SCURL_Add+i);//读取FLASH的生产工作域名
						clear_USART1buff();
						UART_string_newline("AT+HTTPINIT",SIM_Uart);//初始化HTTP
						delay_ms(100); 
						clear_USART1buff();
						UART_string_newline("AT+HTTPPARA=\"CID\",1",SIM_Uart);//设置HTTP会话参数
						delay_ms(100); 	
						clear_USART1buff();
						UART_string_newline("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"",SIM_Uart);//配置http中的post的"CONTENT"
						delay_ms(100);
						clear_USART1buff();
						UART_string("AT+HTTPPARA=\"URL\",\"",SIM_Uart);//设置URL
						UART_string(URL_BUF,1);
						UART_string_newline("/json/RegSim\"",SIM_Uart);//配置push"URL"
						delay_ms(100);
						length_buffer=strlen((char*)MD5_BUF)+strlen((char*)U16IntToStr(once_rand))+strlen((char*)cdkeybuffer)+20;//计算上传数据的长度
						clear_USART1buff();				
						UART_string("AT+HTTPDATA=",SIM_Uart);
						UART_string(U16IntToStr(length_buffer),SIM_Uart);
						UART_string_newline(",10000",SIM_Uart);//配置push的data长度					
						delay_ms(100);
						if(strsearch(GSMUART_buff,"DOWNLOAD"))
						{
								UART_string("SimCode=",SIM_Uart);//8
								UART_string(cdkeybuffer,SIM_Uart);
								UART_string("&sign=",SIM_Uart);//6
								UART_string((unsigned char*)MD5_BUF,SIM_Uart);
								UART_string("&once=",SIM_Uart);//6
								UART_string(U16IntToStr(once_rand),SIM_Uart);
								clear_USART1buff();
								delay_ms(1000);
								UART_string_newline("AT+HTTPACTION=1",SIM_Uart);//开始传送
								for(i=0;i<50;i++)//等待服务器响应200成功
								{
										delay_ms(100);
										if(strsearch(GSMUART_buff,"200"))break;
								}
								if(strsearch(GSMUART_buff,",200,"))//判断响应是否是200
								{
										clear_USART1buff();
										UART_string_newline("AT+HTTPREAD=0,1000",SIM_Uart);//读服务器内容前1000个字节
										delay_ms(100);
										if(strsearch(GSMUART_buff,"\"ResultCode\":1")||strsearch(GSMUART_buff,"\"ResultCode\":2"))//首次注册
										{
												analysis_register();
												UART_string_newline("AT+HTTPTERM",SIM_Uart);
												delay_ms(50);
												return 0;//成功获取
										}
										else
										{
												UART_string_newline("AT+HTTPTERM",SIM_Uart);
												delay_ms(50);
												return 3;
										}
								}
								UART_string_newline("AT+HTTPTERM",SIM_Uart);
								delay_ms(50);		
						}
						UART_string_newline("AT+HTTPTERM",SIM_Uart);
						delay_ms(50);		
				}
				
		}
		return 1;
}

void analysis_register(void)
{
		cJSON *root;
		char *Buf1;
		char *Buf2;
		u8 HostURL_Len=0,Queue_BUFF=0;
		u8 JsonRegisterBuffer[512]={0};//生产注册时获取的URL缓存
		u16 posid1=0,posid2=0,i=0,cdkey_crc16_value=0;
		BaseType_t err;
		posid1=strsearch(GSMUART_buff,"{");//获取关键字位置
		posid2=EndstrsearchGSMUART_buff("}");//获取关键字位置
		CoypArraryAtoArraryB_LR(GSMUART_buff,JsonRegisterBuffer,posid2-posid1+1,posid1-1);//复制要获取的部分数组
		
    //将json数据解析成json结构体
    root= cJSON_Parse((char*)JsonRegisterBuffer);
    if(root)
    { 
				SIM800C_Register.Json_ResultCode=cJSON_GetObjectItem(root, "ResultCode")->valueint;//int型
				SIM800C_Register.Json_SimID=cJSON_GetObjectItem(root, "SimID")->valueint;//int型
			  Buf1=cJSON_GetObjectItem(root, "SimCode")->valuestring;//字符串型
				strcpy(SIM800C_Register.Json_SimCode,Buf1);
				SIM800C_Register.Json_EncryptMode=cJSON_GetObjectItem(root, "EncryptMode")->valueint;//int型
				Buf2=cJSON_GetObjectItem(root,"HostUrl")->valuestring;
				strcpy(SIM800C_Register.Json_HostUrl,Buf2);
				SIM800C_Register.Json_MqttPort=cJSON_GetObjectItem(root, "MqttPort")->valueint;//int型
        //释放资源
        cJSON_Delete(root);
    }
		cdkey_crc16_value=get_crc16_value(cdkeybuffer,cdkey_length);//计算CRC16的值
		//解析完后写入Flash
		FLASH_Unlock();
		HostURL_Len=strlen((char *)SIM800C_Register.Json_HostUrl);
		MyEE_WriteWord(HTTP_URL_LenAdd,HostURL_Len);//有效服务器长度
		for(i=0;i<HostURL_Len;i++)
				MyEE_WriteWord(HTTP_URL_Add+i,SIM800C_Register.Json_HostUrl[i]);
		MyEE_WriteWord(IDAddH,SIM800C_Register.Json_SimID>>16);
		MyEE_WriteWord(IDAddL,SIM800C_Register.Json_SimID);
		MyEE_WriteWord(EncryptMode_Add,SIM800C_Register.Json_EncryptMode);
		MyEE_WriteWord(MqttPort_Add,SIM800C_Register.Json_MqttPort);
		MyEE_WriteWord(CDKEY_CRC16Add,cdkey_crc16_value);//保存CRC到FLASH
		MyEE_WriteWord(SC_SUCCEED_Add,SC_SUCCEED_Symbol);//写生产标志位
		FLASH_Lock();
		Queue_BUFF=Save_Register;err=xQueueSend(ASSIT_Message_Queue,&Queue_BUFF,portMAX_DELAY);
}

void GetRegisterFlash(void)
{
		u16 URL_LEN=0,i=0;
		u8 UartBuffer[24]={0};
		HTTP_PCB_ID=60;//从Flash获取ID
		EncryptMode=1;//从Flash中获取MODE
		FLahMqttPort=1821;
		URL_LEN=24;//读取FLASH的正常工作域名
		for(i=0;i<24;i++)UartBuffer[i]=0;
		UART_string("ID:",Debug_Uart);
		sprintf((char*)PCB_ID_STR,"%d",HTTP_PCB_ID);
		UART_string_newline(PCB_ID_STR,Debug_Uart);
		for(i=0;i<24;i++)UartBuffer[i]=0;
		UART_string("EncryptMode:",Debug_Uart);
		sprintf((char*)UartBuffer,"%d",EncryptMode);
		UART_string_newline(UartBuffer,Debug_Uart);
		for(i=0;i<24;i++)UartBuffer[i]=0;
		UART_string("FLahMqttPort:",Debug_Uart);
		sprintf((char*)UartBuffer,"%d",FLahMqttPort);
		UART_string_newline(UartBuffer,Debug_Uart);
		UART_string("FlashURL:",Debug_Uart);
		UART_string_newline(FlashURL,Debug_Uart);
}

//**********************************************
//描述:检查FALSH中的CRC是否和序列号计算CRC出来的值一致，如果不一致，死循环
void Check_CDKEY(void)
{
		u8 poscd1=0,poscd2=0,plength=0,pbuffer[32]={0};
		u16 pcdkey_crc=0,check_cdkkey_crc=0;
		clear_USART1buff();
		UART_string_newline("AT+CGSN",SIM_Uart);//查询序列号
		delay_ms(50);
		if(GSMUART_buff[0]==0x0d&&GSMUART_buff[1]==0x0a)poscd1=3;
		poscd2=strsearch(GSMUART_buff,"\x0d\x0a\x0d\x0aOK\x0d\x0a");
		if(poscd1!=0&&poscd2!=0)
		{	
				CoypArraryAtoArraryB_LR(GSMUART_buff,pbuffer,poscd2-poscd1,poscd1-1);//复制部分数组
				plength=poscd2-poscd1;
				pcdkey_crc=get_crc16_value(pbuffer,plength);//计算CRC
				check_cdkkey_crc=MyEE_ReadWord(CDKEY_CRC16Add);//获取FLASH的CRC
				if(pcdkey_crc!=check_cdkkey_crc)//判断是否一样
				{
						while(1);//死循环
				}
		}
}
