/* SCCS-info %W% %E% */

/*--------------------------------------------------------------------*/
/*                                                                    */
/*              VCG : Visualization of Compiler Graphs                */
/*              --------------------------------------                */
/*                                                                    */
/*   file:         main.c                                             */
/*   version:      1.00.00                                            */
/*   creation:     1.4.1993                                           */
/*   author:       I. Lemke  (...-Version 0.99.99)                    */
/*                 G. Sander (Version 1.00.00-...)                    */
/*                 Universitaet des Saarlandes, 66041 Saarbruecken    */
/*                 ESPRIT Project #5399 Compare                       */
/*   description:  Top level program                                  */
/*   status:       in work                                            */
/*                                                                    */
/*--------------------------------------------------------------------*/

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
 * $Log: main.c,v $
 * Revision 3.17  1995/02/08  11:11:14  sander
 * Distribution version 1.3.
 *
 * Revision 3.16  1994/12/23  18:12:45  sander
 * Manhatten layout added.
 * Option interface cleared.
 *
 * Revision 3.15  1994/11/25  15:43:29  sander
 * Printer interface added to allow to use VCG as a converter.
 *
 * Revision 3.14  1994/11/23  14:50:47  sander
 * Input behaviour changed. Better behaviour in the case that we have no
 * input file.
 * Option -nocopt added.
 *
 * Revision 3.13  1994/08/08  16:01:47  sander
 * Attributes xraster, xlraster, yraster added.
 *
 * Revision 3.12  1994/08/05  12:13:25  sander
 * Treelayout added. Attributes "treefactor" and "spreadlevel" added.
 * Scaling as abbreviation of "stretch/shrink" added.
 *
 * Revision 3.11  1994/08/03  14:03:59  sander
 * Version Number to 1.1 changed.
 *
 * Revision 3.10  1994/08/02  15:36:12  sander
 * Local crossing unwinding implemented.
 *
 * Revision 3.9  1994/06/07  14:09:59  sander
 * Splines implemented.
 * HP-UX, Linux, AIX, Sun-Os, IRIX compatibility tested.
 * The tool is now ready to be distributed.
 *
 * Revision 3.8  1994/05/17  16:35:59  sander
 * attribute node_align added to allow nodes to be centered in the levels.
 *
 * Revision 3.7  1994/05/16  08:56:03  sander
 * shape attribute (boxes, rhombs, ellipses, triangles) added.
 *
 * Revision 3.6  1994/05/05  08:20:30  sander
 * Algorithm late labels added: If labels are inserted
 * after partitioning, this may yield a better layout.
 *
 * Revision 3.5  1994/04/27  16:05:19  sander
 * Some general changes for the PostScript driver.
 * Horizontal order added. Bug fixes of the folding phases:
 * Folding of nested graphs works now.
 *
 * Revision 3.4  1994/03/04  19:11:24  sander
 * Specification of levels per node added.
 * X11 geometry behaviour (option -geometry) changed such
 * that the window is now opened automatically.
 *
 * Revision 3.3  1994/03/03  14:12:21  sander
 * median centering heuristics added to reduce crossings.
 *
 * Revision 3.2  1994/03/02  11:48:54  sander
 * Layoutalgoritms mindepthslow, maxdepthslow, minindegree, ... mandegree
 * added.
 * Anchors and nearedges are not anymore allowed to be intermixed.
 * Escapes in strings are now allowed.
 *
 * Revision 3.1  1994/03/01  10:59:55  sander
 * Copyright and Gnu Licence message added.
 * Problem with "nearedges: no" and "selfloops" solved.
 *
 * Revision 2.4  1994/01/21  19:33:46  sander
 * VCG Version tested on Silicon Graphics IRIX, IBM R6000 AIX and Sun 3/60.
 * Option handling improved. Option -grabinputfocus installed.
 * X11 Font selection scheme implemented. The user can now select a font
 * during installation.
 * Sun K&R C (a nonansi compiler) tested. Some portabitility problems solved.
 *
 * Revision 2.3  1994/01/03  15:29:06  sander
 * First complete X11 version.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#include "globals.h"
#include "grammar.h"
#include "alloc.h"
#include "usrsignal.h"
#include "folding.h"
#include "fisheye.h"
#include "steps.h"
#include "timelim.h"
#include "main.h"
#include "options.h"
#include "grprint.h"
#include "timing.h"

