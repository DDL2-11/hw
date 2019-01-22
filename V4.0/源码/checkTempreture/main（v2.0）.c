
// ��ʾ�¿ر�׼����˸����ʾ��ǰ�¶Ⱥ���˸��˯��1��


#define MAIN_Fosc		22118400L	//������ʱ��

#include "STC15Fxxxx.H"
#include "Display.H"
#include "SERIAL.H"
#include "KEY.H"
#include "WIFI.H"




/********************** ������ ************************/
void main(void)
{
	u8	i;
	u8 send_buffer[2]; //���ͻ���
	u8 remoteKeyCode = 0;//��¼����
	
	
	//���б���������Ҫ�ں�������ǰ��
	
	//���ڳ�ʼ����ʹ��time1��Ϊ������
	display_init();
	key_init();
	serial_init();	

	
	esp8266_init();
	
	
	while(1)
	{
		
		
		//----------------����ɨ��ģ��------------------//
		//----------------ͨ��ģ��------------------------//
		if((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy))  //���ܵ�ͨ��
        {
					  //RX1_Buffer �������ݻ���
            //�յ����ݣ����������еļ��븳������remoteKeyCode
				   	remoteKeyCode = RX1_Buffer[TX1_Cnt];
            if(++TX1_Cnt >= UART1_BUF_LENGTH)   TX1_Cnt = 0;  //UART1-BUF-LENGTH == 32
        }
		
		//----------------��ʾģ��---------------------//
				
		//SleepDelay = 0;		
				
		if(B_1ms){

			B_1ms = 0;
			
			if(++cnt50ms >= 50) //50msɨ��һ�����м��̣��ж��Ƿ�������
			{
				cnt50ms = 0;
				IO_KeyScan();
			}
			
			//-----------���ͷ�-----------//
			if(KeyCode != 0) //keyCode�������㣬���а��������£���Ҫ����һ��ָ���ȥ
			{
				send_buffer[0] = KeyCode;
				send_buffer[1] = 0;
				PrintString1(send_buffer);
				KeyCode = 0;
			}
			
			//------------���շ�----------//
			if(remoteKeyCode != 0) //���շ����ܵ��м�����
			{
				if(if_change == 0){
					//��λ17��Сʱ++
					if(remoteKeyCode == 17){
						hour++;
						if(hour >= 24) hour = 0;
					}
					//��λ18��Сʱ--
					if(remoteKeyCode == 18){
						hour--;
						if(hour < 0) hour = 23;
					}
					//��λ19������++
					if(remoteKeyCode == 19){
						min++;
						if(min >= 60){
							min = 0;
							hour++;
							if(hour >= 24) hour = 0;
						}
					}
					//��λ20������--
					if(remoteKeyCode == 20){
						min--;
						if(min < 0){
							min = 59;
							hour--;
							if(hour < 0) hour = 23;
						}
					}
					//��λ21������������
					if(remoteKeyCode == 21){
						hour = 12;
						min = 0;
						sec = 0;
					}
				}
				
				//��λ24(�ڶ������һ������ֹͣ�л�ģʽ����ʱ�ӵ���ʱֹͣ
				if(remoteKeyCode == 24){
					if_change = 0;
				}
				//��λ28�����������һ��������ʼ�л�ģʽ����ʱ���ܵ�������ʱ
				if(remoteKeyCode == 28){
					if_change = 1;
				}
				DisplayClock();
				remoteKeyCode = 0;
			}
			
			
			
			if(++msecond >= 1000)	//1�뵽
			{
				//����ʱ
				if(if_change){
					if(hour == 0 && min == 0 && sec == 0){
						run_light();
					}
					else if(sec > 0){
						sec--;
					}
					else{
						sec = 59;
						if(min > 0){
							min--;
						}
						else{
							hour--;
							min = 59;
						}
					}
				}

				
				msecond = 0;		//��1000ms����
				for(i=0; i<8; i++)	LED8[i] = DIS_BLACK;
				
				//------------------��ʾ����--------------//
				
				++SleepDelay;
				if(if_change && SleepDelay % 2 == 0){
				//if(SleepDelay % 2 == 0){
					//�趨�¶�
					LED8[2] = Target_tem % 10;
					LED8[1] = Target_tem % 100 / 10;
					LED8[0] = Target_tem / 100;
					//for(i=0; i<5; i++)	LED8[i] = DIS_BLACK;
					
					//ʵ���¶�

					//j = Get_ADC10bitResult(2);
					
					j = Get_ADC10bitResult(3);	//����0~7,��ѯ��ʽ��һ��ADC, ����ֵ���ǽ��, == 1024 Ϊ����
					j += Get_ADC10bitResult(3);
					j += Get_ADC10bitResult(3);
					j += Get_ADC10bitResult(3);
					
					if(j < 1024*4)
					{
						j =	get_temperature(j);	//�����¶�ֵ

						if(j >= 400)	F0 = 0,	j -= 400;		//�¶� >= 0��
						else			F0 = 1,	j  = 400 - j;	//�¶� <  0��
						settemp(j);								//����wifi 
						LED8[4] = j / 1000;		//��ʾ�¶�ֵ
						LED8[5] = (j % 1000) / 100;
						LED8[6] = (j % 100) / 10 + DIS_DOT;
						LED8[7] = j % 10;
						if(LED8[4] == 0)	LED8[4] = DIS_BLACK;
						if(F0)	LED8[4] = DIS_;		//��ʾ-
					}
					else	//����
					{
						for(i=0; i<8; i++)	LED8[i] = 0;
					}					
				}			
				else{
					if(hour >= 10  && hour <= 23)  LED8[0] = hour / 10;
					else            LED8[0] = DIS_BLACK;
					LED8[1] = hour % 10;
					LED8[2] = DIS_;
					LED8[3] = min / 10;
					LED8[4] = min % 10;
					LED8[6] = sec / 10;
					LED8[7] = sec % 10;
					LED8[5] = DIS_;
				}
				/*
				else if(SleepDelay == 8){ // �ر���ʾ
					SleepDelay = 0;

					P_HC595_SER = 0;
					for(i=0; i<16; i++)		//�ȹر���ʾ��ʡ��
					{
						P_HC595_SRCLK = 1;
						P_HC595_SRCLK = 0;
					}
					P_HC595_RCLK = 1;
					P_HC595_RCLK = 0;		//�����������
					
					SetWakeUpTime(500);	//0.5�������

					PCON |= 0x02;	//Sleep
					_nop_();
					_nop_();
					_nop_();

				}
				else{
					for(i = 0;i<8;i++) LED8[i] = i;
				}
				*/
			}//1��
		}
	}
} 
/**********************************************/





