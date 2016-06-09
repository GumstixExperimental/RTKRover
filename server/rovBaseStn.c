/*------------------------------------------------------------------------------
* str2str.c : console version of stream server
*
*          Copyright (C) 2007-2015 by T.TAKASU, All rights reserved.
*
* version : $Revision: 1.1 $ $Date: 2008/07/17 21:54:53 $
* history : 2009/06/17  1.0  new
*           2011/05/29  1.1  add -f, -l and -x option
*           2011/11/29  1.2  fix bug on recognize ntrips:// (rtklib_2.4.1_p4)
*           2012/12/25  1.3  add format conversion functions
*                            add -msg, -opt and -sta options
*                            modify -p option
*           2013/01/25  1.4  fix bug on showing message
*           2014/02/21  1.5  ignore SIG_HUP
*           2014/08/10  1.5  fix bug on showing message
*           2014/08/26  1.6  support input format gw10, binex and rt17
*           2014/10/14  1.7  use stdin or stdout if option -in or -out omitted
*           2014/11/08  1.8  add option -a, -i and -o
*           2015/03/23  1.9  fix bug on parsing of command line options
*-----------------------------------------------------------------------------*/
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include "rtklib.h"
#include "sockets.h"
#include "csvparser.h"
#include "xtdio.h"
#include <sys/time.h>


static const char rcsid[]="$Id:$";

#define PRGNAME     "str2str"          /* program name */
#define MAXSTR      5                  /* max number of streams */
#define MAXRCVCMD   4096               /* max length of receiver command */
#define TRFILE      "str2str.trace"    /* trace file */

#define MAXSOCKNAME 128
#define DEBUG 1

/* global variables ----------------------------------------------------------*/
static strsvr_t strsvr;                /* stream server */
static volatile int intrflg=0;         /* interrupt flag */

//RTKROV
Socket *sockcli;
char svrname[MAXSOCKNAME];
FILE *nodes_p, *log_p;
float **nodes;
int curr_node;
typedef struct rovMsg_t{
	char msgType;
	float lat;
	float lon;
	float hdg;
	float angle;
	int errorType;
	}rovMsg;



/* help text -----------------------------------------------------------------*/
static const char *help[]={
"",
" usage: str2str [-in stream] [-out stream [-out stream...]] [options]",
"",
" Input data from a stream and divide and output them to multiple streams",
" The input stream can be serial, tcp client, tcp server, ntrip client, or",
" file. The output stream can be serial, tcp client, tcp server, ntrip server,",
" or file. str2str is a resident type application. To stop it, type ctr-c in",
" console if run foreground or send signal SIGINT for background process.",
" if run foreground or send signal SIGINT for background process.",
" if both of the input stream and the output stream follow #format, the",
" format of input messages are converted to output. To specify the output",
" messages, use -msg option. If the option -in or -out omitted, stdin for",
" input or stdout for output is used.",
" Command options are as follows.",
"",
" -in  stream[#format] input  stream path and format",
" -out stream[#format] output stream path and format",
"",
"  stream path",
"    serial       : serial://port[:brate[:bsize[:parity[:stopb[:fctr]]]]]",
"    tcp server   : tcpsvr://:port",
"    tcp client   : tcpcli://addr[:port]",
"    ntrip client : ntrip://[user[:passwd]@]addr[:port][/mntpnt]",
"    ntrip server : ntrips://[:passwd@]addr[:port][/mntpnt[:str]] (only out)",
"    file         : [file://]path[::T][::+start][::xseppd][::S=swap]",
"",
"  format",
"    rtcm2        : RTCM 2 (only in)",
"    rtcm3        : RTCM 3",
"    nov          : NovAtel OEMV/4/6,OEMStar (only in)",
"    oem3         : NovAtel OEM3 (only in)",
"    ubx          : ublox LEA-4T/5T/6T (only in)",
"    ss2          : NovAtel Superstar II (only in)",
"    hemis        : Hemisphere Eclipse/Crescent (only in)",
"    stq          : SkyTraq S1315F (only in)",
"    gw10         : Furuno GW10 (only in)",
"    javad        : Javad (only in)",
"    nvs          : NVS BINR (only in)",
"    binex        : BINEX (only in)",
"    rt17         : Trimble RT17 (only in)",
"",
" -msg \"type[(tint)][,type[(tint)]...]\"",
"                   rtcm message types and output intervals (s)",
" -sta sta          station id",
" -opt opt          receiver dependent options",
" -s  msec          timeout time (ms) [10000]",
" -r  msec          reconnect interval (ms) [10000]",
" -n  msec          nmea request cycle (m) [0]",
" -f  sec           file swap margin (s) [30]",
" -c  file          receiver commands file [no]",
" -p  lat lon hgt   station position (latitude/longitude/height) (deg,m)",
" -a  antinfo       antenna info (separated by ,)",
" -i  rcvinfo       receiver info (separated by ,)",
" -o  e n u         antenna offst (e,n,u) (m)",
" -l  local_dir     ftp/http local directory []",
" -x  proxy_addr    http/ntrip proxy address [no]",
" -t  level         trace level [0]",
//RTKROV
" -sn  name host	Server socket name",
" -nf  filename		GPS node list file",
" -lf  filename		RTKRover log output file",

" -h                print help",
};


