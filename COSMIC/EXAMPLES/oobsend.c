/* /media/Lexar/ssl/oobsend.c: this program
 *   Author: Charles E. Campbell, Jr.
 *   Date:   Aug 19, 2009
 */

/* =====================================================================
 * Header Section: {{{1
 */

/* ---------------------------------------------------------------------
 * Includes: {{{2
 */
#define _BSD_SIGNALS
#include <stdio.h>
#include "xtdio.h"
#include <signal.h>
#include <fcntl.h>
#include "sockets.h"

/* ------------------------------------------------------------------------
 * Definitions: {{{2
 */
#define BUFSIZE	256

/* ------------------------------------------------------------------------
 * Typedefs: {{{2
 */

/* ------------------------------------------------------------------------
 * Local Data Structures: {{{2
 */

/* ------------------------------------------------------------------------
 * Global Data: {{{2
 */

/* ------------------------------------------------------------------------
 * Explanation: {{{2
 */

/* ------------------------------------------------------------------------
 * Prototypes: {{{2
 */

/* ========================================================================
 * Functions: {{{1
 */

/* --------------------------------------------------------------------- */
/* main: {{{2 */
#ifdef __PROTOTYPE__
int main(
  int argc,
  char **argv)
#else	/* __PROTOTYPE__ */
int main(argc,argv)
int argc;
char **argv;
#endif	/* __PROTOTYPE__ */
{
char    buf[BUFSIZE];
char    msg          = 'T';
int     cnt;
pid_t   rcvpid;
Socket *srvr         = NULL;
Socket *skt          = NULL;

rdcolor();

srvr= Sopen("oob","S");
if(!srvr) error(XTDIO_ERROR,"unable to open oob server\n");
printf("opened server<oob>\n");

skt= Saccept(srvr);
printf("accepted client\n");

/* get receiver's pid */
Sscanf(skt,"%d",&rcvpid);
printf("rcvpid=%d\n",rcvpid);

Sputs("something",skt);
printf("sent <something>\n");

/* put an OOB data transfer over the Socket */
if((cnt= send(skt->skt,&msg,1,MSG_OOB)) < 0) {	/* socket error */
	perror("oobsend");
	error(XTDIO_WARNING,"socket error on OOB send\n");
	}
else {
	printf("send oob<T> (cnt=%d)\n",cnt);
	kill(rcvpid,SIGURG);
	printf("sent SIGURG to rcvpid=%d\n",rcvpid);
	if(!Sgets(buf,BUFSIZE,skt)) printf("socket error on Sgets\n");
	else                        printf("reply<%s>\n",sprt(buf));
	}

Sclose(skt);
printf("closed accept socket\n");

Sclose(srvr);
printf("closed server<oob>\n");

return 0;
}

/* --------------------------------------------------------------------- */