#ifdef X11

#include <X11/Xos.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/Xproto.h>
#include <ctype.h>
#include <math.h>
#include "main.h"
#include "X11devpb.h"

#endif

/*--------------------------------------------------------------------*/

/* Prototypes
 * ==========
 */

/*  These functions are device dependent. Instead including all external
 *  device dependent h-files, we declare them here as external. This
 *  simplifies the selection of the device.
 *  Depending on the device, these functions are implemented in sunvdev.c
 *  or X11dev.c.
 */

extern void error(const char *format, ...);
extern void save_input_file_contents(FILE *fp);
extern void draw_main();

extern void display_part	_PP((void));
extern void setScreenSpecifics  _PP((void));
extern void gs_exit             _PP((int x));

static int   f_is_writable	_PP((char *fname));


/* Global variables
 * ================
 */


char short_banner[128];
char version_str[24]  = "V.1.3";
char date_str[48]     = "$Date: 1995/02/08 11:11:14 $";
char revision_str[48] = "$Revision: 3.17 $";

struct Point {
   int x;
   int y;
};

/*--------------------------------------------------------------------*/
/*  Main routines 						      */
/*--------------------------------------------------------------------*/


/*  The main program
 *  ================
 */

int vcg_main(int argc, char *argv[]) {
	char testvar;
	int i;

#ifdef NEVER
	printf("%d %d\n",sizeof(struct gnode),sizeof(struct gedge));
#endif

	testvar = -1;
	if (testvar != -1) {
		FPRINTF(stderr,"Warning: On this system, chars are unsigned.\n");
		FPRINTF(stderr,"This may yield problems with the graph folding operation.\n");
	}

	for (i=0; i<48; i++) {
		if (date_str[i]=='$')     date_str[i]=' ';
		if (revision_str[i]=='$') revision_str[i]=' ';
	}

	SPRINTF(short_banner,"USAAR Visualization Tool VCG/XVCG %s %s",
			version_str, revision_str);

	set_signal();

	G_xmax = G_ymax = -1;

	if ( argc <= 1)
        {
          printf("Please specify the file name in the command line\n");
          error("Aborting");
	}
	else {  /* Get arguments from the command line */

		if (!scanOptions(argc, argv)) print_basic_help(); /* and exit */
	}

	print_version_copyright();
	print_help();

	if (!Dataname[0]) {
		PRINTF("Filename: "); fflush(stdout);
		for (i=0; i<800; i++) {
                        int c = fgetc(stdin);
			Dataname[i] = c;
			if (c==0) break;
			if (c==EOF) { Dataname[i] = 0; break; }
			if (c==10)  { Dataname[i] = 0; break; }
			if (c==13)  { Dataname[i] = 0; break; }
		}
	}
	for (i=0; i<800; i++) { if (Dataname[i]!=' ') break; }
	strncpy(filename,&(Dataname[i]),  800-i);
	filename[ 800] = 0;

	for (i=0; i<800; i++) { if (filename[i]!=' ') break; }
	if (i==800)         print_basic_help(); /* and exit */
	if (filename[i]==0) print_basic_help(); /* and exit */

	if (fastflag) {
		min_baryiterations = 0;
		max_baryiterations = 2;
		min_mediumshifts = 0;
		max_mediumshifts = 2;
		min_centershifts = 0;
		max_centershifts = 2;
		max_edgebendings = 2;
		max_straighttune = 2;
	}

	parse_part();
	visualize_part();

	display_part();

	return(0);
}


/*--------------------------------------------------------------------*/

/* Check whether file is writable
 * ==============================
 * Returns 1 if yes. .
 */

static int f_is_writable(char *fname) {
        FILE *f;
        char *c;

        f = NULL;
        c = fname;
        while (*c) c++;
        if (c>fname) c--;
        if (*c=='/')  return(0);
        if (( strcmp(fname,"-")==0 ) || (*fname==0))  return(0);
        f = fopen(fname,"r");
        if (f != NULL) { fclose(f); return(0); }
        return(1);
}


/*--------------------------------------------------------------------*/

/*  Fatal Errors
 *  ============
 *  Note: the parser uses internally the function fatal_error which is
 *  different.
 */


