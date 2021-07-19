
#include "global.h"
#include "input.h"
#include "output.h"
#include "bsp_core_board_control_io.h"




///********读取外设32位数字量状态*********/

//static void Read_All_Input_Board(unsigned char *dindata_t)
//{
//	*(dindata_t + 0) = 0x00;//read_data(InBoard_1);
//	*(dindata_t + 1) = 0x01;//read_data(InBoard_2);
//	*(dindata_t + 2) = 0x02;//read_data(InBoard_3);
//	*(dindata_t + 3) = 0x03;//read_data(InBoard_4);
//}



//字节按位处理化//
static void Get_DI_Bit(unsigned char *dindata_t, BOOL *sbit_t)
{
	unsigned char i=0,j=0;
	
	for(i=0; i<(sizeof(dindata_t)/sizeof(dindata_t[0])); i++)
	{
		for(j=0; j<8; j++)
		{
			*(sbit_t + i*8 + j) = (BOOL)((*(dindata_t + i) >> j) & 0x01);  
		}
	}
}



static void Byte_To_Bit(unsigned char dindata_t, BOOL *sbit_t, unsigned char len)
{
	unsigned char i=0;
	
	for(i=0; i<len; i++)
	{
		*(sbit_t + i) = (BOOL)((dindata_t >> i) & 0x01);  
	}
}




static void Bit_Filter(BOOL *fdata_t, BOOL *sdata_t, unsigned char timer_t[][2], unsigned char *filter_time, unsigned char len)
{
	unsigned char i=0;
	
	for(i=0; i<len; i++)
	{
		if(*(sdata_t + i))
		{
			timer_t[i][1] = 0;
			timer_t[i][0]++;
			if(timer_t[i][0] >= *(filter_time + i))
			{
				timer_t[i][0] = *(filter_time + i);
				*(fdata_t + i) = *(sdata_t + i);
			}
		}
		else
		{
			timer_t[i][0] = 0;
			timer_t[i][1]++;
			if(timer_t[i][1] >= *(filter_time + i))
			{
				timer_t[i][1] = *(filter_time + i);
				*(fdata_t + i) = *(sdata_t + i);
			}	
		}
	}
}



//32位输入滤波//
static void Input_Filter(BOOL *sbit_t, unsigned char timer_t[32][2], unsigned char *filter_time,unsigned char len)
{
	static BOOL fbit_t[32] = {FALSE};
	
	Bit_Filter(fbit_t, sbit_t, timer_t, filter_time,len);
	
	DI01 = *(fbit_t + 0);
	DI02 = *(fbit_t + 1);
	DI03 = *(fbit_t + 2);
	DI04 = *(fbit_t + 3);
	DI05 = *(fbit_t + 4);
	DI06 = *(fbit_t + 5);
	DI07 = *(fbit_t + 6);
	DI08 = *(fbit_t + 7);

	DI09 = *(fbit_t + 8);
	DI10 = *(fbit_t + 9);
	DI11 = *(fbit_t + 10);
	DI12 = *(fbit_t + 11);
	DI13 = *(fbit_t + 12);
	DI14 = *(fbit_t + 13);
	DI15 = *(fbit_t + 14);
	DI16 = *(fbit_t + 15);


	DI17 = *(fbit_t + 16);
	DI18 = *(fbit_t + 17);
	DI19 = *(fbit_t + 18);
	DI20 = *(fbit_t + 19);
	DI21 = *(fbit_t + 20);
	DI22 = *(fbit_t + 21);
	DI23 = *(fbit_t + 22);
	DI24 = *(fbit_t + 23);
	
	DI25 = *(fbit_t + 24);
	DI26 = *(fbit_t + 25);
	DI27 = *(fbit_t + 26);
	DI28 = *(fbit_t + 27);
	DI29 = *(fbit_t + 28);
	DI30 = *(fbit_t + 29);
	DI31 = *(fbit_t + 30);
	DI32 = *(fbit_t + 31);
}



