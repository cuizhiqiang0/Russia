#include <stdio.h>
#include <time.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/time.h>
#include <signal.h>


/* Define the global parameters */
#define RED 1
#define GREEN 2
#define BLUE 4
#define BRICKXSIZE 9
#define BRICKYSIZE 13
#define BRICKWIDTH 20
#define BRICKHEIGHT 20

static unsigned char gColor=RED;
static GtkWidget *gRedPmap, *gGreenPmap, *gBluePmap;
GtkWidget *window;

static gint g_RBrick[BRICKXSIZE][BRICKYSIZE];

/* global parameters for the brick*/
typedef struct _brickposition {
  gint m_x;// based on 0
  gint m_y;// based on 0
} BrickPosition;

typedef struct _brickblock {
  BrickPosition m_brickpos[4];
  BrickPosition m_areastartpos;
  BrickPosition m_areaendpos;
} BrickBlock;

typedef struct _candidateblock{
  BrickBlock m_brickblock[4];
  gint m_index;
}CandidateBlock;

CandidateBlock g_candidateblock[5];
gint g_currentbrickblockindex=0;
gint g_currentbrickx=3;// based on 0
gint g_currentbricky=0;// based on 0
/* end of global parameters */

/* Whether the brick block conflict or not 
   return 1, OK
   return 0, not OK
 */
gint 
BrickPositionCorrect(gint m_currentblock,gint m_blockstatus,gint m_startx, gint m_starty){
  gint t_loopi, t_tempint,t_indexx,t_indexy,t_brickblockindex, t_tempx, t_tempy;

  t_tempint=0;
  t_brickblockindex=m_blockstatus;

  /* Clear the current exist brick block */
  for(t_loopi=0;t_loopi<4;t_loopi++){
    gint tt_tempindex;
    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
    g_RBrick[t_tempx][t_tempy]=0;
  }

  /* using the input parameter, judge whether it is possible */
  for(t_loopi=0;t_loopi<4;t_loopi++){// for each brick in the brick block
    t_indexx=g_candidateblock[m_currentblock].m_brickblock[t_brickblockindex].m_brickpos[t_loopi].m_x;
    t_indexy=g_candidateblock[m_currentblock].m_brickblock[t_brickblockindex].m_brickpos[t_loopi].m_y;
    t_indexx=t_indexx+m_startx;
    t_indexy=t_indexy+m_starty;
    /* Get the value from matrix */
    if(t_indexx<0||t_indexx>=BRICKXSIZE||t_indexy<0||t_indexy>=BRICKYSIZE) t_tempint++;
    else t_tempint=t_tempint+g_RBrick[t_indexx][t_indexy];
  }// for each brick in the brick block

  /* Recover the current brick  */
  for(t_loopi=0;t_loopi<4;t_loopi++){
    gint tt_tempindex;
    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
    g_RBrick[t_tempx][t_tempy]=1;
  }

  /* return the judgement */
  if(t_tempint>=1) return 0;
  else return 1;
}


/* Create bitmap */
static GtkWidget* 
NewPixmap ( char* filename, GdkWindow *window, GdkColor *background)
{
	GtkWidget *wpixmap;
	GdkPixmap *pixmap;
	GdkBitmap *mask;

	pixmap=gdk_pixmap_create_from_xpm(window, &mask, background,
	filename);
	wpixmap=gtk_pixmap_new(pixmap,mask);
	//	wpixmap=gtk_image_new_from_file(filename);

	return wpixmap;
}

/* Select the desired color*/
static void
SetColor(GtkWidget *widget, gpointer arg)
{
  gColor=(unsigned char) arg;
}

void
Update (GtkWidget * widget, char* timestr)
{
	time_t timeval;

	timeval=time(NULL);
	strcpy (timestr, ctime(&timeval));
}

void
PrintAndExit (GtkWidget * widget, char timestr[][26])
{ 
	int i;
	for(i=0;i<4;i++)
		printf("timestr[%d] is %s", i, timestr[i]);
	gtk_main_quit();
}

/* handle for expose the drawing area
 */
static gint
HandleExpose(GtkWidget* widget, GdkEventExpose *event, gpointer arg)
{
  /* Calculate the area to be redrawn.
     The client area is divided into 9*13 blocks. So it is needed 
     to calculate the blocks cover the event area. The size of the 
     client area is 180*260, so 20*20 each block.
  */
  gint t_loopx,t_loopy;
  GtkPixmap *pixmap;
  //  GdkBitmap *bitmap;
  gint c_xsize=20,c_ysize=20;
  gint t_startx, t_starty, t_endx, t_endy;// based on 0
  t_startx=event->area.x/c_xsize;
  t_starty=event->area.y/c_ysize;
  t_endx=(event->area.x+event->area.width-1)/c_xsize;
  t_endy=(event->area.y+event->area.height-1)/c_ysize;

  /* Select the correct pixmap
   */
  switch (gColor){
  case RED:
    pixmap=GTK_PIXMAP(gRedPmap);
    break;
  case GREEN:
    pixmap=GTK_PIXMAP(gGreenPmap);
    break;
  case BLUE:
    pixmap=GTK_PIXMAP(gBluePmap);
    break;
  default:
    break;
  }
  /* Redraw the client area
   */
  for(t_loopy=t_starty;t_loopy<=t_endy;t_loopy++){// loop y
    for(t_loopx=t_startx;t_loopx<=t_endx;t_loopx++){// loop x
      // get the value from matrix
      if(g_RBrick[t_loopx][t_loopy]==1){// draw one break
	gdk_draw_pixmap(widget->window, widget->style->black_gc,
			pixmap->pixmap, 0,0,t_loopx*c_xsize,t_loopy*c_ysize,-1,-1);
      }else{// end of draw on brick 
	gdk_window_clear_area (widget->window,t_loopx*c_xsize,t_loopy*c_ysize,c_xsize,c_ysize);// clear the area
      }
    }// end of loop x
  }// end of loop y

}
/* handle for the user input action
 */
