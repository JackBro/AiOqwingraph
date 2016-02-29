/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         timelim.c                                          */
/*   version:      1.00.00                                            */
/*   creation:     14.4.1993                                          */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */  
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Time Limit Management                              */
/*   status:       in work                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/

#ifndef lint
static char *id_string="$Id: timelim.c,v 1.6 1995/02/09 20:15:52 sander Exp $";
#endif

/*
 *   Copyright (C) 1993--1995 by Georg Sander, Iris Lemke, and
 *                               the Compare Consortium 
 *
 *  This program and documentation is free software; you can redistribute 
 *  it under the terms of the  GNU General Public License as published by
 *  the  Free Software Foundation;  either version 2  of the License,  or
 *  (at your option) any later version.
 *
 *  This  program  is  distributed  in  the hope that it will be useful,
 *  but  WITHOUT ANY WARRANTY;  without  even  the  implied  warranty of
 *  MERCHANTABILITY  or  FITNESS  FOR  A  PARTICULAR  PURPOSE.  See  the
 *  GNU General Public License for more details.
 *
 *  You  should  have  received a copy of the GNU General Public License
 *  along  with  this  program;  if  not,  write  to  the  Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  The software is available per anonymous ftp at ftp.cs.uni-sb.de.
 *  Contact  sander@cs.uni-sb.de  for additional information.
 */


/* 
 * $Log: timelim.c,v $
 * Revision 1.6  1995/02/09  20:15:52  sander
 * Portability problem with HPUX.
 *
 * Revision 1.2  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 1.1  1994/12/23  18:12:45  sander
 * Initial revision
 *
 */


/****************************************************************************
 * This file is a collection of auxiliary functions that implement the
 * time limit management. Note that this feature need not to be available
 * on every computer.
 * The time limit managements allows to limit the layout time. If the time
 * limit is exceeded, the layout automatically switches to fast and ugly 
 * layout. Further, a time limit is not a hard limit: E.g. the layout of
 * 10000 nodes always needs more than 1 second, even if we set the time
 * limit to 1 second.
 *
 * Time limits are real time !!!
 *
 * This file provides the following functions:
 *
 * init_timelimit       gets the limit and initializes the time limit function,
 *			in the case that it was not initialized before.
 * free_timelimit       release the time limit function. This means that the
 *                      next call of reinit_timelimit recognizes, that the
 *                      time limit must be initialized again.
 * test_timelimit       gets the percentual part of the limit and returns 
 *			true, if the time limit is exceeded.
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

#ifndef NOTIMELIMIT

// This file is rewritten by Ilfak Guilfanov

#include <time.h>

/* Local Variables
 * ---------------
 */

static time_t timelimit = 0L;	/* the actual time limit in sec */

static time_t tpxstart;	        /* the start time     */

static time_t tpxend;     	/* the stop time      */

/* Set the time limit to x seconds and start the clock
 * ---------------------------------------------------
 * This is only done if it was not done before.
 */

void init_timelimit(int x)
{
  debugmessage("init_timelimit","");
  if ( timelimit > 0L ) return;	
  if ( x < 1 ) x = 1;
  timelimit = x;
  tpxstart = time(NULL);
}

/* Free the actual time limit
 * --------------------------
 */

void free_timelimit(void)
{
  debugmessage("free_timelimit","");
  timelimit = 0;
}

/* Check whether the percentual time limit is reached
 * --------------------------------------------------
 * Return true (1) if yes.
 */

int test_timelimit(int perc)
{
  unsigned long sec;
  unsigned long tval;

  debugmessage("test_timelimit","");

  tpxend = time(NULL);
  sec = tpxend - tpxstart;
  tval = (perc*timelimit)/100L;
  if ( tval == 0L ) tval = 1L;
  if ( sec > tval ) return 1;
  return 0;
}

#else

void init_timelimit(int x)
{
	debugmessage("init_timelimit","");
}

void free_timelimit(void)
{
	debugmessage("free_timelimit","");
}

int test_timelimit(int perc)
{
	debugmessage("test_timelimit","");
	return(0);
}

#endif /* NOTIMELIMIT */


