//#include "edv.h"
#include "global.h"
#include "stdlib.h"

//*************************电子分配阀变量********************************************
int BPPressTemp[2] = {0};        //列车管采集值
int workCly_int = 0;             //工作风缸压力
//int memory_workCly_int = 0;
int workClyInit_int = 0;         //工作风缸压力起始值
int inputWorkClyCountNum = 0;    //工作风缸充风计数器
int inputBPClyInit_int = 0;      //进入充风 状态第一周期 列车管值
int lapBPClyInit_int = 0;        //进入 保压状态 第一周期列车管
int outputBPClyInit_int = 0;     //进入制动状态第一周期 列车管
int lapCountNum_int = 0;         //保压计数
int inputCountNum_int = 0;       //充风计数
int outputCountNum_int = 0;      //排风计数
int lapClyBrkInit_int = 0;       //保压状态闸缸目标初始值
int inputClyBrkInit_int = 0;     //充风状态闸缸目标初始值
int outputClyBrkInit_int = 0;    //排风状态闸缸目标初始值
int inputClyChange_int = 0;      //充风状态列车管变化量
int lapClyChange_int = 0;        //保压变化量
int outputClyChange_int = 0;     //排风变化量

int firstBrkCountNum_int = 0;    //初制动计数器
int simuClyBrkInit_int = 0;      //初制动第一周期大闸闸缸目标
int firstBrkBPClyInit_int = 0;   //初制动第一周期列车管

//*************************电子分配阀变量结束****************************************
int autoBrkCtrlTarCly_int;       //大闸目标压力
int memory_autoBrkCtrlTarCly[12] = {0};

float BPChangeRate_real = 0;

unsigned int BP_ADValue[8];
float BP_ADValueChange[4];
float BPrate[3];

unsigned int ERsetpoint_Press_int = 0;
#define SteadyCutin_B 0 


static void Get_BPChangeRate(BCU_PressTypeDef *bcu_press_t)
{
//	if(Can_Nod == 6)
//	{
//		BP_ADValue[7] = BP_ADValue[6];
//		BP_ADValue[6] = BP_ADValue[5];
//		BP_ADValue[5] = BP_ADValue[4];
//		BP_ADValue[4] = BP_ADValue[3];
//		BP_ADValue[3] = BP_ADValue[2];
//		BP_ADValue[2] = BP_ADValue[1];
//		BP_ADValue[1] = BP_ADValue[0];
//		BP_ADValue[0] = AD_data[1];//204BP
//		//BP_ADValue[0] = AD_data[2];//GET_AD(CHANNEL_2);
//		if(BP_ADValue[0]<12221)
//			BP_ADValue[0] = 12221;
//	}
//	else if(Can_Nod == 7)
//	{
//		BP_ADValue[7] = BP_ADValue[6];
//		BP_ADValue[6] = BP_ADValue[5];
//		BP_ADValue[5] = BP_ADValue[4];
//		BP_ADValue[4] = BP_ADValue[3];
//		BP_ADValue[3] = BP_ADValue[2];
//		BP_ADValue[2] = BP_ADValue[1];
//		BP_ADValue[1] = BP_ADValue[0];
//		BP_ADValue[0] = AD_data[0];//212BP
//		//BP_ADValue[0] = AD_data[2];//GET_AD(CHANNEL_2);
//		if(BP_ADValue[0]<12221)
//			BP_ADValue[0] = 12221;
//	}
	/*
	else if(Can_Nod == 8)
	{
		BP_ADValue[7] = BP_ADValue[6];
		BP_ADValue[6] = BP_ADValue[5];
		BP_ADValue[5] = BP_ADValue[4];
		BP_ADValue[4] = BP_ADValue[3];
		BP_ADValue[3] = BP_ADValue[2];
		BP_ADValue[2] = BP_ADValue[1];
		BP_ADValue[1] = BP_ADValue[0];
		BP_ADValue[0] = AD_data[3];
		//BP_ADValue[0] = AD_data[3];
		if(BP_ADValue[0]<13107)
			BP_ADValue[0] = 13107;
	}
	*/
	
	BP_ADValue[7] = BP_ADValue[6];
	BP_ADValue[6] = BP_ADValue[5];
	BP_ADValue[5] = BP_ADValue[4];
	BP_ADValue[4] = BP_ADValue[3];
	BP_ADValue[3] = BP_ADValue[2];
	BP_ADValue[2] = BP_ADValue[1];
	BP_ADValue[1] = BP_ADValue[0];
	BP_ADValue[0] = bcu_press_t->bp;	
	if(BP_ADValue[0]<12221)
		BP_ADValue[0] = 12221;	
	
	
	//三点
	BP_ADValueChange[0] = (BP_ADValue[0] + BP_ADValue[1] + BP_ADValue[2])/3;	//012
	BP_ADValueChange[1] = (BP_ADValue[1] + BP_ADValue[2] + BP_ADValue[3])/3;	//123
	BP_ADValueChange[2] = (BP_ADValue[2] + BP_ADValue[3] + BP_ADValue[4])/3;	//234
	BP_ADValueChange[3] = (BP_ADValue[5] + BP_ADValue[6] + BP_ADValue[7])/3;	//567


	BPrate[0] = ((float)(BP_ADValueChange[2] - BP_ADValueChange[3])/3276*1000/3*20);     //234-567//90ms
	BPrate[1] = ((float)(BP_ADValueChange[1] - BP_ADValueChange[3])/3276*5000);        	//123-567//120ms
	BPrate[2] = ((float)(BP_ADValueChange[0] - BP_ADValueChange[3])/3276*4000);        	//012-567//150ms

	BPChangeRate_real = (BPrate[0] + BPrate[1] + BPrate[2])/3;  //列车管压力变化率kPa/s
}

