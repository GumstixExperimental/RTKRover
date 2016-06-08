/* Saccept.c: this file contains the Socket accept support
 * Authors:      Charles E. Campbell, Jr.
 *               Terry McRoberts
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
#include <errno.h>
#include "sockets.h"

/* ------------------------------------------------------------------------- */
/* Saccept: this routine uses a server Socket to accept connections
 *  The accept() function clones a socket for use with a client connect.
 *  One may close the Saccept generated socket without affecting the
 *  server socket.
 */
#ifdef __PROTOTYPE__
Socket *Saccept(Socket *skt)
#else
Socket *Saccept(skt)
Socket *skt;
#endif
{
#if defined(_AIX)
size_t          addrlen;
#elif defined(__gnu_linux__)
socklen_t       addrlen;
#else
int             addrlen;
#endif
#ifndef SSLNOSETSOCKOPT
int             status=1;
#endif
struct sockaddr addr;
Socket         *acceptskt= NULL;


/* sanity check */
if(!skt) {
	return acceptskt;
	}

/* allocate a Socket */
acceptskt= makeSocket(skt->hostname,skt->sktname,PM_ACCEPT);
if(!acceptskt) {
	return acceptskt;
	}

/* accept a connection (re-tries if EINTR cause of accept() failure;
 * done at suggestion of Kenneth Long)
 */
addrlen= sizeof (addr);
do {
	acceptskt->skt= accept(skt->skt, &addr, &addrlen);
	} while((acceptskt->skt < 0) && (errno == EINTR));
if(acceptskt->skt <= 0) {	/* failure to accept */

	freeSocket(acceptskt);


	return (Socket *) NULL;
	}

/* turn off TCP's buffering algorithm so small packets don't get delayed */
#ifndef SSLNOSETSOCKOPT


if(setsockopt(skt->skt,IPPROTO_TCP,TCP_NODELAY,(char *) &status,sizeof(status)) < 0) {
	}
#endif	/* #ifndef SSLNOSETSOCKOPT ... */

return acceptskt;
}

/* ---------------------------------------------------------------------
 * vim: ts=4
 */