/***获取闸位***/
//void Get_Brake_Position(BRAKE_PosTypeDef *buc_block_t)
void Get_Brake_Position(BRAKE_PortTypeDef *buc_block_t, BRAKE_PosTypeDef *buc_pos_t)
{
	BOOL in_bit_s[32] = {FALSE};
	unsigned char io_filter_timer[32] = {2,2,2,2,2,2,2,2,
									     2,2,2,2,2,2,2,2,
									     16,20,20,2,20,2,2,10,
									     20,2,2,2,2,2,20,2};
	static unsigned char io_timer[32][2] = {0};
	
	Read_All_Input_Board(dindata);		
	Get_DI_Bit(dindata,in_bit_s);		
	Input_Filter(in_bit_s, io_timer, io_filter_timer,32);
	
	/*******************大闸过充位*******************/
	if(DI04 && !DI05)
	{
		buc_block_t->port1.d_oc  = TRUE;
		buc_block_t->port1.d_run = FALSE;
		buc_block_t->port1.d_brk = FALSE;
		buc_block_t->port1.d_res = FALSE;
		buc_block_t->port1.d_mul = FALSE;
		buc_block_t->port1.d_emc = FALSE;
		buc_block_t->port1.d_other = FALSE;
	}
	
	else if(DI10 && !DI09)
	{
		buc_block_t->port2.d_oc  = TRUE;
		buc_block_t->port2.d_run = FALSE;
		buc_block_t->port2.d_brk = FALSE;
		buc_block_t->port2.d_res = FALSE;
		buc_block_t->port2.d_mul = FALSE;
		buc_block_t->port2.d_emc = FALSE;
		buc_block_t->port2.d_other = FALSE;		
	}
	
	else if(DI20 && !DI30)
	{
		buc_block_t->net.d_oc  = TRUE;
		buc_block_t->net.d_run = FALSE;
		buc_block_t->net.d_brk = FALSE;
		buc_block_t->net.d_res = FALSE;
		buc_block_t->net.d_mul = FALSE;
		buc_block_t->net.d_emc = FALSE;
		buc_block_t->net.d_other = FALSE;		
	}

	/*******************大闸运转位*******************/
	else if(DI05 && DI07 && DI08)
	{
		buc_block_t->port1.d_oc  = FALSE;
		buc_block_t->port1.d_run = TRUE;
		buc_block_t->port1.d_brk = FALSE;
		buc_block_t->port1.d_res = FALSE;
		buc_block_t->port1.d_mul = FALSE;
		buc_block_t->port1.d_emc = FALSE;
		buc_block_t->port1.d_other = FALSE;
	}	

	else if(DI05 && DI07 && DI08)
	{
		buc_block_t->port2.d_oc  = FALSE;
		buc_block_t->port2.d_run = TRUE;
		buc_block_t->port2.d_brk = FALSE;
		buc_block_t->port2.d_res = FALSE;
		buc_block_t->port2.d_mul = FALSE;
		buc_block_t->port2.d_emc = FALSE;
		buc_block_t->port2.d_other = FALSE;
	}

	else if(DI05 && DI07 && DI08)
	{
		buc_block_t->net.d_oc  = FALSE;
		buc_block_t->net.d_run = TRUE;
		buc_block_t->net.d_brk = FALSE;
		buc_block_t->net.d_res = FALSE;
		buc_block_t->net.d_mul = FALSE;
		buc_block_t->net.d_emc = FALSE;
		buc_block_t->net.d_other = FALSE;
	}	
	
	/*******************大闸制动位*******************/
	else if(DI09 && DI10)
	{
		buc_block_t->port1.d_oc  = FALSE;
		buc_block_t->port1.d_run = FALSE;
		buc_block_t->port1.d_brk = TRUE;
		buc_block_t->port1.d_res = FALSE;
		buc_block_t->port1.d_mul = FALSE;
		buc_block_t->port1.d_emc = FALSE;
		buc_block_t->port1.d_other = FALSE;
	}

	else if(DI09 && DI10)
	{
		buc_block_t->port2.d_oc  = FALSE;
		buc_block_t->port2.d_run = FALSE;
		buc_block_t->port2.d_brk = TRUE;
		buc_block_t->port2.d_res = FALSE;
		buc_block_t->port2.d_mul = FALSE;
		buc_block_t->port2.d_emc = FALSE;
		buc_block_t->port2.d_other = FALSE;
	}
	
	else if(DI09 && DI10)
	{
		buc_block_t->net.d_oc  = FALSE;
		buc_block_t->net.d_run = FALSE;
		buc_block_t->net.d_brk = TRUE;
		buc_block_t->net.d_res = FALSE;
		buc_block_t->net.d_mul = FALSE;
		buc_block_t->net.d_emc = FALSE;
		buc_block_t->net.d_other = FALSE;
	}
//	
	/*******************大闸抑制位*******************/
	else if(DI07 && DI09)
	{
		buc_block_t->port1.d_oc  = FALSE;
		buc_block_t->port1.d_run = FALSE;
		buc_block_t->port1.d_brk = FALSE;
		buc_block_t->port1.d_res = TRUE;
		buc_block_t->port1.d_mul = FALSE;
		buc_block_t->port1.d_emc = FALSE;
		buc_block_t->port1.d_other = FALSE;
	}	
	
	else if(DI07 && DI09)
	{
		buc_block_t->port2.d_oc  = FALSE;
		buc_block_t->port2.d_run = FALSE;
		buc_block_t->port2.d_brk = FALSE;
		buc_block_t->port2.d_res = TRUE;
		buc_block_t->port2.d_mul = FALSE;
		buc_block_t->port2.d_emc = FALSE;
		buc_block_t->port2.d_other = FALSE;
	}	
	
	else if(DI07 && DI09)
	{
		buc_block_t->net.d_oc  = FALSE;
		buc_block_t->net.d_run = FALSE;
		buc_block_t->net.d_brk = FALSE;
		buc_block_t->net.d_res = TRUE;
		buc_block_t->net.d_mul = FALSE;
		buc_block_t->net.d_emc = FALSE;
		buc_block_t->net.d_other = FALSE;
	}	

	/*******************大闸重联位*******************/
	else if(DI07 && !DI09 && !DI05)
	{
		buc_block_t->port1.d_oc  = FALSE;
		buc_block_t->port1.d_run = FALSE;
		buc_block_t->port1.d_brk = FALSE;
		buc_block_t->port1.d_res = FALSE;
		buc_block_t->port1.d_mul = TRUE;
		buc_block_t->port1.d_emc = FALSE;
		buc_block_t->port1.d_other = FALSE;
	}	
	
	else if(DI07 && !DI09 && !DI05)
	{
		buc_block_t->port2.d_oc  = FALSE;
		buc_block_t->port2.d_run = FALSE;
		buc_block_t->port2.d_brk = FALSE;
		buc_block_t->port2.d_res = FALSE;
		buc_block_t->port2.d_mul = TRUE;
		buc_block_t->port2.d_emc = FALSE;
		buc_block_t->port2.d_other = FALSE;
	}	

	else if(DI07 && !DI09 && !DI05)
	{
		buc_block_t->net.d_oc  = FALSE;
		buc_block_t->net.d_run = FALSE;
		buc_block_t->net.d_brk = FALSE;
		buc_block_t->net.d_res = FALSE;
		buc_block_t->net.d_mul = TRUE;
		buc_block_t->net.d_emc = FALSE;
		buc_block_t->net.d_other = FALSE;
	}	
	
	/*******************大闸紧急位*******************/
	else if(DI05 && !DI07)
	{
		buc_block_t->port1.d_oc  = FALSE;
		buc_block_t->port1.d_run = FALSE;
		buc_block_t->port1.d_brk = FALSE;
		buc_block_t->port1.d_res = FALSE;
		buc_block_t->port1.d_mul = FALSE;
		buc_block_t->port1.d_emc = TRUE;
		buc_block_t->port1.d_other = FALSE;
	}	
	
	else if(DI05 && !DI07)
	{
		buc_block_t->port2.d_oc  = FALSE;
		buc_block_t->port2.d_run = FALSE;
		buc_block_t->port2.d_brk = FALSE;
		buc_block_t->port2.d_res = FALSE;
		buc_block_t->port2.d_mul = FALSE;
		buc_block_t->port2.d_emc = TRUE;
		buc_block_t->port2.d_other = FALSE;
	}	
	
	else if(DI05 && !DI07)
	{
		buc_block_t->net.d_oc  = FALSE;
		buc_block_t->net.d_run = FALSE;
		buc_block_t->net.d_brk = FALSE;
		buc_block_t->net.d_res = FALSE;
		buc_block_t->net.d_mul = FALSE;
		buc_block_t->net.d_emc = TRUE;
		buc_block_t->net.d_other = FALSE;
	}	
//	
	
	else
	{
		buc_block_t->port1.d_oc  = FALSE;
		buc_block_t->port1.d_run = FALSE;
		buc_block_t->port1.d_brk = FALSE;
		buc_block_t->port1.d_res = FALSE;
		buc_block_t->port1.d_mul = FALSE;
		buc_block_t->port1.d_emc = FALSE;
		buc_block_t->port1.d_other = TRUE;

		buc_block_t->port2.d_oc  = FALSE;
		buc_block_t->port2.d_run = FALSE;
		buc_block_t->port2.d_brk = FALSE;
		buc_block_t->port2.d_res = FALSE;
		buc_block_t->port2.d_mul = FALSE;
		buc_block_t->port2.d_emc = FALSE;
		buc_block_t->port2.d_other = TRUE;
		
		buc_block_t->net.d_oc  = FALSE;
		buc_block_t->net.d_run = FALSE;
		buc_block_t->net.d_brk = FALSE;
		buc_block_t->net.d_res = FALSE;
		buc_block_t->net.d_mul = FALSE;
		buc_block_t->net.d_emc = FALSE;
		buc_block_t->net.d_other = TRUE;
	}

	if(buc_block_t->port1.d_oc || buc_block_t->port2.d_oc || buc_block_t->net.d_oc)
	{
		buc_pos_t->d_oc = TRUE;	
		buc_pos_t->d_run = FALSE;
		buc_pos_t->d_brk = FALSE;
		buc_pos_t->d_res = FALSE;
		buc_pos_t->d_mul = FALSE;
		buc_pos_t->d_emc = FALSE;
		buc_pos_t->d_other = FALSE;
	}
	else if(buc_block_t->port1.d_run || buc_block_t->port2.d_run || buc_block_t->net.d_run)
	{
		buc_pos_t->d_oc = FALSE;	
		buc_pos_t->d_run = TRUE;
		buc_pos_t->d_brk = FALSE;
		buc_pos_t->d_res = FALSE;
		buc_pos_t->d_mul = FALSE;
		buc_pos_t->d_emc = FALSE;
		buc_pos_t->d_other = FALSE;
	}
	else if(buc_block_t->port1.d_brk || buc_block_t->port2.d_brk || buc_block_t->net.d_brk)
	{
		buc_pos_t->d_oc = FALSE;	
		buc_pos_t->d_run = FALSE;
		buc_pos_t->d_brk = TRUE;
		buc_pos_t->d_res = FALSE;
		buc_pos_t->d_mul = FALSE;
		buc_pos_t->d_emc = FALSE;
		buc_pos_t->d_other = FALSE;
	}
	else if(buc_block_t->port1.d_res || buc_block_t->port2.d_res || buc_block_t->net.d_res)
	{
		buc_pos_t->d_oc = FALSE;	
		buc_pos_t->d_run = FALSE;
		buc_pos_t->d_brk = FALSE;
		buc_pos_t->d_res = TRUE;
		buc_pos_t->d_mul = FALSE;
		buc_pos_t->d_emc = FALSE;
		buc_pos_t->d_other = FALSE;
	}
	else if(buc_block_t->port1.d_mul || buc_block_t->port2.d_mul || buc_block_t->net.d_mul)
	{
		buc_pos_t->d_oc = FALSE;	
		buc_pos_t->d_run = FALSE;
		buc_pos_t->d_brk = FALSE;
		buc_pos_t->d_res = FALSE;
		buc_pos_t->d_mul = TRUE;
		buc_pos_t->d_emc = FALSE;
		buc_pos_t->d_other = FALSE;
	}
	else if(buc_block_t->port1.d_emc || buc_block_t->port2.d_emc || buc_block_t->net.d_emc)
	{
		buc_pos_t->d_oc = FALSE;	
		buc_pos_t->d_run = FALSE;
		buc_pos_t->d_brk = FALSE;
		buc_pos_t->d_res = FALSE;
		buc_pos_t->d_mul = FALSE;
		buc_pos_t->d_emc = TRUE;
		buc_pos_t->d_other = FALSE;
	}
	else
	{
		buc_pos_t->d_oc = FALSE;	
		buc_pos_t->d_run = FALSE;
		buc_pos_t->d_brk = FALSE;
		buc_pos_t->d_res = FALSE;
		buc_pos_t->d_mul = FALSE;
		buc_pos_t->d_emc = FALSE;
		buc_pos_t->d_other = TRUE;
	}
	
	if(DI21)
	{
		buc_pos_t->s_zero = TRUE;
		buc_pos_t->s_brake_area = FALSE;
		buc_pos_t->s_fast_brake = FALSE;
	}
	else if(DI22)
	{
		buc_pos_t->s_zero = FALSE;
		buc_pos_t->s_brake_area = TRUE;
		buc_pos_t->s_fast_brake = FALSE;
	}	
	else if(DI23)
	{
		buc_pos_t->s_zero = FALSE;
		buc_pos_t->s_brake_area = FALSE;
		buc_pos_t->s_fast_brake = TRUE;
	}	
}