unsigned int EDV(BCU_ModeTypeDef *bcu_mode_t, BCU_SwitchTypeDef *switch_t, BCU_PressTypeDef *bcu_press_t) //电子分配阀
{
	int BPdecrease_int = 0;//列车管减压量(用于闸缸缓解时的列车压力点)
	//static int last_BPPress_int = 0;//上一个列车管压力值
	static unsigned char count_autoBrkCtrlTarCly_init = 0;
	
	Get_BPChangeRate(bcu_press_t);
	
	if(count_autoBrkCtrlTarCly_init == 0)
	{
//		autoBrkCtrlTarCly_int = BCPress_int;
		autoBrkCtrlTarCly_int = bcu_press_t->bc;		
		count_autoBrkCtrlTarCly_init = count_autoBrkCtrlTarCly_init + 1;
	}
	
	if(switch_t->K4)
		ERsetpoint_Press_int = 600;
	else
		ERsetpoint_Press_int = 500;
	
//	if(Trail_status || LeadCutout_status)//补机或单机	
	if(bcu_mode_t->slave)//补机或单机
	{
		BPdecrease_int = 35;//补机或单机时，缓解压力为565/465
		
		workCly_int = MIN(workCly_int,ERsetpoint_Press_int-5);
		if(workCly_int>=ERsetpoint_Press_int-5)
		  inputWorkClyCountNum = 0;
	}
	else if(bcu_mode_t->master)//本机
	{
//		if(!ERsetpoint_B)//0=定压600
		if(switch_t->K4)//0=定压600
			BPdecrease_int = 20;
		else             //定压500
			BPdecrease_int = 15;
			
	}
	
//	if(Can_Nod == 6)//BP
//	{
		BPPressTemp[0] = bcu_press_t->bp;
		BPPressTemp[1] = bcu_press_t->bp;
//		BPPressTemp[0] = A02_204BP_int;
//		BPPressTemp[1] = A02_204BP_int;
//		BPPressTemp[0] = BPPressTemp[2];
//		BPPressTemp[1] = BPPressTemp[2];
//		
//		BPPressTemp[2] = BPPressTemp[3];
//		BPPressTemp[3] = BPPressTemp[4];
//		BPPressTemp[4] = BPPressTemp[5];
//		BPPressTemp[5] = BPPressTemp[6];
//		BPPressTemp[6] = BPPressTemp[7];
//		BPPressTemp[7] = BPPressTemp[8];
//		BPPressTemp[8] = BPPressTemp[9];
//		BPPressTemp[9] = BPPressTemp[10];
//		BPPressTemp[10] = BPPressTemp[11];
//		BPPressTemp[11] = BPPressTemp[12];
//		BPPressTemp[12] = A02_204BP_int;
//		
//	}
//	else if(Can_Nod == 7)//BC
//	{
//		BPPressTemp[0] = A01_212BP_int;
//		BPPressTemp[1] = A01_212BP_int;
//		BPPressTemp[0] = BPPressTemp[2];
//		BPPressTemp[1] = BPPressTemp[2];
//		
//		BPPressTemp[2] = BPPressTemp[3];
//		BPPressTemp[3] = BPPressTemp[4];
//		BPPressTemp[4] = BPPressTemp[5];
//		BPPressTemp[5] = BPPressTemp[6];
//		BPPressTemp[6] = BPPressTemp[7];
//		BPPressTemp[7] = BPPressTemp[8];
//		BPPressTemp[8] = BPPressTemp[9];
//		BPPressTemp[9] = BPPressTemp[10];
//		BPPressTemp[10] = BPPressTemp[11];
//		BPPressTemp[11] = BPPressTemp[12];
//		BPPressTemp[12] = A01_212BP_int;
//	}
	/*
	else if(Can_Nod == 8)//IB
	{
//		BPPressTemp[0] = A04_211BP_int;
//	  BPPressTemp[1] = A04_211BP_int;
	}
	*/
//		if(!GradeRel_B)        //////////////////////////////////////////////////一次缓解
		if(switch_t->K1)        //////////////////////////////////////////////////一次缓解
		{
//			if(!ERsetpoint_B)                                                    //0=定压600
			if(!switch_t->K4)                                                  //0=定压600
			{
				if(autoBrkCtrlTarCly_int>=40 && autoBrkCtrlTarCly_int<440)   //处于制动状态
				{
					if(BPChangeRate_real>=4.0)                                //制动后充风
					{
						inputCountNum_int++;    //充风计数器
						lapCountNum_int = 0;    //保压计数器
						outputCountNum_int = 0; //排风计数///
						
						lapClyBrkInit_int=0;    //进入保压状态闸缸目标值�
						lapClyChange_int=0;     //保压状态时，列车管变化量
					
						if(inputCountNum_int==1)//充风状态第一个周期
						{
							inputBPClyInit_int = BPPressTemp[0];//进入充风状态第一个周期列车管值
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;//记住第一个周期闸缸目标值
						}
						
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;//充风状态列车管变化量
						if(inputClyChange_int>20)
						{					  
							autoBrkCtrlTarCly_int = 0;		
						}
						else
						{
							autoBrkCtrlTarCly_int = (inputClyBrkInit_int>0)?inputClyBrkInit_int:0;
					
						}
					}
					
					else if(BPChangeRate_real>-4.0 && BPChangeRate_real<4.0)    //保压
					{
						lapCountNum_int++;          //保压状态计数器开始计时
						inputCountNum_int = 0;      //充风计数器
						outputCountNum_int = 0;     //排风计数器
						
						outputClyChange_int = 0;    //排风列车管变化量
						inputClyBrkInit_int = 0;    //充风状态大闸目标值
						inputClyChange_int = 0;     //充风状态列车管变化量
				
						if(lapCountNum_int==1)
						{
							lapBPClyInit_int = BPPressTemp[0];
							lapClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
			
						lapClyChange_int = (BPPressTemp[1]-lapBPClyInit_int);
						if(lapClyChange_int>18) //列车管压力上升自然缓解
						{
							if(lapClyChange_int>20)          //消除临界值时目标值抖动 
							{
								autoBrkCtrlTarCly_int = 0; //xiaochuliecheguantiaobianhuanjie
						  }
						}
						else if(lapClyChange_int<-2)//列车管泄漏
						{
							if(lapClyChange_int<-4)       //消除临界值时目标值抖动 
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*1)<180)?(lapClyBrkInit_int+abs(lapClyChange_int)*1):180;
								else//平稳切除
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*5/2)<425+0)?
								(lapClyBrkInit_int+abs(lapClyChange_int)*5/2):425+0;
								
							}
						}
						else 
						{
							autoBrkCtrlTarCly_int = lapClyBrkInit_int;
						}
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 420;
						}
						else if(BPPressTemp[1]>=ERsetpoint_Press_int -BPdecrease_int)//565/580
						{
							autoBrkCtrlTarCly_int = 0;	//xiaochuliecheguantiaobianhuanjie			
						}
					}
					else //BPChangeRate_real<=-4.0              //制动
					{
						outputCountNum_int++; //排风计数器开始计数
						inputCountNum_int=0;  //充风计数器//
						lapCountNum_int = 0;  //保压计数器
						
						lapClyBrkInit_int =0;           //保压状态初始大闸目标值
						
						if(outputCountNum_int == 1)
						{
							outputBPClyInit_int = BPPressTemp[0];
							outputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = outputBPClyInit_int - BPPressTemp[1];
						if(BPPressTemp[1] >= ERsetpoint_Press_int -BPdecrease_int)//580/565
						{
							autoBrkCtrlTarCly_int = 0;
						}
						else if((BPPressTemp[1]>= ERsetpoint_Press_int -60)&&(BPPressTemp[1]< ERsetpoint_Press_int-BPdecrease_int))//540~580/565
						{
							if(outputClyChange_int>1) //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<180)?((workCly_int-BPPressTemp[1])*1+10):180;
								else//jianya50~mubiao118
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0)<425+0)?(((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0):425+0;
							}
						}
/////////////////////////////////////////////////////////////////////////////////////////////////60,100fenduan
						else if((BPPressTemp[1]>= ERsetpoint_Press_int -100)&&(BPPressTemp[1]<ERsetpoint_Press_int -60))//BPPressTemp[1]<=540
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<180)?((workCly_int-BPPressTemp[1])*1+10):180;
								else//
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + 
								(((((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0)<425+0)?(((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0):425+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),425);
							}
						}
						else //BPPressTemp[1]<=ERsetpoint_Press_int -100//100fenduan
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<180)?((workCly_int-BPPressTemp[1])*1+10):180;
								else//100~258,170~425+8,173~432.5+8
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + ((workCly_int-(ERsetpoint_Press_int-40))*13/4)
								+(((((workCly_int-(BPPressTemp[1]+100))*2.5+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*2.5+0):0)<425+0)?(((workCly_int-(BPPressTemp[1]+100))*2.5+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*2.5+0):0):425+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),425);
							}
						}
					///////////////////////////////////////////////////////////////////////////////////	
						
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 420;
						}
					}
				}
				else if(autoBrkCtrlTarCly_int>=440)                   //紧急后分配阀状态
				{
					if(BPChangeRate_real>=4.0)                          //紧急后充风缓解
					{
						inputCountNum_int++;  //充风计数器
						lapCountNum_int = 0;//保压计数器
						outputCountNum_int = 0;  //排风计数
							
						lapClyBrkInit_int = 0;
						lapClyChange_int =0;
						outputClyChange_int = 0;
					
						if(inputCountNum_int == 1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int > 20 && BPPressTemp[1] > ERsetpoint_Press_int-200)//原400
						{
							autoBrkCtrlTarCly_int = 0;
						
						}
						else
						{
							autoBrkCtrlTarCly_int = (inputClyBrkInit_int>0)?inputClyBrkInit_int:0;
						
						}
					}
					else                //缓慢充风时，解决长列时充风，机车闸缸不缓解的问题
					{
						if(BPPressTemp[1] > ERsetpoint_Press_int -200)//原400
						{
							autoBrkCtrlTarCly_int = 0;
						}
						else
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 420;
							inputClyChange_int = 0;
							inputCountNum_int = 0;//充风计数
							lapCountNum_int = 0;  //保压计数器
							outputCountNum_int = 0;      //排风计数
							
						}
					}
				}
				else//autoBrkCtrlTarCly_int<40          初充风
				{
					inputCountNum_int = 0;             //充风计数
					lapCountNum_int = 0;         //保压计数器
					outputCountNum_int = 0;      //排风计数
					
					if(BPChangeRate_real>-4.0)
					{
						firstBrkCountNum_int = 0;        //初制动计数器
						outputClyChange_int = 0;         //排风变化量
						
						if(BPPressTemp[1] >= ERsetpoint_Press_int-BPdecrease_int)//580/565
							autoBrkCtrlTarCly_int = 0;
						
						if(BPPressTemp[1] > workCly_int) //列车管压力大于工作风缸
						{
							inputWorkClyCountNum++;        //工作风缸充风计数器
							if(inputWorkClyCountNum == 1)  //
							{
								workClyInit_int = workCly_int;//工作风缸压力起始值
							}
							else if(inputWorkClyCountNum > 7000)
							{
								inputWorkClyCountNum = 7000;
							}
							
							workCly_int = (((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1]))<ERsetpoint_Press_int?
							(((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1])):ERsetpoint_Press_int;

						}
						
					}
					else                            //BPChangeRate_real<=-4.0,初制动
					{
						firstBrkCountNum_int++;       //初制动计数器
						inputWorkClyCountNum = 0;     //工作风缸充风计数器
						if(firstBrkCountNum_int == 1)
						{
							firstBrkBPClyInit_int = BPPressTemp[0];//初制动第一周期列车管压力值
							simuClyBrkInit_int = autoBrkCtrlTarCly_int;//初制动第一周期大闸目标值
						}
						outputClyChange_int = firstBrkBPClyInit_int-BPPressTemp[1];//列车管减压量
						if((outputClyChange_int>9) && (workCly_int>=BPPressTemp[1]))
						{
							if(SteadyCutin_B)//平稳投入
								autoBrkCtrlTarCly_int = ((workCly_int-BPPressTemp[1])*3/2<180)?(workCly_int-BPPressTemp[1])*3/2:180;
							else
							  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0)<425+0)?
							(((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0):425+0;
							
						}
						else
						{
							autoBrkCtrlTarCly_int = simuClyBrkInit_int;//大闸目标值保持初始值
						}
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 420;
						}
					}

				}
			}
