// =====================================================================
//  dc/test/qt/RSort.cpp
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 11, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:24:41 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.7 $


//INCLUDES...

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include <QObject>
#include <QGraphicsRectItem>
#include <QApplication>
#include <QMouseEvent>
#include <QMenu>
#include <QMenuBar>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QBrush>
#include <QGraphicsEllipseItem>
#include <QPropertyAnimation>

#include "dc.h"

#include "RSort.h"

//#include "qsort1.c"

//DEFINES...

#define N 20
#define dimX 640
#define dimY 480

//machine dependent...
#define ANIM_DELAY 1000

//choose sorting algo:
//0:sel 1:bubble
#define ALGO 0

//TYPES...

typedef struct tagTData
{
	int prev;
	int index;
} TData;

//PROTOTYPES...

void GeneralConstraint (void *);

//GLOBALS...

TData *gDataArray[N];

int setup=1;

int *v;	

int gPrevIndex=-1;

MyQGraphicsRectItem **vr;

QGraphicsScene* myGraphicsScene;
QGraphicsView* myGraphicsView;

QAction *g_sort, *g_quit;

//METHODS...

MyWidget::MyWidget (QWidget *parent)
	: QMainWindow (parent)
{	
	int i;
	
	//installs menus...
    
    g_sort=new QAction ("&Sort", this);
    g_sort->setShortcut (tr ("CTRL+S"));
    g_quit=new QAction ("&Quit", this);
    g_quit->setShortcut (tr ("CTRL+Q"));
    
    QMenu *file;
    file=menuBar()->addMenu ("&File");
    file->addAction (g_sort);
    file->addSeparator ();
    file->addAction (g_quit);
    
    connect (g_sort, SIGNAL(triggered ()), this, SLOT (sort ()));
    connect (g_quit, SIGNAL(triggered ()), qApp, SLOT (quit ()));
	
	//sets main window size...
	this->setGeometry (100, 100, dimX, dimY);
	
	//Graphics View details...
    myGraphicsScene=new QGraphicsScene (this);
    myGraphicsScene->setSceneRect (0, 0, this->width(), this->height());
	
	myGraphicsView=new QGraphicsView (myGraphicsScene, this);
    myGraphicsView->resize (this->width(), this->height()-23);
    myGraphicsView->move (0, 23);
    //myGraphicsView->setViewportUpdateMode (QGraphicsView::FullViewportUpdate);
	
	//lays scene...
	for (i=0; i<N; i++)
	{
		vr[i]=new MyQGraphicsRectItem ((dimX/N-5)*(i+1), dimY/2+v[i]/2, dimX/(N*2.3), -v[i], 0);
		vr[i]->setBrush (QBrush (QColor (255, 10, 10), Qt::SolidPattern));
		myGraphicsScene->addItem (vr[i]);
		vr[i]->show ();
	}

	//installs constraints...
	for (i=0; i<N; i++)
	{
		TData *theData=(TData *)malloc (sizeof (TData));
		theData->index=i;
        
        dc_new_cons (GeneralConstraint, (void *)theData, NULL);
        
		//logs parameter data struct so we can deallocate it later...
		gDataArray[i]=theData;
	}
	//constraint setup completed...
	setup=0;
}

int compare (char *i, char *j)
{
	int a=*(int *)i;
	int b=*(int *)j;
	
	if (a==b)
		return 0;
	
	if (a<b)
		return -1;
	else
		return 1;
}

void scambia (int *v, int i, int j)
{	
	int tmp=v[i];
	v[i]=v[j];
	v[j]=tmp;
}

void bubbleSort (int *v, int n)
{
	int noScambi=1;
	int i, j;
     
	for (i=0; i<n-1; i++)
	{
		for (j=1; j<n-i; j++)
			if (v[j-1]>v[j])
		{
			scambia (v, j-1, j);
			noScambi=0;
		}
             
		if (noScambi)
			break;
		else
			noScambi=1;
	}   
}

void selSort (int *v, int n)
{
	int k, j, m;
     
	for (k=0; k<n-1; k++)
	{
		m=k;
         
		for (j=k+1; j<n; j++)
			if (v[j]<v[m])
				m=j;
         
		scambia (v, m, k);
	}       
}

//posix thred func...
/*void* sortThreadFunc (void *inParam)
{
    //quicksort ((char *)v, N, 4, compare);
    //bubbleSort (v, N);
    selSort (v, N);
    
    return NULL;
}*/

void MyThread::run()
{
    g_sort->setEnabled (false);
    
    if (ALGO==0)
        selSort (v, N);
    if (ALGO==1)
        bubbleSort (v, N);
    //if (ALGO==2)
        //quicksort ((char *)v, N, 4, compare);
}

