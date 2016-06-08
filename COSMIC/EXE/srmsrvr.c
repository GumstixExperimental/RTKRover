/* srmsrvr.c: this program is dangerous, but it does allow one to forcibly
 * remove servers from the PortMaster's desmesne.
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
 * Date: Nov 18, 2010
 */
#include <stdio.h>
#include "xtdio.h"
#include "sockets.h"

/* ---------------------------------------------------------------------- */

/* main: */
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
rdcolor();
Sinit();

for(--argc, ++argv; argc > 0; --argc, ++argv) {
	if(Srmsrvr(*argv) != PM_OK)
	  error(XTDIO_WARNING,"failed to remove <%s%s%s>\n",MAGENTA,*argv,GREEN);
	}

#ifdef vms
return 1;
#else
return 0;
#endif
}

/* ---------------------------------------------------------------------- */

