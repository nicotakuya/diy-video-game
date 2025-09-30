//--------------------------------------------------------------------
// VRAM control
//  by Takuya Matsubara
// http://www.nicotak.com/


#include <avr/io.h>
#include <avr/pgmspace.h>

#include "cpu.h"
#include "vram.h"
#include "sfont.h"	//4x6スモールフォント

unsigned char text_x=0;  // printf用カーソル位置
unsigned char text_y=0;
char text_color=1;

unsigned char vram[VRAMTOTAL];	 // VRAM Bitmap Buffer

int voffset=0;

//------------------copy vram a to b
void vram_copypage(char a,char b)
{
	int i;
	int ptra;
	int ptrb;

	ptra=(int)VRAMSIZE*a;
	ptrb=(int)VRAMSIZE*b;

	for(i=0; i<VRAMSIZE; i++){
		vram[ptrb++] = vram[ptra++];
	}
}
//-----------------select page
void vram_page(char page)
{
	if(page>=PAGEMAX) page=PAGEMAX-1;

	voffset = (int)VRAMSIZE*page;
}

//----------------clear vram
void vram_cls(void)
{
	int i;
	int ptr;

	ptr = voffset;

	for(i=0; i<VRAMSIZE; i++){
		vram[ptr++] = 0;
	}
	text_x = 0;
	text_y = 0;
}

//-------------------------------------ピクセル取得
//引数：X座標。Y座標。
char vram_point(int x,int y)
{
	unsigned char mask;
	int ptr;

	if(x<0)return(0);
	if(y<0)return(0);
	if(x>=VRAMWIDTH)return(0);
	if(y>=VRAMHEIGHT)return(0);

	mask = 1 << (y & 7);
	ptr = ((int)(y / 8)*VRAMWIDTH)+ x;
	ptr += voffset;

	return((vram[ptr] & mask)!= 0);
}

//-------------------------------ピクセル描画
//引数x：X座標。Y座標。
//引数color：カラーコード。0=消灯、1=点灯、2=XOR1
void vram_pset(int x,int y,char color)
{
	unsigned char mask;
	int ptr;

	if(x<0)return;
	if(y<0)return;
	if(x>=VRAMWIDTH)return;
	if(y>=VRAMHEIGHT)return;

	mask = 1 << (y & 7);
	ptr = ((int)(y / 8)*VRAMWIDTH)+ x;
	ptr += voffset;

	if(color==1)
		vram[ptr] |= mask;
	else if(color==0)
		vram[ptr] &= ~mask;
	else if(color==2)
		vram[ptr] ^= mask;
}

//-----------------------------------ライン描画
//引数：X1座標,Y1座標
//引数：X2座標,Y2座標
//引数color：カラーコード。0=消灯、1=点灯（赤色）、2=XOR1
void vram_line(int x1 ,int y1 ,int x2 ,int y2 ,char color)
{
	int xd;    // X2-X1座標の距離
	int yd;    // Y2-Y1座標の距離
	int xs=1;  // X方向の1pixel移動量
	int ys=1;  // Y方向の1pixel移動量
	int e;

	xd = x2 - x1;	 // X2-X1座標の距離
	if(xd < 0){
		xd = -xd;	 // X2-X1座標の絶対値
		xs = -1;	  // X方向の1pixel移動量
	}
	yd = y2 - y1;	 // Y2-Y1座標の距離
	if(yd < 0){
		yd = -yd;	 // Y2-Y1座標の絶対値
		ys = -1;	  // Y方向の1pixel移動量
	}
	vram_pset (x1, y1 ,color); //ドット描画
	e = 0;
	if( yd < xd ) {
		while( x1 != x2) {
			x1 += xs;
			e += (2 * yd);
			if(e >= xd) {
				y1 += ys;
				e -= (2 * xd);
			}
			vram_pset (x1, y1 ,color); //ドット描画
		}
	}else{
		while( y1 != y2) {
			y1 += ys;
			e += (2 * xd);
			if(e >= yd) {
				x1 += xs;
				e -= (2 * yd);
			}
			vram_pset (x1, y1 ,color); //ドット描画
		}
	}
}

