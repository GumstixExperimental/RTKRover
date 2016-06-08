/* Sputs.c: this function "puts" a string which Sgets can receive
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

/* --------------------------------------------------------------------- */

/* Sputs: */
#ifdef __PROTOTYPE__
void Sputs(
  char   *buf,
  Socket *skt)
#else
void Sputs(
  buf,
  skt)
char   *buf;
Socket *skt;
#endif
{

/* write out buf and the null byte */
Swrite(skt,buf,strlen(buf)+1);

}

/* ---------------------------------------------------------------------
 * vim: ts=4
 */
