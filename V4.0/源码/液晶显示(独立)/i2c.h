#ifndef __I2C_H_
#define __I2C_H_

#include<reg52.H>

sbit SCL=P1^2;
sbit SDA=P1^3;

void I2cStart();
void I2cStop();
unsigned char I2cSendByte(unsigned char dat);
unsigned char I2cReadByte();
void At24c02Write(unsigned char addr,unsigned char dat);
unsigned char At24c02Read(unsigned char addr);

#endif
