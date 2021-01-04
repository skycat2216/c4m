#define F_CPU 11059200UL
#include "c4mlib.h"
#include "RC16M128_Lib.h"
#include <string.h>


void arm(int arm1);
unsigned char armmove[16] = {};

void waist(int waist1);
unsigned char waistmove[16] = {};

void head(int head1);
unsigned char headmove[16] = {};

void leg(int leg1);
unsigned char legmove[16]= {};
	
unsigned char finalfnc[16]; //最後手段,直接全域變數

void interpolation_method(uint8_t *start , uint8_t *final);
//---------------------------------       左          左  左                 右  右          右
//---------------------------------   左  手  左  左  大  小  左          右` 小  大  右  右  手  右
//---------------------------------   手  肘  肩  臗  腿  腿  腳  腰  頭  腳  腿  腿  臗  肩  肘  手
//--------------------servo position: 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
//--------------------servo position: 1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16

unsigned char Initial[16]          ={};
//16軸初始位置陣列
unsigned char original[16]         ={45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
//
unsigned char offset_initial[16]   ={0,   0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0};
//
unsigned char down[16]             ={0,  0,  0,  0,  12, -30, 25,  0,  0,-25, 30,  -14,  0,  0,  0,  0};
//
unsigned char offset_Initial_low[16]={0,  0,  0,  0,  6,  -15,13,   0,  0, -13,15,   -7,  0,  0,  0,  0};


//動作對照表
//s1  左手(數字增加 抬起手)   s2 左手肘(數字減少 往前)　  s3  左肩(數字增加 往前)     s4  左臗(數字變小 打開)
//s5  左大腿(數字增加 往前)　　s6 左小腿(數字增加 往前)    s7  左腳(數字增加 往前)     s8  轉腰(數字增加 右轉)
//s9  頭(數字增加 往右看) 	s10 右腳(數字減少 往前)     s11 右小腿(數字減少 往前)   s12 右大腿(數字減少 往前)
//s13 右髖(數字變大 打開)	  	s14 右肩	(數字減少 往前)　　 s15 右手肘(數字增加 往前)   s16 右手(數字減少 抬起手)

unsigned char offset_twohand90_1[16]={0,0,-30,0,0,0,0,0,0,0,0,0,0,30,-30,0};
//兩隻手舉起來輪流舉起來
unsigned char offset_twohand90_2[16]={0,30,-30,0,0,0,0,0,0,0,0,0,0,30,0,0};
//兩隻手舉起來輪流舉起來
unsigned char offset_pose1[16]={30,0,0,0,0,0,0,0,-30,0,0,0,0,30,-30,0};
//一個帥氣pose
unsigned char offset_pose2[16]={0,30,-30,0,0,0,0,0,30,0,0,0,0,0,0,-30};
//一個帥氣pose 換邊
unsigned char offset_twohendup1[16]={0,-30,30,0,0,0,0,25,0,0,0,0,0,0,0,0};
//腰轉+手舉起來
unsigned char offset_twohendup2[16]={0,0,0,0,0,0,0,-25,0,0,0,0,0,-30,30,0};
//腰轉+手舉起來




unsigned char offset_leftup_right[16]={25,-25,0,0,0,0,0,0,0,0,0,0,0,0,25,0};
//左手舉起來，右手在胸前出去
unsigned char offset_leftup_right2[16]={25,-25,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//左手持續舉著，右手在胸前回來
unsigned char offset_rightup_left[16]={0,-25,0,0,0,0,0,0,0,0,0,0,0,0,25,-25};
//右手舉起來，左手在胸前出去
unsigned char offset_rightup_left2[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,25,-25};
//有手舉起來，左手在胸前回來
unsigned char offset_handshake_shoulder_up[16]={0,0,-30,0,0,0,0,0,0,0,0,0,0,30,0,0};
//雙手肩膀平行
unsigned char offset_handshake1[16]={0,30,-30, 0,0,0,0,0,0,0,0,0,0,30,30,0};
//雙手開始搖擺
unsigned char offset_handshake2[16]={0,-30,-30,0,0,0,0,0,0,0,0,0,0,30,-30,0};
//雙手開始反向搖擺
unsigned char offset_bigwaist_right[16]={0,-25,0,0,0,0,0,30,0,0,0,0,0,0,-25,0};
//
unsigned char offset_bigwaist_left[16]={0,25,0,0,0,0,0,-30,0,0,0,0,0,0,25,0};
//



//站立垂手
unsigned char offset_stand[16]     ={0,-25,-25,0,0,0,0,0,0,0,0,0,0,25,25,0};
//雙手垂直胸前
unsigned char offset_two_hand_v[16]={25,-27,0,0,0,0,0,0,0,0,0,0,0,0,30,-25};
//蹲下陣列變動值
unsigned char offset_Kneel[16]     ={0,0,0,0,12,-20,12,0,0,-12,20,-12,0,0,0,0};
//頭部向右變動值
unsigned char offset_head_right[16]={0,0,0,0,2,0,0,0,24,0,0,-2,0,0,0,0};
//頭部向左變動值
unsigned char offset_head_left[16]={0,0,0,0,2,0,0,0,-24,0,0,-2,0,0,0,0};
//腰向右轉變動值
unsigned char offset_waist_right[16]={25,-30,0,0,2,0,0,24,0,0,0,-2,0,0,30,-25};
//腰向左轉變動值
unsigned char offset_waist_left[16]={25,-30,0,0,2,0,0,-24,0,0,0,-2,0,0,30,-25};
//再見_左手1變動值
unsigned char offset_bye_1_left[16]={0,-27,24,0,0,2,-1,0,0,0,0,0,-2,0,0,0};
//再見_左手2變動值
unsigned char offset_bye_2_left[16]={0,-3,24,0,0,2,-1,0,0,0,0,0,-2,0,0,0};


unsigned char stand[16]={};			//站立
unsigned char two_hand_v[16]={};
unsigned char Kneel[16]={};			//蹲下
unsigned char head_right[16]={};	//頭向右轉
unsigned char head_left[16]={};		//頭向左轉
unsigned char waist_right[16]={};	//腰向右轉
unsigned char waist_left[16]={};	//腰向左轉

unsigned char leftup_right[16]={}; //
unsigned char leftup_right2[16]={};//
unsigned char rightup_left[16]={};//
unsigned char rightup_left2[16]={};//
unsigned char handshake_shoulder_up[16]={};//
unsigned char handshake1[16]={};//
unsigned char handshake2[16]={};//
unsigned char bigwaist_right[16]={};//
unsigned char bigwaist_left[16]={};//

unsigned char twohand90_1[16]={};//
unsigned char twohand90_2[16]={};//
unsigned char pose1[16]={};//
unsigned char pose2[16]={};//
unsigned char twohandup1[16]={};//
unsigned char twohandup2[16]={};//

unsigned char Initial_low[16]={};//
unsigned char bye_1_left[16]={};//
unsigned char bye_2_left[16]={};//

int k;

int main(void)
{
	ASA_RC16M128_set();//機器人控制板初始化
	C4M_DEVICE_set();
	RC16M128_Servo_Enable_All();//所有伺服機致能
	//RC16M128_Servo_put( 0, 16, stand); //將初始位置輸出至各伺服機
	for(int j=0; j<=15; j++){
		Initial[j]=offset_initial[j]+original[j];
	}
	
/*	for (int i=0; i<=15;i++){
		
		Initial_low[i]=Initial[i]+offset_Initial_low[i];
		
		stand[i]=Initial[i]+offset_stand[i]+down[i];
		two_hand_v[i]=Initial[i]+offset_two_hand_v[i]+down[i];
		Kneel[i]=Initial[i]+offset_Kneel[i]+down[i];
		head_right[i]=Initial[i]+offset_head_right[i]+down[i];
		head_left[i]=Initial[i]+offset_head_left[i]+down[i];
		waist_right[i]=Initial[i]+offset_waist_right[i]+down[i];
		waist_left[i]=Initial[i]+offset_waist_left[i]+down[i];
		leftup_right[i]=Initial[i]+offset_leftup_right[i]+down[i];
		leftup_right2[i]=Initial[i]+offset_leftup_right2[i]+down[i];
		rightup_left[i]=Initial[i]+offset_rightup_left[i]+down[i];
		rightup_left2[i]=Initial[i]+offset_rightup_left2[i]+down[i];
		handshake_shoulder_up[i]=Initial[i]+offset_handshake_shoulder_up[i]+down[i];
		handshake1[i]=Initial[i]+offset_handshake1[i]+down[i];
		handshake2[i]=Initial[i]+offset_handshake2[i]+down[i];
		bigwaist_right[i]=Initial[i]+offset_bigwaist_right[i]+down[i];
		bigwaist_left[i]=Initial[i]+offset_bigwaist_left[i]+down[i];
		twohand90_1[i]=Initial[i]+offset_twohand90_1[i]+down[i];
		twohand90_2[i]=Initial[i]+offset_twohand90_2[i]+down[i];
		pose1[i]=Initial[i]+offset_pose1[i]+down[i];
		pose2[i]=Initial[i]+offset_pose2[i]+down[i];
		twohandup1[i]=Initial[i]+offset_twohendup1[i]+down[i];
		twohandup2[i]=Initial[i]+offset_twohendup2[i]+down[i];
		bye_1_left[i]=Initial[i]+offset_bye_1_left[i]+down[i];
		bye_2_left[i]=Initial[i]+offset_bye_2_left[i]+down[i];
		
		
		
		
	}*/
	
	
	
	arm(0);
	waist(0);
	leg(0);
	head(0);
	
	unsigned char final_build();
	unsigned char final();
	unsigned char k[16] = {};
	memcpy (j,finalfnc,16);
	
	arm(0);
	waist(0);
	leg(0);
	head(0);
	
	final_build();
	unsigned char j[16]={};
	memcpy (j,finalfnc,16);
	
	
	
	
	interpolation_method(k,j);
	
	
	//RC16M128_Servo_put( 0, 16, stand); 
	//將初始位置輸出至各伺服機
	


}

void interpolation_method(uint8_t *start , uint8_t *final)
{
	uint8_t Output [16];
	for (int i = 0; i <= 50; i++) {
		for (int j = 0; j < 16; j++) {
			Output[j] = start[j]+(final[j]-start[j])*i/50;
			//每一軸分成插值100份
		}
		RC16M128_Servo_put(0, 16, Output); //輸出計算後結果
		if(k==15){
			_delay_ms(10 );//輸出動作之間等待0.015秒
		}
		else if(k==30){
			_delay_ms(23);//輸出動作之間等待0.030秒
		}
		else if(k==5){
			_delay_ms(5);//輸出動作之間等待0.005秒
		}
		
	}
}



void arm(int arm1)
{
	unsigned char *p = armmove;
	switch (arm1)
	{
		case 1:
		{
			*p     = 45;  
			*(p+1) = 45;
			*(p+2) = -30;
			*(p+3) = 45;
			*(p+12)= 45;
			*(p+13)= 30;
			*(p+14)= -30;
			*(p+15)= 45;
			break;
		}
		case 2:
		{
			*p     = 45;
			*(p+1) = 30;
			*(p+2) = -30;
			*(p+3) = 45;
			*(p+12)= 45;
			*(p+13)= 30;
			*(p+14)= 45;
			*(p+15)= 45;
			break;
		}
		case 3:
		{
			*p     = 45;
			*(p+1) = 45;
			*(p+2) = 45;
			*(p+3) = 45;
			*(p+12)= 45;
			*(p+13)= 45;
			*(p+14)= 45;
			*(p+15)= 45;
			break;
		}
		default :
		{
			*p     = 45;
			*(p+1) = 45;
			*(p+2) = 45;
			*(p+3) = 45;
			*(p+12)= 45;
			*(p+13)= 45;
			*(p+14)= 45;
			*(p+15)= 45;
			break;
		}
	}
}

void waist(int waist1)
{
	unsigned char *p = waistmove;
	switch (waist1)
	{
		case 1:
		{
			*(p+7)= 45;
			break;
		}
		case 2:
		{
			*(p+7)= 45;
			break;
		}
		case 3:
		{
			*(p+7)= 45;
			break;
		}
		default :
		{
			*(p+7)= 45;
			break;
		}
	}
}

void head(int head1)
{
	unsigned char *p = headmove;
	switch (head1)
	{
		case 1:
		{
			*(p+8)=45;
			break;
		}
		case 2:
		{
			*(p+8)=45;
			break;
		}
		case 3:
		{
			*(p+8)=45;
			break;
		}
		default :
		{
			*(p+8)=45;
			break;
		}
	}
}

void leg(int leg1)
{
	unsigned char *p = legmove;
	switch (leg1)
	{
		case 1:
		{
			*(p+4) = 45;
			*(p+5) = 45;
			*(p+6) = 45;
			*(p+9) = 45;
			*(p+10)= 45;
			*(p+11)= 45;
			break;
		}
		case 2:
		{
			*(p+4) = 45;
			*(p+5) = 45;
			*(p+6) = 45;
			*(p+9) = 45;
			*(p+10)= 45;
			*(p+11)= 45;
			break;
		}
		case 3:
		{
			*(p+4) = 45;
			*(p+5) = 45;
			*(p+6) = 45;
			*(p+9) = 45;
			*(p+10)= 45;
			*(p+11)= 45;
			break;
		}
		default :
		{
			*(p+4) = 45;
			*(p+5) = 45;
			*(p+6) = 45;
			*(p+9) = 45;
			*(p+10)= 45;
			*(p+11)= 45;
			break;
		}
	}
}

unsigned char *final_build()
{
	unsigned char *armp = armmove;
	unsigned char *waistp = waistmove;
	unsigned char *headp = headmove;
	unsigned char *legp = legmove;
	
	
	
	for(int i=0;i<=15;i++)
	{
		switch(i)
		{
			case 0: case 1: case 2: case 3: case 12: case 13: case 14: case 15:
			{
				finalfnc[i] = *(armp+i);
				break;
			}
			case 4: case 5: case 6: case 9: case 10: case 11:
			{
				finalfnc[i] = *(legp+i);
				break;
			}
			case  7:
			{
				finalfnc[i] = *(waistp+i);
				break;
			}
			case  8:
			{
				finalfnc[i] = *(headp+i);
				break;
			}
		}
	}	
	return finalfnc;
}


