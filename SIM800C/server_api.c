#include "server_api.h"

u8 FlagBIT_BUF[17]={0};//��־λ����

Once_ProbeConfig_PGS_ES PushDETEPIDProbeConfig_PGS_ES;

Once_ProbeConfig_IAS PushDETEPIDProbeConfig_IAS[10]={0};

u16 poshex=0;
void Analyse_ConfigHexData(void)
{
		u8 Queue_BUFF=0,i=0;
		BaseType_t err;
	
		poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x01",5);
		PushDETEPIDProbeConfig_PGS_ES.LA_ES=GSMUART_buff[poshex+5+3+1-1]<<8|GSMUART_buff[poshex+5+3+2-1];
		PushDETEPIDProbeConfig_PGS_ES.CAA_ES=GSMUART_buff[poshex+5+18+1-1]<<8|GSMUART_buff[poshex+5+18+2-1];
		PushDETEPIDProbeConfig_PGS_ES.CBA_ES=GSMUART_buff[poshex+5+21+1-1]<<8|GSMUART_buff[poshex+5+21+2-1];
		PushDETEPIDProbeConfig_PGS_ES.CCA_ES=GSMUART_buff[poshex+5+24+1-1]<<8|GSMUART_buff[poshex+5+24+2-1];
		PushDETEPIDProbeConfig_PGS_ES.VABCA_ES=GSMUART_buff[poshex+5+27+1-1]<<8|GSMUART_buff[poshex+5+27+2-1];
		printf("PushDETEPIDProbeConfig_PGS_ES.LA_ES:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.LA_ES);
		printf("PushDETEPIDProbeConfig_PGS_ES.CAA_ES:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.CAA_ES);
		printf("PushDETEPIDProbeConfig_PGS_ES.CBA_ES:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.CBA_ES);
		printf("PushDETEPIDProbeConfig_PGS_ES.CCA_ES:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.CCA_ES);
		printf("PushDETEPIDProbeConfig_PGS_ES.VABCA_ES:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.VABCA_ES);
		
		poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x02",5);
		PushDETEPIDProbeConfig_PGS_ES.LA_PGS=GSMUART_buff[poshex+5+3+1-1]<<8|GSMUART_buff[poshex+5+3+2-1];
		PushDETEPIDProbeConfig_PGS_ES.CAA_PGS=GSMUART_buff[poshex+5+18+1-1]<<8|GSMUART_buff[poshex+5+18+2-1];
		PushDETEPIDProbeConfig_PGS_ES.CBA_PGS=GSMUART_buff[poshex+5+21+1-1]<<8|GSMUART_buff[poshex+5+21+2-1];
		PushDETEPIDProbeConfig_PGS_ES.CCA_PGS=GSMUART_buff[poshex+5+24+1-1]<<8|GSMUART_buff[poshex+5+24+2-1];
		PushDETEPIDProbeConfig_PGS_ES.VABCA_PGS=GSMUART_buff[poshex+5+27+1-1]<<8|GSMUART_buff[poshex+5+27+2-1];
		printf("PushDETEPIDProbeConfig_PGS_ES.LA_PGS:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.LA_PGS);
		printf("PushDETEPIDProbeConfig_PGS_ES.CAA_PGS:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.CAA_PGS);
		printf("PushDETEPIDProbeConfig_PGS_ES.CBA_PGS:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.CBA_PGS);
		printf("PushDETEPIDProbeConfig_PGS_ES.CCA_PGS:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.CCA_PGS);
		printf("PushDETEPIDProbeConfig_PGS_ES.VABCA_PGS:%d\r\n",PushDETEPIDProbeConfig_PGS_ES.VABCA_PGS);
		
		poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x03",5);
		//
		poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x04",5);
		//
		poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x05",5);
		//
		for(i=0;i<10;i++)
		{
				switch(i)
				{
						case 0:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x06",5);break;
						case 1:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x07",5);break;
						case 2:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x08",5);break;
						case 3:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x09",5);break;
						case 4:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x0A",5);break;
						case 5:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x0B",5);break;
						case 6:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x0C",5);break;
						case 7:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x0D",5);break;
						case 8:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x0E",5);break;
						case 9:poshex=HEXsearchforGSMUART_buff("\x01\x00\x00\x00\x0F",5);break;
					  default:break;
						
				}
				PushDETEPIDProbeConfig_IAS[i].LA_IAS=GSMUART_buff[poshex+5+3+1-1]<<8|GSMUART_buff[poshex+5+3+2-1];
				PushDETEPIDProbeConfig_IAS[i].T1A_IAS=GSMUART_buff[poshex+5+6+1-1]<<8|GSMUART_buff[poshex+5+6+2-1];
				PushDETEPIDProbeConfig_IAS[i].T2A_IAS=GSMUART_buff[poshex+5+9+1-1]<<8|GSMUART_buff[poshex+5+9+2-1];
				PushDETEPIDProbeConfig_IAS[i].T3A_IAS=GSMUART_buff[poshex+5+12+1-1]<<8|GSMUART_buff[poshex+5+12+2-1];
				PushDETEPIDProbeConfig_IAS[i].T4A_IAS=GSMUART_buff[poshex+5+15+1-1]<<8|GSMUART_buff[poshex+5+15+2-1];
				PushDETEPIDProbeConfig_IAS[i].CAA_IAS=GSMUART_buff[poshex+5+18+1-1]<<8|GSMUART_buff[poshex+5+18+2-1];
				PushDETEPIDProbeConfig_IAS[i].CBA_IAS=GSMUART_buff[poshex+5+21+1-1]<<8|GSMUART_buff[poshex+5+21+2-1];
				PushDETEPIDProbeConfig_IAS[i].CCA_IAS=GSMUART_buff[poshex+5+24+1-1]<<8|GSMUART_buff[poshex+5+24+2-1];
				PushDETEPIDProbeConfig_IAS[i].VABCA_IAS=GSMUART_buff[poshex+5+27+1-1]<<8|GSMUART_buff[poshex+5+27+2-1];
				printf("<<<<<<<<IAS=%d>>>>>>>>\r\n",i);
				printf("PushDETEPIDProbeConfig_IAS.LA_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].LA_IAS);
				printf("PushDETEPIDProbeConfig_IAS.T1A_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].T1A_IAS);
				printf("PushDETEPIDProbeConfig_IAS.T2A_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].T2A_IAS);
				printf("PushDETEPIDProbeConfig_IAS.T3A_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].T3A_IAS);
				printf("PushDETEPIDProbeConfig_IAS.T4A_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].T4A_IAS);
				printf("PushDETEPIDProbeConfig_IAS.CAA_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].CAA_IAS);
				printf("PushDETEPIDProbeConfig_IAS.CBA_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].CBA_IAS);
				printf("PushDETEPIDProbeConfig_IAS.CCA_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].CCA_IAS);
				printf("PushDETEPIDProbeConfig_IAS.VABCA_IAS:%d\r\n",PushDETEPIDProbeConfig_IAS[i].VABCA_IAS);
		}
		
		//��������ֵ��̽����
		Download_AlarmOfValue();  
		Queue_BUFF=W_DETE_Alarming_Value;
		err=xQueueSendToFront(DETE_Message_Queue,&Queue_BUFF,portMAX_DELAY);//����̽��������ֵ
		
}

void Find_FlagBit(u8 mode)
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
						Queue_BUFF=MQTT_SendDate;
						err=xQueueSendToFront(GSM_Message_Queue,&Queue_BUFF,portMAX_DELAY);//�ϴ�����
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
								Queue_BUFF=MQTT_GetConfig;
								err=xQueueSendToFront(GSM_Message_Queue,&Queue_BUFF,portMAX_DELAY);//��ȥ��ȡ������Ϣ
								UART_string_newline("��ȥ��ȡ������Ϣ",Debug_Uart);
						}
				}
				if(FlagbitBuffer[4]=='1')//���·���������
				{
						FlagBITcnt=4;
						FlagBIT_BUF[4]='1';
						Queue_BUFF=MQTT_UpdataConfig;
						err=xQueueSendToFront(GSM_Message_Queue,&Queue_BUFF,portMAX_DELAY);//�޸�̽��������ֵ
						UART_string_newline("�յ�UpConfigSignal!!!",Debug_Uart);
				}
				if(FlagBITcnt!=99)
				{
						UART_string_newline("�����������",Debug_Uart);
						Queue_BUFF=MQTT_ClearFlagBit;
						err=xQueueSendToFront(GSM_Message_Queue,&Queue_BUFF,portMAX_DELAY);
				}				
							

		}

}

//����:����ϴ��ı�־λ����
void clear_FlagBIT(void)
{
		u8 i=0;
		for(i=0;i<16;i++)FlagBIT_BUF[i]='0';
		FlagBIT_BUF[16]=0;
}


void Download_AlarmOfValue(void)
{
	   u8 slave =0;
	   //�����������ֵ����
		 Elec_Alarmset.A_T_Cur_A = PushDETEPIDProbeConfig_PGS_ES.CAA_ES;
		 Elec_Alarmset.A_T_Cur_B = PushDETEPIDProbeConfig_PGS_ES.CBA_ES;
		 Elec_Alarmset.A_T_Cur_C = PushDETEPIDProbeConfig_PGS_ES.CCA_ES;
		 Elec_Alarmset.A_T_Cur_L = PushDETEPIDProbeConfig_PGS_ES.LA_ES;
		 Elec_Alarmset.A_C_Volt_A = PushDETEPIDProbeConfig_PGS_ES.VABCA_ES;
	   Elec_Alarmset.A_C_Volt_B = PushDETEPIDProbeConfig_PGS_ES.VABCA_ES;
		 Elec_Alarmset.A_C_Volt_C = PushDETEPIDProbeConfig_PGS_ES.VABCA_ES;
		 Elec_Alarmset.A_D_Volt_A = PushDETEPIDProbeConfig_PGS_ES.VABCA_PGS;
		 Elec_Alarmset.A_D_Volt_B = PushDETEPIDProbeConfig_PGS_ES.VABCA_PGS;
		 Elec_Alarmset.A_D_Volt_C = PushDETEPIDProbeConfig_PGS_ES.VABCA_PGS;
	   
	  //�ֻ�·����ֵ����
	  for(slave = 0; slave <10;slave++)
	  {
		  IAS_Alarmset[slave].A_Loop_Volt_Over    = (int)PushDETEPIDProbeConfig_IAS[slave].VABCA_IAS*1.25;
			IAS_Alarmset[slave].A_Loop_Volt_Under   = (int)PushDETEPIDProbeConfig_IAS[slave].VABCA_IAS*0.75;
		  IAS_Alarmset[slave].A_Loop_Curr_L       = PushDETEPIDProbeConfig_IAS[slave].LA_IAS;
			IAS_Alarmset[slave].A_Loop_Temp         = PushDETEPIDProbeConfig_IAS[slave].T1A_IAS;
			IAS_Alarmset[slave].A_Loop_Curr         = PushDETEPIDProbeConfig_IAS[slave].CAA_IAS;
			IAS_Alarmset[slave].A_Loop_Volt_PreOver = (int)PushDETEPIDProbeConfig_IAS[slave].VABCA_IAS*1.2;
			IAS_Alarmset[slave].A_Loop_Volt_PreUnder= (int)PushDETEPIDProbeConfig_IAS[slave].VABCA_IAS*0.8;
			delay_ms(100);
	  }
}

