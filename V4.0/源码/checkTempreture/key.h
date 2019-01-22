#ifndef STC_KEY_H_
#define STC_KEY_H_

// Using STC Library
/*************  本程序功能说明  **************

行列扫描按键键码为17~32.

按键只支持单键按下, 不支持多键同时按下, 那样将会有不可预知的结果.

键按下超过1秒后,将以10键/秒的速度提供重键输出. 用户只需要检测KeyCode是否非0来判断键是否按下.

有效键:
键码17: 
键码18: 
键码19: 
键码20: 



/****************************** 用户定义宏 ***********************************/


#define Timer0_Reload   (65536UL -(MAIN_Fosc / 1000))       //Timer 0 中断频率, 1000次/秒

/*****************************************************************************/



/*************  本地变量声明    **************/

bit B_1ms;          //1ms标志

u8 IO_KeyState, IO_KeyState1, IO_KeyHoldCnt;    //行列键盘变量
u8  KeyHoldCnt; //键按下计时
u8  KeyCode;    //给用户使用的键码, 1~16有效
u8  cnt50ms;
u16 msecond;


/*************  本地函数声明    **************/
void    CalculateAdcKey(u16 adc);
void    IO_KeyScan(void);   //50ms call
void    WriteNbyte(u8 addr, u8 *p, u8 number);
void    ReadNbyte( u8 addr, u8 *p, u8 number);



/*****************************************************
    行列键扫描程序
    使用XY查找4x4键的方法，只能单键，速度快

   Y     P04      P05      P06      P07
          |        |        |        |
X         |        |        |        |
P00 ---- K00 ---- K01 ---- K02 ---- K03 ----
          |        |        |        |
P01 ---- K04 ---- K05 ---- K06 ---- K07 ----
          |        |        |        |
P02 ---- K08 ---- K09 ---- K10 ---- K11 ----
          |        |        |        |
P03 ---- K12 ---- K13 ---- K14 ---- K15 ----
          |        |        |        |
******************************************************/


u8 code T_KeyTable[16] = {0,1,2,0,3,0,0,0,4,0,0,0,0,0,0,0};

void IO_KeyDelay(void)
{
    u8 i;
    i = 60;
    while(--i)  ;
}

void    IO_KeyScan(void)    //50ms call
{
    u8  j;

    j = IO_KeyState1;   //保存上一次状态

    P0 = 0xf0;  //X低，读Y
    IO_KeyDelay();
    IO_KeyState1 = P0 & 0xf0;

    P0 = 0x0f;  //Y低，读X
    IO_KeyDelay();
    IO_KeyState1 |= (P0 & 0x0f);
    IO_KeyState1 ^= 0xff;   //取反
    
    if(j == IO_KeyState1)   //连续两次读相等
    {
        j = IO_KeyState;
        IO_KeyState = IO_KeyState1;
        if(IO_KeyState != 0)    //有键按下
        {
            F0 = 0;
            if(j == 0)  F0 = 1; //第一次按下
            else if(j == IO_KeyState)
            {
                if(++IO_KeyHoldCnt >= 20)   //1秒后重键
                {
                    IO_KeyHoldCnt = 18;
                    F0 = 1;
                }
            }
            if(F0)
            {
                j = T_KeyTable[IO_KeyState >> 4];
                if((j != 0) && (T_KeyTable[IO_KeyState& 0x0f] != 0)) 
                    KeyCode = (j - 1) * 4 + T_KeyTable[IO_KeyState & 0x0f] + 16;    //计算键码，17~32
            }
        }
        else    IO_KeyHoldCnt = 0;
    }
    P0 = 0xff;
}



#define SLAW    0xA2
#define SLAR    0xA3

sbit    SDA = P1^1; //定义SDA  PIN5
sbit    SCL = P1^0; //定义SCL  PIN6

/****************************/
void    I2C_Delay(void) //for normal MCS51, delay (2 * dly + 4) T, for STC12Cxxxx delay (4 * dly + 10) T
{
    u8  dly;
    dly = MAIN_Fosc / 2000000UL;        //按2us计算
    while(--dly)    ;
}

/****************************/
void I2C_Start(void)               //start the I2C, SDA High-to-low when SCL is high
{
    SDA = 1;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SDA = 0;
    I2C_Delay();
    SCL = 0;
    I2C_Delay();
}       


