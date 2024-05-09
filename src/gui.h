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
#include <QRadioButton>
#include <QVBoxLayout>
#include <QKeyEvent>

// eBUS SDK
#include <PvDisplayWnd.h>

// project
#include "receiver.h"
#include "signalhandler.h"

static const std::string SEND_STR = "1 0 800    5 0 0    5 100 0 2 100 512     1 199 1200    5 200 0 2 200 300000 5 498 0    5 700 0";

class Gui : public QWidget, public SignalHandler
{
    Q_OBJECT
    
    public:
        explicit Gui(QWidget *parent = 0);
        void setImagePath(const std::string& path);
        bool handleSignal(int signal);
        void quit();

    protected:
        void keyPressEvent(QKeyEvent* event) override;

    private:
        // Ui element functions
        void createLayout();
        QVBoxLayout* createMenu();
        void createDisplay();
        void updateParameters();

    public slots:
        void onExposureEdit();
        void onBinningEdit();
        void onGainEdit();

    private:
        // Ui element variables
        QLineEdit* name_field;
        QLineEdit* ip_field;
        QLineEdit* mac_field;
        QLineEdit* gain_field;
        QLineEdit* m_exp_field;
        QRadioButton* bin_field;
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