static void Get_Punish_State(BCU_PunishTypeDef *punish_t)
{
	if(DI10)
		punish_t->ATP = TRUE;
	else
		punish_t->ATP = FALSE;

	if(DI11)
		punish_t->CCU = TRUE;
	else
		punish_t->CCU = FALSE;

	if(DI12)
		punish_t->HD = TRUE;
	else
		punish_t->HD = FALSE;
	
	if(DI13)
		punish_t->DS = TRUE;
	else
		punish_t->DS = FALSE;
}


/***获取按键状态***/
static void Get_Key_Value(unsigned char skey_t, BCU_KeyTypeDef *key_t)
{
	BOOL s_key[4] = {FALSE};
	static BOOL f_key[4] = {FALSE};
	unsigned char filter_timer[4] = {20,20,20,20};
	static unsigned char key_timer[4][2] = {0};
	
	Byte_To_Bit(skey_t, s_key, 4);
	Bit_Filter(f_key, s_key, key_timer, filter_timer, 4);
	
	key_t->F1 = *(f_key+0);	
	key_t->F2 = *(f_key+1);		
	key_t->F3 = *(f_key+2);	
	key_t->F4 = *(f_key+3);		
}



/**来自模拟板的串口数据**/
static void Get_Toggle_Switch(unsigned char *dindata_t, BCU_SwitchTypeDef *switch_t)
{
	BOOL s_switch[4] = {FALSE};
	static BOOL f_switch[4] = {FALSE};
	
	unsigned char filter_timer[4] = {20,20,20,20};
	static unsigned char switch_timer[4][2] = {0};
	

	Byte_To_Bit(*(dindata_t + 20), s_switch, 4);
	Bit_Filter(f_switch, s_switch, switch_timer, filter_timer, 4);
	
	switch_t->K1 = *(f_switch + 0);	
	switch_t->K2 = *(f_switch + 1);
	switch_t->K3 = *(f_switch + 2);	
	switch_t->K4 = *(f_switch + 3);
}




