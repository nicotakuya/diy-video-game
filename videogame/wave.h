void wave_driver( void );
void wave_set(char tracknum,int hz);

#define SNDTRK0		0
#define SNDTRK1		1
#define SNDTRK2		2
#define SNDTRK3		3
#define SNDTRKMAX	1

void beep_ctrl(void);
void beep_set(int hz,int length);

