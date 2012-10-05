// =====================================================================
//  dc/test/qt/Othello.cpp
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 15, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:24:41 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.7 $

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

#include "Othello.h"

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
    
    g_rec_file=fopen ("Othello_clickstream.txt", "r");
    
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
    
    printf ("Event: %d\n", e->type ());*/    
    
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
    
    g_rec_file=fopen ("Othello_clickstream.txt", "w");
    
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
	int h, k;
	
	//double avgAct=0.0;
	//int maxAct=0;
	//double avgFinalAct=0.0;
	//int maxFinalAct=0;
	//double avgTime=0.0;
	//double maxTime=0.0;
	
	//initializes w[][] (pointers to ellipses)...
	for (h=0; h<N; h++)
		for (k=0; k<N; k++)
			ww[h][k]=NULL;
	
	//LCP_Init ();
	
	//allocates vector...
	// max N is 16...
	//v=(int *)LWP_AllocPages (1);
	v=(int *)malloc (sizeof (int)*N*N);
	
	//installs constraints...
	/*for (h=0; h<N; h++)
		for (k=0; k<N; k++)
		{
			//we are assuming lots of things here...
			int theData=(h<<16)+k;
			
			LCP_NewCons (GeneralConstraint, NULL, (void *)theData); 
		}*/
	
	//QApplication a( argc, argv );
    MyApplication a( argc, argv );

	w=new MyWidget ();

	//a.setMainWidget( w );
	w->show();
	
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
	
	//LCP_CleanUp ();
	
	//frees main page...
	//LWP_FreePages (v, 1);
	free (v);

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
    
	/*debugging...*/
	//printf ("\n[DEBUG] LMemory_GetBlocksCount() says: %u\n", (unsigned int)LMemory_GetBlocksCount ());

#if 0
	printf ("Total number of moves: %d\n", gNumMoves);
	
	for (i=1; i<=gNumMoves; i++)
	{
		avgTime=avgTime+gMoveTimes[i];
		
		if (gMoveTimes[i]>maxTime)
			maxTime=gMoveTimes[i];
	}
	printf ("Seconds per move (average): %f\n", avgTime/gNumMoves);
	printf ("Seconds per move (maximum): %f\n", maxTime);
	
	avgTime=0.0;
	maxTime=0.0;
	for (i=1; i<=gNumEvt; i++)
	{
		avgTime=avgTime+gMousePressTimes[i];
		
		if (gMousePressTimes[i]>maxTime)
			maxTime=gMousePressTimes[i];
	}
	printf ("Seconds per mouse event (average): %f\n", avgTime/gNumEvt);
	printf ("Seconds per mouse event (maximum): %f\n", maxTime);
