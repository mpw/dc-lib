// =====================================================================
//  dc/test/qt/RButtonGrid.cpp
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 15, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:04:10 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.20 $

//INCLUDES...

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <QApplication>
#include <QPushButton>
#include <QResizeEvent>
#include <QEvent>
#include <QThread>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>

#include "dc.h"
#include "profile.h"
#include "dc_profile.h"

//DEFINES...

#define N 24

#define numIter 100

#define MaxXSize N*40
#define MaxYSize N*40*8/12

#define MinXSize MaxXSize/2
#define MinYSize MaxYSize/2

#define NumButtX N
#define NumButtY N

#define RESIZE_DELAY_MICROSECS 1000000

//STATS STUFF...

time_rec_t begin_t, inter_t, end_t;
elapsed_time_t inter_array[numIter];
elapsed_time_t log_array[numIter];
dc_profile log_array_prof[numIter];
unsigned iter = 0, status = 0;

//TYPES...

//pos and dim of buttons and main window...
typedef struct tagProtectedData
{	
	int x, y, dx, dy;	
} ProtectedData;

//PROTOTYPES...

void ButtonConstraint (void *buttonPtr);
void FinalButtonConstraint (void *buttonPtr);

//GLOBALS...

void* mainPage;
void* page[NumButtX][NumButtY];

//TLogEntry log_array[LOG_ENTRIES];
//unsigned int log_c=0;

//struct timeval tv;

//CLASSES...

class MyThread : public QThread
{
    public:
        void run();
};


class MyApplication : public QApplication
{
    public:
        MyApplication (int argc, char** argv);
        bool notify (QObject* receiver, QEvent* e);
};

class MyButton : public QPushButton
{
	public:
		int i, j;
		
		dc_cons *cons;
		
		MyButton (const QString &text, QWidget *parent);
		~MyButton ();
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

MyApplication::MyApplication (int argc, char** argv)
    : QApplication (argc, argv)
{

}

bool MyApplication::notify (QObject* receiver, QEvent* e)
{

    // filters out non main window events...
    if (receiver != w) 
        return QApplication::notify (receiver, e);
    
    int evt_type = e->type ();
    bool res = QApplication::notify (receiver, e);

    // process "redraw completion" event
    if (evt_type == 1000 && status == 2) {
        if (iter < numIter) {
            get_time (&end_t);
    
            compute_elapsed_time(&begin_t, &inter_t, &inter_array[iter]);
            compute_elapsed_time(&begin_t, &end_t, &log_array[iter]);
    
            if (dc_profile_on()) 
                dc_fetch_profile_info(&log_array_prof[iter]);
    
            printf ("<< end resize event #%d\n\n", iter);
            iter++;
        }
        status = 0;
    }

    // paint event
    else if (evt_type == 12) {
        // schedule "redraw completion" event
        QEvent* dummy = new QEvent(QEvent::User);
        QCoreApplication::postEvent(w, dummy);
    }
    
    return res;
}

MyButton::MyButton (const QString &text, QWidget *parent)
	: QPushButton (text, parent)
{
	
}

MyButton::~MyButton ()
{
	/*printf ("Invoking deletecons on cons #%ld\n", cons);
	LCP_DeleteCons (cons);
	printf ("Deletecons successfully executed!\n");
	*/
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
			
			ProtectedData *pd=(ProtectedData *)page[i][j];

			pd->x=i*MaxXSize;
			pd->y=j*MaxYSize;
			pd->dx=MaxXSize;
			pd->dy=MaxYSize;

			button[i][j]->setGeometry (
                pd->x/NumButtX, pd->y/NumButtY, 
                pd->dx/NumButtX, pd->dy/NumButtY);
			
			//button[i][j]->cons=LCP_NewCons (ButtonConstraint, FinalButtonConstraint, button[i][j]);
            button[i][j]->cons=dc_new_cons (ButtonConstraint, (void *)button[i][j], NULL);
		}
}

void MyWidget::resizeEvent (QResizeEvent *e)
{   
	QSize size=e->size ();
	ProtectedData *pd=(ProtectedData *)mainPage;

    if (status == 1) {
        printf (">> begin resize event #%d\n", iter);
        get_time (&begin_t);
        status = 2;
    }

	dc_begin_at ();
	pd->dx=size.width ();
	pd->dy=size.height ();
	dc_end_at ();

    get_time (&inter_t);
}

void ButtonConstraint (void *buttonPtr)
{
	MyButton* myButt=(MyButton *)buttonPtr;
	
	int i=myButt->i;
	int j=myButt->j;
	
    ProtectedData *pd;
    ProtectedData *pd2=(ProtectedData *)page[i][j];
	
	if (i==0 && j==0)
	{
		pd=(ProtectedData *)mainPage;
        //pd2=(ProtectedData *)page[i][j];
		
		//writes new button size to protected page [0,0]...
		pd2->dx=pd->dx;
		pd2->dy=pd->dy;
	}
	
	
	if (i==0 && j!=0)
	{
		pd=(ProtectedData *)page[i][j-1];
		//pd2=(ProtectedData *)page[i][j];
		
		//writes new pos and size...
		pd2->x=0;
		pd2->y=j*pd->dy;
		pd2->dx=pd->dx;
		pd2->dy=pd->dy;
	}
	
	if (i!=0)
	{
		pd=(ProtectedData *)page[i-1][j];
		//pd2=(ProtectedData *)page[i][j];
		
		//writes new pos and size...
		pd2->x=i*pd->dx;
		pd2->y=j*pd->dy;
		pd2->dx=pd->dx;
		pd2->dy=pd->dy;
	}
	
	//dc_schedule_final (dc_get_curr_cons (), FinalButtonConstraint); 
    dc_schedule_final (dc_get_curr_cons (), NULL);
    
    myButt->setGeometry (pd2->x/NumButtX, pd2->y/NumButtY, pd2->dx/NumButtX, pd2->dy/NumButtY);
}