void Fatal_error(char *x,char *y) {
  FPRINTF(stderr, "Fatal error: %s %s !\n",x,y);
  error("Aborted !\n");
}



/*--------------------------------------------------------------------*/

/*  Call of the parser
 *  ==================
 */

void	parse_part(void) {
	int 	errs,i;

	start_time();
	debugmessage("parse_part","");

	/* We start from the scratch */

	info_name_available = 0;
	for (i=0; i<3; i++) info_names[i]=NULL;

	free_memory();
	yyin = NULL;

	if ( strcmp(Dataname,"-")==0 ) yyin = stdin;
	else {
	 	yyin = fopen(Dataname,"r");
		if (yyin == NULL) Fatal_error("Cannot open",Dataname);
                save_input_file_contents(yyin);
       	}

	free_timelimit();
	if (G_timelimit>0) init_timelimit(G_timelimit);
        errs = parse();

        if ((yyin)&&(yyin!=stdin)) fclose(yyin);

        if (errs>0)
          Fatal_error("Syntax error","");
        else
  	  assert((Syntax_Tree!=NULL));

	stop_time("parse_part");
}


/*--------------------------------------------------------------------*/

/*  Call of the visualizer
 *  ======================
 */

#ifdef X11
static char geom_buffer[128];
#endif

void	visualize_part(void) {
	debugmessage("visualize_part","");

	/* Init of the default values */

        G_title         = myalloc(256);
       	strncpy(G_title,Dataname,254);
	G_title[255] = 0;
        G_x             = -1L;
        G_y             = -1L;

	/* Check the output device */
	if (!exfile) {
        	setScreenSpecifics(); 	/* this sets G_width, G_height */
	}
	else {
		G_width = G_height = 700;
	}
	G_width_set  = 0;	/* because they are not set by */
	G_height_set = 0;	/* the specification           */
	if (!G_xymax_final) G_xmax = G_width+10;
        if (!G_xymax_final) G_ymax = G_height+10;
        G_xbase         = 5;
        G_ybase         = 5;
        G_dspace        = 0;
        G_xspace        = 20;
        G_yspace        = 70;
        G_orientation   = TOP_TO_BOTTOM;
        G_folding       = 0;
        G_color         = WHITE;
//        G_displayel     = NO;
        G_dirtyel       = NO;
        G_shrink        = 1;
        G_stretch       = 1;
	G_yalign        = AL_CENTER;
	G_portsharing   = YES;
	G_arrowmode     = AMFIXED;
	G_xraster	= 1;
 	G_yraster	= 1;
 	G_dxraster	= 1;

	/* No edge class is hidden: initialize this */

	clear_hide_class();

	/* Check colors */

	if (colored== -1) {
#ifdef VMS
		if (RootWinMaxDepth ==1) colored = 0;
#else
        	if (maxDepth == 1)       colored = 0;
#endif
        	else /* maxDepth == 8 */ colored = 1;
	}

	/*  Analyze specification and allocate graph */

	step0_main();
	if (nr_errors!=0) Fatal_error("Wrong specification","");
	check_graph_consistency();

#ifdef X11
	if (!Xmygeometry) {
		if ((G_width_set)&&(G_height_set)) {
			if ((G_x != -1L) && (G_y != -1L))
				SPRINTF(geom_buffer,"%dx%d+%ld+%ld",
					G_width,G_height,G_x,G_y);
			else 	SPRINTF(geom_buffer,"%dx%d",G_width,G_height);
			Xmygeometry   = geom_buffer;
		}
		else if ((G_x != -1) && (G_y != -1)) {
				SPRINTF(geom_buffer,"+%ld+%ld",
					G_x,G_y);
			Xmygeometry   = geom_buffer;
		}
	}
#endif

	/* Set drawing area */

	G_xymax_final = 1;
        V_xmin = V_xmin_initial;
        V_xmax = V_xmin + (long)G_xmax;
        V_ymin = V_ymin_initial;
        V_ymax = V_ymin + (long)(G_ymax + COFFSET);

	relayout();
}


/* Relayout the graph
 * ------------------
 */