void I2C_Stop(void)                 //STOP the I2C, SDA Low-to-high when SCL is high
{
    SDA = 0;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SDA = 1;
    I2C_Delay();
}

void S_ACK(void)              //Send ACK (LOW)
{
    SDA = 0;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SCL = 0;
    I2C_Delay();
}

void S_NoACK(void)           //Send No ACK (High)
{
    SDA = 1;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    SCL = 0;
    I2C_Delay();
}
        
void I2C_Check_ACK(void)         //Check ACK, If F0=0, then right, if F0=1, then error
{
    SDA = 1;
    I2C_Delay();
    SCL = 1;
    I2C_Delay();
    F0  = SDA;
    SCL = 0;
    I2C_Delay();
}

/****************************/
void I2C_WriteAbyte(u8 dat)     //write a byte to I2C
{
    u8 i;
    i = 8;
    do
    {
        if(dat & 0x80)  SDA = 1;
        else            SDA = 0;
        dat <<= 1;
        I2C_Delay();
        SCL = 1;
        I2C_Delay();
        SCL = 0;
        I2C_Delay();
    }
    while(--i);
}

/****************************/
u8 I2C_ReadAbyte(void)          //read A byte from I2C
{
    u8 i,dat;
    i = 8;
    SDA = 1;
    do
    {
        SCL = 1;
        I2C_Delay();
        dat <<= 1;
        if(SDA)     dat++;
        SCL  = 0;
        I2C_Delay();
    }
    while(--i);
    return(dat);
}

/****************************/
void WriteNbyte(u8 addr, u8 *p, u8 number)          /*  WordAddress,First Data Address,Byte lenth   */
                                                            //F0=0,right, F0=1,error
{
    I2C_Start();
    I2C_WriteAbyte(SLAW);
    I2C_Check_ACK();
    if(!F0)
    {
        I2C_WriteAbyte(addr);
        I2C_Check_ACK();
        if(!F0)
        {
            do
            {
                I2C_WriteAbyte(*p);     p++;
                I2C_Check_ACK();
                if(F0)  break;
            }
            while(--number);
        }
    }
    I2C_Stop();
}


/****************************/
void ReadNbyte(u8 addr, u8 *p, u8 number)       /*  WordAddress,First Data Address,Byte lenth   */
{
    I2C_Start(); //启动I2C总线
    I2C_WriteAbyte(SLAW);
    I2C_Check_ACK();
    if(!F0)
    {
        I2C_WriteAbyte(addr);
        I2C_Check_ACK();
        if(!F0)
        {
            I2C_Start();
            I2C_WriteAbyte(SLAR);
            I2C_Check_ACK();
            if(!F0)
            {
                do
                {
                    *p = I2C_ReadAbyte();   p++;
                    if(number != 1)     S_ACK();    //send ACK
                }
                while(--number);
                S_NoACK();          //send no ACK
            }
        }
    }
    I2C_Stop();
}
/****************************/

//初始化按键
void key_init(void) {
    u8 i;
    P0M1 = 0;   P0M0 = 0;   //设置为准双向口
    P1M1 = 0;   P1M0 = 0;   //设置为准双向口
    P2M1 = 0;   P2M0 = 0;   //设置为准双向口
    P3M1 = 0;   P3M0 = 0;   //设置为准双向口
    P4M1 = 0;   P4M0 = 0;   //设置为准双向口
    P5M1 = 0;   P5M0 = 0;   //设置为准双向口
    P6M1 = 0;   P6M0 = 0;   //设置为准双向口
    P7M1 = 0;   P7M0 = 0;   //设置为准双向口
    
    //display_index = 0;
    
    AUXR = 0x80;    //Timer0 set as 1T, 16 bits timer auto-reload, 
    TH0 = (u8)(Timer0_Reload / 256);
    TL0 = (u8)(Timer0_Reload % 256);
    ET0 = 1;    //Timer0 interrupt enable
    TR0 = 1;    //Tiner0 run
    EA = 1;     //打开总中断
    
    for(i=0; i<8; i++)  LED8[i] = 0x10; //上电消隐

    KeyHoldCnt = 0; //键按下计时
    KeyCode = 0;    //给用户使用的键码, 1~16有效

    IO_KeyState = 0;
    IO_KeyState1 = 0;
    IO_KeyHoldCnt = 0;
    cnt50ms = 0;
}

#endif  // STC_KEY_H_
