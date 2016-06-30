#include"RovCtl.h"
#include<stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
 
#define BANK_RATIO 65.0
#define SPEED 65.0

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
 
  ch = getchar();
 
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);
 
  if(ch != EOF)
  {
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}
 

int main()
{
	char dir='f', key,mode;
	int ratio;
	int period;
	float speed=SPEED;
	pwmStatus left, right;
	initPwms(&left, &right);
	printf("Pwm[0]=%s\nPwm[1]=%s\n",left.path,right.path);
	fullStop(&left,&right);
	
	while(key!='x')
	{
		if(kbhit()||key=='r'||key=='R'||key=='f'||key=='F')
		{
			if(key=='r'||key=='R'||key=='f'||key=='F')
				key=mode;
			else
			{
				printf("key hit\n");
				key=getchar();
			}
			
			
			switch(key)
			{
			case 'w':
			case 'W':
				dir='f';
				drive(speed, 'f', &left, &right);
				mode='w';
				break;
			case 'a':
			case 'A':
				hardTurn(speed, 'l', &left, &right);
				mode='a';
				break;
			case 's':
			case 'S':
				dir='b';
				drive(speed,'b', &left, &right);
				mode='s';
				break;
			case 'd':
			case 'D':
				hardTurn(speed, 'r', &left, &right);
				mode='d';
				break;
			case ' ':
				fullStop(&left, &right);
				speed=SPEED;
				break;
			case 'r':
			case 'R':
				if(speed<=97.5)
					speed+=2.5;
				break;
			case 'f':
			case 'F':
				if(speed>=2.5)
					speed-=2.5;
				break;
			case 'q':
			case 'Q':
				drive(speed, dir, &left, &right);
				bankTurn(BANK_RATIO,'l',&left,&right);
				mode='q';
				break;
			case 'e':
			case 'E':
				drive(speed, dir, &left, &right);
				bankTurn(BANK_RATIO,'r',&left,&right);
				mode='e';
				break;
			default:
				break;
			}
			
		}
	}
	printf("Loop Exit\n");
	
	#if 0
	while(!getchar());
	drive(50.0, 'f', &left, &right);
	printf("going forward");
	while(!getchar());
	printf("banking left");
	bankTurn(50.0,'l',&left,&right);
	while(!getchar());
	fullStop(&left,&right);
	period=50000;
	left.period=right.period=period;
	
	
	for(ratio=1;ratio<100;ratio++)
	{
		printf("ratio=%d\n",ratio);
		left.duty=right.duty=period*(1.0-(float)ratio/100.0);
		printf("%d:Duty=%d\n-----------------\n",left.enable,left.duty);
		setPwm(&left);
		setPwm(&right);
		while(!getchar());
	}
	#endif
	
	closePwms(&left,&right);
	
	return 0;
}
