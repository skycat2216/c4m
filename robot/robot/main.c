#define F_CPU 11059200UL
#include "c4mlib.h"
#include "RC16M128_Lib.h"
#define raid_lenght 5

void interpolation_method(uint8_t *start , uint8_t *final);

void continue_acting(unsigned char method[raid_lenght][16]); 

//---------------------------------              左          左  左                   右  右          右
//---------------------------------              左  手  左  左  大   小   左          右  小  大  右  右  手  右
//---------------------------------              手  肘  肩  臗  腿   腿   腳  腰  頭  腳  腿  腿  臗  肩  肘  手
//--------------------servo position:            1   2   3   4   5   6   7   8   9  10  11  12  13  14  15  16
unsigned char Initial[16]					  ={45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
//16軸初始位置陣列
unsigned char left_hand1[16]				  ={45, 20, 69, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
//動:左手1
unsigned char left_hand2[16]				  ={45, 35, 69, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45};
//動:左手2
unsigned char movement[raid_lenght][16]=
{  	  {45, 69, 69, 45, 45, 45, 45, 53, 45, 45, 45, 45, 45, 69, 32, 45},//揮手(左)
	  {45, 69, 69, 45, 45, 45, 45, 53, 45, 45, 45, 45, 45, 69, 21, 45},
	  {45, 20, 69, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45},
	  {45, 35, 69, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45},
   	  {45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45}
};
int main(void)
{
	ASA_RC16M128_set();//機器人控制板初始化
	C4M_DEVICE_set();
	RC16M128_Servo_Enable_All();//所有伺服機致能
	RC16M128_Servo_put( 0, 16, Initial); //將初始位置輸出至各伺服機
	interpolation_method(Initial,  Initial);
	while (1)
	{
		_delay_ms(500); //等待1秒
		// 		RC16M128_Servo_put( 0, 16, Rotate_head_right);
		// 		RC16M128_Servo_put( 0, 16, Initial);
		
		interpolation_method(left_hand1, left_hand2);
		interpolation_method(left_hand2, left_hand1);
		_delay_ms(500); //等待1秒
		continue_acting(movement);
		
	}
	
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
		_delay_ms(10); //輸出動作之間等待0.01秒
	}
}


void continue_acting(unsigned char method[raid_lenght][16])
	{
		//本函數為方便編輯,請輸入二維陣列
		uint8_t	temp_start[16] , temp_final[16];
		
		for(int i=1;i<=raid_lenght;i++)
			{
				for (int j=0;j<=15;j++)
				{
					temp_start[j] = movement[i-1][j];
					temp_final[j] = movement[i][j];
				}
				interpolation_method(temp_start,temp_final);			
			}
				
	}
	
