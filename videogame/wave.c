//--------------------------------------
// wave driver
//
//original program by hyoi
// http://www7.big.jp/~kerokero/avr/

// by takuya matsubara
// http://nicotak.com

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "cpu.h"
#include "wave.h"

typedef struct{
	unsigned int FREQW;			// freq(0-ffff)
	unsigned int TWRK;			// work *dont change
	unsigned char VOLUME;		// volume(00-f0)
} ST_SND;

ST_SND wave[SNDTRKMAX];

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

#define FREQ_H 15750	// horizontal frequency[Hz]

void wave_driver(void)
{
	ST_SND *p = &wave[0];
	unsigned char amp=0;
	char count = SNDTRKMAX; //ƒ`ƒƒƒ“ƒlƒ‹”

	while(count--){
		p->TWRK += p->FREQW;	//Žü”g”ˆÊ‘Š‰ÁŽZ
		amp |= (signed char)(p->TWRK >> 15);
		p++;
	}
	pob.AUDIO = amp;
}

//--------------------------------------
void wave_set(char tracknum,int hz)
{
	ST_SND *p;

	p = &wave[(int)tracknum];

	p->FREQW = ((unsigned long)hz * 15750L)>> 11;
}

//--------------------------------------
int beepcnt=0;
int beephz=0;

void beep_ctrl(void)
{
	if(beepcnt){
		wave_set(SNDTRK0,beephz-(beepcnt*10));
		beepcnt--;
		if(beepcnt==0)
			wave_set(SNDTRK0,0);
	}
}

//--------------------------------------
void beep_set(int hz,int length)
{
	beephz = hz;
	beepcnt = length;
}