void relayout(void) {
	debugmessage("relayout","");
        start_time();

	if (G_timelimit>0) init_timelimit(G_timelimit);
	free_all_lists();
        if (nr_errors==0) folding();
        stop_time("folding");

        if (!locFlag) {

		if (min_mediumshifts>max_mediumshifts)
			max_mediumshifts = min_mediumshifts;
		if (min_centershifts>max_centershifts)
			max_centershifts = min_centershifts;
		if (min_baryiterations>max_baryiterations)
			max_baryiterations = min_baryiterations;
		if (one_line_manhatten==1) manhatten_edges = 1;
		if ((manhatten_edges==1)&&(prio_phase==0)) prio_phase = 1;
		if ((prio_phase==1)&&(straight_phase==0)) straight_phase = 1;
		if ((prio_phase==0)&&(straight_phase==1)) prio_phase = 1;
		if (prio_phase==1)     {
			min_centershifts = 0;
			max_centershifts = 0;
		}
		if (G_dxraster<=0) G_dxraster = 1;
		if (G_xraster<=0)  G_xraster  = 1;
		if (G_xraster % G_dxraster) {
			G_xraster = (G_xraster/G_dxraster) * G_dxraster;
		}

		/* Calculate new layout */

                step1_main();
		if (nr_errors!=0) Fatal_error("Wrong specification","");

		/* step1_main calls tree_main, if TREE_LAYOUT.
		 */

		if (layout_flag != TREE_LAYOUT) {
                	step2_main();
			if (nr_errors!=0) Fatal_error("Wrong specification","");
                	step3_main();
			if (nr_errors!=0) Fatal_error("Wrong specification","");
		}
        	step4_main();
		if (nr_errors!=0) Fatal_error("Wrong specification","");
	}
	else {
		/* Prepare given layout: calculate co-ordinate of edges
 		 * width and height of nodes etc.
		 */

		prepare_nodes();
	}
	free_timelimit();
}

/*--------------------------------------------------------------------*/
void display_part(void)
{
  init_fe(0,300,0,300,200,200);
  if ( remove_input_file ) unlink(Dataname);
}


//void gs_exit(int x) { cleanup_streams(); exit(x); }
void gs_exit(int x) { exit(x); }


void setScreenSpecifics(void) 	/* this sets G_width, G_height */
{
}

void gs_wait_message(int code)
{
  // process code and return
  (void)code;
}

/*--------------------------------------------------------------------*/
static void (*line_cb)(int x1,int y1,int x2,int y2,int color, void *painter);
static void (*rectangle_cb)(long x,long y,int w,int h,int color, void *painter);
static void (*polygon_cb)(struct Point *hp, int j, int color, void *painter);
static void *painter;

void gs_line(int fx,int fy,int tx,int ty,int color)
{
  int x1 = fx - V_xmin;
  int y1 = fy - V_ymin;
  int x2 = tx - V_xmin;
  int y2 = ty - V_ymin;
  if ( fisheye_view != 0 )
  {
    int d = 1;
    fe_g_to_s(fx, fy, &x1, &y1);
    fe_g_to_s(tx, ty, &x2, &y2);
    while ( (x1-x2)*(x1-x2)+(y1-y2)*(y1-y2) > 50*50)
    {
      int x3, y3;
      int hx = fx + d * (tx-fx)/7;
      int hy = fy + d * (ty-fy)/7;
      fe_g_to_s(hx,hy,&x3,&y3);
      line_cb(x1, y1, x3, y3, color, painter);
      x1 = x3;
      y1 = y3;
      d++;
    }
  }
  line_cb(x1, y1, x2, y2, color, painter);
}

