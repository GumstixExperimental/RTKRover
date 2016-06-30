/********************************************
*	Author:	Keith Lee (Gumstix Gadgets)		*
*											*
*	Desc: 	Gumstix BBB Rover PWM Controller*
*	for Wheeled Robot						*
*											*
*	(C) Gumstix 2016						*
********************************************/

#include "RovCtl.h"
#include<dirent.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#define DEBUG 1



int xport_pwms(DIR *d, char* path, pwmStatus *left, pwmStatus *right)
{
	int count=0;
	int pwmCount=0;
	int npwm;
	int seek=1;
	struct dirent *entry;
	char *fname;
	FILE *fp;
	char c_npwm[5];
	while((entry=readdir(d))!=NULL)
	{
		
		if(!strcmp(entry->d_name,"npwm"))
		{
			fname=(char*)malloc((strlen(path)+strlen(entry->d_name)+1)*sizeof(char));
			sprintf(fname,"%s/%s",path,entry->d_name);
			if(!(fp=fopen(fname,"r")))
				return 0;
			fscanf(fp,"%s",c_npwm);
			npwm=atoi(c_npwm);
			seek=0;
			fclose(fp);
			free(fname);
			break;
		}
	}
	if(seek)
		return 0;
	fname=(char*)malloc((strlen(path)+strlen("export")+1)*sizeof(char));
	sprintf(fname, "%s/export",path);
	for(count=0;count<npwm;count++)
	{
		if(!(fp=fopen(fname,"w")))
			break;
			
		fprintf(fp,"%d",count);
		fclose(fp);
	}
	free(fname);
	rewinddir(d);
	while((entry=readdir(d))!=NULL)
	{
		if(strstr(entry->d_name, "pwm")==entry->d_name)
		{
			if(pwmCount==0)
			{
				left->path=(char*)malloc((strlen(path)+strlen(entry->d_name))*sizeof(char));
				sprintf(left->path,"%s/%s",path,entry->d_name);
			}
			if(pwmCount==1)
			{
				right->path=(char*)malloc((strlen(path)+strlen(entry->d_name))*sizeof(char));
				sprintf(right->path,"%s/%s",path,entry->d_name);
			}
			pwmCount++;
		}			
	}
	#if DEBUG
	fprintf(stderr,"total pwms:%d\n",pwmCount);
	#endif
	left->period=DEFAULT_PERIOD;
	right->period=DEFAULT_PERIOD;
	left->duty=DEFAULT_PERIOD/2;
	right->duty=DEFAULT_PERIOD/2;
	left->enable=1;
	right->enable=1;

	setPwm(left);
	setPwm(right);
	return count;
}





int initPwms(pwmStatus *left, pwmStatus *right)
{
	int pwmCount=0;
	DIR *d;
	DIR *subd;
	DIR *subsubd;
	struct dirent *entry;
	char *path;
	char LnkPath[2048];
	ssize_t pathLen;
	pwmCount=0;

	d=opendir(PWD);
	if(d)
	{
		while((entry=readdir(d))!=NULL && pwmCount < 2)
		{
			if(entry->d_type==DT_LNK)
			{
				char *p;
				path=(char*)malloc((strlen(PWD)+strlen(entry->d_name))*sizeof(char));
				sprintf(path,"%s/%s",PWD,entry->d_name);
				pathLen=readlink(path,LnkPath,2048);
				p=strstr(LnkPath,"pwmchip");
				*(p+9)=0;
				sprintf(LnkPath,"%s%s","/sys",(char*)(LnkPath+5));
				if((subd=opendir(LnkPath)))
				{
					if(pwmCount==0)
					{
						if(!xport_pwms(subd, LnkPath, left, right))
						#if DEBUG
							fprintf(stderr, "Failed to export PWMs from %s\n",entry->d_name);
						#endif
						pwmCount++;
					}
				}
			}
		}
	}
	closedir(d);
	return 0;
}


int setPwm(pwmStatus *pwm)
{
	FILE *fp;
	char *path=(char*)malloc((strlen(pwm->path)+16)*sizeof(char));
	if(!pwm->enable)
	{
		sprintf(path,"%s/enable",pwm->path);
		fp=fopen(path,"w");
		if(!fp)
			return 1;
		fprintf(fp,"%d",0);
		fclose(fp);
		
	}
	
	sprintf(path,"%s/period",pwm->path);
	fp=fopen(path,"w");
	if(!fp)
		return 2;
	fprintf(fp,"%d",pwm->period);
	fclose(fp);
	
	sprintf(path,"%s/duty_cycle",pwm->path);
	fp=fopen(path, "w");
	if(!fp)
		return 3;
	fprintf(fp, "%d", pwm->duty);
	fclose(fp);
	
	if(pwm->enable)
	{
		sprintf(path,"%s/enable",pwm->path);
		fp=fopen(path, "w");
		if(!fp)
			return 1;
		fprintf(fp, "%d", 1);
		fclose(fp);
	}
	return 0;
}


