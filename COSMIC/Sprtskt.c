/* Sprtskt.c:
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
#include "sockets.h"

/* ------------------------------------------------------------------------
 * Local Definitions:
 */
#define BUFSIZE 128

/* ------------------------------------------------------------------------ */

/* Sprtskt: this function prints out a socket description */
#ifdef __PROTOTYPE__
char *Sprtskt(Socket *skt)
#else
char *Sprtskt(skt)
Socket *skt;
#endif
{
static char buf1[BUFSIZE];
static char buf2[BUFSIZE];
static char *b= buf1;

if(!skt) return "null socket";

/* toggles between the two static buffers */
if(b == buf1) b= buf2;
else          b= buf1;

sprintf(b,"#%d:p%d:%s:%s@%s",
  skt->skt,
  skt->port,
  (skt->type == PM_SERVER)? "server"      :
  (skt->type == PM_CLIENT)? "client"      :
  (skt->type == PM_ACCEPT)? "accept"      : "???",
  skt->sktname?             skt->sktname  : "null-sktname",
  skt->hostname?            skt->hostname : "null-host");

return b;
}

/* ---------------------------------------------------------------------
 * vim: ts=4
 */
