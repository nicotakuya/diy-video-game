#define VRAMWIDTH  40     // VRAMドット幅
//#define VRAMWIDTH  128     // VRAMドット幅
#define VRAMHEIGHT 24   // VRAMドット高さ
//#define VRAMHEIGHT 48   // VRAMドット高さ
#define VRAMXMIN  0
#define VRAMXMAX  (VRAMWIDTH-1)
#define VRAMYMIN  0
#define VRAMYMAX  (VRAMHEIGHT-1)
#define VRAMSIZE  ((VRAMWIDTH/8)*VRAMHEIGHT)
#define PAGEMAX  2
#define VRAMTOTAL   (PAGEMAX*VRAMSIZE)

extern unsigned char vram[VRAMTOTAL];	 // Bitmap Buffer

void vram_cls(void);
void vram_pset(int x,int y,char color);
void vram_line(int x1 ,int y1 ,int x2 ,int y2 ,char color);
void vram_scroll(char x1,char y1);
void vram_locate(unsigned char x,unsigned char y);
char vram_point(int x,int y);
void vram_putch(char ch);
void vram_putdec(unsigned int x);
void vram_puthex(unsigned char x);
void vram_puthexw(unsigned int x);
void vram_putstr(char *p);
void vram_putstrpgm(PGM_P p);
void vram_spput(char x,char y, PGM_P p,char size,char color);
void vram_bmpput(char x,char y,char w,char h, PGM_P p);
void vram_page(char page);
void vram_copypage(char a,char b);

