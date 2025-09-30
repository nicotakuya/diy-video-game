//-----------------------------------------------------------
// TIMER CONTROL
// by takuya matsubara
// http://nicotak.com

#include <avr/io.h>
#include <avr/interrupt.h>
#include "cpu.h"
#include "timer.h"

#define TMR_PS   8        //プリスケーラ値(分周比)
#define TMR_PSCR 2        //プリスケーラ設定値

//-----------------------------------------------------------
// タイマ初期化
void timer_init(void)
{
	TCCR0A = 0;	// Normal port operation
	TCCR0B = TMR_PSCR;	// Clock select(div8)
}

//-----------------------------------------------------------
//　タイマウエイト
//引数:ウエイト時間 1～255[usec]
void timer_waitusec(unsigned char usec)
{
    TCNT0 = (unsigned char)(0x100-(usec*(CPUFREQ/8000000)));
    TIFR0 |= (1 << TOV0);  // TOVxビットを0クリア

    while(!(TIFR0 & (1 << TOV0)));
    // TOVxビットが1になるまで
}

//-----------------------------------------------------------
//　タイマウエイト
//引数：ウエイト時間　1～6553[msec]
void timer_waitmsec(unsigned int i)
{
	i*=10;

	while(i--){
		timer_waitusec(100);
	}
}