static gint
KeyPress(GtkWidget *widget, GdkEventKey *event, gpointer arg)
{
  gint t_tempindex,t_tempx,t_tempy,t_retv, t_loopi,tt_loopi;
  BrickPosition t_oldstart,t_oldend,t_newstart,t_newend;
  GdkEvent t_event;

  t_oldstart.m_x=g_currentbrickx;
  t_oldstart.m_y=g_currentbricky;
  t_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
  t_oldend=g_candidateblock[g_currentbrickblockindex].m_brickblock[t_tempindex].m_areaendpos;
  t_oldend.m_x=t_oldend.m_x+g_currentbrickx;
  t_oldend.m_y=t_oldend.m_y+g_currentbricky;

  t_retv=0;
  switch(event->keyval)
    {
    case GDK_space:
      {
	t_tempindex=(g_candidateblock[g_currentbrickblockindex].m_index+1)%4;
	t_retv=BrickPositionCorrect(g_currentbrickblockindex,t_tempindex,g_currentbrickx, g_currentbricky);
	if(t_retv==1){
	  /* Clear the old value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=0;
	  }
	  /* action */
	  g_candidateblock[g_currentbrickblockindex].m_index=t_tempindex;
	  /* Set the new value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=1;
	  }// end of set new value
	}// end of brick block changed.
      }
      break;
    case GDK_Down:
      {
	t_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	t_retv=BrickPositionCorrect(g_currentbrickblockindex,t_tempindex,g_currentbrickx, g_currentbricky+1);
	if(t_retv==1){
	  /* Clear the old value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=0;
	  }
	  /* action */
	  g_currentbricky++;
	  /* Set the new value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=1;
	  }// end of set new value
	}// end of brich changed
	else{// matrix bottom met. New brick created
	  t_retv=1;
	  /* Create a brick block randomly */
	  g_currentbrickblockindex=g_random_int_range(0,5);
	  g_currentbrickx=3;
	  g_currentbricky=0;
	  g_candidateblock[g_currentbrickblockindex].m_index=g_random_int_range(0,4);
	  /* Clear the all 1 rows */
	  gint tt_src=BRICKYSIZE-1;
	  gint tt_dst=BRICKYSIZE-1;
	  for(t_loopi=BRICKYSIZE-1;t_loopi>=0;t_loopi--){// adjust g_RBrick
	  
	    if(tt_src!=tt_dst){// should copy
	      for( tt_loopi=0;tt_loopi<BRICKXSIZE;tt_loopi++){// copy row
		g_RBrick[tt_loopi][tt_dst]=g_RBrick[tt_loopi][tt_src];
	      }// end of copy row
	    }// should copy
	    /* move src upward */
	    tt_src--;
	    /* can dest move upward? depends on whether the row are all 1 */
	    gint tt_sumrow=0;
	    for( tt_loopi=0;tt_loopi<BRICKXSIZE;tt_loopi++){// sum row
	      tt_sumrow=tt_sumrow+g_RBrick[tt_loopi][tt_dst];
	    }// end of sum row
	    if(tt_sumrow<BRICKXSIZE) tt_dst--;// not all 1
	  }// end of adjust g_RBrick matrix

	  /* Clear the empty matrix */
	  for(t_loopi=0;t_loopi<=tt_dst;t_loopi++){// clear the upper row
	    for(t_tempx=0;t_tempx<BRICKXSIZE;t_tempx++){
	      g_RBrick[t_tempx][t_loopi]=0;
	    }
	  }// clear the upper row

	  /* Set the matrix new value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=1;
	  }// end of set new matrix value

	  /* Reset the redraw area to all area */
	  t_oldstart.m_x=0;t_oldstart.m_y=0;
	  t_oldend.m_x=BRICKXSIZE-1;t_oldend.m_y=BRICKYSIZE-1;

	}// end of new brick
      }
      break;
    case GDK_Right:
      {
	t_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	t_retv=BrickPositionCorrect(g_currentbrickblockindex,t_tempindex,g_currentbrickx+1, g_currentbricky);
	if(t_retv==1){
	  /* Clear the old value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=0;
	  }
	  /* action */
	  g_currentbrickx++;
	  /* Set the new value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=1;
	  }// end of set new value
	}// end of brick changed
      }
      break;
    case GDK_Left:
      {
	t_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	t_retv=BrickPositionCorrect(g_currentbrickblockindex,t_tempindex,g_currentbrickx-1, g_currentbricky);
	if(t_retv==1){
	  /* Clear the old value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=0;
	  }
	  /* action */
	  g_currentbrickx--;
	  /* Set the new value */
	  for(t_loopi=0;t_loopi<4;t_loopi++){
	    gint tt_tempindex;
	    tt_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
	    t_tempx=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_x+g_currentbrickx;
	    t_tempy=g_candidateblock[g_currentbrickblockindex].m_brickblock[tt_tempindex].m_brickpos[t_loopi].m_y+g_currentbricky;
	    g_RBrick[t_tempx][t_tempy]=1;
	  }// end of set new value
	}// end of brick changed
      }
      break;
    }


    
  if(t_retv==1){// redraw the related area
    t_newstart.m_x=g_currentbrickx;
    t_newstart.m_y=g_currentbricky;
    t_tempindex=g_candidateblock[g_currentbrickblockindex].m_index;
    t_newend=g_candidateblock[g_currentbrickblockindex].m_brickblock[t_tempindex].m_areaendpos; 
    t_newend.m_x=t_newend.m_x+t_newstart.m_x;
    t_newend.m_y=t_newend.m_y+t_newstart.m_y;
    if(t_newstart.m_x>t_oldstart.m_x)t_newstart.m_x=t_oldstart.m_x;
    if(t_newstart.m_y>t_oldstart.m_y)t_newstart.m_y=t_oldstart.m_y;
    if(t_newend.m_x<t_oldend.m_x)t_newend.m_x=t_oldend.m_x;
    if(t_newend.m_y<t_oldend.m_y)t_newend.m_y=t_oldend.m_y;
    
    /* active the area redraw */
    t_event.type=GDK_EXPOSE;
    ((GdkEventExpose*)&t_event)->window=((GtkWidget* )arg)->window;
    ((GdkEventExpose*)&t_event)->send_event=TRUE;
    ((GdkEventExpose*)&t_event)->area.x=t_newstart.m_x*BRICKWIDTH;
    ((GdkEventExpose*)&t_event)->area.y=t_newstart.m_y*BRICKHEIGHT;
    ((GdkEventExpose*)&t_event)->area.width=(t_newend.m_x-t_newstart.m_x+1)*BRICKWIDTH;
    ((GdkEventExpose*)&t_event)->area.height=(t_newend.m_y-t_newstart.m_y+1)*BRICKHEIGHT;
    ((GdkEventExpose*)&t_event)->region=gdk_region_rectangle(&((GdkEventExpose*)&t_event)->area);
    ((GdkEventExpose*)&t_event)->count=0;
      gdk_event_put(&t_event);
  }
}

