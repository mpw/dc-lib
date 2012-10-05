// =====================================================================
//  dc/test/qt/ROthello.cpp
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 15, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:04:10 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.14 $

//INCLUDES...

#include <stdio.h>
#include <stdlib.h>

#include <QApplication>
#include <QMouseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBrush>
#include <QGraphicsEllipseItem>
#include <QThread>
#include <QEvent>
#include <QPoint>

#include <sys/time.h>


#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "dc.h"
#include "profile.h"
#include "dc_profile.h"

#include "ROthello.h"

//DEFINES...

#define N 20
#define dimX 800
#define dimY 700

#define numIter N*N

//STATS STUFF...

time_rec_t begin_t, inter_t, end_t;
elapsed_time_t inter_array[numIter];
elapsed_time_t log_array[numIter];
dc_profile log_array_prof[numIter];
unsigned iter = 0, status = 0;

//TYPES...

//PROTOTYPES...

void GeneralConstraint (void *);
void FinalConstraint (void *);

void _printBoard ();
int _whiteMove (int, int);
int _blackMove (int, int);

//GLOBALS...

//active player (black moves first)...
int gWhoseMove=-1;

//setup flag...
int gSetup=1;

//flipped flag...
int gFlipped=0;

//board...
int *v;

//chips...
QGraphicsEllipseItem* ww[N][N];

//text (white move, black move)...
QGraphicsSimpleTextItem *blackST, *whiteST;

//Qt Graphics View details...
QGraphicsScene* myGraphicsScene;
QGraphicsView* myGraphicsView;

//main widget...
MyWidget *w;

//CLASSES...

class MyApplication : public QApplication
{
    public:
        MyApplication (int argc, char** argv);
        bool notify (QObject* receiver, QEvent* e);
};

class MyThread : public QThread
{
    public:
        void run();
};

//rec stuff...
int recording=0;
FILE *g_rec_file;
MyThread *replayThread;
#define REPLAY_DELAY_MICROSECS 250000

//METHODS...

