#include "http_api.h"

const u8 login_password[]="antenicantenic888";//��¼�˻�����

Json_Register SIM800C_Register;
u8 cdkey_length=0;//CDKEY����

u8 cdkeybuffer[18]={0};//CDKEY����
u32 HTTP_PCB_ID=0;//�ӷ����ȡ��ע��ID
u8 EncryptMode=0;
u16 FLahMqttPort=0;
u8 FlashURL[100]="http://pilot.antenic.com";
char PCB_ID_STR[11]={0};

//***********************
//����:POSTCDKEY
//����:0:�ɹ� 1:ʧ��
u8 POST_HTTP_CDKEY_MOTHOD(void)
{
		u8 sign_buf[64]={0},URL_BUF[50]={0};
		u16 poscd1=0,poscd2=0,URL_LEN=0,i=0,once_rand=0,length_buffer=0;
		clear_USART1buff();
		UART_string_newline("AT+SAPBR=2,1",SIM_Uart);//��ѯGPRS�������Ƿ�����
		delay_ms(500); 
		if(!strsearch(GSMUART_buff,"0.0.0.0"))
		{	
				clear_USART1buff();
				UART_string_newline("AT+CGSN",SIM_Uart);//��ѯ���к�
				delay_ms(50);
				if(GSMUART_buff[0]==0x0d&&GSMUART_buff[1]==0x0a)poscd1=3;
				poscd2=strsearch(GSMUART_buff,"\x0d\x0a\x0d\x0aOK\x0d\x0a");
				if(poscd1!=0&&poscd2!=0)
				{	
						CoypArraryAtoArraryB_LR(GSMUART_buff,cdkeybuffer,poscd2-poscd1,poscd1-1);//�����кŸ��Ƶ�cdkeybuffer
						cdkey_length=poscd2-poscd1;
						once_rand=system_rand;//��ȡ��ʱϵͳ�����
						strcat((char*)sign_buf,(char*)login_password);//��������
						strcat((char*)sign_buf,(char*)U16IntToStr(once_rand));//��������
						MD5(sign_buf);//����MD5
						URL_LEN=MyEE_ReadWord(HTTP_SCURL_LenAdd);//��ȡFLASH���������������ĳ���
						for(i=0;i<URL_LEN;i++)URL_BUF[i]=MyEE_ReadWord(HTTP_SCURL_Add+i);//��ȡFLASH��������������
						clear_USART1buff();
						UART_string_newline("AT+HTTPINIT",SIM_Uart);//��ʼ��HTTP
						delay_ms(100); 
						clear_USART1buff();
						UART_string_newline("AT+HTTPPARA=\"CID\",1",SIM_Uart);//����HTTP�Ự����
						delay_ms(100); 	
						clear_USART1buff();
						UART_string_newline("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"",SIM_Uart);//����http�е�post��"CONTENT"
						delay_ms(100);
						clear_USART1buff();
						UART_string("AT+HTTPPARA=\"URL\",\"",SIM_Uart);//����URL
						UART_string(URL_BUF,1);
						UART_string_newline("/json/RegSim\"",SIM_Uart);//����push"URL"
						delay_ms(100);
						length_buffer=strlen((char*)MD5_BUF)+strlen((char*)U16IntToStr(once_rand))+strlen((char*)cdkeybuffer)+20;//�����ϴ����ݵĳ���
						clear_USART1buff();				
						UART_string("AT+HTTPDATA=",SIM_Uart);
						UART_string(U16IntToStr(length_buffer),SIM_Uart);
						UART_string_newline(",10000",SIM_Uart);//����push��data����					
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
								UART_string_newline("AT+HTTPACTION=1",SIM_Uart);//��ʼ����
								for(i=0;i<50;i++)//�ȴ���������Ӧ200�ɹ�
								{
										delay_ms(100);
										if(strsearch(GSMUART_buff,"200"))break;
								}
								if(strsearch(GSMUART_buff,",200,"))//�ж���Ӧ�Ƿ���200
								{
										clear_USART1buff();
										UART_string_newline("AT+HTTPREAD=0,1000",SIM_Uart);//������������ǰ1000���ֽ�
										delay_ms(100);
										if(strsearch(GSMUART_buff,"\"ResultCode\":1")||strsearch(GSMUART_buff,"\"ResultCode\":2"))//�״�ע��
										{
												analysis_register();
												UART_string_newline("AT+HTTPTERM",SIM_Uart);
												delay_ms(50);
												return 0;//�ɹ���ȡ
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
		u8 JsonRegisterBuffer[512]={0};//����ע��ʱ��ȡ��URL����
		u16 posid1=0,posid2=0,i=0,cdkey_crc16_value=0;
		BaseType_t err;
		posid1=strsearch(GSMUART_buff,"{");//��ȡ�ؼ���λ��
		posid2=EndstrsearchGSMUART_buff("}");//��ȡ�ؼ���λ��
		CoypArraryAtoArraryB_LR(GSMUART_buff,JsonRegisterBuffer,posid2-posid1+1,posid1-1);//����Ҫ��ȡ�Ĳ�������
		
    //��json���ݽ�����json�ṹ��
    root= cJSON_Parse((char*)JsonRegisterBuffer);
    if(root)
    { 
				SIM800C_Register.Json_ResultCode=cJSON_GetObjectItem(root, "ResultCode")->valueint;//int��
				SIM800C_Register.Json_SimID=cJSON_GetObjectItem(root, "SimID")->valueint;//int��
			  Buf1=cJSON_GetObjectItem(root, "SimCode")->valuestring;//�ַ�����
				strcpy(SIM800C_Register.Json_SimCode,Buf1);
				SIM800C_Register.Json_EncryptMode=cJSON_GetObjectItem(root, "EncryptMode")->valueint;//int��
				Buf2=cJSON_GetObjectItem(root,"HostUrl")->valuestring;
				strcpy(SIM800C_Register.Json_HostUrl,Buf2);
				SIM800C_Register.Json_MqttPort=cJSON_GetObjectItem(root, "MqttPort")->valueint;//int��
        //�ͷ���Դ
        cJSON_Delete(root);
    }
		cdkey_crc16_value=get_crc16_value(cdkeybuffer,cdkey_length);//����CRC16��ֵ
		//�������д��Flash
		FLASH_Unlock();
		HostURL_Len=strlen((char *)SIM800C_Register.Json_HostUrl);
		MyEE_WriteWord(HTTP_URL_LenAdd,HostURL_Len);//��Ч����������
		for(i=0;i<HostURL_Len;i++)
				MyEE_WriteWord(HTTP_URL_Add+i,SIM800C_Register.Json_HostUrl[i]);
		MyEE_WriteWord(IDAddH,SIM800C_Register.Json_SimID>>16);
		MyEE_WriteWord(IDAddL,SIM800C_Register.Json_SimID);
		MyEE_WriteWord(EncryptMode_Add,SIM800C_Register.Json_EncryptMode);
		MyEE_WriteWord(MqttPort_Add,SIM800C_Register.Json_MqttPort);
		MyEE_WriteWord(CDKEY_CRC16Add,cdkey_crc16_value);//����CRC��FLASH
		MyEE_WriteWord(SC_SUCCEED_Add,SC_SUCCEED_Symbol);//д������־λ
		FLASH_Lock();
		Queue_BUFF=Save_Register;err=xQueueSend(ASSIT_Message_Queue,&Queue_BUFF,portMAX_DELAY);
}

void GetRegisterFlash(void)
{
		u16 URL_LEN=0,i=0;
		u8 UartBuffer[24]={0};
		HTTP_PCB_ID=60;//��Flash��ȡID
		EncryptMode=1;//��Flash�л�ȡMODE
		FLahMqttPort=1821;
		URL_LEN=24;//��ȡFLASH��������������
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
//����:���FALSH�е�CRC�Ƿ�����кż���CRC������ֵһ�£������һ�£���ѭ��
void Check_CDKEY(void)
{
		u8 poscd1=0,poscd2=0,plength=0,pbuffer[32]={0};
		u16 pcdkey_crc=0,check_cdkkey_crc=0;
		clear_USART1buff();
		UART_string_newline("AT+CGSN",SIM_Uart);//��ѯ���к�
		delay_ms(50);
		if(GSMUART_buff[0]==0x0d&&GSMUART_buff[1]==0x0a)poscd1=3;
		poscd2=strsearch(GSMUART_buff,"\x0d\x0a\x0d\x0aOK\x0d\x0a");
		if(poscd1!=0&&poscd2!=0)
		{	
				CoypArraryAtoArraryB_LR(GSMUART_buff,pbuffer,poscd2-poscd1,poscd1-1);//���Ʋ�������
				plength=poscd2-poscd1;
				pcdkey_crc=get_crc16_value(pbuffer,plength);//����CRC
				check_cdkkey_crc=MyEE_ReadWord(CDKEY_CRC16Add);//��ȡFLASH��CRC
				if(pcdkey_crc!=check_cdkkey_crc)//�ж��Ƿ�һ��
				{
						while(1);//��ѭ��
				}
		}
}