////////////////////////////////////////////////////////////////////////////////一次缓解
		  else                                                                    //定压500
			{
				if(autoBrkCtrlTarCly_int>=40 && autoBrkCtrlTarCly_int<380)
				{
					if(BPChangeRate_real>=4.0)
					{
						inputCountNum_int++;
						lapCountNum_int = 0;
						outputCountNum_int = 0;
						lapClyBrkInit_int=0;
						lapClyChange_int=0;
						if(inputCountNum_int==1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int>20)
						{
							autoBrkCtrlTarCly_int = 0;
						}
						else
							autoBrkCtrlTarCly_int = (inputClyBrkInit_int>0)?inputClyBrkInit_int:0;
					}
					else if(BPChangeRate_real>-4.0 && BPChangeRate_real<4.0)
					{
						lapCountNum_int++;
						inputCountNum_int = 0;
						outputCountNum_int = 0;
						outputClyChange_int = 0;
						inputClyBrkInit_int = 0;
						inputClyChange_int = 0;
						if(lapCountNum_int==1)
						{
							lapBPClyInit_int = BPPressTemp[0];
							lapClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						lapClyChange_int = BPPressTemp[1]-lapBPClyInit_int;
						
						if(lapClyChange_int>18)
						{
							if(lapClyChange_int>20)
								autoBrkCtrlTarCly_int = 0;
						}
						else if(lapClyChange_int<-2)
						{
							if(lapClyChange_int<-4)
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*1)<150)?(lapClyBrkInit_int+abs(lapClyChange_int)*1):150;
								else
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*5/2)<365+0)?
								(lapClyBrkInit_int+abs(lapClyChange_int)*5/2):365+0;
							}
						}
						else 
							autoBrkCtrlTarCly_int = lapClyBrkInit_int;
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 320;
						}
						else if(BPPressTemp[1] >=ERsetpoint_Press_int-BPdecrease_int)//485/465
							autoBrkCtrlTarCly_int = 0;
					}
					else    // BPChangeRate_real<=-4.0
					{
						outputCountNum_int++;
						inputCountNum_int=0;
						lapCountNum_int = 0;
						
						lapClyBrkInit_int =0;
						if(outputCountNum_int == 1)
						{
							outputBPClyInit_int = BPPressTemp[0];
							outputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = outputBPClyInit_int - BPPressTemp[1];
						if(BPPressTemp[1] >=ERsetpoint_Press_int-BPdecrease_int) //485/465
							autoBrkCtrlTarCly_int = 0;
						else if(BPPressTemp[1] >=ERsetpoint_Press_int-60 && BPPressTemp[1] <ERsetpoint_Press_int-BPdecrease_int) //440~485/465
						{
							if(outputClyChange_int > 1) 
							{
								if(SteadyCutin_B)
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<150)?((workCly_int-BPPressTemp[1])*1+10):150;
								else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0)<365+0)?(((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0):365+0;
							}
						}
//						else//260~440
//						{
//							if(outputClyChange_int>1) 
//							{
//								if(SteadyCutin_B)//平稳投入
//									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<150)?((workCly_int-BPPressTemp[1])*1+10):150;
//								else
//								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*5/2+8)>0?
//								((workCly_int-BPPressTemp[1])*5/2+8):0)<365+0)?(((workCly_int-BPPressTemp[1])*5/2+8)>0?
//								((workCly_int-BPPressTemp[1])*5/2+8):0):365+0;
//							}
//						}
//						
						////////////////////////////////////////////////////////////////////////////////////////60,100fenduan
						else if((BPPressTemp[1]>= ERsetpoint_Press_int -100)&&(BPPressTemp[1]<ERsetpoint_Press_int -60))//BPPressTemp[1]<=540
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<150)?((workCly_int-BPPressTemp[1])*1+10):150;
								else//
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + 
								(((((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0)<365+0)?(((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0):365+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),365);
							}
						}
						else //BPPressTemp[1]<=ERsetpoint_Press_int -100//100fenduan
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<150)?((workCly_int-BPPressTemp[1])*1+10):150; 
								else//100~258,170~425+8,173~432.5+8
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + ((workCly_int-(ERsetpoint_Press_int-40))*13/4)
								+(((((workCly_int-(BPPressTemp[1]+100))*23/8+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*23/8+0):0)<365+0)?(((workCly_int-(BPPressTemp[1]+100))*23/8+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*23/8+0):0):365+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),365);
							}
						}
					///////////////////////////////////////////////////////////////////////////////////	
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 320;
						}
					}
				}
				else if(autoBrkCtrlTarCly_int>=380)   //紧急后分配阀状态
				{
					if(BPChangeRate_real>=4.0)
					{
						inputCountNum_int++;
						lapCountNum_int = 0;
						outputCountNum_int = 0;      //排风计数
						lapClyBrkInit_int = 0;
						lapClyChange_int =0;
						outputClyChange_int = 0;
						
						if(inputCountNum_int == 1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int > 20 && BPPressTemp[1] > ERsetpoint_Press_int-140)//360
							autoBrkCtrlTarCly_int = 0;
						else
							autoBrkCtrlTarCly_int = (inputClyBrkInit_int>0)?inputClyBrkInit_int:0;
					}
					else   //解决长列时充风，机车闸缸不缓解的问题
					{
						if(BPPressTemp[1] > ERsetpoint_Press_int-140)//360
							autoBrkCtrlTarCly_int = 0;
						else
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 320;
							inputClyChange_int = 0;
							inputCountNum_int = 0;
							lapCountNum_int = 0;  //保压计数器
						  outputCountNum_int = 0;      //排风计数
						}
					}
				}
				else   //autoBrkCtrlTarCly_int<40
				{
					
					inputCountNum_int = 0;
					lapCountNum_int = 0;  //保压计数器
					outputCountNum_int = 0;      //排风计数
					if(BPChangeRate_real>-4.0)
					{
						firstBrkCountNum_int = 0;
						outputClyChange_int = 0;
						
						if(BPPressTemp[1] >= ERsetpoint_Press_int-BPdecrease_int)//485/465
							autoBrkCtrlTarCly_int = 0;
						
						if(BPPressTemp[1] > workCly_int)
						{
							inputWorkClyCountNum++;
							
							if(inputWorkClyCountNum == 1)
							{
								workClyInit_int = workCly_int;
							}
							else if(inputWorkClyCountNum > 7000)
							{
								inputWorkClyCountNum = 7000;
							}

							workCly_int = (((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1]))<ERsetpoint_Press_int?
							(((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1])):ERsetpoint_Press_int;						
						}
					
					}
					else
					{
						firstBrkCountNum_int++;
						inputWorkClyCountNum = 0;
						if(firstBrkCountNum_int == 1)
						{
							firstBrkBPClyInit_int = BPPressTemp[0];
							simuClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = firstBrkBPClyInit_int-BPPressTemp[1];
						if((outputClyChange_int>9)&&(workCly_int>BPPressTemp[1]))
						{
							if(SteadyCutin_B)//平稳投入
								autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*3/2)<365)?((workCly_int-BPPressTemp[1])*3/2):365;
							else
							  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0)<365+0)?
							(((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0):365+0;
						}
						else
							autoBrkCtrlTarCly_int = simuClyBrkInit_int;
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							workCly_int = 320;
						}
					}
				}
			}
  	}
