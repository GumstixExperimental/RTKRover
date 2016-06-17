#include"RovCtl.h"



int main()
{
	char dir='l';
	int ratio;
	pwmStatus left, right;
	initPwms(&left, &right);
	printf("Pwm[0]=%s\nPwm[1]=%s\n",left.path,right.path);
	fullStop(&left,&right);
	while(!getchar());
	drive(50.0, 'f', &left, &right);
	printf("going forward");
	while(!getchar());
	printf("banking left");
	bankTurn(50.0,'l',&left,&right);
	while(!getchar());
	fullStop(&left,&right);
	closePwms(&left,&right);
	return 0;
}