/* print help ----------------------------------------------------------------*/
static void printhelp(void)
{
    int i;
    for (i=0;i<sizeof(help)/sizeof(*help);i++) fprintf(stderr,"%s\n",help[i]);
    exit(0);
}
/* signal handler ------------------------------------------------------------*/
static void sigfunc(int sig)
{
    intrflg=1;
}
/* decode format -------------------------------------------------------------*/
static void decodefmt(char *path, int *fmt)
{
    char *p;
    
    *fmt=-1;
    
    if ((p=strrchr(path,'#'))) {
        if      (!strcmp(p,"#rtcm2")) *fmt=STRFMT_RTCM2;
        else if (!strcmp(p,"#rtcm3")) *fmt=STRFMT_RTCM3;
        else if (!strcmp(p,"#nov"  )) *fmt=STRFMT_OEM4;
        else if (!strcmp(p,"#oem3" )) *fmt=STRFMT_OEM3;
        else if (!strcmp(p,"#ubx"  )) *fmt=STRFMT_UBX;
        else if (!strcmp(p,"#ss2"  )) *fmt=STRFMT_SS2;
        else if (!strcmp(p,"#hemis")) *fmt=STRFMT_CRES;
        else if (!strcmp(p,"#stq"  )) *fmt=STRFMT_STQ;
        else if (!strcmp(p,"#gw10" )) *fmt=STRFMT_GW10;
        else if (!strcmp(p,"#javad")) *fmt=STRFMT_JAVAD;
        else if (!strcmp(p,"#nvs"  )) *fmt=STRFMT_NVS;
        else if (!strcmp(p,"#binex")) *fmt=STRFMT_BINEX;
        else if (!strcmp(p,"#rt17" )) *fmt=STRFMT_RT17;
        else return;
        *p='\0';
    }
}
/* decode stream path --------------------------------------------------------*/
static int decodepath(const char *path, int *type, char *strpath, int *fmt)
{
    char buff[1024],*p;
    
    strcpy(buff,path);
    
    /* decode format */
    decodefmt(buff,fmt);
    
    /* decode type */
    if (!(p=strstr(buff,"://"))) {
        strcpy(strpath,buff);
        *type=STR_FILE;
        return 1;
    }
    if      (!strncmp(path,"serial",6)) *type=STR_SERIAL;
    else if (!strncmp(path,"tcpsvr",6)) *type=STR_TCPSVR;
    else if (!strncmp(path,"tcpcli",6)) *type=STR_TCPCLI;
    else if (!strncmp(path,"ntrips",6)) *type=STR_NTRIPSVR;
    else if (!strncmp(path,"ntrip", 5)) *type=STR_NTRIPCLI;
    else if (!strncmp(path,"file",  4)) *type=STR_FILE;
    else {
        fprintf(stderr,"stream path error: %s\n",buff);
        return 0;
    }
    strcpy(strpath,p+3);
    return 1;
}
/* read receiver commands ----------------------------------------------------*/
static void readcmd(const char *file, char *cmd, int type)
{
    FILE *fp;
    char buff[MAXSTR],*p=cmd;
    int i=0;
    
    *p='\0';
    
    if (!(fp=fopen(file,"r"))) return;
    
    while (fgets(buff,sizeof(buff),fp)) {
        if (*buff=='@') i=1;
        else if (i==type&&p+strlen(buff)+1<cmd+MAXRCVCMD) {
            p+=sprintf(p,"%s",buff);
        }
    }
    fclose(fp);
}