///////////////////////////////////////////////////////////////////////////////////////////////////////
		else                                                                     //gradeRel_uc=1,阶段缓解  
		{
			if(switch_t->K4)                                                   //定压600
			{
				if(autoBrkCtrlTarCly_int>=40 && autoBrkCtrlTarCly_int<440)   //处于制动状态
				{
//					if(workCly_int!=420)
//					{
//						memory_workCly_int = workCly_int;
//					}
					if(BPChangeRate_real>=4.0)                                //制动后充风
					{
						inputCountNum_int++;
						lapCountNum_int = 0;
						outputCountNum_int = 0;
						lapClyBrkInit_int=0;
						lapClyChange_int=0;
					
						if(inputCountNum_int==1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int>3)
						{
							if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//580/565
								autoBrkCtrlTarCly_int = 0;
							if(BPPressTemp[1]>ERsetpoint_Press_int-60 && BPPressTemp[1]<ERsetpoint_Press_int-BPdecrease_int) //540~580/565
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+14)<180)?((workCly_int-BPPressTemp[1])*1+14):180;
							  else
							    autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0)<425+0)?(((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0):425+0;
							}
						
							else if(BPPressTemp[1]<=ERsetpoint_Press_int-60 && BPPressTemp[1]>=ERsetpoint_Press_int-160)//440~540
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+14)<180)?((workCly_int-BPPressTemp[1])*1+14):180;
							  else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*11/5+0)>0?
								((workCly_int-BPPressTemp[1])*11/5+0):0)<425+0)?(((workCly_int-BPPressTemp[1])*11/5+0)>0?
								((workCly_int-BPPressTemp[1])*11/5+0):0):425+0;
							}
							else if(BPPressTemp[1]<=ERsetpoint_Press_int-160 && BPPressTemp[1]>=260)//260~440
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+14)<180)?((workCly_int-BPPressTemp[1])*1+14):180;
							  else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*5/2+0)>0?
								((workCly_int-BPPressTemp[1])*5/2+0):0)<425+0)?(((workCly_int-BPPressTemp[1])*5/2+0)>0?
								((workCly_int-BPPressTemp[1])*5/2+0):0):425+0;
							}
							else if(BPPressTemp[1]<260)
							{
								autoBrkCtrlTarCly_int=455;
								//workCly_int=420;
							}
						}
					//else autoBrkCtrlTarCly_int=inputClyBrkInit_int;临界值抖动
					}
					else if(BPChangeRate_real>-4.0 && BPChangeRate_real<4.0)    //保压
					{
						lapCountNum_int++;
						inputCountNum_int = 0;
						outputCountNum_int = 0;
						outputClyChange_int = 0;
						inputClyBrkInit_int = 0;
						inputClyChange_int = 0;
				
						if(lapCountNum_int==1)
						{
							lapBPClyInit_int = BPPressTemp[0];
							lapClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
			
						lapClyChange_int = (BPPressTemp[1]-lapBPClyInit_int);
						if(lapClyChange_int>2)
						{
							if(lapClyChange_int>4)          //消除临界值时目标值抖动 
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int-abs(lapClyChange_int)*1)<180)?(lapClyBrkInit_int-abs(lapClyChange_int)*1):180;
							  else
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int-abs(lapClyChange_int)*5/2)<425+0)?
								(lapClyBrkInit_int-abs(lapClyChange_int)*5/2):425+0;
							}
						}
						else if(lapClyChange_int<-2)
						{
							if(lapClyChange_int<-4)
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*1)<180)?(lapClyBrkInit_int+abs(lapClyChange_int)*1):180;
							  else
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*5/2)<425+0)?
								(lapClyBrkInit_int+abs(lapClyChange_int)*5/2):425+0;
							}
						}
						else 
						{
							autoBrkCtrlTarCly_int = lapClyBrkInit_int;
						}
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 420;
						}
						else if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//580/565
						{
							autoBrkCtrlTarCly_int = 0;
						}
					}
					else    // BPChangeRate_real<=-4.0                   //制动
					{
						outputCountNum_int++;
						inputCountNum_int=0;
						lapCountNum_int = 0;
						lapClyBrkInit_int =0;
						if(outputCountNum_int == 1)
						{
							outputBPClyInit_int = BPPressTemp[0];
							outputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = abs(outputBPClyInit_int - BPPressTemp[1]);
						//if(outputClyChange_int<=2)
							 //autoBrkCtrlTarCly_int=outputClyBrkInit_int;
						if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//580/565
							autoBrkCtrlTarCly_int = 0;
						else if(BPPressTemp[1]>=ERsetpoint_Press_int-60 && BPPressTemp[1]<ERsetpoint_Press_int-BPdecrease_int)//540~580/565
						{	
							if(outputClyChange_int>1) 
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<180)?((workCly_int-BPPressTemp[1])*1+10):180;
							  else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0)<425+0)?(((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0):425+0;
							}
						}
						/*
						else if(BPPressTemp[1]>=260 && BPPressTemp[1]<=ERsetpoint_Press_int-60)//260~540
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+14)<180)?((workCly_int-BPPressTemp[1])*1+14):180;
								else
									autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*5/2+8)>0?
								((workCly_int-BPPressTemp[1])*5/2+8):0)<425+0)?(((workCly_int-BPPressTemp[1])*5/2+8)>0?
								((workCly_int-BPPressTemp[1])*5/2+8):0):425+0;
							}
						}
						*/
						////////////////////////////////////////////////////////////////////
						else if((BPPressTemp[1]>= ERsetpoint_Press_int -100)&&(BPPressTemp[1]<ERsetpoint_Press_int -60))//BPPressTemp[1]<=540
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<180)?((workCly_int-BPPressTemp[1])*1+10):180;
								else//
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + 
								(((((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0)<425+0)?(((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0):425+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),425);
							}
						}
						else if(BPPressTemp[1]>=260 && BPPressTemp[1]<ERsetpoint_Press_int-100)
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<180)?((workCly_int-BPPressTemp[1])*1+10):180;
								else//100~258,170~425+8,173~432.5+8
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + ((workCly_int-(ERsetpoint_Press_int-40))*13/4)
								+(((((workCly_int-(BPPressTemp[1]+100))*2.5+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*2.5+0):0)<425+0)?(((workCly_int-(BPPressTemp[1]+100))*2.5+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*2.5+0):0):425+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),425);
							}
						}
					///////////////////////////////////////////////////////////////////////////////////	
						else if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 420;
						}
					}
				}
				else if(autoBrkCtrlTarCly_int>=440)                   //紧急后分配阀状态
				{
					if(BPChangeRate_real>=4.0)                          //紧急后充风缓解
					{
						inputCountNum_int++;
						lapCountNum_int = 0;
						outputCountNum_int = 0;
						lapClyBrkInit_int = 0;
						lapClyChange_int =0;
						outputClyChange_int = 0;
					
						if(inputCountNum_int == 1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int > 4 && BPPressTemp[1] > 400)
						{
							//autoBrkCtrlTarCly_int = 0;
							autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*5/2+0)>0?
								((workCly_int-BPPressTemp[1])*5/2+0):0)<425)?(((workCly_int-BPPressTemp[1])*5/2)>0?
								((workCly_int-BPPressTemp[1])*5/2+0):0):425;
						}
						else
						{
							autoBrkCtrlTarCly_int = (inputClyBrkInit_int>0)?inputClyBrkInit_int:0;
						}
					}
					else                //缓慢充风时，解决长列时充风，机车闸缸不缓解的问题
					{
						if(BPPressTemp[1] > 400)
						{
							autoBrkCtrlTarCly_int = MIN(MAX(((workCly_int-BPPressTemp[1])*5/2+0),0),425+0);
							//autoBrkCtrlTarCly_int = 0;
						}
						else
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 420;
							inputClyChange_int = 0;
							inputCountNum_int = 0;
							lapCountNum_int = 0;
							outputCountNum_int = 0;
						}
					}
				}
				else                //autoBrkCtrlTarCly_int<40初充风
				{
					inputCountNum_int = 0;
					lapCountNum_int = 0;
					outputCountNum_int = 0;
					if(BPChangeRate_real>-4.0)
					{
						firstBrkCountNum_int = 0;
						outputClyChange_int = 0;
						
						if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//580/565
							autoBrkCtrlTarCly_int = 0;
						
						if(BPPressTemp[1] > workCly_int)
						{
							inputWorkClyCountNum++;
							
							if(inputWorkClyCountNum == 1)
							{
								workClyInit_int = workCly_int;
							}
							else if(inputWorkClyCountNum > 7000)
							{
								inputWorkClyCountNum = 7000;
							}
							
							workCly_int = (((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1]))<ERsetpoint_Press_int?
							(((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1])):ERsetpoint_Press_int;
							
							//memory_workCly_int = workCly_int;
							
						}
					}
					else                            //初制动BPChangeRate_real<=-4.0
					{
						firstBrkCountNum_int++;
						inputWorkClyCountNum = 0;
						if(firstBrkCountNum_int == 1)
						{
							firstBrkBPClyInit_int = BPPressTemp[0];
							simuClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = firstBrkBPClyInit_int-BPPressTemp[1];
						if((outputClyChange_int>9) && (workCly_int>=BPPressTemp[1]))
						{
							if(SteadyCutin_B)//平稳投入
								autoBrkCtrlTarCly_int = ((workCly_int-BPPressTemp[1])*3/2<180)?(workCly_int-BPPressTemp[1])*3/2:180;
							else
							  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0)<425+0)?
							(((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0):425+0;
						}
						else
							autoBrkCtrlTarCly_int = simuClyBrkInit_int;
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 420;
						}
					}
				}
				
			}
	///////////////////////////////////////////////////////////////////////////阶段缓解 
			else                                                                   //定压500
			{
				if(autoBrkCtrlTarCly_int>=40 && autoBrkCtrlTarCly_int<380)   //处于制动状态
				{
					if(BPChangeRate_real>=4.0)                                //制动后充风
					{
						inputCountNum_int++;
						lapCountNum_int = 0;
						outputCountNum_int = 0;
						lapClyBrkInit_int=0;
						lapClyChange_int=0;
					
						if(inputCountNum_int==1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int>3)
						{
							if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//485/465
								autoBrkCtrlTarCly_int = 0;
							if(BPPressTemp[1]>ERsetpoint_Press_int-60 && BPPressTemp[1]<ERsetpoint_Press_int-BPdecrease_int) //440~485/465
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1)<150)?((workCly_int-BPPressTemp[1])*1):150;
							  else
							    autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0)<365+0)?(((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0):365+0;
							}
							else if(BPPressTemp[1]<=ERsetpoint_Press_int-60 && BPPressTemp[1]>=260)//260~440
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1)<150)?((workCly_int-BPPressTemp[1])*1):150;
							  else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*5/2+0)>0?
								((workCly_int-BPPressTemp[1])*5/2+0):0)<365+0)?(((workCly_int-BPPressTemp[1])*5/2+0)>0?
								((workCly_int-BPPressTemp[1])*5/2+0):0):365+0;
							}
							else if(BPPressTemp[1]<260)
							{
								autoBrkCtrlTarCly_int=455;
								//workCly_int=320;
							}
						}
					//else autoBrkCtrlTarCly_int=inputClyBrkInit_int;临界值抖动
					}
					else if(BPChangeRate_real>-4.0 && BPChangeRate_real<4.0)    //保压
					{
						lapCountNum_int++;
						inputCountNum_int = 0;
						outputCountNum_int = 0;
						outputClyChange_int = 0;
						inputClyBrkInit_int = 0;
						inputClyChange_int = 0;
				
						if(lapCountNum_int==1)
						{
							lapBPClyInit_int = BPPressTemp[0];
							lapClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
			
						lapClyChange_int = (BPPressTemp[1]-lapBPClyInit_int);
						if(lapClyChange_int>2)
						{
							if(lapClyChange_int>4)          //消除临界值时目标值抖动 
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int-abs(lapClyChange_int)*1-4)<150)?(lapClyBrkInit_int-abs(lapClyChange_int)*1-4):150;
							  else
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int-abs(lapClyChange_int)*5/2)<365+0)?
								(lapClyBrkInit_int-abs(lapClyChange_int)*5/2):365+0;
							}
						}
						else if(lapClyChange_int<-2)
						{
							if(lapClyChange_int<-4)
              {
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*1)<150)?(lapClyBrkInit_int+abs(lapClyChange_int)*1):150;
							  else
								  autoBrkCtrlTarCly_int = ((lapClyBrkInit_int+abs(lapClyChange_int)*5/2)<365+0)?
								(lapClyBrkInit_int+abs(lapClyChange_int)*5/2):365+0;
							}
						}
						else 
						{
							autoBrkCtrlTarCly_int = lapClyBrkInit_int;
						}
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 320;
						}
						else if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//485/465
						{
							autoBrkCtrlTarCly_int = 0;
						}
					}
					else    // BPChangeRate_real<=-4.0                   //制动
					{
						outputCountNum_int++;
						inputCountNum_int=0;
						lapCountNum_int = 0;
						lapClyBrkInit_int =0;
						if(outputCountNum_int == 1)
						{
							outputBPClyInit_int = BPPressTemp[0];
							outputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = abs(outputBPClyInit_int - BPPressTemp[1]);
						if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//485/465
							autoBrkCtrlTarCly_int = 0;
						
						else if(BPPressTemp[1]>=ERsetpoint_Press_int - 60 && BPPressTemp[1]<ERsetpoint_Press_int -BPdecrease_int)//440~485/465
						{
							if(outputClyChange_int>1) 
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+14)<150)?((workCly_int-BPPressTemp[1])*1+14):150;
							  else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0)<365+0)?(((workCly_int-BPPressTemp[1])*2+0)>0?
								((workCly_int-BPPressTemp[1])*2+0):0):365+0;
							}
						}
						
						////////////////////////////////////////////////////////////////////////////////////////60,100fenduan
						else if((BPPressTemp[1]>= ERsetpoint_Press_int -100)&&(BPPressTemp[1]<ERsetpoint_Press_int -60))//BPPressTemp[1]<=540
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<150)?((workCly_int-BPPressTemp[1])*1+10):150;
								else//
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + 
								(((((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0)<365+0)?(((workCly_int-(BPPressTemp[1]+60))*13/4+0)>0?
								((workCly_int-(BPPressTemp[1]+60))*13/4+0):0):365+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),365);
							}
						}
						else //BPPressTemp[1]<=ERsetpoint_Press_int -100//100fenduan
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
									autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+10)<150)?((workCly_int-BPPressTemp[1])*1+10):150; 
								else//100~258,170~425+8,173~432.5+8
								autoBrkCtrlTarCly_int = (workCly_int-(ERsetpoint_Press_int-60))*2 + ((workCly_int-(ERsetpoint_Press_int-40))*13/4)
								+(((((workCly_int-(BPPressTemp[1]+100))*23/8+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*23/8+0):0)<365+0)?(((workCly_int-(BPPressTemp[1]+100))*23/8+0)>0?
								((workCly_int-(BPPressTemp[1]+100))*23/8+0):0):365+0);
								autoBrkCtrlTarCly_int = MIN(MAX(autoBrkCtrlTarCly_int,0),365);
							}
						}
						
						/*
						else//260~440
						{
							if(outputClyChange_int>1)           //防止速率跳变引起的目标上升
							{
								if(SteadyCutin_B)//平稳投入
								  autoBrkCtrlTarCly_int = (((workCly_int-BPPressTemp[1])*1+14)<150)?((workCly_int-BPPressTemp[1])*1+14):150;
							  else
								  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*5/2+8)>0?
								((workCly_int-BPPressTemp[1])*5/2+8):0)<365+0)?(((workCly_int-BPPressTemp[1])*5/2+8)>0?
								((workCly_int-BPPressTemp[1])*5/2+8):0):365+0;
							}
						}
						*/
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 320;
						}
					}
				}
				else if(autoBrkCtrlTarCly_int>=380)                   //紧急后分配阀状态
				{
					if(BPChangeRate_real>=4.0)                          //紧急后充风缓解
					{
						inputCountNum_int++;
						lapCountNum_int = 0;
						outputCountNum_int = 0;
						lapClyBrkInit_int = 0;
						lapClyChange_int =0;
						outputClyChange_int = 0;
					
						if(inputCountNum_int == 1)
						{
							inputBPClyInit_int = BPPressTemp[0];
							inputClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						inputClyChange_int = BPPressTemp[1]-inputBPClyInit_int;
						if(inputClyChange_int > 20 && BPPressTemp[1] > ERsetpoint_Press_int - 140)//360
						{    //紧急阶段缓解
							
							autoBrkCtrlTarCly_int = MIN(MAX(((workCly_int-BPPressTemp[1])*5/2+0),0),365);
						}
						else
						{
							autoBrkCtrlTarCly_int = (inputClyBrkInit_int>0)?inputClyBrkInit_int:0;
						}
					}
					else                //缓慢充风时，解决长列时充风，机车闸缸不缓解的问题
					{
						if(BPPressTemp[1] > ERsetpoint_Press_int-140)//360
						{
							autoBrkCtrlTarCly_int = MIN(MAX(((workCly_int-BPPressTemp[1])*5/2+0),0),365);
							//autoBrkCtrlTarCly_int = 0;
						}
						else
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 320;
							inputClyChange_int = 0;
							inputCountNum_int = 0;
							lapCountNum_int = 0;
							outputCountNum_int = 0;
						}
					}
				}
				else                //autoBrkCtrlTarCly_int<40初充风
				{
					
					inputCountNum_int = 0;
					lapCountNum_int = 0; 
					outputCountNum_int = 0;
					if(BPChangeRate_real>-4.0)
					{
						firstBrkCountNum_int = 0;
						outputClyChange_int = 0;
						
						if(BPPressTemp[1]>=ERsetpoint_Press_int-BPdecrease_int)//485/465
							autoBrkCtrlTarCly_int = 0;
						if(BPPressTemp[1] > workCly_int)
						{
							inputWorkClyCountNum++;
							
							if(inputWorkClyCountNum == 1)
							{
								workClyInit_int = workCly_int;
							}
							else if(inputWorkClyCountNum > 7000)
							{
								inputWorkClyCountNum = 7000;
							}
							
							workCly_int = (((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1]))<ERsetpoint_Press_int?
							(((workClyInit_int+inputWorkClyCountNum /2)<BPPressTemp[1])?
							(workClyInit_int+inputWorkClyCountNum /2):(BPPressTemp[1])):ERsetpoint_Press_int;						
						}
				
					}
					else                            //初制动
					{
						firstBrkCountNum_int++;
						inputWorkClyCountNum = 0;
						if(firstBrkCountNum_int == 1)
						{
							firstBrkBPClyInit_int = BPPressTemp[0];
							simuClyBrkInit_int = autoBrkCtrlTarCly_int;
						}
						outputClyChange_int = firstBrkBPClyInit_int-BPPressTemp[1];
						if((outputClyChange_int>9) && (workCly_int>=BPPressTemp[1]))
						{
							if(SteadyCutin_B)//平稳投入
								autoBrkCtrlTarCly_int = ((workCly_int-BPPressTemp[1])*3/2<150)?(workCly_int-BPPressTemp[1])*3/2:150;
							else
							  autoBrkCtrlTarCly_int = ((((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0)<365+0)?
							(((workCly_int-BPPressTemp[1])*2+0)>0?
							((workCly_int-BPPressTemp[1])*2+0):0):365+0;
						}
						else
							autoBrkCtrlTarCly_int = simuClyBrkInit_int;
						
						if(BPPressTemp[1] < 260)
						{
							autoBrkCtrlTarCly_int = 455;
							//workCly_int = 320;
						}
					}
				}
				
			}
		}
		//12*30ms=360ms
