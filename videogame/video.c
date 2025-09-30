// Video control
// original program by hyoi
// http://www7.big.jp/~kerokero/avr/

// by takuya matsubara
// http://nicotak.com

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include "cpu.h"
#include "vram.h"
#include "video.h"
#include "wave.h"

SIGNAL (TIMER1_OVF_vect){
	wave_driver();
	video_driver();
}


volatile unsigned char tick=0;
volatile unsigned char tickback=0;

struct PB{
	char VID_DOT:1;		// dot
	char VID_SYNC:1;	// sync
	char AUDIO:1;		// 
	char :1;			//
	char :1;			//
	char :1;
	char :1;
	char :1;
};

#define pib (*(volatile struct PB*)&PINB)
#define pdb (*(volatile struct PB*)&DDRB)
#define pob (*(volatile struct PB*)&PORTB)


#define FREQ_H 15750	// horizontal sync frequency[Hz]


const unsigned char masktable[] ={
	0x01,
	0x02,
	0x04,
	0x08,
	0x10,
	0x20,
	0x40,
	0x80
};


unsigned int linenum=0;

#define OFFSETX (CPUFREQ/(1000000/11))	//左行の余白[clock]
#define OFFSETY 30	//余白[line]
#define DOTH 8		//1dotのheight[line/pixel]
#define VWIDTH VRAMWIDTH	//Video out width[pixel]
#define VHEIGHT	VRAMHEIGHT	//Video put height[pixel]


// 1sec/15750Hz =  63.49...usec
// (1sec/60) / (1sec/15750) = 262.5

//   1～9:垂直同期信号
//       1～3前置等価パルス
//       4～6垂直同期パルス
//       7～9後置等価パルス
//   10～21:テスト信号など

void video_driver(void)
{
	linenum++;
	if(linenum <= OFFSETY){	// line001-line021 
		if(linenum == 1){
			OCR1A = CPUFREQ / (1000000/62);		//V sync(62usec)
		}else if(linenum == 10){
			OCR1A = CPUFREQ / (1000000/4);		//H sync(4us)
		}
		return;
	}

	unsigned char row;
	row = (linenum - OFFSETY) / DOTH;

	if(row >= VHEIGHT){
		if(linenum >= 262){
			linenum = 0;
			tick++;		// 1/60sec
		}
		return;
	}

	unsigned char mask = masktable[row & 7];
	unsigned char *p = &vram[(row >> 3)* VRAMWIDTH];

	while(TCNT1L < OFFSETX);	//// 時間調整 ////

	char cnt = VWIDTH;
	while(cnt--){
		pob.VID_DOT = ((mask & *p)!=0);
		p++;
	}
	asm("nop");
	pob.VID_DOT = 0;
}


//--------------------------
void idle(void){
	sleep_mode();		//
}

//-------------------------- 1/60sec
void vsync(void)
{
	set_sleep_mode(SLEEP_MODE_IDLE);

	while(tickback == tick){
		idle();
	}
	tickback=tick;
}

//---------------------------
void video_init(void)
{
	TCNT1 = 0;
	ICR1 = (CPUFREQ / FREQ_H) - 1;	/// TOP値

	TCCR1A = (3<<COM1A0)|(2<<WGM10);
//	TCCR1A = (3<<COM1A0)|(2<<COM1B0)|(2<<WGM10);
//video
//Set   OC1A on Compare Match,and Clear OC1A at BOTTOM (inverting mode).
//  operation=Fast PWM / top=OCR1A / update=BOTTOM / flagset=TOP

	TCCR1B = (3<<WGM12)|(1<<CS10);
//audio
//Clear OC1B on Compare Match,and Set OC1B at BOTTOM (non-inverting mode).
//  operation=Fast PWM / top=ICR1 / update=BOTTOM / flagset=TOP

//Waveform Generation Mode WGM1x = 0b1110
//clock select= clk div1

	TIMSK1 |= (1<<TOIE1);	// タイマー１溢れ割込み

	pdb.VID_DOT  = 1;
	pdb.VID_SYNC = 1;
	pdb.AUDIO = 1;
}

/////////////////////////////////////////////////////

