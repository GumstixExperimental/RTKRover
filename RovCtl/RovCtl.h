/********************************************
*	Author:	Keith Lee (Gumstix Gadgets)		*
*											*
*	Desc: 	Gumstix BBB Rover PWM Controller*
*	for Wheeled Robot						*
*											*
*	(C) Gumstix 2016						*
********************************************/

#ifndef __ROVCTL_H__
#define __ROVCTL_H__

#define FWD "normal"
#define BAK "inversed"
#define DEFAULT_PERIOD 50000

#include<stdio.h>
#include<dirent.h>
#define PWD "/sys/class/pwm"

typedef struct pwmStatus_t{
	char *path;
	int period;
	int duty;
	int polarity;
	int enable;}pwmStatus;


int initPwms(pwmStatus *left, pwmStatus *right);

int closePwms(pwmStatus *left, pwmStatus *right);

int getPwm(pwmStatus *pwm);

int setPwm(pwmStatus *pwm);

void fullStop(pwmStatus *left, pwmStatus *right);

void bankTurn(float ratio, char dir, pwmStatus *left, pwmStatus *right);

void hardTurn(float speed, char dir, pwmStatus *left, pwmStatus *right);

void drive(float speed, char dir, pwmStatus *left, pwmStatus *right);



#endif