void FinalButtonConstraint (void *buttonPtr)
{
	MyButton* myButt=(MyButton *)buttonPtr;
	
	int i=myButt->i;
	int j=myButt->j;
	
	ProtectedData *pd=(ProtectedData *)page[i][j];
	
	//resizes button...
	myButt->setGeometry (
        pd->x/NumButtX, pd->y/NumButtY, 
        pd->dx/NumButtX, pd->dy/NumButtY);
	
	/*if (i==NumButtX-1 && j==NumButtY-1 && log_c<LOG_ENTRIES)
	{
		gettimeofday (&tv, NULL);
		log_array[log_c].last_button_resize=tv.tv_sec+(tv.tv_usec*0.000001);
		log_c++;
	}*/
}


void MyThread::run()
{
    static int flip = 0;
    usleep(RESIZE_DELAY_MICROSECS);
    for (iter = 0; iter < numIter;) {
        QResizeEvent* resize;
        int ww, hh;
        if (flip)
        {
            ww = MaxXSize;
            hh = MaxYSize;
        }
        else
        {
            ww = MinXSize;
            hh = MinYSize;
        }
        QSize s(ww, hh);
        resize = new QResizeEvent(s, w->size());
        flip = !flip;
        status = 1;
        QCoreApplication::postEvent(w, resize);
        usleep(RESIZE_DELAY_MICROSECS);
        if (status > 0) {
            printf("can't keep pace with resize events... "
                   "bailing out");
            while (status > 0) usleep(RESIZE_DELAY_MICROSECS/2);
        }
    }
}


//MAIN...

int main( int argc, char **argv )
{
	int i, j;
	
	dc_init ();
	
	//allocates main page...
	mainPage=dc_malloc (sizeof (ProtectedData));
	
	//allocates one protected page per button...
	for (i=0; i<NumButtX; i++)
		for (j=0; j<NumButtY; j++)
			page[i][j]=dc_malloc (sizeof (ProtectedData));
	
	//QApplication a( argc, argv );
    MyApplication a( argc, argv );

	w=new MyWidget ();

	w->setGeometry (0, 0, MaxXSize, MaxYSize);
	w->show();

    MyThread *resizeThread = new MyThread ();
    resizeThread->start();

	int ret_value=a.exec();

    delete resizeThread;

	//frees main page...
	dc_free (mainPage);
	
	//frees protected pages...
	for (i=0; i<NumButtX; i++)
		for (j=0; j<NumButtY; j++)
			dc_free (page[i][j]);
	
	//for (i=0; i<LOG_ENTRIES; i++)
		//printf ("%f - %f\n", log_array[i].main_win_resize, log_array[i].last_button_resize);

    // dump DC profiling info
    if (dc_profile_on()) 
        dc_dump_profile_diff(stdout, 
                             &log_array_prof[numIter-4], 
                             &log_array_prof[numIter-3]);
    else {

        //computes avg and max resize evt processing time...
        elapsed_time_t avg;
        avg.real=0;
        avg.user=0;
        avg.system=0;
        avg.child_user=0;
        avg.child_system=0;
        
        double max=log_array[1].real;
        for (i=1; i<numIter-2; i++)
        {
            avg.real=avg.real+log_array[i].real;
            avg.user=avg.user+log_array[i].user;
            avg.system=avg.system+log_array[i].system;
            avg.child_user=avg.child_user+log_array[i].child_user;
            avg.child_system=avg.child_system+log_array[i].child_system;
            
            if (log_array[i].real>max)
                max=log_array[i].real;
        }

        divide_elapsed_time_by(&avg, numIter-2);
        printf ("\nTotal times per resize event:\n");
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
        max=inter_array[1].real;
        for (i=1; i<numIter-2; i++)
        {
            avg.real=avg.real+inter_array[i].real;
            avg.user=avg.user+inter_array[i].user;
            avg.system=avg.system+inter_array[i].system;
            avg.child_user=avg.child_user+inter_array[i].child_user;
            avg.child_system=avg.child_system+inter_array[i].child_system;
            
            if (inter_array[i].real>max)
                max=inter_array[i].real;
        }
        
        divide_elapsed_time_by(&avg, numIter-2);
        printf ("\nChange propagation times only:\n");
        printf ("Avg real = %f\n", avg.real);
        printf ("Avg user = %f\n", avg.user);
        printf ("Avg system = %f\n", avg.system);
        printf ("Avg child_user = %f\n", avg.child_user);
        printf ("Avg child_system = %f\n", avg.child_system);
        printf ("Max real = %f\n", max);
    }

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