/***压力传感器模拟量换算***/
static unsigned int Get_Press_Value(unsigned int adc_data_t)
{
	unsigned int conv_value=0,temp=0;
	temp = (adc_data_t > 12221)? adc_data_t : 12221;
	temp = (adc_data_t > 61105)? 61105 : adc_data_t;

	conv_value = (unsigned int)(((float)(temp - 12221)*1000)/48884);		//16位采样
	return (conv_value);
}


/***制动区模拟量换算***/
static unsigned int Get_Area_Value(unsigned int adc_data_t)
{
	unsigned int conv_value=0,temp=0;
	temp = (adc_data_t > 2323)? adc_data_t : 2323;
	
	conv_value = MAX((unsigned int)((float)(temp-3232)*( M_JIANYA_MAX - 50)/59000) + 50, M_JIANYA_MAX);
	return (conv_value);
}


/**来自EP-顶板的IO总线数据**/
void Get_BCU_Press(unsigned char *aindata_t, BCU_PressTypeDef *bcu_press_t)
{
	unsigned int bc=0,mr=0,bp=0,er=0,cv=0,ebv1=0,ebv2=0,ebv3=0;
	
	bc = *(aindata_t + 1);
	bc <<= 8;
	bc |= *(aindata_t + 0);
	bcu_press_t->bc = Get_Press_Value(bc);
	
	mr = *(aindata_t + 3);
	mr <<= 8;
	mr |= *(aindata_t + 2);
	bcu_press_t->mr = Get_Press_Value(mr);
	
	bp = *(aindata_t + 5);
	bp <<= 8;
	bp |= *(aindata_t + 4);
	bcu_press_t->bp = Get_Press_Value(bp);
	
	er = *(aindata_t + 7);
	er <<= 8;
	er |= *(aindata_t + 6);
	bcu_press_t->er = Get_Press_Value(er);	
	
	cv = *(aindata_t + 9);
	cv <<= 8;
	cv |= *(aindata_t + 8);
	bcu_press_t->cv = Get_Press_Value(cv);

	ebv1 = *(aindata_t + 11);
	ebv1 <<= 8;
	ebv1 |= *(aindata_t + 10);
	bcu_press_t->ebv1 = Get_Area_Value(ebv1);
	
	ebv2 = *(aindata_t + 13);
	ebv2 <<= 8;
	ebv2 |= *(aindata_t + 12);
	bcu_press_t->ebv2 = Get_Area_Value(ebv2);
	
	ebv3 = *(aindata_t + 15);
	ebv3 <<= 8;
	ebv3 |= *(aindata_t + 14);
	bcu_press_t->ebv3 = Get_Area_Value(ebv3);

}	



