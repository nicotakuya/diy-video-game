// random
// by takuya matsubara
// http://nicotak.com

#include <avr/io.h>

unsigned char rand_temp=0;

void rand_init(void)
{
    TCCR0A= 0;    // �^�C�} ���[�h 
    TCCR0B= 1;    // �^�C�} �v���X�P�[�� 
}



unsigned char rand_get(unsigned char max)
{
	rand_temp += rand_temp;
	rand_temp ++;
	rand_temp += TCNT0;

	return(rand_temp % max);  // �^�C�} �J�E���g�𗐐��ɂ��܂�
}
