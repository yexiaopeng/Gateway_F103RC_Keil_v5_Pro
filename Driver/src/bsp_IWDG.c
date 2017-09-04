#include "bsp_IWDG"

/*
@函数名：IWDG_Init
@函数功能：初始化配置IWDG看门狗
@输入参数：
		 Tout = prv/40 * rlv (s) prv可以是[4,8,16,32,64,128,256]
		 *            prv:预分频器值，取值如下：
	  	参数 IWDG_Prescaler_4: IWDG prescaler set to 4
	  	参数 IWDG_Prescaler_8: IWDG prescaler set to 8
	  	参数 IWDG_Prescaler_16: IWDG prescaler set to 16
	  	参数 IWDG_Prescaler_32: IWDG prescaler set to 32
	  	参数 IWDG_Prescaler_64: IWDG prescaler set to 64
	  	参数 IWDG_Prescaler_128: IWDG prescaler set to 128
	  	参数 IWDG_Prescaler_256: IWDG prescaler set to 256
		rlv:预分频器值，取值范围为：0-0XFFF
*/
extern void  IWDG_Init(u8 prv ,u16 rlv){
	
}
/*
@函数名：IWDG_Feed
@函数功能：喂狗
*/
extern void IWDG_Feed(void){
	
}