//RTKROV
/* getNodes ------------------------------------------------------------------*/
int getNodes(char *filename)
{
	int n_nodes=0;
	
	char buf[1024];
	
	
	while(!feof(nodes_p))
	{
		if(fgetc(nodes_p)=='\n')
			n_nodes++;
	}
	rewind(nodes_p);
	fprintf(stderr,"%d nodes\n",n_nodes);
	
	nodes = (float**)malloc(n_nodes*sizeof(float*));
	int i;
	for(i=0;i<n_nodes;i++)
		nodes[i]=(float*)malloc(2*sizeof(float));
	
	CsvParser *csvparser = CsvParser_new(filename, ",", 0);
	CsvRow *row;
	i=0;
	while(row = CsvParser_getRow(csvparser))
	{
		const char **rowFields = CsvParser_getFields(row);
		if(CsvParser_getNumFields(row)==2)
		{
			nodes[i][0]=atof(rowFields[0]);
			nodes[i][1]=atof(rowFields[1]);
			i++;
		}
		else n_nodes--;
	}
#if DEBUG
	for(i=0;i<n_nodes;i++)
	{
		fprintf(stderr, "%f, %f\n",nodes[i][0],nodes[i][1]);
	}
#endif
	
	return n_nodes;	
}

/* rovComm -------------------------------------------------------------------*/
int rovComm()
{
	rovMsg rmsg;
	rovMsg imsg;
	char buf[1024];
	char *tokens[12];
	int i;
	int j;
	for(i=0;i<12;i++)
		tokens[i]=NULL;
		
	Smasktime(0L,1L);
	Smaskset(sockcli);
	rmsg.msgType='x';
	rmsg.lat=0.0;
	rmsg.lon=0.0;
	rmsg.hdg=0.0;
	rmsg.angle=0.0;
	rmsg.errorType=0;
	
	imsg.msgType='x';
	imsg.lat=0.0;
	imsg.lon=0.0;
	imsg.hdg=0.0;
	imsg.angle=0.0;
	imsg.errorType=0;
	
	if(Smaskwait())
	{
		Sscanf(sockcli,"%s",buf);
			fprintf(stderr, "%s\n", buf);
		for(i=0;i<12;i++)
		{
			if(i==0)tokens[i]=strtok(buf, ":");
			else tokens[i]=strtok(0,":");
			if(tokens[i]==NULL)break;
			#if DEBUG
			fprintf(stderr,"%s\n",tokens[i]);
			#endif
		}
		printf("%d\n",i);
		
		for(j=0;j<i;j++)
		{
			if(*tokens[j]=='t')
			{
				printf("%c is the value\n", tokens[j+1][0]);
				rmsg.msgType=tokens[++j][0];
				
			}
			else if(*tokens[j]=='a')
			{
			#ifdef DEBUG
				printf("lat=%s\n",tokens[j+1]);
			#endif
				rmsg.lat=atof(tokens[++j]);
			}
			else if(*tokens[j]=='o')
				rmsg.lon=atof(tokens[++j]);
			else if(*tokens[j]=='h')
				rmsg.hdg=atof(tokens[++j]);
			else if(tokens[j]=="n")
				rmsg.angle=atof(tokens[++j]);
			else if(tokens[j]=="e")
				rmsg.errorType=atoi(tokens[++j]);
			
			
		}
		#if DEBUG
		fprintf(stderr, "lat=%f\nlon=%f\nhdg=%f\n",rmsg.lat, rmsg.lon, rmsg.hdg);
		fprintf(stderr, "into switch.  MSG TYPE=%c\n",rmsg.msgType);
		#endif
		switch(rmsg.msgType)
			{
			case 'i': //idle
				
				break;
			case 'y': //confirm
				
				break;
			case 'w': //waypoint
			
				break;
			case 'e': //error
				
				break;
			default:
				
				break;
			}
	}
	

	return 0;
	
}