//		memory_autoBrkCtrlTarCly[11] = memory_autoBrkCtrlTarCly[10];
//		memory_autoBrkCtrlTarCly[10] = memory_autoBrkCtrlTarCly[9];
//		memory_autoBrkCtrlTarCly[9] = memory_autoBrkCtrlTarCly[8];
//		memory_autoBrkCtrlTarCly[8] = memory_autoBrkCtrlTarCly[7];
//		memory_autoBrkCtrlTarCly[7] = memory_autoBrkCtrlTarCly[6];
//		memory_autoBrkCtrlTarCly[6] = memory_autoBrkCtrlTarCly[5];
//		memory_autoBrkCtrlTarCly[5] = memory_autoBrkCtrlTarCly[4];
//		memory_autoBrkCtrlTarCly[4] = memory_autoBrkCtrlTarCly[3];
//		memory_autoBrkCtrlTarCly[3] = memory_autoBrkCtrlTarCly[2];
//		memory_autoBrkCtrlTarCly[2] = memory_autoBrkCtrlTarCly[1];
//		memory_autoBrkCtrlTarCly[1] = memory_autoBrkCtrlTarCly[0];
//		memory_autoBrkCtrlTarCly[0] = autoBrkCtrlTarCly_int;

//	return memory_autoBrkCtrlTarCly[11];       //大闸目标压力
	return autoBrkCtrlTarCly_int;       		//大闸目标压力
}

