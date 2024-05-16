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
#include <QToolButton>
#include <QSlider>

// eBUS SDK
#include <PvDisplayWnd.h>

// project
#include "receiver.h"
#include "signalhandler.h"

static const std::string SEND_STR = "1 0 800    5 0 0   2 100 512 5 100 0      1 199 2500    2 200 400000 5 500 0   5 750 0 5 850 0";

class Gui : public QWidget, public SignalHandler
{
    Q_OBJECT
    
    public:
        explicit Gui(QWidget *parent = 0);
        bool isInitialised();
        void setImagePath(const std::string& path);
        bool handleSignal(int signal);
        void quit();

    protected:
        void keyPressEvent(QKeyEvent* event) override;
        void closeEvent(QCloseEvent *event) override;

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
        void onTorchClick();
        void onCommandClick();
        void onCommandEdit();

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
        QLineEdit* command_field;
        QSlider* torch_slider;
        QToolButton* torch_button;
        QToolButton* command_button;

        // The display widget is the container widget of the image display
        QWidget* display_widget;

        // PvDisplayWnd is an interface for displaying images
        PvDisplayWnd* display_wnd;

        // RECIEVER CLASS
        Receiver* receiver;
        
        bool init = false;

};

#endif // __GUI_H__