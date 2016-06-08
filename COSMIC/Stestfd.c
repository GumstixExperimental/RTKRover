/* Stestfd.c: this file contains code which allows one to test if data
 *            is present on the specific file descriptor
 *
 *  Returns: number of bytes of data awaiting perusal (which may be 0)
 *	         or EOF if unable to "select" the socket
 *                  or select returns 1 but nothing is on socket
 *
 * Author:       Charles E. Campbell, Jr.
 * Copyright:    Copyright (C) 1999-2010 Charles E. Campbell, Jr.
 *               Permission is hereby granted to use and distribute this code,
 *               with or without modifications, provided that this copyright
 *               notice is copied with it. Like anything else that's free,
 *               Saccept.c is provided *as is* and comes with no warranty
 *               of any kind, either expressed or implied. By using this
 *               software, you agree that in no event will the copyright
 *               holder be liable for any damages resulting from the use
 *               of this software.  There is no merchantability or fitness
 *               for a particular purpose.
 * Date:         Nov 18, 2010
 */
#include <stdio.h>
#define SSLNEEDTIME
#include "sockets.h"

/* =====================================================================
 * Functions: {{{1
 */

/* --------------------------------------------------------------------- */
/* Stest: {{{2 */
#ifdef __PROTOTYPE__
int Stestfd(int fd)
#else
int Stestfd(fd)
int fd;
#endif
{
fd_set         emask;
fd_set         rmask;
fd_set         wmask;
int            ret;
short          result;
static char    buf[PM_BIGBUF];
struct timeval timeout;


FD_ZERO(&rmask);
FD_SET(fd,&rmask);
FD_ZERO(&wmask);
FD_ZERO(&emask);

timeout.tv_sec = 0;
timeout.tv_usec= 0;

#ifdef SSLNOPEEK
result = select(fd+1,rmask.fds_bits,wmask.fds_bits,emask.fds_bits,&timeout);
#else
result = select(fd+1,&rmask,&wmask,&emask,&timeout);
#endif	/* SSLNOPEEK */

if(result < 0) {
	return EOF;
	}

#ifdef SSLNOPEEK

if(FD_ISSET(fd,&rmask)) {
	return 1;
	}

#else	/* #ifdef SSLNOPEEK ... #else ... #endif */

/* test if message available from socket, return qty bytes avail */

if(FD_ISSET(fd,&rmask)) {
	ret= recv(fd,buf,PM_BIGBUF-1,MSG_PEEK);

	/* return error indication when select returned a result of 1
	 * (indicating that *something* is on the socket)
	 * and recv indicates that *nothing* is on the socket
	 */
	if(result == 1 && ret == 0) ret= EOF;
	return ret;
	}

#endif	/* #ifdef SSLNOPEEK ... #else ... #endif	*/

/* file descriptor is empty */
return 0;
}

/* =====================================================================
 * DEBUG_TEST: {{{1
 */
#ifdef DEBUG_TEST
#define BUFSIZE 256

/* --------------------------------------------------------------------- */
/* main: test routine begins here... */
int main(int argc,char **argv)
{
char buf[BUFSIZE];

rdcolor();				/* initialize color names (GREEN, RED, etc.)    */

printf("waiting...\n");
Smaskfdset(STDIN_FILENO);
Smaskwait();
while(fgets(buf,BUFSIZE,stdin)) {
	srmtrblk(buf);
	printf("rcvd<%s>\n",sprt(buf));
	if(!strcmp(buf,"quit")) break;
	printf("waiting...\n");
	Smaskwait();
	printf("ok, Smaskwait returned!\n");
	}

return 0;
}
#endif	/* DEBUG_TEST */

/* =====================================================================
 * Modelines: {{{1
 *  vim: fdm=marker
 */
