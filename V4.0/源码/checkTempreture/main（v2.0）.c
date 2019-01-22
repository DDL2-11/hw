
// 显示温控标准后闪烁、显示当前温度后闪烁、睡眠1秒


#define MAIN_Fosc		22118400L	//定义主时钟

#include "STC15Fxxxx.H"
#include "Display.H"
#include "SERIAL.H"
#include "KEY.H"
#include "WIFI.H"




/********************** 主函数 ************************/
void main(void)
{
	u8	i;
	u8 send_buffer[2]; //发送缓冲
	u8 remoteKeyCode = 0;//记录键码
	
	
	//所有变量声明都要在函数调用前面
	
	//串口初始化，使用time1作为波特率
	display_init();
	key_init();
	serial_init();	

	
	esp8266_init();
	
	
	while(1)
	{
		
		
		//----------------按键扫描模块------------------//
		//----------------通信模块------------------------//
		if((TX1_Cnt != RX1_Cnt) && (!B_TX1_Busy))  //接受到通信
        {
					  //RX1_Buffer 接受数据缓冲
            //收到数据，并将数据中的键码赋给变量remoteKeyCode
				   	remoteKeyCode = RX1_Buffer[TX1_Cnt];
            if(++TX1_Cnt >= UART1_BUF_LENGTH)   TX1_Cnt = 0;  //UART1-BUF-LENGTH == 32
        }
		
		//----------------显示模块---------------------//
				
		//SleepDelay = 0;		
				
		if(B_1ms){

			B_1ms = 0;
			
			if(++cnt50ms >= 50) //50ms扫描一次行列键盘，判断是否有输入
			{
				cnt50ms = 0;
				IO_KeyScan();
			}
			
			//-----------发送方-----------//
			if(KeyCode != 0) //keyCode不等于零，即有按键被按下，需要发送一个指令出去
			{
				send_buffer[0] = KeyCode;
				send_buffer[1] = 0;
				PrintString1(send_buffer);
				KeyCode = 0;
			}
			
			//------------接收方----------//
			if(remoteKeyCode != 0) //接收方接受到有键按下
			{
				if(if_change == 0){
					//键位17给小时++
					if(remoteKeyCode == 17){
						hour++;
						if(hour >= 24) hour = 0;
					}
					//键位18给小时--
					if(remoteKeyCode == 18){
						hour--;
						if(hour < 0) hour = 23;
					}
					//键位19给分钟++
					if(remoteKeyCode == 19){
						min++;
						if(min >= 60){
							min = 0;
							hour++;
							if(hour >= 24) hour = 0;
						}
					}
					//键位20给分钟--
					if(remoteKeyCode == 20){
						min--;
						if(min < 0){
							min = 59;
							hour--;
							if(hour < 0) hour = 23;
						}
					}
					//键位21，将日期重置
					if(remoteKeyCode == 21){
						hour = 12;
						min = 0;
						sec = 0;
					}
				}
				
				//键位24(第二排最后一个），停止切换模式并且时钟倒计时停止
				if(remoteKeyCode == 24){
					if_change = 0;
				}
				//键位28（第三排最后一个），开始切换模式，此时不能调整倒计时
				if(remoteKeyCode == 28){
					if_change = 1;
				}
				DisplayClock();
				remoteKeyCode = 0;
			}
			
			
			
			if(++msecond >= 1000)	//1秒到
			{
				//倒计时
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

				
				msecond = 0;		//清1000ms计数
				for(i=0; i<8; i++)	LED8[i] = DIS_BLACK;
				
				//------------------显示部分--------------//
				
				++SleepDelay;
				if(if_change && SleepDelay % 2 == 0){
				//if(SleepDelay % 2 == 0){
					//设定温度
					LED8[2] = Target_tem % 10;
					LED8[1] = Target_tem % 100 / 10;
					LED8[0] = Target_tem / 100;
					//for(i=0; i<5; i++)	LED8[i] = DIS_BLACK;
					
					//实际温度

					//j = Get_ADC10bitResult(2);
					
					j = Get_ADC10bitResult(3);	//参数0~7,查询方式做一次ADC, 返回值就是结果, == 1024 为错误
					j += Get_ADC10bitResult(3);
					j += Get_ADC10bitResult(3);
					j += Get_ADC10bitResult(3);
					
					if(j < 1024*4)
					{
						j =	get_temperature(j);	//计算温度值

						if(j >= 400)	F0 = 0,	j -= 400;		//温度 >= 0度
						else			F0 = 1,	j  = 400 - j;	//温度 <  0度
						settemp(j);								//传给wifi 
						LED8[4] = j / 1000;		//显示温度值
						LED8[5] = (j % 1000) / 100;
						LED8[6] = (j % 100) / 10 + DIS_DOT;
						LED8[7] = j % 10;
						if(LED8[4] == 0)	LED8[4] = DIS_BLACK;
						if(F0)	LED8[4] = DIS_;		//显示-
					}
					else	//错误
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
				else if(SleepDelay == 8){ // 关闭显示
					SleepDelay = 0;

					P_HC595_SER = 0;
					for(i=0; i<16; i++)		//先关闭显示，省电
					{
						P_HC595_SRCLK = 1;
						P_HC595_SRCLK = 0;
					}
					P_HC595_RCLK = 1;
					P_HC595_RCLK = 0;		//锁存输出数据
					
					SetWakeUpTime(500);	//0.5秒后醒来

					PCON |= 0x02;	//Sleep
					_nop_();
					_nop_();
					_nop_();

				}
				else{
					for(i = 0;i<8;i++) LED8[i] = i;
				}
				*/
			}//1秒
		}
	}
} 
/**********************************************/





