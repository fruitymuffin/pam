#include "gui.h"

#include <QLabel>
#include <QSizePolicy>


Gui::Gui(QWidget *parent)
    : QWidget(parent), display_wnd(nullptr), display_widget(nullptr)
{
    // Create display adapter
    display_wnd = new PvDisplayWnd;
    display_wnd->SetBackgroundColor(255, 200, 255);
    receiver = new Receiver(display_wnd);
    display_widget = new QWidget();
    display_wnd->Create(display_widget);
    // Create the window layout
    createLayout();

    
    // Set window size
    //setWindowState(Qt::WindowMaximized);
    setFixedSize(720, 480);

    if(receiver->isConnected())
    {
        updateParameters();
    }
}

void Gui::createLayout()
{
    // Left: params grid
    // Right: image display
    // ___________________________
    // | _______ |               |
    // | |__|__| |               |
    // | |__|__| |               |
    // | |__|__| |    DISPLAY    |
    // | |__|__| |               |
    // | |__|__| |               |
    // |_________|_______________|

    createMenu();
    createDisplay();

    QHBoxLayout* main_layout = new QHBoxLayout(this);

    main_layout->addLayout(createMenu(), Qt::AlignLeft );
    main_layout->addWidget(display_widget);
    main_layout->setStretch(0, 2);
    main_layout->setStretch(1, 5);
}

void Gui::createDisplay()
{
    // Create some sort of display widget/context for outputting images
}

void Gui::updateParameters()
{
    DeviceParams dp = receiver->getDeviceParams();
    // When updating the display fields, if any edits were made
    name_field->setText(QString::fromStdString(dp.name));
    ip_field->setText(QString::fromStdString(dp.ip));
    mac_field->setText(QString::fromStdString(dp.mac));
    gain_field->setText(QString::fromStdString(dp.gain));
    m_exp_field->setText(QString::fromStdString(dp.exposure));
    bin_field->setText(QString::fromStdString(dp.binning));
    width_field->setText(QString::fromStdString(dp.width));
    height_field->setText(QString::fromStdString(dp.height));
}

QVBoxLayout* Gui::createMenu()
{
    //  QGridLayout   
    //  ___________ 
    // |_____|_____|
    // |_____|_____|
    // |_____|_____|
    // |_____|_____|

    // Parameter field Device
    QLabel* name_label = new QLabel(tr( "Name" ));
    name_field = new QLineEdit;
    name_field->setReadOnly( true );
    name_field->setEnabled( false );

    // Parameter field IP
    QLabel* ip_label = new QLabel(tr( "IP" ));
    ip_field = new QLineEdit;
    ip_field->setReadOnly( true );
    ip_field->setEnabled( false );

    // Parameter field MAC
    QLabel* mac_label = new QLabel(tr( "MAC" ));
    mac_field = new QLineEdit;
    mac_field->setReadOnly( true );
    mac_field->setEnabled( false );

    // Parameter field gain
    QLabel* gain_label = new QLabel(tr( "Gain" ));
    gain_field = new QLineEdit;
    gain_field->setReadOnly( true );
    gain_field->setEnabled( false );

    // Parameter field Measuring Exposure
    QLabel* m_exp_label = new QLabel(tr( "Meas Exp (us)" ));
    m_exp_field = new QLineEdit;
    m_exp_field->setReadOnly( true );
    m_exp_field->setEnabled( false );

    QLabel* bin_label = new QLabel(tr( "Pixel Binning" ));
    bin_field = new QLineEdit;
    bin_field->setReadOnly( false );
    bin_field->setEnabled( false );

    QLabel* width_label = new QLabel(tr( "Width" ));
    width_field = new QLineEdit;
    width_field->setReadOnly( true );
    width_field->setEnabled( false );

    QLabel* height_label = new QLabel(tr( "Height" ));
    height_field = new QLineEdit;
    height_field->setReadOnly( true );
    height_field->setEnabled( false );

    // Add fields to grid
    int row = 0;
    QGridLayout* grid_layout = new QGridLayout;
    grid_layout->addWidget(name_label, row, 0); row++;
    grid_layout->addWidget(name_field, row, 0); row++;
    grid_layout->addWidget(ip_label, row, 0); row++;
    grid_layout->addWidget(ip_field, row, 0); row++;
    grid_layout->addWidget(mac_label, row, 0); row++;
    grid_layout->addWidget(mac_field, row, 0); row++;
    grid_layout->addWidget(gain_label, row, 0); row++;
    grid_layout->addWidget(gain_field, row, 0); row++;
    grid_layout->addWidget(m_exp_label, row, 0); row++;
    grid_layout->addWidget(m_exp_field, row, 0); row++;
    grid_layout->addWidget(bin_label, row, 0); row++;
    grid_layout->addWidget(bin_field, row, 0); row++;
    grid_layout->addWidget(width_label, row, 0); row++;
    grid_layout->addWidget(width_field, row, 0); row++;
    grid_layout->addWidget(height_label, row, 0); row++;
    grid_layout->addWidget(height_field, row, 0);

    QVBoxLayout* menu_layout = new QVBoxLayout;
    menu_layout->addLayout(grid_layout);

    // Stretch element at the bottom of the layout files the remaining vertical space
    menu_layout->addStretch();

    return menu_layout;
}

// Keypress event
void Gui::keyPressEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_Space:
            // start/stop stream
            if (receiver->isConnected())
            {
                if (receiver->isAcquiring())
                {
                    receiver->stopAcquisition();
                }
                else
                {
                    receiver->startAcquisition();
                }

            }
            break;

        case Qt::Key_B:
            if (receiver->isConnected())
            {
                receiver->toggleBinning();
            }
            break;
    }
}