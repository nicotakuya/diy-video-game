// video game
// by takuya matsubara

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "timer.h"
#include "vram.h"
#include "video.h"
#include "rand.h"
#include "game.h"
#include "sw.h"

int main(void)
{
	timer_init();
	video_init();
	sw_init();

	sei();

	while(1){
		game();
	}
}