///***获取按键状态***/
//void Get_Key_Value(BOOL *key_value_t, unsigned char filter_time)
//{
//	BOOL skey1,skey2,skey3,skey4;
//	
//	skey1 = (BOOL)HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2);
//	skey2 = (BOOL)HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15);
//	skey3 = (BOOL)HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_14);
//	skey4 = (BOOL)HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_13);	
//	
//	Bit_Filter(*(key_value_t + 0), skey1, filter_time);	
//	Bit_Filter(*(key_value_t + 1), skey2, filter_time);	
//	Bit_Filter(*(key_value_t + 2), skey3, filter_time);	
//	Bit_Filter(*(key_value_t + 3), skey4, filter_time);	
//}



void KEY_Function(void)
{
//	static unsigned char key_start=0;
//	static unsigned char cur_funcion_num=0;
//	
//	BOOL k1,k2,k3,k4;
//	unsigned char time[3]={0};
//	
//	Get_Key_Value(key_value, bcu_key, 6);

//	k1 = bcu_key.K1;
//	k2 = bcu_key.K2;
//	k3 = bcu_key.K3;
//	k4 = bcu_key.K4;
//	
////	if(funcion_num == 0)
////	{
//	/****启动按键设定***/
//	if((k1 && ~k2 && ~k3 && ~k4) && key_start == 0)
//	{
//		if(time[0]++ > 60)
//		{
//			time[0] = 0;
//			key_start = 1;
//		}	
//	}
//	
//	/****进入按键设定***/
//	if(key_start > 0)
//	{
//		if(k2 && ~k1 && ~k3 && ~k4)				//k2键，功能号减１
//		{
//			if(cur_funcion_num-- < 1)
//				cur_funcion_num = 0;
//		}
//		
//		else if(k4 && ~k1 && ~k2 && ~k3)		//k4键，功能号加１
//		{
//			if(cur_funcion_num++ > MAX_FUNC_NUM)
//				cur_funcion_num  = MAX_FUNC_NUM;
//		}
//			
//		else if(~k1 && ~k2 && ~k3 && ~k4)
//		{
//			if(time[1]++ > 100)
//			{
//				time[1] = 0;
//				funcion_num = 0;
//				cur_funcion_num = 0;
//				key_start = 0;
//			}
//		}
//		
//		if(cur_funcion_num == 1)
//		{
//			if(k2 && k3 && ~k1 && ~k4)			//k2和k3键，启动自检
//			{
//				if(time[1]++ > 100)
//				{
//					time[1] = 0;
//					funcion_num = cur_funcion_num;
//					cur_funcion_num = 0;
//					key_start = 0;
//				}
//			}		
//		}
//		else
//		{
//			if(k3 && ~k1 && ~k2 && ~k4)		//k3键，功能确认
//			{
//				funcion_num = cur_funcion_num;
//				cur_funcion_num = 0;
//				key_start = 0;
//			}
//		}
//	}	
////	}
//	
//	switch(funcion_num)
//	{
//		case 0:	//按键功能设置	
//			if(bcu_enable == 0)
//			{
//				Display_Num(888);
//			}
//			else
//			{
//				if(1)	//本机
//					Display_Text("BCU");
//				else	//补机
//					Display_Text("bcu");
//			}
//			break;
//		
//		case 2:
//			Fault_Display();
//			break;		
//		
//		case 4:
//			Fault_Clear();
//			break;		

//		case 5:		//显示均衡冗余压力
//			Press_Display(bcu_press.er_backup);
//			break;	

//		case 7:		//关闭紧急连锁DI17
//			
//			break;
//		
//		case 8:		//关闭总风传感器封锁牵引
//			
//			break;

//		case 9:
//			Press_Display(bcu_press.er_backup);	//PO2
//			break;

//		case 10:
//			Press_Display(bcu_press.er);			//显示均衡压力
//			break;

//		case 11:
//			Press_Display(bcu_press.bp);			//显示列车压力
//			break;

//		case 12:
//			Press_Display(bcu_press.ma);			//显示总风压力
//			break;

//		case 13:
//			Press_Display(bcu_press.bc1);		//显示制动缸1压力
//			break;

//		case 14:
//			Press_Display(bcu_press.er_backup);	//PO1
//			break;

//		case 16:
//			Press_Display(bcu_press.bc_pre);		//显示闸缸预控压力
//			break;

//		default:
//			funcion_num = 0;
//			break;
//	}
}



