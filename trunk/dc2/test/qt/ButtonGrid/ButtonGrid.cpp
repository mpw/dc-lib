// =====================================================================
//  dc/test/qt/ButtonGrid.cpp
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 15, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:24:41 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.5 $


//INCLUDES...

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <QApplication>
#include <QPushButton>
#include <QResizeEvent>
#include <QThread>

//DEFINES...

#define WinWidth 400
#define WinHeight 400

#define NumButtX 16
#define NumButtY 16

#define LOG_ENTRIES 100

#define RESIZE_DELAY_MICROSECS 500000

//TYPES...

typedef struct tagLogEntry
{
	double main_win_resize, last_button_resize;
	
} TLogEntry;

//GLOBALS...

TLogEntry log_array[LOG_ENTRIES];
unsigned int log_c=0;

struct timeval tv;

//CLASSES...

class MyThread : public QThread
{
    public:
        void run();
};

class MyButton : public QPushButton
{
	public:
		int i, j;
		
		MyButton (const QString &text, QWidget *parent);
		void resizeEvent (QResizeEvent *e);	
};

class MyWidget : public QWidget
{
	public:
		MyButton* button[NumButtX][NumButtY];
		
		MyWidget (QWidget *parent=0);
		void resizeEvent (QResizeEvent *e);
};

MyWidget *w;

//METHODS...

MyButton::MyButton (const QString &text, QWidget *parent)
	: QPushButton (text, parent)
{
	
}

void MyButton::resizeEvent (QResizeEvent *e)
{
    //printf ("Entering button resize handler...\n");
    
	QSize size=e->size ();
	
	MyWidget *parent=(MyWidget *)this->parent ();
	
	//resizes and moves other buttons...
	if (i==0 && NumButtX>1 && j<NumButtY-1)
	{
		//right and below...
		parent->button[i+1][j]->setGeometry ((i+1)*size.width(), j*size.height(), size.width(), size.height());
		parent->button[i][j+1]->setGeometry (i*size.width(), (j+1)*size.height(), size.width(), size.height());
		
		//parent->button[i+1][j]->update ();
		//parent->button[i][j+1]->update ();
	}
	
	if (i==0 && NumButtX>1 && j==NumButtY-1)
	{
		//right...
		parent->button[i+1][j]->setGeometry ((i+1)*size.width(), j*size.height(), size.width(), size.height());
		
		//parent->button[i+1][j]->update ();
	}
	
	if (i!=0 && i<NumButtX-1)
	{
		//right...
		parent->button[i+1][j]->setGeometry ((i+1)*size.width(), j*size.height(), size.width(), size.height());
		
		//parent->button[i+1][j]->update ();
	}
	
	/*if (i==NumButtX-1 && j==NumButtY-1 && log_c<LOG_ENTRIES)
	{
		gettimeofday (&tv, NULL);
		log_array[log_c].last_button_resize=tv.tv_sec+(tv.tv_usec*0.000001);
		log_c++;
	}*/
    
    //printf ("Exiting button resize handler...\n");
}

MyWidget::MyWidget (QWidget *parent)
	: QWidget (parent)
{
	int i, j;
	
	//creates button grid...
	for (i=0; i<NumButtX; i++)
		for (j=0; j<NumButtY; j++)
		{
			button[i][j]=new MyButton ("b", this);
			button[i][j]->i=i;
			button[i][j]->j=j;
			button[i][j]->setGeometry (i*WinWidth/NumButtX, j*WinHeight/NumButtY, WinWidth/NumButtX, WinHeight/NumButtY);
		}
		
}

void MyWidget::resizeEvent (QResizeEvent *e)
{
	QSize size=e->size ();
	
	//resizes button in upper left corner...
	button[0][0]->resize (size.width()/NumButtX, size.height()/NumButtY);
	//button[0][0]->update ();
	
	if (log_c<LOG_ENTRIES)
	{
		gettimeofday (&tv, NULL);
		log_array[log_c].main_win_resize=tv.tv_sec+(tv.tv_usec*0.000001);
		log_c++;
		if (log_c==LOG_ENTRIES)
		{
			printf ("Enough!\n");
			fflush (stdout);
		}
	}	
	//printf ("%f - ", tv.tv_sec+(tv.tv_usec*0.000001));
}

//SERVICE THREAD...

void MyThread::run()
{
	unsigned int i;
	double randomNumber;
	unsigned int newDX, newDY;	
	
	printf ("[Service thread] Sleeping for 5 seconds...\n");
	sleep (5);
	printf ("[Service thread] Resizing main window %u times (delay: %u microsecs)...\n", LOG_ENTRIES, RESIZE_DELAY_MICROSECS);
	
	//MyWidget *w=(MyWidget *)arg;
	
	/*generates random seed...*/
	srand((unsigned)time (NULL));
	
	for (i=0; i<LOG_ENTRIES; i++)
	{
		usleep (RESIZE_DELAY_MICROSECS);
		
		/*generates random number in (0,1)...*/
		randomNumber=rand ()/(RAND_MAX+1.0);
		//newDX...
		newDX=randomNumber*350+50;
		
		/*generates random number in (0,1)...*/
		randomNumber=rand ()/(RAND_MAX+1.0);
		//newDY...
		newDY=randomNumber*350+50;
		
		//printf ("[Service thread] (%u, %u)\n", newDX, newDY);
		//resizes main window...
		w->resize (newDX, newDY);
		w->update ();
	}
	
	printf ("[Service thread] Quitting...\n");
}

//MAIN...

int main( int argc, char **argv )
{
	int i;
	double avg=0.0;
	
	QApplication a( argc, argv );

	//MyWidget* w;
    w=new MyWidget ();

	w->setGeometry (100, 100, WinWidth, WinHeight);
	//a.setMainWidget( &w );
	w->show();
	
	//pthread_t serviceThread;
	//pthread_create (&serviceThread, NULL, serviceThreadFunc, &w);
	//MyThread *resizeThread=new MyThread ();
    //resizeThread->start ();
    
	int ret_value=a.exec();
	
	//for (i=0; i<LOG_ENTRIES; i++)
		//printf ("%f - %f\n", log_array[i].main_win_resize, log_array[i].last_button_resize);
		//printf ("%f\n", log_array[i].main_win_resize);
	
	for (i=0; i<LOG_ENTRIES-1; i++)
		avg=avg+log_array[i+1].main_win_resize-log_array[i].main_win_resize;
	avg=avg/(LOG_ENTRIES-1);
	printf ("\navg: %f secs\n", avg);
	
	return ret_value;
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
