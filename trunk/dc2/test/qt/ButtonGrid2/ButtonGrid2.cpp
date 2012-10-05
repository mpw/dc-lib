// =====================================================================
//  dc/test/qt/ButtonGrid2.cpp
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 24, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:24:41 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.11 $

//INCLUDES...

#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>

#include <QPushButton>
#include <QResizeEvent>
#include <QWidget>


#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QX11Info>

#include "ButtonGrid2.h"
#include "profile.h"
#include "dc_profile.h"

//GLOBALS...

time_rec_t begin_t, inter_t, end_t;
elapsed_time_t inter_array[numIter];
elapsed_time_t log_array[numIter];
dc_profile log_array_prof[numIter];
unsigned iter = 0, status = 0;

MyWidget *w;

//METHODS...

MyApplication::MyApplication (int argc, char** argv)
    : QApplication (argc, argv)
{

}

void MyButton::MB_GeoChangeSlot (int width, int height)
{
    //printf ("Entering geochange slot on button %p\n", this);
    
    //gets parent...
    //MyWidget *parent=(MyWidget *)this->parent ();
    
    //gets parent size...
    //QSize size=parent->size ();
    //printf ("dx = %u - dy = %u\n", size.width (), size.height ());
    
    int new_x=i*width;//size.width();
    int new_y=j*height;//size.height();
    int new_dx=width;//size.width();
    int new_dy=height;//size.height();
    
    QSize old_size=this->size ();
    
    if (new_x!=x()*NumButtX || new_y!=y()*NumButtY || new_dx!=old_size.width()*NumButtX || new_dy!=old_size.height()*NumButtY)
    {
        //changes button's geometry...
        //this->setGeometry (i*size.width()/NumButtX, j*size.height()/NumButtY, size.width()/NumButtX, size.height()/NumButtY);
        this->setGeometry (new_x/NumButtX, new_y/NumButtY, new_dx/NumButtX, new_dy/NumButtY);
    
        //emits 'geochange' signal...
        emit MB_GeoChangeSignal (width, height);
    }
    
    //printf ("Exiting geochange slot on button %p\n", this);
}

MyButton::MyButton (const QString &text, QWidget *parent)
	: QPushButton (text, parent)
{
	
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
			button[i][j]->setGeometry (i*MaxXSize/NumButtX, j*MaxYSize/NumButtY, MaxXSize/NumButtX, MaxYSize/NumButtY);
		}
		
	//connects buttons...
    for (i=0; i<NumButtX; i++)
        for (j=0; j<NumButtY; j++)
        {
            if (i==0 && j==0)
            {
                //connects main widget to upper-left-corner button...
                connect (this, SIGNAL (MW_ResizedSignal (int, int)), button[0][0], SLOT (MB_GeoChangeSlot (int, int)));
            }
            
            if (i==0 && NumButtX>1 && j<NumButtY-1)
            {
                //right and below...
                connect (button[i][j], SIGNAL (MB_GeoChangeSignal (int, int)), button[i][j+1], SLOT (MB_GeoChangeSlot (int, int)));
                connect (button[i][j], SIGNAL (MB_GeoChangeSignal (int, int)), button[i+1][j], SLOT (MB_GeoChangeSlot (int, int)));
            }
            
            if (i==0 && NumButtX>1 && j==NumButtY-1)
            {
                //right
                connect (button[i][j], SIGNAL (MB_GeoChangeSignal (int, int)), button[i+1][j], SLOT (MB_GeoChangeSlot (int, int)));
            }
            
            if (i!=0 && i<NumButtX-1)
            {
                //right...
                connect (button[i][j], SIGNAL (MB_GeoChangeSignal (int, int)), button[i+1][j], SLOT (MB_GeoChangeSlot (int, int)));
            }
        }
}

void MyWidget::resizeEvent (QResizeEvent *e)
{
    //printf ("Entering resizeEvent handler: ");
	QSize size=e->size ();
    //printf ("dx = %u - dy = %u\n", size.width (), size.height ());
	
	//resizes button in upper left corner...
	//button[0][0]->resize (size.width()/NumButtX, size.height()/NumButtY);
    
    //get_time (&begin_t);
    
    if (status == 1) {
        printf (">> begin resize event #%d\n", iter);
        get_time (&begin_t);
        status = 2;
    }
    
    //emits 'resized' signal...
    emit MW_ResizedSignal (size.width (), size.height ());
    
    //printf ("resize signal emitted\n");
    
    //get_time (&end_t);
    get_time (&inter_t);
}

//SERVICE THREAD...

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
        //printf ("[THREAD] Event posted...\n");
        usleep(RESIZE_DELAY_MICROSECS);
        if (status > 0) {
            printf("can't keep pace with resize events... "
                   "bailing out");
            while (status > 0) usleep(RESIZE_DELAY_MICROSECS/2);
        }
    }
}

bool MyApplication::notify (QObject* receiver, QEvent* e)
{
    //if (receiver==w)
        //printf ("Receiver: %p - Event: %d\n", receiver, e->type ());
    
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
        //printf ("Scheduling user event...\n");
        QEvent* dummy = new QEvent(QEvent::User);
        QCoreApplication::postEvent(w, dummy);
    }
    
    return res;
}

//MAIN...

int main( int argc, char **argv )
{
	int i;
	
	//QApplication a( argc, argv );
    MyApplication a( argc, argv );

	w=new MyWidget ();

	w->setGeometry (0, 0, MaxXSize, MaxYSize);
	w->show();
	
    MyThread *resizeThread = new MyThread ();
    resizeThread->start();
    
	int ret_value=a.exec();
    
    delete resizeThread;
	
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