void Get_Brake_Mode(BCU_ModeTypeDef *bcu_mode_t)
{
	if(DI18)
	{
		bcu_mode_t->master = TRUE;		//主机
		bcu_mode_t->slave = FALSE;		//从机
			
		Display_String('B',0);
		Display_String('C',1);
		Display_String('U',2);
	}
	else
	{
		bcu_mode_t->master = FALSE;		//主机
		bcu_mode_t->slave = TRUE;		//从机
		
		Display_String('b',0);
		Display_String('c',1);
		Display_String('u',2);
	}
}



BOOL Brake_Pos_Move(BRAKE_PosTypeDef *buc_cur_pos_t, BRAKE_PosTypeDef *buc_last_pos_t)
{
	if(
		buc_last_pos_t->d_oc  != buc_cur_pos_t->d_oc 	||
		buc_last_pos_t->d_run != buc_cur_pos_t->d_run	||
		buc_last_pos_t->d_brk != buc_cur_pos_t->d_brk	||
		buc_last_pos_t->d_res != buc_cur_pos_t->d_res	||
		buc_last_pos_t->d_mul != buc_cur_pos_t->d_mul	||
		buc_last_pos_t->d_emc != buc_cur_pos_t->d_emc
	)
		return TRUE;
	else
		return FALSE;
}


