#include	<string.h>
#include	"STC15Fxxxx.H"

typedef unsigned char uchar;

u8 ptr;					//receive pointer
u8 receive[50];
u16 temperature;

void settemp(u16 temp)
{
	temperature = temp;
}

void delay_ms(u8 ms)
{
	u16 time;
	do{
		time = MAIN_Fosc / 13000;
		while(--time);
	}while(--ms);
}

void connect_init()
{
	char *e = "AT+CIPMUX=1\r\n";
	char *f = "AT+CIPSERVER=1,8080\r\n";
	
	while(*e != '\0') {
		SBUF = *e;
		while(!TI);
		TI = 0;
		e++;
	}
	delay_ms(10);
	while(*f != '\0') {
		SBUF = *f;
		while(!TI);
		TI = 0;
		f++;
	}
}

void UartConfiguration()
{
	TMOD = 0x20;	//timer1 mode2
	TH1 = 0xfd;		//timer1 init
	TL1 = 0xfd;		//timer1 init
	PCON = 0x00;	//baud no multi
	SCON = 0x50;	//mode1 receive open
	ES = 1;				//ttl interrupt ok
	TR1 = 1;			//timer1 run
}

void esp8266_init()
{
	UartConfiguration();
	TI = 0;
	connect_init();
}

void sendInt(u16 i)
{
	ES = 0;
	SBUF = i;
	while(!TI);
	TI = 0;
	ES = 1;
}

void Uart() interrupt 4
{
	sizeof(j);
	char *send = "AT+CIPSEND=0,4\r\n";
	char res;
	res = SBUF;
	RI = 0;
	if (res==':'||ptr>0) {
		if (res == '\n') {
			if (*receive == ":gettemp") {
				while(*send != '\0') {
					SBUF = *send;
					while(!TI);
					TI = 0;
					send++;
				}
				delay_ms(10);
				sendInt(temperature);
			}
			ptr = 0;
			memset(receive,0,50);
		} else {
			receive[ptr] = res;
			ptr++;
		}
	}
}