void MyWidget::sort ()
{
	//int i;
    
    //pthread_t sortThread;
    MyThread *sortThread=new MyThread ();
	
	/*printf ("\nVector before sorting:\n");
	for (i=0; i<N; i++)
		printf ("%d ", v[i]);
	printf ("\n\n");*/
	
	//quicksort ((char *)v, N, 4, compare);
	//bubbleSort (v, N);
	//selSort (v, N);
    
    //pthread_create (&sortThread, NULL, sortThreadFunc, NULL);
    
    sortThread->start ();
    //sortThread->wait ();
	
	/*printf ("Vector after sorting:\n");
	for (i=0; i<N; i++)
		printf ("%d ", v[i]);
	printf ("\n\n");*/
}

//MAIN...

MyWidget *w;

int main( int argc, char **argv )
{
	int i, j;
	
    dc_init ();
	
	//allocates vector...
	// max N is 1024...
    v=(int *)dc_malloc (4096);
	
	//allocates MyQGraphicsRectItem vector...
	vr=(MyQGraphicsRectItem **)malloc (N*sizeof (MyQGraphicsRectItem **));
	
	/*generates random seed...*/
	srand((unsigned)time (NULL));
	
	//fills vector with values...
	printf ("Generated vector: \n");
	for (i=0; i<N; i++)
	{
		/*generates random number in (0,1)...*/
		j = rand() % (N*N);
		//without this printf it won't work with -O2...
		//printf ("%d ", j); 
		v[i] = j;
	}
	printf ("\n");
	
	QApplication a( argc, argv );

	w=new MyWidget ();

	w->show();
	
	int ret_value=a.exec();
		
	//frees main page...
    dc_free (v);
	//frees MyQGraphicsRectItem vector...
	free (vr);
	
	//deallocates constraint parameter data structs...
	for (i=0; i<N; i++)
	{
		free (gDataArray[i]);
	}

	return ret_value;
} 

//CONSTRAINTS

void GeneralConstraint (void *data)
{
	TData *theData=(TData *)data;
	int i=theData->index;
	int prevValue=theData->prev;
	int j;
    
    //printf ("Executing constraint...\n");
	
    //animated swap...
	
	//if not in setup...
	if (setup!=1)
	{
		theData->prev=v[i];
		
		//if no previous log...
		//logs variation...
		if (gPrevIndex==-1)
		{
			//printf ("Logging variation...\n");
			gPrevIndex=i;
			return;
		}
		
		//printf ("Implementing animation...\n");
		//implements swap animation...
		
		int x_i=vr[i]->x ();
        int x_p=vr[gPrevIndex]->x ();
		
		if (i<gPrevIndex)
		{ 
            /*
            QPropertyAnimation *animation1 = new QPropertyAnimation(vr[i], "pos");
            animation1->setDuration (5000);
            animation1->setStartValue(QPointF(x_i, 0));
            animation1->setEndValue(QPointF (x_p, 0));
            
            QPropertyAnimation *animation2 = new QPropertyAnimation(vr[gPrevIndex], "pos");
            animation2->setDuration (5000);
            animation2->setStartValue(QPointF(x_p, 0));
            animation2->setEndValue(QPointF (x_i, 0));
            
            animation1->start ();
            animation2->start ();
            usleep (5000);
            */

			//printf ("(1)Start: %d - End: %d\n", (dimX/N-5)*(i+1), (dimX/N-5)*(gPrevIndex+1));
			for (j=1; j<=((dimX/N-5)*(gPrevIndex+1))-((dimX/N-5)*(i+1)); j++)
            {
				//printf ("Moving(1)...\n");                
				
                vr[i]->setPos (x_i+j, 0);
                vr[gPrevIndex]->setPos (x_p-j, 0);
                
                myGraphicsScene->update ();
                
				usleep (ANIM_DELAY);
			}
		}
		else
		{
			//printf ("(2)Start: %d - End: %d\n", (dimX/N-5)*(i+1), (dimX/N-5)*(gPrevIndex+1));
            for (j=1; j<=((dimX/N-5)*(i+1))-((dimX/N-5)*(gPrevIndex+1)); j++)
			{
				//printf ("Moving(2)...\n");
                
                vr[i]->setPos (x_i-j, 0);
                vr[gPrevIndex]->setPos (x_p+j, 0);
                
                myGraphicsScene->update ();
                
                usleep (ANIM_DELAY);
			}
		}
		
		MyQGraphicsRectItem *tmp=vr[i];
		vr[i]=vr[gPrevIndex];
		vr[gPrevIndex]=tmp;
		
		//printf ("Log reset...\n");
		gPrevIndex=-1;

		return;
	 }
	
    //touches cell to log dependency...
	theData->prev=v[i];
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