BOOL Switch_Move(BCU_SwitchTypeDef *buc_cur_switch_t, BCU_SwitchTypeDef *buc_last_switch_t)
{
	if(
		buc_last_switch_t->K1 != buc_cur_switch_t->K1 	||
		buc_last_switch_t->K2 != buc_cur_switch_t->K2	||
		buc_last_switch_t->K3 != buc_cur_switch_t->K3	||
		buc_last_switch_t->K4 != buc_cur_switch_t->K4	
	)
		return TRUE;
	else
		return FALSE;
}



void Updata_Brake_Pos(BRAKE_PosTypeDef *buc_cur_pos_t, BRAKE_PosTypeDef *buc_last_pos_t)
{
	buc_last_pos_t->d_oc  = buc_cur_pos_t->d_oc;
	buc_last_pos_t->d_run = buc_cur_pos_t->d_run;
	buc_last_pos_t->d_brk = buc_cur_pos_t->d_brk;
	buc_last_pos_t->d_res = buc_cur_pos_t->d_res;
	buc_last_pos_t->d_mul = buc_cur_pos_t->d_mul;
	buc_last_pos_t->d_emc = buc_cur_pos_t->d_emc;
}


void Updata_Switch(BCU_SwitchTypeDef *buc_cur_switch_t, BCU_SwitchTypeDef *buc_last_switch_t)
{
	buc_last_switch_t->K1 = buc_cur_switch_t->K1;
	buc_last_switch_t->K2 = buc_cur_switch_t->K2;
	buc_last_switch_t->K3 = buc_cur_switch_t->K3;
	buc_last_switch_t->K4 = buc_cur_switch_t->K4;
}


/***获取32位数字量状态及过滤设置***/
void Get_Input(void)
{
	Get_Brake_Position(&bcu_port,&bcu_pos);
	Get_Brake_Mode(&bcu_mode);
	Get_Key_Value(key_value, &bcu_key);
	Get_Toggle_Switch(&switch_value, &bcu_switch);
	Get_BCU_Press(IO_rx, &bcu_press);
	Get_Punish_State(&bcu_punish);
}