int getPwm(pwmStatus *pwm)
{
	FILE *fp;
	char *path=(char*)malloc((strlen(pwm->path)+16)*sizeof(char));

	sprintf(path,"%s/enable",pwm->path);
	fp=fopen(path,"r");
	if(!fp)
		return 1;
	fscanf(fp,"%d",pwm->enable);
	fclose(fp);

	sprintf(path,"%s/period",pwm->path);
	fopen(path,"r");
	if(!fp)
		return 2;
	fscanf(fp,"%d",pwm->period);
	fclose(fp);
	
	sprintf(path,"%s/duty_cycle",pwm->path);
	fopen(path, "r");
	if(!fp)
		return 3;
	fscanf(fp, "%d", pwm->duty);
	fclose(fp);
	
	
	return 0;
}

void normalize_period(pwmStatus *a, pwmStatus *b)
{
	if(a->period!=b->period)
	{
		if(a->period>b->period)
			a->period=b->period;
		else
			b->period=a->period;
	}
}

void fullStop(pwmStatus *left, pwmStatus *right)
{
	normalize_period(left, right);
	left->duty=left->period/2;
	right->duty=right->period/2;
	
	setPwm(left);
	setPwm(right);
}

void bankTurn(float ratio, char dir, pwmStatus *left, pwmStatus *right)
{
	float abs_l,abs_r,curr_spd;
	short motor_dir;
	
	normalize_period(left, right);
	
	abs_l=left->duty-left->period/2;
	abs_r=right->duty-right->period/2;
	
	if(abs_l>0 && abs_r>0)
	{
		motor_dir=-1;
		if(abs_l<abs_r)
			curr_spd=abs_l;
		else curr_spd=abs_r;
	}
	else if(abs_l<0 && abs_r<0)
	{
		motor_dir=1;
		if(abs_l<abs_r)
			curr_spd=abs_r;
		else curr_spd=abs_l;
	}
	else return;
		
	if(dir=='r' && ratio!=0.0)
	{
		left->duty=right->period/2+(motor_dir*curr_spd*(1-(ratio/2)/100));
		right->duty=right->period/2+(motor_dir*curr_spd*(1+(ratio/2)/100));
	}
	else if(dir=='l' && ratio!=0)
	{
		right->duty=left->period/2+(motor_dir*curr_spd*(1-(ratio/2)/100));
		left->duty=left->period/2+(motor_dir*curr_spd*(1+(ratio/2)/100));
	}
	setPwm(left);
	setPwm(right);
}


	
void hardTurn(float speed, char dir, pwmStatus *left, pwmStatus *right)
{
	normalize_period(left, right);
	
	if(dir!='l' && dir!='r')
		return;
	
	pwmStatus *fwd, *bak;
	if(dir=='l')
	{
		fwd=right;
		bak=left;
	}
	else if(dir=='r')
	{
		fwd=left;
		bak=right;
	}
	else return;
	
	fullStop(fwd,bak);
	
	
	fwd->enable=1;
	bak->enable=1;
	fwd->duty=(int)((float)fwd->period/2*(1.0+(speed/100.0)));
	bak->duty=(int)((float)fwd->period/2*(1.0-(speed/100.0)));
	setPwm(fwd);
	setPwm(bak);	
}


void drive(float speed, char dir, pwmStatus *left, pwmStatus *right)
{
	normalize_period(left,right);
	short n_dir;
	if(dir=='f')
		n_dir=-1;
	else if(dir=='b')
		n_dir=1;
	else
		return;
	
	left->duty=right->duty=(int)((float)left->period/2*(1.0+(n_dir*(speed/100.0))));
	left->enable=1;
	right->enable=1;
	setPwm(left);
	setPwm(right);
}

int closePwms(pwmStatus *left, pwmStatus *right)
{
	FILE* fp;
	char *a;
	char *path=(char*)malloc((strlen(left->path)+10)*sizeof(char));
	fullStop(left,right);
	sprintf(path,"%s",left->path);
	a=strstr(path, "pwm0");
		
	if(!a)
	{
		a=strstr(path,"pwm1");
	}
	if(!a)
	{
	#if DEBUG
		printf("failed to find path\n");
	#endif	
		return 1;
	}
	sprintf(a, "unexport");
#if DEBUG
	printf("%s\n",path);
#endif
	fp=fopen(path,"w");
	if(!fp)
		return 1;
	fprintf(fp,"0");
	fclose(fp);
	fp=fopen(path,"w");
	if(!fp)
		return 1;
	fprintf(fp,"1");
	fclose(fp);
	return 0;
}
