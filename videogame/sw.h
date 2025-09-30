void sw_init(void);
unsigned char sw_get(void);


#define SW_B		(1<<7)
#define SW_A		(1<<6)
#define SW_RIGHT	(1<<5)
#define SW_LEFT		(1<<4)
#define SW_DOWN		(1<<3)
#define SW_UP		(1<<2)
#define SW_ALL (SW_A|SW_B|SW_RIGHT|SW_LEFT|SW_UP|SW_DOWN)