/* str2str -------------------------------------------------------------------*/
int main(int argc, char **argv)
{
//RTKROV
	curr_node=0;
	sockcli=NULL;
	log_p=0;
	nodes_p=0;
	struct timeval t1,t2;
	double elapsedTime;
	
    static char cmd[MAXRCVCMD]="";
    const char ss[]={'E','-','W','C','C'};
    strconv_t *conv[MAXSTR]={NULL};
    double pos[3],stapos[3]={0},off[3]={0};
    char *paths[MAXSTR],s[MAXSTR][MAXSTRPATH]={{0}},*cmdfile="";
    char *local="",*proxy="",*msg="1004,1019",*opt="",buff[256],*p;
    char strmsg[MAXSTRMSG]="",*antinfo="",*rcvinfo="";
    char *ant[]={"","",""},*rcv[]={"","",""};
    int i,j,n=0,dispint=5000,trlevel=0,opts[]={10000,10000,2000,32768,10,0,30};
    int types[MAXSTR]={STR_FILE,STR_FILE},stat[MAXSTR]={0},byte[MAXSTR]={0};
    int bps[MAXSTR]={0},fmts[MAXSTR]={0},sta=0;
    int n_nodes;
    
    for (i=0;i<MAXSTR;i++) paths[i]=s[i];
    
    
    for (i=1;i<argc;i++) {
        if (!strcmp(argv[i],"-in")&&i+1<argc) {
            if (!decodepath(argv[++i],types,paths[0],fmts)) return -1;
        }
        else if (!strcmp(argv[i],"-out")&&i+1<argc&&n<MAXSTR-1) {
            if (!decodepath(argv[++i],types+n+1,paths[n+1],fmts+n+1)) return -1;
            n++;
        }
        else if (!strcmp(argv[i],"-p")&&i+3<argc) {
            pos[0]=atof(argv[++i])*D2R;
            pos[1]=atof(argv[++i])*D2R;
            pos[2]=atof(argv[++i]);
            pos2ecef(pos,stapos);
        }
        else if (!strcmp(argv[i],"-o")&&i+3<argc) {
            off[0]=atof(argv[++i]);
            off[1]=atof(argv[++i]);
            off[2]=atof(argv[++i]);
        }
        else if (!strcmp(argv[i],"-msg")&&i+1<argc) msg=argv[++i];
        else if (!strcmp(argv[i],"-opt")&&i+1<argc) opt=argv[++i];
        else if (!strcmp(argv[i],"-sta")&&i+1<argc) sta=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-d"  )&&i+1<argc) dispint=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-s"  )&&i+1<argc) opts[0]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-r"  )&&i+1<argc) opts[1]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-n"  )&&i+1<argc) opts[5]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-f"  )&&i+1<argc) opts[6]=atoi(argv[++i]);
        else if (!strcmp(argv[i],"-c"  )&&i+1<argc) cmdfile=argv[++i];
        else if (!strcmp(argv[i],"-a"  )&&i+1<argc) antinfo=argv[++i];
        else if (!strcmp(argv[i],"-i"  )&&i+1<argc) rcvinfo=argv[++i];
        else if (!strcmp(argv[i],"-l"  )&&i+1<argc) local=argv[++i];
        else if (!strcmp(argv[i],"-x"  )&&i+1<argc) proxy=argv[++i];
        else if (!strcmp(argv[i],"-t"  )&&i+1<argc) trlevel=atoi(argv[++i]);
       
        //RTKROV
       
       
       
        else if (!strcmp(argv[i],"-sn")  &&i+1<argc)
        {
        	sprintf(svrname, "%s", argv[++i]);
        }
        else if (!strcmp(argv[i],"-nf")  &&i+1<argc)
        {
        	nodes_p=fopen(argv[i+1], "r");
        	n_nodes=getNodes(argv[++i]);
        	fclose(nodes_p);
        }
        else if (!strcmp(argv[i],"-lf") &&i+1<argc)
        	log_p=fopen(argv[++i], "w");
        
        
        else if (*argv[i]=='-') printhelp();
    }
    if (n<=0) n=1; /* stdout */
    
    for (i=0;i<n;i++) {
        if (fmts[i+1]<=0) continue;
        if (fmts[i+1]!=STRFMT_RTCM3) {
            fprintf(stderr,"unsupported output format\n");
            return -1;
        }
        if (fmts[0]<0) {
            fprintf(stderr,"specify input format\n");
            return -1;
        }
        if (!(conv[i]=strconvnew(fmts[0],fmts[i+1],msg,sta,sta!=0,opt))) {
            fprintf(stderr,"stream conversion error\n");
            return -1;
        }
        strcpy(buff,antinfo);
        for (p=strtok(buff,","),j=0;p&&j<3;p=strtok(NULL,",")) ant[j++]=p;
        strcpy(conv[i]->out.sta.antdes,ant[0]);
        strcpy(conv[i]->out.sta.antsno,ant[1]);
        conv[i]->out.sta.antsetup=atoi(ant[2]);
        strcpy(buff,rcvinfo);
        for (p=strtok(buff,","),j=0;p&&j<3;p=strtok(NULL,",")) rcv[j++]=p;
        strcpy(conv[i]->out.sta.rectype,rcv[0]);
        strcpy(conv[i]->out.sta.recver ,rcv[1]);
        strcpy(conv[i]->out.sta.recsno ,rcv[2]);
        matcpy(conv[i]->out.sta.pos,pos,3,1);
        matcpy(conv[i]->out.sta.del,off,3,1);
    }
    signal(SIGTERM,sigfunc);
    signal(SIGINT ,sigfunc);
    signal(SIGKILL,sigfunc);
    signal(SIGHUP ,SIG_IGN);
    signal(SIGPIPE,SIG_IGN);
    
    strsvrinit(&strsvr,n+1);
    
    if (trlevel>0) {
        traceopen(TRFILE);
        tracelevel(trlevel);
    }
    fprintf(stderr,"stream server start\n");
    
    strsetdir(local);
    strsetproxy(proxy);
    
    if (*cmdfile) readcmd(cmdfile,cmd,0);
    
    /* start stream server */
    if (!strsvrstart(&strsvr,opts,types,paths,conv,*cmd?cmd:NULL,stapos)) {
        fprintf(stderr,"stream server start error\n");
        return -1;
    }
    
