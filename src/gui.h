// *****************************************************************************
//
// Gui.h
// Qt widget for displaying images and changing camera settings
//
// *****************************************************************************

#ifndef __GUI_H__
#define __GUI_H__

// Qt5
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>

// eBUS SDK
#include <PvDisplayWnd.h>

// project
#include "receiver.h"

class Gui : public QWidget
{
    Q_OBJECT
    
    public:
        explicit Gui(QWidget *parent = 0);
        void createLayout();
    private:
        // GRAPHICAL ELEMENTS FOR UI
        QVBoxLayout* createMenu();
        void createDisplay();

        // PvDisplayWnd is an interface for displaying images
        PvDisplayWnd* display_wnd;
        

        // RECIEVER CLASS
        // Receiver receiver;

};

#endif // __GUI_H__