/* timer handle. Send a down arrow key event every 500ms */
void HandleTimer(int arg)
{
  GdkEvent t_event;
  t_event.type=GDK_KEY_PRESS;
  ((GdkEventKey*)&t_event)->window=((GtkWidget* )window)->window;
  ((GdkEventKey*)&t_event)->send_event=TRUE;
  ((GdkEventKey*)&t_event)->time=0;
  ((GdkEventKey*)&t_event)->state=0;
  ((GdkEventKey*)&t_event)->keyval=GDK_Down;
  ((GdkEventKey*)&t_event)->length=4;
  ((GdkEventKey*)&t_event)->string="Down";
  ((GdkEventKey*)&t_event)->hardware_keycode=0;
  ((GdkEventKey*)&t_event)->group=0;
  gdk_event_put(&t_event);
}

int 
main(int argc, char*argv[])
{
	GtkWidget *gameclient, *box,*boxh, *button,*toolbar,*tempwin;
	GdkWindowHints geo_mask;
	GdkGeometry geometry;
	static char times[4][26]=
	{ "Unset\n","Unset\n","Unset\n","Unset\n"};

	/* init the global matrix for game client */
	//	g_RBrick[0][0]=1;g_RBrick[1][1]=1;g_RBrick[8][10]=1;

	/* init the global brick candidate*/
	g_candidateblock[0].m_index=0;
	g_candidateblock[0].m_brickblock[0].m_brickpos[0].m_x=0;
	g_candidateblock[0].m_brickblock[0].m_brickpos[0].m_y=1;
	g_candidateblock[0].m_brickblock[0].m_brickpos[1].m_x=1;
	g_candidateblock[0].m_brickblock[0].m_brickpos[1].m_y=1;
	g_candidateblock[0].m_brickblock[0].m_brickpos[2].m_x=2;
	g_candidateblock[0].m_brickblock[0].m_brickpos[2].m_y=1;
	g_candidateblock[0].m_brickblock[0].m_brickpos[3].m_x=1;
	g_candidateblock[0].m_brickblock[0].m_brickpos[3].m_y=0;
	g_candidateblock[0].m_brickblock[0].m_areastartpos.m_x=0;
	g_candidateblock[0].m_brickblock[0].m_areastartpos.m_y=0;
	g_candidateblock[0].m_brickblock[0].m_areaendpos.m_x=2;
	g_candidateblock[0].m_brickblock[0].m_areaendpos.m_y=1;

	g_candidateblock[0].m_brickblock[1].m_brickpos[0].m_x=0;
	g_candidateblock[0].m_brickblock[1].m_brickpos[0].m_y=0;
	g_candidateblock[0].m_brickblock[1].m_brickpos[1].m_x=0;
	g_candidateblock[0].m_brickblock[1].m_brickpos[1].m_y=1;
	g_candidateblock[0].m_brickblock[1].m_brickpos[2].m_x=0;
	g_candidateblock[0].m_brickblock[1].m_brickpos[2].m_y=2;
	g_candidateblock[0].m_brickblock[1].m_brickpos[3].m_x=1;
	g_candidateblock[0].m_brickblock[1].m_brickpos[3].m_y=1;
	g_candidateblock[0].m_brickblock[1].m_areastartpos.m_x=0;
	g_candidateblock[0].m_brickblock[1].m_areastartpos.m_y=0;
	g_candidateblock[0].m_brickblock[1].m_areaendpos.m_x=1;
	g_candidateblock[0].m_brickblock[1].m_areaendpos.m_y=2;

	g_candidateblock[0].m_brickblock[2].m_brickpos[0].m_x=0;
	g_candidateblock[0].m_brickblock[2].m_brickpos[0].m_y=0;
	g_candidateblock[0].m_brickblock[2].m_brickpos[1].m_x=1;
	g_candidateblock[0].m_brickblock[2].m_brickpos[1].m_y=0;
	g_candidateblock[0].m_brickblock[2].m_brickpos[2].m_x=2;
	g_candidateblock[0].m_brickblock[2].m_brickpos[2].m_y=0;
	g_candidateblock[0].m_brickblock[2].m_brickpos[3].m_x=1;
	g_candidateblock[0].m_brickblock[2].m_brickpos[3].m_y=1;
	g_candidateblock[0].m_brickblock[2].m_areastartpos.m_x=0;
	g_candidateblock[0].m_brickblock[2].m_areastartpos.m_y=0;
	g_candidateblock[0].m_brickblock[2].m_areaendpos.m_x=2;
	g_candidateblock[0].m_brickblock[2].m_areaendpos.m_y=1;

	g_candidateblock[0].m_brickblock[3].m_brickpos[0].m_x=1;
	g_candidateblock[0].m_brickblock[3].m_brickpos[0].m_y=0;
	g_candidateblock[0].m_brickblock[3].m_brickpos[1].m_x=1;
	g_candidateblock[0].m_brickblock[3].m_brickpos[1].m_y=1;
	g_candidateblock[0].m_brickblock[3].m_brickpos[2].m_x=1;
	g_candidateblock[0].m_brickblock[3].m_brickpos[2].m_y=2;
	g_candidateblock[0].m_brickblock[3].m_brickpos[3].m_x=0;
	g_candidateblock[0].m_brickblock[3].m_brickpos[3].m_y=1;
	g_candidateblock[0].m_brickblock[3].m_areastartpos.m_x=0;
	g_candidateblock[0].m_brickblock[3].m_areastartpos.m_y=0;
	g_candidateblock[0].m_brickblock[3].m_areaendpos.m_x=1;
	g_candidateblock[0].m_brickblock[3].m_areaendpos.m_y=2;

	/* brick block 2 */
	g_candidateblock[1].m_index=0;
	g_candidateblock[1].m_brickblock[0].m_brickpos[0].m_x=0;
	g_candidateblock[1].m_brickblock[0].m_brickpos[0].m_y=0;
	g_candidateblock[1].m_brickblock[0].m_brickpos[1].m_x=1;
	g_candidateblock[1].m_brickblock[0].m_brickpos[1].m_y=0;
	g_candidateblock[1].m_brickblock[0].m_brickpos[2].m_x=2;
	g_candidateblock[1].m_brickblock[0].m_brickpos[2].m_y=0;
	g_candidateblock[1].m_brickblock[0].m_brickpos[3].m_x=3;
	g_candidateblock[1].m_brickblock[0].m_brickpos[3].m_y=0;
	g_candidateblock[1].m_brickblock[0].m_areastartpos.m_x=0;
	g_candidateblock[1].m_brickblock[0].m_areastartpos.m_y=0;
	g_candidateblock[1].m_brickblock[0].m_areaendpos.m_x=3;
	g_candidateblock[1].m_brickblock[0].m_areaendpos.m_y=0;

	g_candidateblock[1].m_brickblock[1].m_brickpos[0].m_x=0;
	g_candidateblock[1].m_brickblock[1].m_brickpos[0].m_y=0;
	g_candidateblock[1].m_brickblock[1].m_brickpos[1].m_x=0;
	g_candidateblock[1].m_brickblock[1].m_brickpos[1].m_y=1;
	g_candidateblock[1].m_brickblock[1].m_brickpos[2].m_x=0;
	g_candidateblock[1].m_brickblock[1].m_brickpos[2].m_y=2;
	g_candidateblock[1].m_brickblock[1].m_brickpos[3].m_x=0;
	g_candidateblock[1].m_brickblock[1].m_brickpos[3].m_y=3;
	g_candidateblock[1].m_brickblock[1].m_areastartpos.m_x=0;
	g_candidateblock[1].m_brickblock[1].m_areastartpos.m_y=0;
	g_candidateblock[1].m_brickblock[1].m_areaendpos.m_x=0;
	g_candidateblock[1].m_brickblock[1].m_areaendpos.m_y=3;

	g_candidateblock[1].m_brickblock[2].m_brickpos[0].m_x=0;
	g_candidateblock[1].m_brickblock[2].m_brickpos[0].m_y=0;
	g_candidateblock[1].m_brickblock[2].m_brickpos[1].m_x=1;
	g_candidateblock[1].m_brickblock[2].m_brickpos[1].m_y=0;
	g_candidateblock[1].m_brickblock[2].m_brickpos[2].m_x=2;
	g_candidateblock[1].m_brickblock[2].m_brickpos[2].m_y=0;
	g_candidateblock[1].m_brickblock[2].m_brickpos[3].m_x=3;
	g_candidateblock[1].m_brickblock[2].m_brickpos[3].m_y=0;
	g_candidateblock[1].m_brickblock[2].m_areastartpos.m_x=0;
	g_candidateblock[1].m_brickblock[2].m_areastartpos.m_y=0;
	g_candidateblock[1].m_brickblock[2].m_areaendpos.m_x=3;
	g_candidateblock[1].m_brickblock[2].m_areaendpos.m_y=0;

	g_candidateblock[1].m_brickblock[3].m_brickpos[0].m_x=0;
	g_candidateblock[1].m_brickblock[3].m_brickpos[0].m_y=0;
	g_candidateblock[1].m_brickblock[3].m_brickpos[1].m_x=0;
	g_candidateblock[1].m_brickblock[3].m_brickpos[1].m_y=1;
	g_candidateblock[1].m_brickblock[3].m_brickpos[2].m_x=0;
	g_candidateblock[1].m_brickblock[3].m_brickpos[2].m_y=2;
	g_candidateblock[1].m_brickblock[3].m_brickpos[3].m_x=0;
	g_candidateblock[1].m_brickblock[3].m_brickpos[3].m_y=3;
	g_candidateblock[1].m_brickblock[3].m_areastartpos.m_x=0;
	g_candidateblock[1].m_brickblock[3].m_areastartpos.m_y=0;
	g_candidateblock[1].m_brickblock[3].m_areaendpos.m_x=0;
	g_candidateblock[1].m_brickblock[3].m_areaendpos.m_y=3;

	/* brick block 3 */
	g_candidateblock[2].m_index=0;
	g_candidateblock[2].m_brickblock[0].m_brickpos[0].m_x=0;
	g_candidateblock[2].m_brickblock[0].m_brickpos[0].m_y=1;
	g_candidateblock[2].m_brickblock[0].m_brickpos[1].m_x=1;
	g_candidateblock[2].m_brickblock[0].m_brickpos[1].m_y=1;
	g_candidateblock[2].m_brickblock[0].m_brickpos[2].m_x=2;
	g_candidateblock[2].m_brickblock[0].m_brickpos[2].m_y=1;
	g_candidateblock[2].m_brickblock[0].m_brickpos[3].m_x=2;
	g_candidateblock[2].m_brickblock[0].m_brickpos[3].m_y=0;
	g_candidateblock[2].m_brickblock[0].m_areastartpos.m_x=0;
	g_candidateblock[2].m_brickblock[0].m_areastartpos.m_y=0;
	g_candidateblock[2].m_brickblock[0].m_areaendpos.m_x=2;
	g_candidateblock[2].m_brickblock[0].m_areaendpos.m_y=1;

	g_candidateblock[2].m_brickblock[1].m_brickpos[0].m_x=0;
	g_candidateblock[2].m_brickblock[1].m_brickpos[0].m_y=0;
	g_candidateblock[2].m_brickblock[1].m_brickpos[1].m_x=0;
	g_candidateblock[2].m_brickblock[1].m_brickpos[1].m_y=1;
	g_candidateblock[2].m_brickblock[1].m_brickpos[2].m_x=0;
	g_candidateblock[2].m_brickblock[1].m_brickpos[2].m_y=2;
	g_candidateblock[2].m_brickblock[1].m_brickpos[3].m_x=1;
	g_candidateblock[2].m_brickblock[1].m_brickpos[3].m_y=2;
	g_candidateblock[2].m_brickblock[1].m_areastartpos.m_x=0;
	g_candidateblock[2].m_brickblock[1].m_areastartpos.m_y=0;
	g_candidateblock[2].m_brickblock[1].m_areaendpos.m_x=1;
	g_candidateblock[2].m_brickblock[1].m_areaendpos.m_y=2;

	g_candidateblock[2].m_brickblock[2].m_brickpos[0].m_x=0;
	g_candidateblock[2].m_brickblock[2].m_brickpos[0].m_y=0;
	g_candidateblock[2].m_brickblock[2].m_brickpos[1].m_x=1;
	g_candidateblock[2].m_brickblock[2].m_brickpos[1].m_y=0;
	g_candidateblock[2].m_brickblock[2].m_brickpos[2].m_x=2;
	g_candidateblock[2].m_brickblock[2].m_brickpos[2].m_y=0;
	g_candidateblock[2].m_brickblock[2].m_brickpos[3].m_x=0;
	g_candidateblock[2].m_brickblock[2].m_brickpos[3].m_y=1;
	g_candidateblock[2].m_brickblock[2].m_areastartpos.m_x=0;
	g_candidateblock[2].m_brickblock[2].m_areastartpos.m_y=0;
	g_candidateblock[2].m_brickblock[2].m_areaendpos.m_x=2;
	g_candidateblock[2].m_brickblock[2].m_areaendpos.m_y=1;

	g_candidateblock[2].m_brickblock[3].m_brickpos[0].m_x=0;
	g_candidateblock[2].m_brickblock[3].m_brickpos[0].m_y=0;
	g_candidateblock[2].m_brickblock[3].m_brickpos[1].m_x=1;
	g_candidateblock[2].m_brickblock[3].m_brickpos[1].m_y=0;
	g_candidateblock[2].m_brickblock[3].m_brickpos[2].m_x=1;
	g_candidateblock[2].m_brickblock[3].m_brickpos[2].m_y=1;
	g_candidateblock[2].m_brickblock[3].m_brickpos[3].m_x=1;
	g_candidateblock[2].m_brickblock[3].m_brickpos[3].m_y=2;
	g_candidateblock[2].m_brickblock[3].m_areastartpos.m_x=0;
	g_candidateblock[2].m_brickblock[3].m_areastartpos.m_y=0;
	g_candidateblock[2].m_brickblock[3].m_areaendpos.m_x=1;
	g_candidateblock[2].m_brickblock[3].m_areaendpos.m_y=2;

	/* brick 4 */
	g_candidateblock[3].m_index=0;
	g_candidateblock[3].m_brickblock[0].m_brickpos[0].m_x=0;
	g_candidateblock[3].m_brickblock[0].m_brickpos[0].m_y=0;
	g_candidateblock[3].m_brickblock[0].m_brickpos[1].m_x=1;
	g_candidateblock[3].m_brickblock[0].m_brickpos[1].m_y=0;
	g_candidateblock[3].m_brickblock[0].m_brickpos[2].m_x=0;
	g_candidateblock[3].m_brickblock[0].m_brickpos[2].m_y=1;
	g_candidateblock[3].m_brickblock[0].m_brickpos[3].m_x=1;
	g_candidateblock[3].m_brickblock[0].m_brickpos[3].m_y=1;
	g_candidateblock[3].m_brickblock[0].m_areastartpos.m_x=0;
	g_candidateblock[3].m_brickblock[0].m_areastartpos.m_y=0;
	g_candidateblock[3].m_brickblock[0].m_areaendpos.m_x=1;
	g_candidateblock[3].m_brickblock[0].m_areaendpos.m_y=1;

	g_candidateblock[3].m_brickblock[1].m_brickpos[0].m_x=0;
	g_candidateblock[3].m_brickblock[1].m_brickpos[0].m_y=0;
	g_candidateblock[3].m_brickblock[1].m_brickpos[1].m_x=0;
	g_candidateblock[3].m_brickblock[1].m_brickpos[1].m_y=1;
	g_candidateblock[3].m_brickblock[1].m_brickpos[2].m_x=1;
	g_candidateblock[3].m_brickblock[1].m_brickpos[2].m_y=0;
	g_candidateblock[3].m_brickblock[1].m_brickpos[3].m_x=1;
	g_candidateblock[3].m_brickblock[1].m_brickpos[3].m_y=1;
	g_candidateblock[3].m_brickblock[1].m_areastartpos.m_x=0;
	g_candidateblock[3].m_brickblock[1].m_areastartpos.m_y=0;
	g_candidateblock[3].m_brickblock[1].m_areaendpos.m_x=1;
	g_candidateblock[3].m_brickblock[1].m_areaendpos.m_y=1;

	g_candidateblock[3].m_brickblock[2].m_brickpos[0].m_x=0;
	g_candidateblock[3].m_brickblock[2].m_brickpos[0].m_y=0;
	g_candidateblock[3].m_brickblock[2].m_brickpos[1].m_x=1;
	g_candidateblock[3].m_brickblock[2].m_brickpos[1].m_y=0;
	g_candidateblock[3].m_brickblock[2].m_brickpos[2].m_x=0;
	g_candidateblock[3].m_brickblock[2].m_brickpos[2].m_y=1;
	g_candidateblock[3].m_brickblock[2].m_brickpos[3].m_x=1;
	g_candidateblock[3].m_brickblock[2].m_brickpos[3].m_y=1;
	g_candidateblock[3].m_brickblock[2].m_areastartpos.m_x=0;
	g_candidateblock[3].m_brickblock[2].m_areastartpos.m_y=0;
	g_candidateblock[3].m_brickblock[2].m_areaendpos.m_x=1;
	g_candidateblock[3].m_brickblock[2].m_areaendpos.m_y=1;

	g_candidateblock[3].m_brickblock[3].m_brickpos[0].m_x=0;
	g_candidateblock[3].m_brickblock[3].m_brickpos[0].m_y=0;
	g_candidateblock[3].m_brickblock[3].m_brickpos[1].m_x=0;
	g_candidateblock[3].m_brickblock[3].m_brickpos[1].m_y=1;
	g_candidateblock[3].m_brickblock[3].m_brickpos[2].m_x=1;
	g_candidateblock[3].m_brickblock[3].m_brickpos[2].m_y=0;
	g_candidateblock[3].m_brickblock[3].m_brickpos[3].m_x=1;
	g_candidateblock[3].m_brickblock[3].m_brickpos[3].m_y=1;
	g_candidateblock[3].m_brickblock[3].m_areastartpos.m_x=0;
	g_candidateblock[3].m_brickblock[3].m_areastartpos.m_y=0;
	g_candidateblock[3].m_brickblock[3].m_areaendpos.m_x=1;
	g_candidateblock[3].m_brickblock[3].m_areaendpos.m_y=1;

	/* brick 5 */
	g_candidateblock[4].m_index=0;
	g_candidateblock[4].m_brickblock[0].m_brickpos[0].m_x=0;
	g_candidateblock[4].m_brickblock[0].m_brickpos[0].m_y=0;
	g_candidateblock[4].m_brickblock[0].m_brickpos[1].m_x=0;
	g_candidateblock[4].m_brickblock[0].m_brickpos[1].m_y=1;
	g_candidateblock[4].m_brickblock[0].m_brickpos[2].m_x=1;
	g_candidateblock[4].m_brickblock[0].m_brickpos[2].m_y=1;
	g_candidateblock[4].m_brickblock[0].m_brickpos[3].m_x=2;
	g_candidateblock[4].m_brickblock[0].m_brickpos[3].m_y=1;
	g_candidateblock[4].m_brickblock[0].m_areastartpos.m_x=0;
	g_candidateblock[4].m_brickblock[0].m_areastartpos.m_y=0;
	g_candidateblock[4].m_brickblock[0].m_areaendpos.m_x=2;
	g_candidateblock[4].m_brickblock[0].m_areaendpos.m_y=1;

	g_candidateblock[4].m_brickblock[1].m_brickpos[0].m_x=0;
	g_candidateblock[4].m_brickblock[1].m_brickpos[0].m_y=0;
	g_candidateblock[4].m_brickblock[1].m_brickpos[1].m_x=0;
	g_candidateblock[4].m_brickblock[1].m_brickpos[1].m_y=1;
	g_candidateblock[4].m_brickblock[1].m_brickpos[2].m_x=0;
	g_candidateblock[4].m_brickblock[1].m_brickpos[2].m_y=2;
	g_candidateblock[4].m_brickblock[1].m_brickpos[3].m_x=1;
	g_candidateblock[4].m_brickblock[1].m_brickpos[3].m_y=0;
	g_candidateblock[4].m_brickblock[1].m_areastartpos.m_x=0;
	g_candidateblock[4].m_brickblock[1].m_areastartpos.m_y=0;
	g_candidateblock[4].m_brickblock[1].m_areaendpos.m_x=1;
	g_candidateblock[4].m_brickblock[1].m_areaendpos.m_y=2;

	g_candidateblock[4].m_brickblock[2].m_brickpos[0].m_x=0;
	g_candidateblock[4].m_brickblock[2].m_brickpos[0].m_y=0;
	g_candidateblock[4].m_brickblock[2].m_brickpos[1].m_x=1;
	g_candidateblock[4].m_brickblock[2].m_brickpos[1].m_y=0;
	g_candidateblock[4].m_brickblock[2].m_brickpos[2].m_x=2;
	g_candidateblock[4].m_brickblock[2].m_brickpos[2].m_y=0;
	g_candidateblock[4].m_brickblock[2].m_brickpos[3].m_x=2;
	g_candidateblock[4].m_brickblock[2].m_brickpos[3].m_y=1;
	g_candidateblock[4].m_brickblock[2].m_areastartpos.m_x=0;
	g_candidateblock[4].m_brickblock[2].m_areastartpos.m_y=0;
	g_candidateblock[4].m_brickblock[2].m_areaendpos.m_x=2;
	g_candidateblock[4].m_brickblock[2].m_areaendpos.m_y=1;

	g_candidateblock[4].m_brickblock[3].m_brickpos[0].m_x=1;
	g_candidateblock[4].m_brickblock[3].m_brickpos[0].m_y=0;
	g_candidateblock[4].m_brickblock[3].m_brickpos[1].m_x=1;
	g_candidateblock[4].m_brickblock[3].m_brickpos[1].m_y=1;
	g_candidateblock[4].m_brickblock[3].m_brickpos[2].m_x=1;
	g_candidateblock[4].m_brickblock[3].m_brickpos[2].m_y=2;
	g_candidateblock[4].m_brickblock[3].m_brickpos[3].m_x=0;
	g_candidateblock[4].m_brickblock[3].m_brickpos[3].m_y=2;
	g_candidateblock[4].m_brickblock[3].m_areastartpos.m_x=0;
	g_candidateblock[4].m_brickblock[3].m_areastartpos.m_y=0;
	g_candidateblock[4].m_brickblock[3].m_areaendpos.m_x=1;
	g_candidateblock[4].m_brickblock[3].m_areaendpos.m_y=2;

	/* end of init brick */

	gtk_set_locale();
	gtk_init(&argc,&argv);

	window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
	//	window=gtk_dialog_new();
	gtk_window_set_title(window,"GYZ Player");
	gtk_window_set_position(window,GTK_WIN_POS_CENTER);
	geometry.min_width=300;
	geometry.min_height=200;
	geo_mask=GDK_HINT_MIN_SIZE;
	gtk_window_set_geometry_hints(window,window,&geometry,geo_mask);

	gtk_signal_connect(GTK_OBJECT(window),"destroy",
		GTK_SIGNAL_FUNC(PrintAndExit),times);
	
	//	gtk_window_set_title(GTK_WINDOW(window),"Signals 1");
	gtk_container_border_width(GTK_CONTAINER(window),0);
	// add by GYZ, to set the window's GDKWindow item
	gtk_widget_show(window);

	box=gtk_vbox_new(FALSE,0);
	gtk_container_add(GTK_CONTAINER(window),box);



	// create the toolbar by GYZ
	toolbar=gtk_toolbar_new();
	gRedPmap=NewPixmap("red.xpm",window->window,
			   &window->style->bg[GTK_STATE_NORMAL]);
	gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),"Red",
				"Draw red square", NULL, gRedPmap,
				(GtkSignalFunc) SetColor, (gpointer) RED);
	gGreenPmap=NewPixmap("green.xpm", window->window,
		&window->style->bg[GTK_STATE_NORMAL]),
		gtk_toolbar_append_item(GTK_TOOLBAR(toolbar), "Green",
		"Draw green square", NULL, gGreenPmap,
		(GtkSignalFunc) SetColor, (gpointer) GREEN);
	gBluePmap=NewPixmap("blue.xpm",window->window,
		&window->style->bg[GTK_STATE_NORMAL]),
		gtk_toolbar_append_item(GTK_TOOLBAR(toolbar),"Blue",
		"Draw blue square", NULL, gBluePmap,
		(GtkSignalFunc) SetColor, (gpointer) BLUE);

	gtk_box_pack_start(box, toolbar,FALSE,TRUE,0);


	// create the horizen box by GYZ
	boxh=gtk_hbox_new(FALSE,0);
	gtk_box_pack_start(box, boxh,TRUE,TRUE,0);


	// add drawing area
	gameclient=gtk_drawing_area_new();
	gtk_drawing_area_size(GTK_DRAWING_AREA(gameclient),180,260);
	//try to set the size of drawing area
	geometry.min_width=180;
	geometry.min_height=260;
	geo_mask=GDK_HINT_MIN_SIZE|GDK_HINT_MAX_SIZE;
	gtk_window_set_geometry_hints(window,gameclient,&geometry,geo_mask);

	/* Set the event mask and handle for the drawing area
	 */
	gtk_signal_connect(GTK_OBJECT(gameclient),"expose-event",
		GTK_SIGNAL_FUNC(HandleExpose), NULL);
	gtk_signal_connect(GTK_OBJECT(gameclient),"key-press-event",
			   GTK_SIGNAL_FUNC(KeyPress),(void*)gameclient);
	gtk_widget_set_events(gameclient,
		GDK_EXPOSURE_MASK|GDK_KEY_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(window),"key-press-event",
			   GTK_SIGNAL_FUNC(KeyPress),(void*)gameclient);
	gtk_widget_set_events(window,GDK_KEY_PRESS_MASK);


	gtk_box_pack_start(GTK_BOX(boxh),gameclient,TRUE,TRUE,0);

	// end of addition by GYZ
	box=gtk_vbox_new(FALSE,0);
	//	gtk_container_add(GTK_CONTAINER(window),box);
	gtk_box_pack_start(GTK_BOX(boxh),box,TRUE,TRUE,0);

	button=gtk_button_new_with_label("Update 0");
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
		GTK_SIGNAL_FUNC(Update),&times[0]);
	gtk_box_pack_start(GTK_BOX(box),button,TRUE,TRUE,0);

	button=gtk_button_new_with_label("Update 1");
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
		GTK_SIGNAL_FUNC(Update),&times[1]);
	gtk_box_pack_start(GTK_BOX(box),button,TRUE,TRUE,0);

	button=gtk_button_new_with_label("Update 2");
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
		GTK_SIGNAL_FUNC(Update),&times[2]);
	gtk_box_pack_start(GTK_BOX(box),button,TRUE,TRUE,0);

	button=gtk_button_new_with_label("Update 3");
	gtk_signal_connect(GTK_OBJECT(button),"clicked",
		GTK_SIGNAL_FUNC(Update),&times[3]);
	gtk_box_pack_start(GTK_BOX(box),button,TRUE,TRUE,0);


	gtk_widget_show_all(window);

	/* Set timer */
	struct itimerval m_newtimeval,m_oldtimeval;
	
	m_newtimeval.it_value.tv_sec=1;
	m_newtimeval.it_value.tv_usec=00000;
	m_newtimeval.it_interval.tv_sec=1;
	m_newtimeval.it_interval.tv_usec=00000;
	signal(SIGALRM, HandleTimer);
	setitimer(ITIMER_REAL,&m_newtimeval,&m_oldtimeval);

	gtk_main();

	return(0);
}
