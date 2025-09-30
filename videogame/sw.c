// switch control
// by Takuya Matsubara 
// http://nicotak.com

#include <avr/io.h>
#include "sw.h"

#define SW_PORT	PORTD
#define SW_DDR DDRD
#define SW_PIN	PIND


//-----------------------------------------------------------スイッチ初期化
//   引数:初期化するポートのマスク
void sw_init(void)
{
	SW_DDR &= ~SW_ALL;	//input
	SW_PORT |= SW_ALL;	//pullup
}

//-----------------------------------------------------------input switch
unsigned char sw_get(void)
{
	return(~SW_PIN);
}


