#include "c4mlib.h"
#include "RC16M128_Lib.h"

// Global value define:
volatile unsigned char ServoPWMOutputDataL = 0, ServoPWMOutputDataH = 0;
volatile unsigned char ServoEnableSetRegisterL = 0, ServoEnableSetRegisterH = 0;
volatile unsigned char ServoCommand[ServoNum] = {45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
volatile unsigned int  ServoPeriodCount = 0;

/*----------------------副函式----------------------*/
void BasicTimer_Init(void)
{
	/*計時器中斷設定*/	
	TIM_fpt(&TCCR3B,0x07,0,1);//選擇timer3 T/Cn除頻1
	TIM_fpt(&TCCR3A,0x03,0,0);//set timer3 CTC可調方波step1
	TIM_fpt(&TCCR3B,0x18,3,1);//set timer3 CTC可調方波step2
	TIM_fpt(&ETIMSK,0x10,4,1);//timer3 interrupt enable
	unsigned int ocr3=245;
	TIM_put(&OCR3AL,2,&ocr3);//use OCR3;
}


void RC16M128_Servo_Init(void)
{
	BasicTimer_Init();
	ServoDDRL  = 0x00;
	ServoPORTL = 0x00;
	ServoDDRH  = 0x00;
	ServoPORTH = 0x00;
}

char RC16M128_Servo_put(char LSByte, char Bytes, void* Data_p)
{
	unsigned char i;

	// 參數檢查
	if( LSByte > 15 || LSByte < 0)
	return 1;
	if( (LSByte+Bytes) > 16 || (LSByte+Bytes) < 1 )
	return 2;

	// 輸出資料
	for(i=0; i<Bytes; i++)
	ServoCommand[LSByte+i] = ((unsigned char*)Data_p)[i];

	return 0;
}

char RC16M128_Servo_get(char LSByte, char Bytes, void* Data_p)
{
	unsigned char i, RegMode=0;	// RegMode=0 is Put Reg., RegMode=1 is Get Reg., RegMode=2 is Set Reg.;

	// 參數檢查
	if( LSByte > 15 || LSByte < 0)
	return 1;
	if( (LSByte+Bytes) > 16 || (LSByte+Bytes) < 1 )
	RegMode = 0;
	//else if( LSByte >=216 && LSByte <= 201 )
	//	RegMode = 1;	// Get register mode (RegMode = 1) is reserved for the function of servo that can be read position in the future.
	else if( LSByte == 200 || LSByte == 201)
	RegMode = 2;
	else
	return 2;

	// 輸出資料
	for(i=0; i<Bytes; i++)
	((unsigned char*)Data_p)[i] = ServoCommand[LSByte+i];



	// Put跟Set回應
	if( RegMode == 0 )
	for(i=0; i<Bytes; i++)
	((char*)Data_p)[i] = ServoCommand[LSByte+i];
	else
	{
		if( (LSByte+Bytes) == 201 )
		((char*)Data_p)[0] = ServoDDRL;
		if( (LSByte+Bytes) == 202 )
		{
			((char*)Data_p)[1] = ServoDDRH;
			if( Bytes == 2 )
			((char*)Data_p)[0] = ServoDDRL;
		}
	}

	return 0;
}

char RC16M128_Servo_set(char LSByte, char Mask, char shift, char Data)
{
	char set_Data=0;

	// 參數檢查
	if( shift > 7 || shift < 0 )
	return 2;

	// 設定資料
	if( LSByte == 200 )
	{
		set_Data = ServoDDRL;
		set_Data = (set_Data & (~Mask)) | ((Data<<shift) & Mask);
		ServoDDRL = set_Data;
	}
	else if( LSByte == 201 )
	{
		set_Data = ServoDDRH;
		set_Data = (set_Data & (~Mask)) | ((Data<<shift) & Mask);
		ServoDDRH = set_Data;
	}
	else
	return 1;

	return 0;
}

char ASA_RC16M128_set(void)
{
	char Result = 0;
	RC16M128_Servo_Init();
	sei();
	
	return Result;
}

// 16通道伺服機PWM訊號產生器
ISR( TIMER3_COMPA_vect )
{
	// RC Servo PWM Command Timing Diagram
	//       _____                                 _____                                 _____
	// _____|     |_______________________________|     |_______________________________|     |______
	//      |<-1->|
	//      |<-----------------2----------------->|
	// 1: ServoCommand		0~120
	//    Period Range  0.5ms~2.5ms
	// 2: ServoPeriodCount	0~1199
	// ServoBasePeriod: 1200 = PWM one Wave(2) Frequency: 50Hz(20ms)
	int CountTemp;
	
	// 比對到脈寬結束時機，該通道拉到Low
	CountTemp = ServoPeriodCount-22;	// Shift 0.5ms

	if( CountTemp <= 90)
	{
		if( CountTemp == ServoCommand[0] )
		ServoPORTL -= 1;
		if( CountTemp == ServoCommand[1] )
		ServoPORTL -= 2;
		if( CountTemp == ServoCommand[2] )
		ServoPORTL -= 4;
		if( CountTemp == ServoCommand[3] )
		ServoPORTL -= 8;
		if( CountTemp == ServoCommand[4] )
		ServoPORTL -= 16;
		if( CountTemp == ServoCommand[5] )
		ServoPORTL -= 32;
		if( CountTemp == ServoCommand[6] )
		ServoPORTL -= 64;
		if( CountTemp == ServoCommand[7] )
		ServoPORTL -= 128;
		if( CountTemp == ServoCommand[8] )
		ServoPORTH -= 1;
		if( CountTemp == ServoCommand[9] )
		ServoPORTH -= 2;
		if( CountTemp == ServoCommand[10] )
		ServoPORTH -= 4;
		if( CountTemp == ServoCommand[11] )
		ServoPORTH -= 8;
		if( CountTemp == ServoCommand[12] )
		ServoPORTH -= 16;
		if( CountTemp == ServoCommand[13] )
		ServoPORTH -= 32;
		if( CountTemp == ServoCommand[14] )
		ServoPORTH -= 64;
		if( CountTemp == ServoCommand[15] )
		ServoPORTH -= 128;
	}
	ServoPeriodCount++;
	// 新一輪週期開始，全部拉到High
	if( ServoPeriodCount > ServoBasePeriod )
	{
		ServoPeriodCount = 0;
		ServoPORTL = 255;
		ServoPORTH = 255;
	}
}
