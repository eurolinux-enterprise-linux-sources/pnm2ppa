/***************************************************************************
    syslog.c  -  handles syslog messages
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 1998-2000 by pnm2ppa project
    email                :
 ***************************************************************************/
 
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/*Michael Mancini <gwaihir@email.com>
  16 Jan 2000
  BeOS syslog() wrapper
  syslog.c
  modified Duncan Haldane <duncan_haldane@users.sourceforge.net>
  Oct 2000. all syslog messages arrive here wrapped.
*/

#include  <stdio.h>
#include "syslog.h"
#include "global.h"



/*
  For some reason, BeOS doesn't seem to implement syslog(), even though it is
  included with the system.  Wierd.  This wraps the syslog functions used
  and writes the info to the stderr stream.
*/


void 
wrap_syslog(int log_pri, char *fmt, char *message )
{
  if  (!(gSilent))
#ifdef __BEOS__  
    {
      gSilent = true;
      gVerbose = true;
    }
#else
  {
    syslog ( log_pri , "%s", message );
  }
#endif

  if (gVerbose)
    {
      /* Send the syslog data to the stderr stream */
      fprintf(stderr,"pnm2ppa: %s",message);
    }
  return ;
}

void wrap_openlog( char *ident, int level )
{    
#ifndef __BEOS__  
  if ( level )
    {
      /* level 1 has LOG_PERROR (prints to stderr) */
      openlog ( ident, LOG_PERROR | LOG_CONS | LOG_PID, LOG_LPR);
    }
  else
    {
      /*  standard level 0  */
      openlog ( ident, LOG_CONS | LOG_PID, LOG_LPR);
    }
#endif
  return;
}

void 
wrap_closelog( void)
{
#ifndef __BEOS__  
  closelog();
#endif
  return;
}












