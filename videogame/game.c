//---------------------------------------------------------------------
// space fight for ATmega168
// by Takuya Matsubara
// http://www.nicotak.com/
//
//---------------------------------------------------------------------

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "cpu.h"
#include "video.h"
#include "vram.h"
#include "sw.h"
#include "rand.h"
#include "game.h"
#include "wave.h"
//#include "lcd.h"

#define TEKIMAX (3*6)
#define DOTSIZE 3

const unsigned char pat[][DOTSIZE] PROGMEM = {
	{
		0b010,
		0b111,
		0b111	//player
	},{
		0b010,
		0b111,
		0b101	//teki1
	},{
		0b010,
		0b111,
		0b010	//teki2
	},{
		0b101,
		0b000,
		0b101	//beam3
	},{
		0b111,
		0b101,
		0b111	//boom2
	},{
		0b010,
		0b111,
		0b010	//boom1
	},{
		0b010,
		0b111,
		0b000	//UFO
	}
};

const unsigned char strready[] PROGMEM ="READY";
const unsigned char strover[] PROGMEM ="GAMEOVER";
const unsigned char strspfight[] PROGMEM ="SPACEFIGHT";

//-------------------------------------------------
int fnc_abs(int a){
	if(a<0)a = -a;
	return (a);
}

//-------------------------------------------------
int fnc_sgn(int a){
	if(a<0)return(-1);
	return (1);
}

//--------------------------
void vsyncs(int i)
{
	while(i--){
		vsync();
//		lcd_control();
	}
}

