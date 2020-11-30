#ifndef __RC16M128_LIB_H__
#define __RC16M128_LIB_H__

#include "c4mlib.h"
//#include <avr\io.h>
//#include <avr\interrupt.h>
//#include <util\delay.h>

//函式原型
char ASA_RC16M128_set(void);
char RC16M128_Servo_put(char LSByte, char Bytes, void* Data_p);
char RC16M128_Servo_get(char LSByte, char Bytes, void* Data_p);
char RC16M128_Servo_set(char LSByte, char Mask, char shift, char Data);
#define RC16M128_Servo_Enable_All()  RC16M128_Servo_set(200,255,0,255);RC16M128_Servo_set(201,255,0,255)
#define RC16M128_Servo_Disable_All() RC16M128_Servo_set(200,255,0,0);RC16M128_Servo_set(201,255,0,0)

#define	CheckBit(data,bit) ((data&(1<<bit))==(1<<bit))
#define	ClearBit(data,bit) (data&=~(1<<bit))
#define	SetBit(data,bit)   (data|=(1<<bit))

// Servo Define:
#define ServoDDRL  DDRA	// Low Byte S1~S8
#define ServoPORTL PORTA
#define ServoDDRH  DDRC	// High Byte S9~S16
#define ServoPORTH PORTC
#define ServoNum 16
#define ServoBasePeriod 899


#endif