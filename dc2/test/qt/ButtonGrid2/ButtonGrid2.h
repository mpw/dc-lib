// =====================================================================
//  dc/test/qt/ButtonGrid2.h
// =====================================================================

//  Author:         (C) 2011 Camil Demetrescu, Andrea Ribichini
//  License:        See the end of this file for license information
//  Created:        February 24, 2011
//  Module:         dc/test/qt

//  Last changed:   $Date: 2012/07/24 14:24:41 $
//  Changed by:     $Author: ribbi $
//  Revision:       $Revision: 1.11 $


#include <QApplication>
#include <QPushButton>
#include <QWidget>
#include <QObject>
#include <QThread>

#define N 24

#define numIter 100

#define MaxXSize N*40
#define MaxYSize N*40*8/12

#define MinXSize MaxXSize/2
#define MinYSize MaxYSize/2

#define NumButtX N
#define NumButtY N

#define RESIZE_DELAY_MICROSECS 1000000

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
    Q_OBJECT
    
    public:
        int i, j;
        
        MyButton (const QString &text, QWidget *parent);
        //void resizeEvent (QResizeEvent *e); 
        
    signals:
        void MB_GeoChangeSignal (int, int);
        
    public slots:
        void MB_GeoChangeSlot (int, int);
};

class MyWidget : public QWidget
{
    Q_OBJECT
    
    public:
        MyButton* button[NumButtX][NumButtY];
        
        MyWidget (QWidget *parent=0);
        void resizeEvent (QResizeEvent *e);
        
    signals:
        void MW_ResizedSignal (int, int);
};

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