#endif

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
	int k, l, m;
	
	//fails if outside board...
	if (i<0 || i>N-1 || j<0 || j>N-1)
		return -1;
	
	//fails if square is already occupied...
	if (v[i*N+j]!=0)
		return -1;
	
	//fetches initial time...
	//gettimeofday (&tvBegin, NULL);
	
	//increases moves counter...
	//gNumMoves++;
	
	//resets global var...
	gFlipped=0;
	
	if (gSetup==1)
	{
		v[i*N+j]=1;
		
		//if ellipse does not exist...
		if (ww[i][j]==NULL)
		{
			//creates ellipse...
			ww[i][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
            ww[i][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
            ww[i][j]->setPos (i*30+50+15, j*30+50+15);
            myGraphicsScene->addItem (ww[i][j]);
            ww[i][j]->show ();
		}
		else //ellipse already exists...
		{
			ww[i][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
		}
		
		//fetches final time...
		//gettimeofday (&tvEnd, NULL);
		//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
		//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
		
		//chip placed successfully...
		return 0;
	}
	
	//fetches initial time...
	//gettimeofday (&tvSolveBegin, NULL);
	//3: conventional value for newly placed white chip...
	//v[i*N+j]=3;
	//fetches final time...
	//gettimeofday (&tvSolveEnd, NULL);
	//gSolveTimes[gNumMoves]=tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
	//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
	
	//constraints adjust other chips here...
	//NO, THEY DON'T IN THIS VERSION...
	//WE HAVE TO DO IT...
	
	//down left...
	k=j-1;
	l=i+1;
	while (k>0 && l<N && v[l*N+k]==-1)
	{
		k--;
		l++;
	}
	
	if (k!=j-1 && k>=0 && l<N && v[l*N+k]==1)
	{
		gFlipped=1;
		for (m=0; m<l-i; m++)
		{
			v[(i+m)*N+j-m]=1;
			
			//if ellipse does not exist...
			if (ww[i+m][j-m]==NULL)
			{
                //creates ellipse...
				ww[i+m][j-m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i+m][j-m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[i+m][j-m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i+m][j-m]);
                ww[i+m][j-m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i+m][j-m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}		
	}
	
	//down right...
	k=j-1;
	l=i-1;
	while (k>0 && l>0 && v[l*N+k]==-1)
	{ 
		k--;
		l--;
	}
	
	if (k!=j-1 && k>=0 && l>=0 && v[l*N+k]==1)
	{
		gFlipped=1;
		for (m=0; m<i-l; m++)
		{
			v[(i-m)*N+j-m]=1;
			
			//if ellipse does not exist...
			if (ww[i-m][j-m]==NULL)
			{
                //creates ellipse...
				ww[i-m][j-m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i-m][j-m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[i-m][j-m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i-m][j-m]);
                ww[i-m][j-m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i-m][j-m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}
	}
	
	//up right...
	k=j+1;
	l=i-1;
	while (k<N && l>=0 && v[l*N+k]==-1)
	{
		k++;
		l--;
	}
	
	if (k!=j+1 && k<N && l>=0 && v[l*N+k]==1)
	{
		gFlipped=1;
		for (m=0; m<i-l; m++)
		{
			v[(i-m)*N+j+m]=1;
			
			//if ellipse does not exist...
			if (ww[i-m][j+m]==NULL)
			{
                //creates ellipse...
				ww[i-m][j+m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i-m][j+m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[i-m][j+m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i-m][j+m]);
                ww[i-m][j+m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i-m][j+m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}
	}
	
	//up left...
	k=j+1;
	l=i+1;
	while (k<N && l<N && v[l*N+k]==-1) 
	{
		k++;
		l++;
	}
	
	if (k!=j+1 && k<N && l<N && v[l*N+k]==1)
	{
		gFlipped=1;
		for (m=0; m<l-i; m++)
		{
			v[(i+m)*N+j+m]=1;
			
			//if ellipse does not exist...
			if (ww[i+m][j+m]==NULL)
			{
                //creates ellipse...
				ww[i+m][j+m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i+m][j+m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[i+m][j+m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i+m][j+m]);
                ww[i+m][j+m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i+m][j+m]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}
	}
		
	//down...		
	k=j-1;
	while (k>0 && v[i*N+k]==-1)
		k--;
	
	if (k!=j-1 && k>=0 && v[i*N+k]==1)
	{
		gFlipped=1;
		for (l=j; l>=k; l--)	
		{
			v[i*N+l]=1;
				
			//if ellipse does not exist...
			if (ww[i][l]==NULL)
			{
                //creates ellipse...
				ww[i][l]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i][l]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[i][l]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i][l]);
                ww[i][l]->show ();
			}
			else //ellipse already exists...
			{
				ww[i][l]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}			
	}
		
	//up...
	k=j+1;
	while (k<N && v[i*N+k]==-1)
		k++;
	
	if (k!=j+1 && k<N && v[i*N+k]==1)
	{
		gFlipped=1;
		for (l=j; l<=k; l++)	
		{
			v[i*N+l]=1;
			
			//if ellipse does not exist...
			if (ww[i][l]==NULL)
			{
                //creates ellipse...
				ww[i][l]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i][l]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[i][l]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i][l]);
                ww[i][l]->show ();
			}
			else //ellipse already exists...
			{
				ww[i][l]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}			
	}
	
	//left...
	k=i-1;
	while (k>0 && v[k*N+j]==-1)
		k--;
	
	if (k!=i-1 && k>=0 && v[k*N+j]==1)
	{
		gFlipped=1;
		for (l=i; l>=k; l--)
		{
			v[l*N+j]=1;
			
			//if ellipse does not exist...
			if (ww[l][j]==NULL)
			{
                //creates ellipse...
				ww[l][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[l][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[l][j]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[l][j]);
                ww[l][j]->show ();
			}
			else //ellipse already exists...
			{
				ww[l][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}
	}
	
	//right...
	k=i+1;
	while (k<N && v[k*N+j]==-1)
		k++;
	
	if (k!=i+1 && k<N && v[k*N+j]==1)
	{
		gFlipped=1;
		for (l=i; l<=k; l++)
		{
			v[l*N+j]=1;
			
			//if ellipse does not exist...
			if (ww[l][j]==NULL)
			{
                //creates ellipse...
				ww[l][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[l][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
                ww[l][j]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[l][j]);
                ww[l][j]->show ();
			}
			else //ellipse already exists...
			{
				ww[l][j]->setBrush (QBrush (QColor (255, 255, 255), Qt::SolidPattern));
			}
		}
	}
	
	//if no flips and not in setup mode...
	if (gFlipped==0 && gSetup==0)
	{
		//fetches final time...
		//gettimeofday (&tvEnd, NULL);
		//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
		//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
		
		//invalid move...
		return -1;
	}
	
	//fetches final time...
	//gettimeofday (&tvEnd, NULL);
	//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
	//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
	
	//chip placed successfully...
	return 0;
}

int _blackMove (int i, int j)
{
	int k, l, m;
	
	//fails if outside board...
	if (i<0 || i>N-1 || j<0 || j>N-1)
		return -1;
	
	//fails if square is already occupied...
	if (v[i*N+j]!=0)
		return -1;
	
	//fetches initial time...
	//gettimeofday (&tvBegin, NULL);
	
	//increases moves counter...
	//gNumMoves++;
	
	//resets global var...
	gFlipped=0;
	
	if (gSetup==1)
	{
		v[i*N+j]=-1;
		
		//if ellipse does not exist...
		if (ww[i][j]==NULL)
		{
			//creates ellipse...
			ww[i][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
            ww[i][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
            ww[i][j]->setPos (i*30+50+15, j*30+50+15);
            myGraphicsScene->addItem (ww[i][j]);
            ww[i][j]->show ();
		}
		else //ellipse already exists...
		{
			ww[i][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
		}
		
		//fetches final time...
		//gettimeofday (&tvEnd, NULL);
		//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
		//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
		
		//chip placed successfully...
		return 0;
	}
	
	//fetches initial time...
	//gettimeofday (&tvSolveBegin, NULL);
	//3: conventional value for newly placed white chip...
	//v[i*N+j]=3;
	//fetches final time...
	//gettimeofday (&tvSolveEnd, NULL);
	//gSolveTimes[gNumMoves]=tvSolveEnd.tv_sec+(tvSolveEnd.tv_usec*0.000001)
	//		-tvSolveBegin.tv_sec-(tvSolveBegin.tv_usec*0.000001);
	
	//constraints adjust other chips here...
	//NO, THEY DON'T IN THIS VERSION...
	//WE HAVE TO DO IT...
	
	//down left...
	k=j-1;
	l=i+1;
	while (k>0 && l<N && v[l*N+k]==1)
	{
		k--;
		l++;
	}
	
	if (k!=j-1 && k>=0 && l<N && v[l*N+k]==-1)
	{
		gFlipped=1;
		for (m=0; m<l-i; m++)
		{
			v[(i+m)*N+j-m]=-1;
			
			//if ellipse does not exist...
			if (ww[i+m][j-m]==NULL)
			{
                //creates ellipse...
				ww[i+m][j-m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i+m][j-m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[i+m][j-m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i+m][j-m]);
                ww[i+m][j-m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i+m][j-m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}	
		}
	}
	
	//down right...
	k=j-1;
	l=i-1;
	while (k>0 && l>0 && v[l*N+k]==1)
	{ 
		k--;
		l--;
	}
	
	if (k!=j-1 && k>=0 && l>=0 && v[l*N+k]==-1)
	{
		gFlipped=1;
		for (m=0; m<i-l; m++)
		{
			v[(i-m)*N+j-m]=-1;
			
			//if ellipse does not exist...
			if (ww[i-m][j-m]==NULL)
			{
                //creates ellipse...
				ww[i-m][j-m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i-m][j-m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[i-m][j-m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i-m][j-m]);
                ww[i-m][j-m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i-m][j-m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
	
	//up right...
	k=j+1;
	l=i-1;
	while (k<N && l>=0 && v[l*N+k]==1)
	{
		k++;
		l--;
	}
	
	if (k!=j+1 && k<N && l>=0 && v[l*N+k]==-1)
	{
		gFlipped=1;
		for (m=0; m<i-l; m++)
		{
			v[(i-m)*N+j+m]=-1;
			
			//if ellipse does not exist...
			if (ww[i-m][j+m]==NULL)
			{
                //creates ellipse...
				ww[i-m][j+m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i-m][j+m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[i-m][j+m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i-m][j+m]);
                ww[i-m][j+m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i-m][j+m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
	
	//up left...
	k=j+1;
	l=i+1;
	while (k<N && l<N && v[l*N+k]==1) 
	{
		k++;
		l++;
	}
	
	if (k!=j+1 && k<N && l<N && v[l*N+k]==-1)
	{
		gFlipped=1;
		for (m=0; m<l-i; m++)
		{
			v[(i+m)*N+j+m]=-1;
			
			//if ellipse does not exist...
			if (ww[i+m][j+m]==NULL)
			{
                //creates ellipse...
				ww[i+m][j+m]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i+m][j+m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[i+m][j+m]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i+m][j+m]);
                ww[i+m][j+m]->show ();
			}
			else //ellipse already exists...
			{
				ww[i+m][j+m]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
		
	//down...		
	k=j-1;
	while (k>0 && v[i*N+k]==1)
		k--;
	
	if (k!=j-1 && k>=0 && v[i*N+k]==-1)
	{
		gFlipped=1;
		for (l=j; l>=k; l--)
		{	
			v[i*N+l]=-1;
			
			//if ellipse does not exist...
			if (ww[i][l]==NULL)
			{
                //creates ellipse...
				ww[i][l]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i][l]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[i][l]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i][l]);
                ww[i][l]->show ();
			}
			else //ellipse already exists...
			{
                ww[i][l]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
		
	//up...
	k=j+1;
	while (k<N && v[i*N+k]==1)
		k++;
	
	if (k!=j+1 && k<N && v[i*N+k]==-1)
	{
		gFlipped=1;
		for (l=j; l<=k; l++)	
		{
			v[i*N+l]=-1;
			
			//if ellipse does not exist...
			if (ww[i][l]==NULL)
			{
                //creates ellipse...
				ww[i][l]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[i][l]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[i][l]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[i][l]);
                ww[i][l]->show ();
			}
			else //ellipse already exists...
			{
                    ww[i][l]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
	
	//left...
	k=i-1;
	while (k>0 && v[k*N+j]==1)
		k--;
	
	if (k!=i-1 && k>=0 && v[k*N+j]==-1)
	{
		gFlipped=1;
		for (l=i; l>=k; l--)
		{
			v[l*N+j]=-1;
			
			//if ellipse does not exist...
			if (ww[l][j]==NULL)
			{
                //creates ellipse...
				ww[l][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[l][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[l][j]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[l][j]);
                ww[l][j]->show ();
			}
			else //ellipse already exists...
			{
                    ww[l][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
	
	//right...
	k=i+1;
	while (k<N && v[k*N+j]==1)
		k++;
	
	if (k!=i+1 && k<N && v[k*N+j]==-1)
	{
		gFlipped=1;
		for (l=i; l<=k; l++)
		{
			v[l*N+j]=-1;
			
			//if ellipse does not exist...
			if (ww[l][j]==NULL)
			{
                //creates ellipse...
				ww[l][j]=new QGraphicsEllipseItem (-12.5, -12.5, 25, 25);
                ww[l][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
                ww[l][j]->setPos (i*30+50+15, j*30+50+15);
                myGraphicsScene->addItem (ww[l][j]);
                ww[l][j]->show ();
			}
			else //ellipse already exists...
			{
                ww[l][j]->setBrush (QBrush (QColor (0, 0, 0), Qt::SolidPattern));
			}
		}
	}
	
	//if no flips and not in setup mode...
	if (gFlipped==0 && gSetup==0)
	{
		//fetches final time...
		//gettimeofday (&tvEnd, NULL);
		//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
		//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
		
		//invalid move...
		return -1;
	}
	
	//fetches final time...
	//gettimeofday (&tvEnd, NULL);
	//gMoveTimes[gNumMoves]=tvEnd.tv_sec+(tvEnd.tv_usec*0.000001)
	//		-tvBegin.tv_sec-(tvBegin.tv_usec*0.000001);
	
	//chip placed successfully...
	return 0;
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
