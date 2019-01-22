#include<reg52.h>
#include"LCD.h"
#include"i2c.h"
#include"temp.h"
#include<string.h>


	
#define open     'A'
#define stop     'B'
#define uint    unsigned int                          //�ض����޷�����������
#define uchar   unsigned char                         //�ض����޷����ַ�����

bit  flag_REC=0; 
bit  flag    =0;  
bit  flagrun =0;

uchar  i=0,redbit=0;
uchar  dat=0;
uchar  buff[5]=0; //���ջ����ֽ�
/************************************************************************/
	uint redread()
	{
	  if(P0^1==0)
		{
		  return
			redbit=1;
		}
		else
		{
		  return
			redbit=0;
		}	
	}

/************************************************************************/
 void  stoprun(void)
 {
   P2=0xff;  //P2^0
 }
 void  run(void)
 {
   P2=0xfe;
 }	 
/************************************************************************/	
//��ʱ����	
   void delay(uint k)
{    
     uint x,y;
	 for(x=0;x<k;x++) 
	   for(y=0;y<2000;y++);
}
/************************************************************************/

void TimInit()
{
  	TMOD=0x20;  
    TH1=0xFd;  		   //11.0592M����9600������
    TL1=0xFd;
    SCON=0x50;  
    PCON=0x00; 
    TR1=1;
	  ES=1;   
    EA=1;
}
void sint() interrupt 4	  //�жϽ���3���ֽ�
{ 
 
    if(RI)	                 //�Ƿ�����ж�
   {
       RI=0;
       dat=SBUF;
       if(dat=='O'&&(i==0)) //�������ݵ�һ֡
         {
            buff[i]=dat;
            flag=1;        //��ʼ��������
         }
       else 
			 if(flag==1)
			 {
				i++;
				buff[i]=dat;
				if(i>=2)
				{i=0;flag=0;flag_REC=1 ;}  // ֹͣ����
			 }
	 }
}
/*********************************************************************/		 
/*--������--*/
void main(void)
{
  	int temp;
	char p[2];
	TimInit();
  	Ds18b20Init();
  	LCD_Init();	
	while(1)							/*����ѭ��*/
	{ 
	  if(flag_REC==1)				    //
	   {
				flag_REC=0;
				if(buff[0]=='O'&&buff[1]=='N')	//��һ���ֽ�ΪO���ڶ����ֽ�ΪN���������ֽ�Ϊ������
				switch(buff[2])
				 {
						case open :						    // ����
						flagrun=1; 
						break;
						
						case stop:						// ֹͣ
						flagrun=0;
						break;
				 }				 
	   }
		 if(flagrun)
			 run();
		 else
			 stoprun();
		temp=Ds18b20ReadTemp();
     	p[0]=temp/10;
		p[1]=temp%10;
		LCD_P6x8Str(54,0,p);
	}
}	