void MyThread::run()
{
    int x, y, gx, gy;
    QMouseEvent *myEvent;
    
    usleep(REPLAY_DELAY_MICROSECS); 
    
    g_rec_file=fopen ("ROthello_clickstream.txt", "r");
    
    fscanf (g_rec_file, "%d", &x);
    fscanf (g_rec_file, "%d", &y);
    fscanf (g_rec_file, "%d", &gx);
    fscanf (g_rec_file, "%d", &gy);
    while (!feof (g_rec_file))
    {    
        //printf ("%d %d %d %d\n", x, y, gx, gy);
        
        myEvent=new QMouseEvent (QEvent::MouseButtonPress, QPoint (x, y), QPoint (gx, gy), Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
        //printf ("event created!\n");
        QCoreApplication::postEvent(w, myEvent);
        //w->mousePressEvent (myEvent);
        
        //myEvent=new QMouseEvent (QEvent::MouseButtonRelease, *myPoint, Qt::LeftButton, Qt::NoButton, Qt::NoModifier);        
        //QCoreApplication::postEvent(w, myEvent);        
        //w->mousePressEvent (myEvent);
       
        fscanf (g_rec_file, "%d", &x);
        fscanf (g_rec_file, "%d", &y);
        fscanf (g_rec_file, "%d", &gx);
        fscanf (g_rec_file, "%d", &gy);
        usleep(REPLAY_DELAY_MICROSECS);
    }
}

MyApplication::MyApplication (int argc, char** argv)
    : QApplication (argc, argv)
{

}

bool MyApplication::notify (QObject* receiver, QEvent* e)
{    
    /*if (receiver==qApp)
        printf ("Receiver: qApp - ");
    else if (receiver==myGraphicsScene)
        printf ("Receiver: myGraphicsScene - ");
    else if (receiver==myGraphicsView)
        printf ("Receiver: myGraphicsView - ");
    else if (receiver==w)
        printf ("Receiver: w - ");
    else
	printf ("Receiver: %p - ", receiver);
    
    printf ("Event: %d\n", e->type ()); */
    
    //saves evt type...
    int evt_type = e->type ();
    
    //if mouse clicked...
    /*if (evt_type==2)
    {
        printf (">> begin move #%d\n", iter);
        
        //fetches initial times...
        get_time (&begin_t);
        
        //sets status...
        status=1;
    }*/
    
    //if completion event...
    if (e->type () == 1000)
    {        
        //fetches final times...
        if (iter < numIter) {
            get_time (&end_t);
    
            compute_elapsed_time(&begin_t, &inter_t, &inter_array[iter]);
            compute_elapsed_time(&begin_t, &end_t, &log_array[iter]);
    
            if (dc_profile_on()) 
                dc_fetch_profile_info(&log_array_prof[iter]);
        }
        
        printf ("<< end move #%d\n", iter);
        
        iter++;
    }
    
    //standard event processing...
    bool res=QApplication::notify (receiver, e);
    
    //if event WAS 'paint' following 'mouseclick'...
    if (e->type () == 12 && status==1) 
    {
        //resets status...
        status=0;
        
        //schedules completion event...
        QEvent* dummy = new QEvent(QEvent::User);
        QCoreApplication::postEvent(w, dummy);
    }
    
    return res;
}

void MyWidget::mousePressEvent (QMouseEvent *e)
{
    //printf ("w mousepress handler here!\n");
    
	//fetches initial time...
	//gettimeofday (&tvMousePressBegin, NULL);
	
	if (e->button ()==Qt::LeftButton)
	{
        printf (">> begin move #%d\n", iter);
        
        //fetches initial times...
        get_time (&begin_t);
        
        //sets status...
        status=1;
        
        //if recording mousepress events...
        if (recording==1)
        {
            fprintf (g_rec_file, "%d %d %d %d\n", e->x (), e->y (), e->globalX (), e->globalY ());
            fflush (g_rec_file);
        }
        
		//printf ("X = %d - Y = %d\n", (e->x ()-50)/30, (e->y ()-50)/30);
		
		if (gWhoseMove==-1)
		{
			//if move is allowed...
			if (0==_blackMove ((e->x ()-50)/30, (e->y ()-50)/30))
			{
				gWhoseMove=1;
				
                blackST->hide ();
                whiteST->show ();
			}
		}
		else
		{
			//if move is allowed...
			if (0==_whiteMove ((e->x ()-50)/30, (e->y ()-50)/30))
			{
				gWhoseMove=-1;
				
                whiteST->hide ();
                blackST->show ();
			}
		}
		
		get_time (&inter_t);
	}
	
	//fetches final time...
	//gettimeofday (&tvMousePressEnd, NULL);
	//gMousePressTimes[gNumEvt]=tvMousePressEnd.tv_sec+(tvMousePressEnd.tv_usec*0.000001)
	//		-tvMousePressBegin.tv_sec-(tvMousePressBegin.tv_usec*0.000001);
	//gNumEvt++;
}

MyWidget::MyWidget (QWidget *parent)
	: QMainWindow (parent)
{	
	int i, j;
	
	//installs menus...
	QAction *pass=new QAction ("&Pass", this);
    pass->setShortcut (tr ("CTRL+P"));
    QAction *quit=new QAction ("&Quit", this);
    quit->setShortcut (tr ("CTRL+Q"));
    
    QAction *record=new QAction ("&Record", this);
    record->setShortcut (tr ("CTRL+R"));
    QAction *replay=new QAction ("Repla&y", this);
    replay->setShortcut (tr ("CTRL+Y"));
    
    QMenu *file;
    file=menuBar()->addMenu ("&File");
    file->addAction (pass);
    file->addSeparator ();
    file->addAction (record);
    file->addAction (replay);
    file->addSeparator ();
    file->addAction (quit);
    
    connect (pass, SIGNAL(triggered ()), this, SLOT (pass ()));
    connect (quit, SIGNAL(triggered ()), qApp, SLOT (quit ()));
    
    connect (record, SIGNAL(triggered ()), this, SLOT (record ()));
    connect (replay, SIGNAL(triggered ()), this, SLOT (replay ()));
	
	//sets main window size...
	this->setGeometry (50, 50, dimX, dimY);
	
	//Graphics View details...
    myGraphicsScene=new QGraphicsScene (this);
    myGraphicsScene->setSceneRect (0, 0, this->width(), this->height());
    myGraphicsScene->setBackgroundBrush (QBrush (QColor (0, 128, 0), Qt::SolidPattern));
    
    blackST=new QGraphicsSimpleTextItem (0);
    blackST->setPos (50.0, 400.0);
    blackST->setText ("Black move");
    blackST->setBrush (Qt::black);
    myGraphicsScene->addItem (blackST);
    blackST->show ();
    
    whiteST=new QGraphicsSimpleTextItem ();
    whiteST->setPos (50.0, 400.0);
    whiteST->setText ("White move");
    whiteST->setBrush (Qt::white);
    whiteST->hide ();
    myGraphicsScene->addItem (whiteST);
	
	for (i=0; i<N; i++)
		for (j=0; j<N; j++)
		{
			QGraphicsRectItem* theRectItem=new QGraphicsRectItem (i*30+50, j*30+50, 30, 30, 0);
            myGraphicsScene->addItem (theRectItem);
            theRectItem->show ();
		}
		
	myGraphicsView=new QGraphicsView (myGraphicsScene, this);
    myGraphicsView->resize (this->width(), this->height()-23);
    myGraphicsView->move (0, 23);
	

}

void MyWidget::record ()
{
    if (recording==1)
        return;
    
    g_rec_file=fopen ("ROthello_clickstream.txt", "w");
    
    printf ("Recording...\n");
    
    recording=1;
}

void MyWidget::replay ()
{
    printf ("Replaying...\n");
    
    replayThread->start ();
}

void MyWidget::pass ()
{
	//printf ("Pass...\n");
	
	if (gWhoseMove==-1)
    {
        gWhoseMove=1;
        blackST->hide ();
        whiteST->show ();
    }
    else
    {
        gWhoseMove=-1;
        whiteST->hide ();
        blackST->show ();
    }
    
    myGraphicsScene->update ();
}

//MAIN...

int main( int argc, char **argv )
{
	int i=0;
	/*double avgAct=0.0;
	int maxAct=0;
	double avgFinalAct=0.0;
	int maxFinalAct=0;
	double avgTime=0.0;
	double maxTime=0.0;*/
	
	dc_init ();
	
	//allocates vector...
	// max N is 16...
	//v=(int *)dc_malloc (4096);
	//printf ("Prot page at %p\n", v);
    v=(int *)dc_malloc (N*N*sizeof (int));
	
	//QApplication a( argc, argv );
    MyApplication a( argc, argv );

	w=new MyWidget ();

	//a.setMainWidget( w );
	w->show();
    
    //printf ("w's parent: %p\n", w->parent ());
    //printf ("w: %p - w_win: %p\n", w, w->window ());
	
	//sets up board...
	_whiteMove (N/2-1, N/2-1);
	_whiteMove (N/2, N/2);
	_blackMove (N/2-1, N/2);
	_blackMove (N/2, N/2-1);
	gSetup=0;
	
	//just testing...
/*	printf ("\nInitial board:\n");
	_printBoard ();
	while (i<1)
	{
	int a, b;
	printf ("Black move:\ni: ");
	scanf ("%d", &a);
	printf ("j: ");
	scanf ("%d", &b);
	_blackMove (a, b);
	_printBoard ();
		
	printf ("White move:\ni: ");
	scanf ("%d", &a);
	printf ("j: ");
	scanf ("%d", &b);
	_whiteMove (a, b);
	_printBoard ();
	
	i++;
	}*/

    replayThread = new MyThread ();
	
	int ret_value=a.exec();
    
    delete replayThread;
		
	//frees prot page...
	dc_free (v);
    
    // dump DC profiling info
    if (dc_profile_on()) 
        dc_dump_profile_diff(stdout, 
                             &log_array_prof[0], 
                             &log_array_prof[iter-1]);
    else {

        //computes avg and max resize evt processing time...
        elapsed_time_t avg;
        avg.real=0;
        avg.user=0;
        avg.system=0;
        avg.child_user=0;
        avg.child_system=0;
        
        double max=log_array[0].real;
        for (i=0; i<iter; i++)
        {
            avg.real=avg.real+log_array[i].real;
            avg.user=avg.user+log_array[i].user;
            avg.system=avg.system+log_array[i].system;
            avg.child_user=avg.child_user+log_array[i].child_user;
            avg.child_system=avg.child_system+log_array[i].child_system;
            
            if (log_array[i].real>max)
                max=log_array[i].real;
        }

        divide_elapsed_time_by(&avg, iter);
        printf ("\nTotal times per move:\n");
        printf ("Avg real = %f\n", avg.real);
        printf ("Avg user = %f\n", avg.user);
        printf ("Avg system = %f\n", avg.system);
        printf ("Avg child_user = %f\n", avg.child_user);
        printf ("Avg child_system = %f\n", avg.child_system);
        printf ("Max real = %f\n", max);
        
        avg.real=0;
        avg.user=0;
        avg.system=0;
        avg.child_user=0;
        avg.child_system=0;
        max=inter_array[0].real;
        for (i=0; i<iter; i++)
        {
            avg.real=avg.real+inter_array[i].real;
            avg.user=avg.user+inter_array[i].user;
            avg.system=avg.system+inter_array[i].system;
            avg.child_user=avg.child_user+inter_array[i].child_user;
            avg.child_system=avg.child_system+inter_array[i].child_system;
            
            if (inter_array[i].real>max)
                max=inter_array[i].real;
        }
        
        divide_elapsed_time_by(&avg, iter);
        printf ("\nPropagation times only:\n");
        printf ("Avg real = %f\n", avg.real);
        printf ("Avg user = %f\n", avg.user);
        printf ("Avg system = %f\n", avg.system);
        printf ("Avg child_user = %f\n", avg.child_user);
        printf ("Avg child_system = %f\n", avg.child_system);
        printf ("Max real = %f\n", max);
    }
    
	return ret_value;
} 

//service routines...
/*void _printBoard ()
{
	int i, j;
	
	printf ("   ");
	for (i=0; i<N; i++)
		printf ("%d  ", i);
	printf ("\n");
	
	for (i=0; i<N; i++)
	{
		printf ("%d  ", i);
		for (j=0; j<N; j++)
		{
			if (v[i*N+j]<0)
				printf ("%d ", v[j*N+i]);
			else
				printf ("%d  ", v[j*N+i]);
		}
		printf ("\n");
	}
}*/

int _whiteMove (int i, int j)
{	
	//fails if outside board...
	if (i<0 || i>N-1 || j<0 || j>N-1)
		return -1;
	
	//we are assuming lots of things here...
	int theData=(i<<16)+j;
			
	//fails if square is already occupied...
	if (v[i*N+j]!=0)
		return -1;
	
	//fetches initial time...
	//gettimeofday (&tvBegin, NULL);
	
	//increases moves counter...
	//gNumMoves++;
	
	//resets global var...
	gFlipped=0;
	
	//fetches initial time...
	//gettimeofday (&tvSolveBegin, NULL);
	//3: conventional value for newly placed white chip...
	v[i*N+j]=3;
	//fetches final time...
	//gettimeofday (&tvSolveEnd, NULL);
	//gSolveTimes[gNumMoves]=tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
	//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
	
	//constraints adjust other chips here...
	
	//if no flips and not in setup mode...
	if (gFlipped==0 && gSetup==0)
	{
		//fetches initial time...
		//gettimeofday (&tvSolveBegin, NULL);
		//this write triggers a new constraint evaluation session...
		v[i*N+j]=0;
		//fetches final time...
		//gettimeofday (&tvSolveEnd, NULL);
		//gSolveTimes[gNumMoves]=gSolveTimes[gNumMoves]+tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
		//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
		
		//fetches final time...
		//gettimeofday (&tvEnd, NULL);
		//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
		//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
		
		return -1;
	}
	
	//fetches initial time...
	//gettimeofday (&tvSolveBegin, NULL);
	//1: conventional value for white chips...
	//this write triggers a new constraint evaluation session...
	v[i*N+j]=1;
	
	//installs new constraint...
	//LCP_NewCons (GeneralConstraint, FinalConstraint, (void *)theData); 
    dc_new_cons (GeneralConstraint, (void *)theData, NULL);
    
	//fetches final time...
	//gettimeofday (&tvSolveEnd, NULL);
	//gSolveTimes[gNumMoves]=gSolveTimes[gNumMoves]+tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
	//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
	
	//draws new chip (Qt version...)
	//w[i][j]=new QCanvasEllipse (25, 25, myCanvas);
    ww[i][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
    //w[i][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
    ww[i][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
    //w[i][j]->move (i*30+50+15, j*30+50+15);
    //w[i][j]->show ();
    ww[i][j]->setPos (i*30+50+15, j*30+50+15);
    myGraphicsScene->addItem (ww[i][j]);
    ww[i][j]->show ();  
	
	//fetches final time...
	//gettimeofday (&tvEnd, NULL);
	//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
	//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
	
	//chip placed successfully...
	return 0;
}

int _blackMove (int i, int j)
{
	//fails if outside board...
	if (i<0 || i>N-1 || j<0 || j>N-1)
		return -1;
	
	//we are assuming lots of things here...
	int theData=(i<<16)+j;
			
	//fails if square is already occupied...
	if (v[i*N+j]!=0)
		return -1;
	
	//fetches initial time...
	//gettimeofday (&tvBegin, NULL);
	
	//increases moves counter...
	//gNumMoves++;
	
	//resets global var...
	gFlipped=0;
	
	//fetches initial time...
	//gettimeofday (&tvSolveBegin, NULL);
	//-3: conventional value for newly placed white chip...
	v[i*N+j]=-3;
	//fetches final time...
	//gettimeofday (&tvSolveEnd, NULL);
	//gSolveTimes[gNumMoves]=tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
	//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
	
	//constraints adjust other chips here...
	
	//if no flips and not in setup mode...
	if (gFlipped==0 && gSetup==0)
	{
		//fetches initial time...
		//gettimeofday (&tvSolveBegin, NULL);
		//this write triggers a new constraint evaluation session...
		v[i*N+j]=0;
		//fetches final time...
		//gettimeofday (&tvSolveEnd, NULL);
		//gSolveTimes[gNumMoves]=gSolveTimes[gNumMoves]+tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
		//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
		
		//fetches final time...
		//gettimeofday (&tvEnd, NULL);
		//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
		//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
		
		return -1;
	}
	
	//fetches initial time...
	//gettimeofday (&tvSolveBegin, NULL);
	//-1: conventional value for black chips...
	//this write triggers a new constraint evaluation session...
	v[i*N+j]=-1;
	
	//installs new constraint...
	//LCP_NewCons (GeneralConstraint, FinalConstraint, (void *)theData); 
    dc_new_cons (GeneralConstraint, (void *)theData, NULL);
    
	//fetches final time...
	//gettimeofday (&tvSolveEnd, NULL);
	//gSolveTimes[gNumMoves]=gSolveTimes[gNumMoves]+tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
	//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
	
	//draws new chip (Qt version...)
	//w[i][j]=new QCanvasEllipse (25, 25, myCanvas);
	ww[i][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
	//w[i][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
	ww[i][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
	//w[i][j]->move (i*30+50+15, j*30+50+15);
	//w[i][j]->show ();
    ww[i][j]->setPos (i*30+50+15, j*30+50+15);
    myGraphicsScene->addItem (ww[i][j]);
    ww[i][j]->show ();	
	
    //fetches final time...
	//gettimeofday (&tvEnd, NULL);
	//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
	//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
	
	//chip placed successfully...
	return 0;
}

//CONSTRAINTS

void GeneralConstraint (void *data)
{
	int i=((int)data)>>16;
	int j=((int)data)&65535;
	int theColor=v[i*N+j];
	
	int k, l;
	int leftValue, rightValue, upValue, downValue;
	int upleftValue, uprightValue, downleftValue, downrightValue;
	
	//increases number of activations in current move...
	//gNumAct[gNumMoves]++;
	
	if (gSetup==1)
	{
		//logs dependencies of initial chips...
		
		//upleft...
		if (i>0 && j>0)
		{
			k=j-1;
			l=i-1;
			while (k>=0 && l>=0)
			{
				if (v[l*N+k]!=theColor)
					break;
				else
				{
					k--;
					l--;
				}
			}
		}
		
		//upright...
		if (i>0 && j<N-1)
		{
			k=j+1;
			l=i-1;
			while (k<=N-1 && l>=0)
			{
				if (v[l*N+k]!=theColor)
					break;
				else
				{
					k++;
					l--;
				}
			}
		}
		
		//downleft...
		if (i<N-1 && j>0)
		{
			k=j-1;
			l=i+1;
			while (k>=0 && l<=N-1)
			{
				if (v[l*N+k]!=theColor)
					break;
				else
				{
					k--;
					l++;
				}
			}
		}
		
		//downright...
		if (i<N-1 && j<N-1)
		{
			k=j+1;
			l=i+1;
			while (k<=N-1 && l<=N-1)
			{
				if (v[l*N+k]!=theColor)
					break;
				else
				{
					k++;
					l++;
				}
			}
		}
		
		//left...
		if (j>0)
		{
			k=j-1;
			while (k>=0)
			{
				if (v[i*N+k]!=theColor)
					break;
				else
					k--;
			}
		}
		
		//right...
		if (j<N-1)
		{
			k=j+1;
			while (k<=N-1)
			{
				if (v[i*N+k]!=theColor)
					break;
				else
					k++;
			}
		}
		
		//up...
		if (i>0)
		{
			k=i-1;
			while (k>=0)
			{
				if (v[k*N+j]!=theColor)
					break;
				else
					k--;
			}
		}
		
		//down...
		if (i<N-1)
		{
			k=i+1;
			while (k<=N-1)
			{
				if (v[k*N+j]!=theColor)
					break;
				else
					k++;
			}
		}
		
		return;
	}
	
	//general constraint...
	
	//downleft...
	
	//white...
	if (theColor>0)
	{
		if (j>0 && i<N-1)
		{
			k=j-1;
			l=i+1;
			while (k>=0 && l<=N-1)
			{
				if (v[l*N+k]==-3 || v[l*N+k]==-1 || v[l*N+k]==0)
					break;
				else
				{
					k--;
					l++;
				}
			}
			
			if (k<0 || l>N-1)
			{
				k++;
				l--;
			}
			/*
			if (k<0)
				k++;
			if (l>N-1)
				l--;*/
			
			downleftValue=v[l*N+k];
		}
		else
			downleftValue=0;//outside the board...
		
		
	}
	
	//black...
	if (theColor<0)
	{
		if (j>0 && i<N-1)
		{
			k=j-1;
			l=i+1;
			while (k>=0 && l<=N-1)
			{
				if (v[l*N+k]==3 || v[l*N+k]==1 || v[l*N+k]==0)
					break;
				else
				{
					k--;
					l++;
				}
			}
			
			if (k<0 || l>N-1)
			{
				k++;
				l--;
			}
			/*
			if (k<0)
				k++;
			if (l>N-1)
				l--;*/
			
			downleftValue=v[l*N+k];
		}
		else
			downleftValue=0;//outside the board...
		
		
	}
	
	//upright...
	
	//white...
	if (theColor>0)
	{
		if (j<N-1 && i>0)
		{
			k=j+1;
			l=i-1;
			while (k<=N-1 && l>=0)
			{
				if (v[l*N+k]==-3 || v[l*N+k]==-1 || v[l*N+k]==0)
					break;
				else
				{
					k++;
					l--;
				}
			}
			
			if (k>N-1 || l<0)
			{
				k--;
				l++;
			}
			/*
			if (k>N-1)
				k--;
			if (l<0)
				l++;*/
			
			uprightValue=v[l*N+k];
		}
		else
			uprightValue=0;//outside the board...
		
		
	}
	
	//black...
	if (theColor<0)
	{
		if (j<N-1 && i>0)
		{
			k=j+1;
			l=i-1;
			while (k<=N-1 && l>=0)
			{
				if (v[l*N+k]==3 || v[l*N+k]==1 || v[l*N+k]==0)
					break;
				else
				{
					k++;
					l--;
				}
			}
			
			if (k>N-1 || l<0)
			{
				k--;
				l++;
			}
			/*
			if (k>N-1)
				k--;
			if (l<0)
				l++;*/
			
			uprightValue=v[l*N+k];
		}
		else
			uprightValue=0;//outside the board...
		
		
	}
		
	//downright...
	
	//white...
	if (theColor>0)
	{
		if (j<N-1 && i<N-1)
		{
			k=j+1;
			l=i+1;
			while (k<=N-1 && l<=N-1)
			{
				if (v[l*N+k]==-3 || v[l*N+k]==-1 || v[l*N+k]==0)
					break;
				else
				{
					k++;
					l++;
				}
			}
			
			if (k>N-1 || l>N-1)
			{
				k--;
				l--;
			}
			/*
			if (k>N-1)
				k--;
			if (l>N-1)
				l--;*/
			
			downrightValue=v[l*N+k];
		}
		else
			downrightValue=0;//outside the board...
		
		
	}
	
	//black...
	if (theColor<0)
	{
		if (j<N-1 && i<N-1)
		{
			k=j+1;
			l=i+1;
			while (k<=N-1 && l<=N-1)
			{
				if (v[l*N+k]==3 || v[l*N+k]==1 || v[l*N+k]==0)
					break;
				else
				{
					k++;
					l++;
				}
			}
			
			if (k>N-1 || l>N-1)
			{
				k--;
				l--;
			}	
			
			/*
			if (k>N-1)
				k--;
			if (l>N-1)
				l--;*/
			
			downrightValue=v[l*N+k];
		}
		else
			downrightValue=0;//outside the board...
		
		
	}
	
	//upleft...
	
	//white...
	if (theColor>0)
	{
		if (j>0 && i>0)
		{
			k=j-1;
			l=i-1;
			while (k>=0 && l>=0)
			{
				if (v[l*N+k]==-3 || v[l*N+k]==-1 || v[l*N+k]==0)
					break;
				else
				{
					k--;
					l--;
				}
			}
			
			if (k<0 || l<0)
			{
				k++;
				l++;
			}
			
			/*
			if (k<0)
				k++;
			if (l<0)
				l++;*/
			
			upleftValue=v[l*N+k];
		}
		else
			upleftValue=0;//outside the board...
		
		
	}
	
	//black...
	if (theColor<0)
	{
		if (j>0 && i>0)
		{
			k=j-1;
			l=i-1;
			while (k>=0 && l>=0)
			{
				if (v[l*N+k]==3 || v[l*N+k]==1 || v[l*N+k]==0)
					break;
				else
				{
					k--;
					l--;
				}
			}
			
			if (k<0 || l<0)
			{
				k++;
				l++;
			}
			
			/*
			if (k<0)
				k++;
			if (l<0)
				l++;*/
			
			upleftValue=v[l*N+k];
		}
		else
			upleftValue=0;//outside the board...
		
		
	}
	
	//left...
	
	//white...
	if (theColor>0)
	{
		if (j>0)
		{
			k=j-1;
			while (k>=0)
			{
				if (v[i*N+k]==-3 || v[i*N+k]==-1 || v[i*N+k]==0)
					break;
				else
					k--;
			}
			
			if (k<0)
				leftValue=v[i*N];
			else
				leftValue=v[i*N+k];
		}
		else
			leftValue=0;//outside the board...
		
		
	}
	
	//black...
	if (theColor<0)
	{
		if (j>0)
		{
			k=j-1;
			while (k>=0)
			{
				if (v[i*N+k]==3 || v[i*N+k]==1 || v[i*N+k]==0)
					break;
				else
					k--;
			}
			
			if (k<0)
				leftValue=v[i*N];
			else
				leftValue=v[i*N+k];
		}
		else
			leftValue=0;//outside the board...
		
	}
	
	//right...
	
	//white...
	if (theColor>0)
	{
		if (j<N-1)
		{
			k=j+1;
			while (k<=N-1)
			{
				if (v[i*N+k]==-3 || v[i*N+k]==-1 || v[i*N+k]==0)
					break;
				else
					k++;
			}
			
			if (k>N-1)
				rightValue=v[i*N+N-1];
			else
				rightValue=v[i*N+k];
		}
		else
			rightValue=0;//outside the board...
		
	}
	
	//black...
	if (theColor<0)
	{
		if (j<N-1)
		{
			k=j+1;
			while (k<=N-1)
			{
				if (v[i*N+k]==3 || v[i*N+k]==1 || v[i*N+k]==0)
					break;
				else
					k++;
			}
			
			if (k>N-1)
				rightValue=v[i*N+N-1];
			else
				rightValue=v[i*N+k];
		}
		else
			rightValue=0;//outside the board...
		
	}
	
	//up...
	
	//white...
	if (theColor>0)
	{
		if (i>0)
		{
			k=i-1;
			while (k>=0)
			{
				if (v[k*N+j]==-3 || v[k*N+j]==-1 || v[k*N+j]==0)
					break;
				else
					k--;
			}
			
			if (k<0)
				upValue=v[j];
			else
				upValue=v[k*N+j];
		}
		else
			upValue=0;//outside the board...
		
	}
	
	//black...
	if (theColor<0)
	{
		if (i>0)
		{
			k=i-1;
			while (k>=0)
			{
				if (v[k*N+j]==3 || v[k*N+j]==1 || v[k*N+j]==0)
					break;
				else
					k--;
			}
			
			if (k<0)
				upValue=v[j];
			else
				upValue=v[k*N+j];
		}
		else
			upValue=0;//outside the board...
		
	}
	
	//down...
	
	//white...
	if (theColor>0)
	{
		if (i<N-1)
		{
			k=i+1;
			while (k<=N-1)
			{
				if (v[k*N+j]==-3 || v[k*N+j]==-1 || v[k*N+j]==0)
					break;
				else
					k++;
			}
			
			if (k>N-1)
				downValue=v[(N-1)*N+j];
			else
				downValue=v[k*N+j];
		}
		else
			downValue=0;//outside the board...
		
	}
	
	//black...
	if (theColor<0)
	{
		if (i<N-1)
		{
			k=i+1;
			while (k<=N-1)
			{
				if (v[k*N+j]==3 || v[k*N+j]==1 || v[k*N+j]==0)
					break;
				else
					k++;
			}
			
			if (k>N-1)
				downValue=v[(N-1)*N+j];
			else
				downValue=v[k*N+j];
		}
		else
			downValue=0;//outside the board...
		
	}
	
	//flip because of left/right?
	
	//white...
	if (theColor==1 && ((leftValue==-3 && rightValue==-1) || (leftValue==-1 && rightValue==-3)))
		v[i*N+j]=-2;
	
	//black...
	if (theColor==-1 && ((leftValue==3 && rightValue==1) || (leftValue==1 && rightValue==3)))
		v[i*N+j]=2;
	
	//flip because of up/down?
	
	//white...
	if (theColor==1 && ((upValue==-3 && downValue==-1) || (upValue==-1 && downValue==-3)))
		v[i*N+j]=-2;
	
	//black...
	if (theColor==-1 && ((upValue==3 && downValue==1) || (upValue==1 && downValue==3)))
		v[i*N+j]=2;
	
	//flip because of upleft/downright?
	
	//white...
	if (theColor==1 && ((upleftValue==-3 && downrightValue==-1) || (upleftValue==-1 && downrightValue==-3)))
		v[i*N+j]=-2;
	
	//black...
	if (theColor==-1 && ((upleftValue==3 && downrightValue==1) || (upleftValue==1 && downrightValue==3)))
		v[i*N+j]=2;
	
	//flip because of upright/downleft?
	
	//white...
	if (theColor==1 && ((uprightValue==-3 && downleftValue==-1) || (uprightValue==-1 && downleftValue==-3)))
		v[i*N+j]=-2;
	
	//black...
	if (theColor==-1 && ((uprightValue==3 && downleftValue==1) || (uprightValue==1 && downleftValue==3)))
		v[i*N+j]=2;
    
    
    dc_schedule_final (dc_get_curr_cons (), FinalConstraint);
}

void FinalConstraint (void *data)
{
	int i=((int)data)>>16;
	int j=((int)data)&65535;
	int flipped=0;
	
	//debug...
	//printf ("[DEBUG] i=%d - j=%d\n", i, j);
	
	//increases number of final activations in current move...
	//gNumFinalAct[gNumMoves]++;
	
	//sets value of freshly flipped chips from 2 (-2) to 1 (-1)...
	if (v[i*N+j]==2)
	{
		v[i*N+j]=1;
		flipped=1;
		gFlipped=1;
	}
	
	if (v[i*N+j]==-2)
	{
		v[i*N+j]=-1;
		flipped=1;
		gFlipped=1;
	}
		
	//redraws the chip if necessary (Qt version...)
	if (flipped==1 /*|| gSetup==1*/)
	{
		if (v[i*N+j]==1)
		{
			//fetches initial time...
			//gettimeofday (&tvSolveBeginF, NULL);
			ww[i][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			//myCanvas->update ();
			//fetches final time...
			//gettimeofday (&tvSolveEndF, NULL);
			//gSolveTimes[gNumMoves]=gSolveTimes[gNumMoves]-tvSolveEndF.tv_sec-(tvSolveEndF.tv_usec*0.000001)
			//		+tvSolveBeginF.tv_sec+(tvSolveBeginF.tv_usec*0.000001);
		}
		
		if (v[i*N+j]==-1)
		{
			//fetches initial time...
			//gettimeofday (&tvSolveBeginF, NULL);
			ww[i][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			//myCanvas->update ();
			//fetches final time...
			//gettimeofday (&tvSolveEndF, NULL);
			//gSolveTimes[gNumMoves]=gSolveTimes[gNumMoves]-tvSolveEndF.tv_sec-(tvSolveEndF.tv_usec*0.000001)
			//		+tvSolveBeginF.tv_sec+(tvSolveBeginF.tv_usec*0.000001);
		}
	}
	
	//myCanvasView->repaintContents (FALSE);
}

// Copyright (C) 2011 Camil Demetrescu, Andrea Ribichini

// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.

// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