//---------------------------- 画面スクロール
// ・VRAMの中身を強制的にシフトします。
// 引数x1: X方向移動量。
// 引数y1: Y方向移動量。
void vram_scroll(char x1,char y1)
{
	int x,y,xa,ya;
	int w,h;
	unsigned char c;

	if(y1<0){
		ya=-1;
		y=VRAMHEIGHT-1;
	}else{
		ya=1;
		y=0;
	}
	h=VRAMHEIGHT;
	while(h--){
		w=VRAMWIDTH;
		if(x1<0){
			xa=-1;
			x=VRAMWIDTH-1;
		}else{
			xa=1;
			x=0;
		}

		while(w--){
			c = vram_point(x+x1,y+y1);
			vram_pset(x, y ,c);	//ドット描画
			x += xa;
		}

		y += ya;
	}
}

//------------------------------- カーソル位置設定
// 引数x：X座標。一定以上だと自動改行。
// 引数y：Y座標。一定以上だと下へ画面スクロールします。
void vram_locate(unsigned char tx,unsigned char ty)
{
	text_x = tx;
	text_y = ty;
}

//------------------------------- 1キャラクタをVRAM転送
// ・4x6スモールフォント用
// 引数ch：キャラクターコード（0x00-0xff）
// 戻り値：0を返します。
void vram_putch(char ch)
{
	unsigned char tx,ty;
	unsigned char i,j;
	unsigned char bitdata;
	PGM_P p;

	if((ch==10)||(text_x > (VRAMWIDTH-4))){
		text_x = 0;
		text_y += 6;
	}
	ty=(VRAMHEIGHT-6);
	if(text_y > ty){
		vram_scroll(0,text_y-ty);
		text_y = ty;
	}
	if((unsigned char)ch < 0x20)return;

	p = (PGM_P)smallfont;
	p += ((int)((unsigned char)ch - 0x20) * 3);

	ty = text_y;
	for(i=0 ;i<6 ;i++) {
		bitdata = pgm_read_byte(p);
		if((i % 2)==0){
			bitdata >>= 4;
		}else{
			p++;
		}
		bitdata &= 0xf;
		tx = text_x;
		for(j=0;j<4;j++){
			if(bitdata & 0b1000){
				vram_pset(tx ,ty ,text_color);
			}
			bitdata <<= 1;
			tx++;
		}
		ty ++;
	}
	text_x += 4;    // カーソル移動
	if((unsigned char)ch>=0xA0)
		text_x += 1;    // カーソル移動(カタカナ用)

}


//--------------------------------------------------16進数word表示
void vram_puthexw(unsigned int x)
{
	vram_puthex(x >> 8);
	vram_puthex(x & 0xFF);
}

//--------------------------------------------------10進数表示
void vram_putdec(unsigned int x)
{
    unsigned char ch;
    unsigned int shift=10000;
	
	while(shift > 0){
		ch = (x / shift) % 10;
		ch += '0';
		vram_putch(ch);
		shift /= 10;
	}
}

//--------------------------------------------------16進数byte表示
void vram_puthex(unsigned char x)
{
    unsigned char ch;
    unsigned char shift=0x10;
	
	while(shift > 0){
		ch = (x / shift) % 0x10;
		if(ch < 10){
			ch += '0';
		}else{
			ch += ('A'-10);
		}
		vram_putch(ch);
		shift /= 0x10;
	}
}

//------------------------------------------------------文字列表示
void vram_putstr(char *p)
{
	while(*p != 0){
		vram_putch(*p++);
	}
}

//-----------------------------------------------------キャラクターを表示
void vram_spput(char x,char y, PGM_P p,char size,char color)
{
	char i,j;
	unsigned char dat;

	x -= (size/2);
	y -= (size/2);

	for(j=0; j<size; j++){
		dat = pgm_read_byte(p++);
		for(i=0; i<size; i++){
			if(dat & (1<<(size-1))){
				vram_pset(x+i,y+j,color);
			}
			dat <<= 1;
		}
	}
}
//-------------------------------------------------
void vram_bmpput(char x,char y,char w,char h, PGM_P p)
{
	char i,j,x2,y2;
	unsigned char dat=0;

	for(j=0; j<h; j++){
		for(i=0; i<w; i++){
			if((i % 8)==0){
				dat = pgm_read_byte(p++);
			}
			if(dat & 0x80){
				x2=x+i*2;
				y2=y+j*2;
				vram_pset(x2  ,y2  ,1);
				vram_pset(x2+1,y2  ,1);
				vram_pset(x2  ,y2+1,1);
				vram_pset(x2+1,y2+1,1);
			}
			dat <<= 1;
		}
	}
}
//-------------------------------------------------
void vram_putstrpgm(PGM_P p)
{
	char ch;

	while(1){
		ch = pgm_read_byte(p++);
		if(ch==0)
			break;

		vram_putch(ch);
	}
}