//RTKROV
    if(strlen(svrname)>1)
	{
		do
		{
			sockcli=Sopen(svrname, "c");
			if(!sockcli)sleep(1);
		}while(!sockcli && !intrflg);
	}
    gettimeofday(&t2, NULL);
    gettimeofday(&t1, NULL);
    
    
    
    
    
    for (intrflg=0;!intrflg;) {
    
    
    //RTKROV
    gettimeofday(&t2,NULL);
    elapsedTime=(t2.tv_sec - t1.tv_sec) *1000.0;
    elapsedTime+=(t2.tv_usec - t1.tv_usec) / 1000.0;
    
    if(elapsedTime>=dispint)
    { 
    
    
    
        /* get stream server status */
        strsvrstat(&strsvr,stat,byte,bps,strmsg);
        
        /* show stream server status */
        for (i=0,p=buff;i<MAXSTR;i++) p+=sprintf(p,"%c",ss[stat[i]+1]);
        
        fprintf(stderr,"%s [%s] %10d B %7d bps %s\n",
                time_str(utc2gpst(timeget()),0),buff,byte[0],bps[0],strmsg);
        
//RTKROV
		gettimeofday(&t1, NULL);
	}
        rovComm();
        
    }
    if (*cmdfile) readcmd(cmdfile,cmd,1);
    
//RTKROV
    Sclose(sockcli);
    if(log_p)fclose(log_p);
    
    
    
    
    /* stop stream server */
    strsvrstop(&strsvr,*cmd?cmd:NULL);
    
    for (i=0;i<n;i++) {
        strconvfree(conv[i]);
    }
    if (trlevel>0) {
        traceclose();
    }
    fprintf(stderr,"stream server stop\n");
    return 0;
}
