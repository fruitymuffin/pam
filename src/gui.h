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
#include <QLineEdit>
#include <QVBoxLayout>
#include <QKeyEvent>

// eBUS SDK
#include <PvDisplayWnd.h>

// project
#include "receiver.h"

#define MENU_WIDTH 120

class Gui : public QWidget
{
    Q_OBJECT
    
    public:
        explicit Gui(QWidget *parent = 0);
        void createLayout();

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        // Ui element functions
        QVBoxLayout* createMenu();
        void createDisplay();
        void updateParameters();

    private:
        // Ui element variables
        QLineEdit* name_field;
        QLineEdit* ip_field;
        QLineEdit* mac_field;
        QLineEdit* gain_field;
        QLineEdit* m_exp_field;
        QLineEdit* bin_field;
        QLineEdit* width_field;
        QLineEdit* height_field;

        // The display widget is the container widget of the image display
        QWidget* display_widget;

        // PvDisplayWnd is an interface for displaying images
        PvDisplayWnd* display_wnd;

        // RECIEVER CLASS
        Receiver* receiver;
        
        bool init = false;

};

#endif // __GUI_H__