//---------------------------------------------------
// メイン処理
void game(void)
{
#define MOVEPITCH 50	//移動カウント
#define MOVEYOKO (VRAMWIDTH/4)
#define MOVETATE 1
#define MOVESEQ   (MOVEYOKO*2+MOVETATE*2)	//移動シーケンス
#define SLDY 19		//shield y

	signed char tx[TEKIMAX];	//敵座標
	signed char ty[TEKIMAX];
	char tb[TEKIMAX];
	unsigned char playcnt;	//Enemy Timing
	signed char tex1,tey1;	//Enemy
	char turn=0;	//Enemy
	unsigned char tekiani=0;
	char speed=0;	//game speed
	int score=0;	//Score
	char tnum;	//Enemy Count
	signed char ax;
	signed char ay;	//My Ship X,Y
	signed char bx;
	signed char by;	//My Ship Beam
	signed char cx;
	signed char cy;	//Enemy Beam
	signed char x,y;
	unsigned char btn;
	int i;
	char overflag=1;
	unsigned char shield[VRAMWIDTH];

	while(1){
		if(overflag){
			vram_cls();
			vram_locate(0,5);
			vram_putstrpgm((PGM_P)strspfight);	// title
			vsyncs(180);
//			while(1){
//				btn = sw_get();
//				if(btn & (SW_A | SW_B))break;
//				vsyncs(2);
//			}
			overflag=0;
			score=0;
			speed=MOVEPITCH;
			ax=VRAMWIDTH/2;	//プレーヤ座標
			ay=VRAMHEIGHT-2;

			vram_cls();
			vram_locate((VRAMWIDTH/2)-(5*4/2),5);
			vram_putstrpgm((PGM_P)strready);
			vsyncs(90);
		}
		bx=-1;
		by=-1;		//自弾座標
		cx=-1;
		cy=-1;		//敵弾座標

		for(i=0;i<TEKIMAX;i++){
			tx[i] = ((i % 6)*5)+2;
			ty[i] = ((i / 6)*4)+2;
			tb[i] = 0;
		}

		tex1 = 1;
		tey1 = 0;
		playcnt = 0;
		for(i=0;i<VRAMWIDTH;i++){
			shield[i] = ((i % 6)/3);
		}
		//---------main loop
		while(overflag==0){
			beep_ctrl();
			vram_page(1);	// page behind
			vram_cls();
			btn = sw_get();
			if(btn & SW_RIGHT)ax++;
			if(btn & SW_LEFT )ax--;
			if(ax<2)ax=2;
			if(ax>(VRAMXMAX-2))ax=VRAMXMAX-2;
			vram_spput(ax, ay,(PGM_P)pat[0],DOTSIZE,1);		//------自機をvramに転送

			if(by < 0){
				if(btn & (SW_A | SW_B)){
					bx=ax;
					by=ay;
					beep_set(300,5);
				}
			}else{
				by--;
				vram_pset(bx,by,1);

				if(SLDY==by){
					if(shield[bx]){
						beep_set(500,7);
						by = -1;
						shield[bx] = 0;
					}
				}
			}

			playcnt++;
			if(playcnt>=speed)playcnt=0;

			//Enemy move
			tnum=0;
			for(i=0 ;i<TEKIMAX ;i++) {
				x=tx[i];
				y=ty[i];
				if(y == -1)continue;
				tnum++;

				if(tb[i]){	//bakuhatsu
					tb[i]--;
					vram_spput(x,y,(PGM_P)pat[3+((tb[i]/3)%3)],DOTSIZE,1);
					if(tb[i]==0)
						ty[i]=-1;
					continue;
				}
				if((fnc_abs(by-y)<DOTSIZE-1)&&(fnc_abs(bx-x)<DOTSIZE-1)){ //命中
					if(score < 9999)score++;
					if(speed > 3)speed--;
					tb[i]=15;
					by=-1;
					beep_set(700,10);
					continue;
				}
				if(playcnt ==0){		//move
					if(tey1>0){
						y=y+tey1;
					}else{
						x=x+tex1;
						if((tex1<0)&&(x<=2))turn=1;
						if((tex1>0)&&(x>=(VRAMXMAX-2)))turn=1;
					}
					if(y >= ay){	//touch down
						overflag=1;
					}
				}
				tx[i]=x;
				ty[i]=y;
				vram_spput(x,y ,(PGM_P)pat[1+(tekiani & 1)],DOTSIZE,1);
				continue;
			}
			if(playcnt==0){		//move
				tekiani++;
				if(tey1>0)tey1=0;		
				if(turn){
					turn=0;
					tex1=-tex1;
					tey1=1;
				}
			}
			
			//--------------------Enemy Beam
			if(cy < 0){
				i=rand_get(TEKIMAX);
				if((tb[i]==0)&&(ty[i] != -1)){
					cx = tx[i];
					cy = ty[i];
				}
			}else{
				if (playcnt & 1){
					cy++;
				}
				vram_pset(cx,cy,1);
				if(cy > VRAMHEIGHT){
					cy = -1;
				}
				if((fnc_abs(cy-ay)<DOTSIZE-1)&&(fnc_abs(cx-ax)<DOTSIZE-1)){
					overflag=1;
				}
				if(SLDY==cy){
					if(shield[cx]){
						beep_set(500,7);
						cy = -1;
						shield[cx] = 0;
					}
				}
			}
			//---------------shield
			for(i=0;i<VRAMWIDTH;i++){
				if(shield[i])
					vram_pset(i,SLDY,1);
			}
			vram_copypage(1,0);	//page1-->page0
			vsyncs(2);
			if(tnum==0){	//敵全滅
				speed+=8;
				break;
			}
		}
		if(overflag){	//game over
			beep_set(600,15);
			vram_page(0);
			i=60;
			while(i--){
				vram_cls();
				vram_spput(ax,ay,(PGM_P)pat[3+((i/3)%3)],DOTSIZE,1);
				beep_ctrl();
				vsyncs(2);
			}
			vram_cls();
			vram_locate((VRAMWIDTH/2)-(8*4/2),1);
			vram_putstrpgm((PGM_P)strover);
			vram_locate((VRAMWIDTH/2)-(5*4/2),8);
			vram_putdec(score);
			i=120;
			while(i--){
				beep_ctrl();
				vsyncs(2);
			}
			break;
		}
	}
}