void gs_rectangle(long x,long y,int w,int h,int color)
{
  int x2, y2, hx, hy, j, d;
  struct Point hp[29];

  int x1 = x - V_xmin;
  int y1 = y - V_ymin;
  if ( fisheye_view != 0 )
  {
    switch ( fisheye_view )
    {
      case FPSCF_VIEW:
      case PSCF_VIEW:
    	j = 0;
    	fe_g_to_s(x, y, &x1, &y1);
    	fe_g_to_s(x, y+h, &x2, &y2);
    	hp[j].x = x1;
    	hp[j++].y = y1;
    	d = 1;
    	while ( (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1) > 50*50 )
        {
    	  hx = x;
    	  hy = y + d * h/7;
    	  fe_g_to_s(hx, hy, &x1, &y1);
    	  hp[j].x = x1;
    	  hp[j++].y = y1;
    	  d++;
    	  if ( j > 7 ) break;
    	}
    	x1 = hp[j].x = x2;
    	y1 = hp[j++].y = y2;
    	fe_g_to_s(x+w,y+h,&x2,&y2);
    	d = 1;
    	while ( (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1) > 50*50 )
        {
    	  hx = x + d * w/7;
    	  hy = y + h;
    	  fe_g_to_s(hx, hy, &x1, &y1);
    	  hp[j].x = x1;
    	  hp[j++].y = y1;
    	  d++;
    	  if ( j > 14 ) break;
    	}
    	x1 = hp[j].x = x2;
    	y1 = hp[j++].y = y2;
    	fe_g_to_s(x+w,y,&x2,&y2);
    	d = 1;
    	while ( (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1) > 50*50 )
        {
    	  hx = x + w;
    	  hy = y + h - d * h/7;
    	  fe_g_to_s(hx, hy, &x1, &y1);
    	  hp[j].x = x1;
    	  hp[j++].y = y1;
    	  d++;
    	  if ( j > 21 ) break;
    	}
    	x1 = hp[j].x = x2;
    	y1 = hp[j++].y = y2;
    	x2 = hp[0].x;
    	y2 = hp[0].y;
    	d = 1;
    	while ( (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1) > 50*50 )
        {
    	  hx = x + w - d * w/7;
    	  hy = y;
    	  fe_g_to_s(hx, hy, &x1, &y1);
    	  hp[j].x = x1;
    	  hp[j++].y = y1;
    	  d++;
    	  if ( j > 28 ) break;
    	}
        polygon_cb(hp, j, color, painter);
    	return;
    case FCSCF_VIEW:
    case CSCF_VIEW:
    	fe_g_to_s(x,y,&x1,&y1);
    	fe_g_to_s(x+w,y+h,&x2,&y2);
    	w = x2-x1;
    	h = y2-y1;
    	if ( (w<=0) && (h<=0) ) return;
    	if ( w<=0 ) w = 1;
    	if ( h<=0 ) h = 1;
    	break;
    }
  }
  rectangle_cb(x1, y1, w, h, color, painter);
}

/*--------------------------------------------------------------------*/
int set_drawing_rectangle(int width, int height)
{
  int changed = 0;
  G_width  = width;
  G_height = height;
  if ( G_xmax < G_width )
  {
    G_xmax = G_width;
    changed = 1;
  }
  if ( G_ymax < G_height )
  {
    G_ymax = G_height;
    changed = 1;
  }
  change_fe_winsize(0, width, 0, height);
  return changed;
}

/*--------------------------------------------------------------------*/
void draw_graph(void (*_line_cb)(int x1,int y1,int x2,int y2,int color, void *painter),
                void (*_rectangle_cb)(long x,long y,int w,int h,int color, void *painter),
                void (*_polygon_cb)(struct Point *hp, int j, int color, void *painter),
                void *_painter)
{
  line_cb = _line_cb;
  rectangle_cb = _rectangle_cb;
  polygon_cb = _polygon_cb;
  painter = _painter;
  draw_main();
}

/*--------------------------------------------------------------------*/
void m_reload(void)
{
  parse_part();
  visualize_part();
  init_fe(0,300,0,300,200,200);
}

/*--------------------------------------------------------------------*/
void m_validate_fe(int code)
{
  static const int codes[] = { PSCF_VIEW, FPSCF_VIEW, CSCF_VIEW, FCSCF_VIEW, 0 };
  long h;

  exit_fe();
  fisheye_view = codes[code];
  h = gfishdist;
  init_fe(0,G_width,
  	0,G_height,200,200);
  set_gfishdist(h);
}

/*--------------------------------------------------------------------*/
void display_complete_graph(void)
{
  int window_width  = G_width;
  int window_height = G_height;
  normal_fe_focus();
  if (maximal_xpos*window_height > maximal_ypos*window_width)
  {
    if (window_width <=40 )
      set_fe_scaling(1, maximal_xpos);
    else
      set_fe_scaling(window_width, maximal_xpos);
  }
  else
  {
    if (window_height<=60-35)
      set_fe_scaling(1, maximal_ypos);
    else
      set_fe_scaling(window_height, maximal_ypos);
  }
}
