#include "gui.h"
#include <QLabel>
#include <QSizePolicy>
#include <QCloseEvent>
#include "tools.h"


Gui::Gui(QWidget *parent)
    : QWidget(parent), display_wnd(nullptr), display_widget(nullptr), SignalHandler(SignalHandler::SIG_INT)
{
    // Create display adapter
    display_wnd = new PvDisplayWnd;
    display_wnd->SetBackgroundColor(255, 200, 255);
    display_widget = new QWidget();
    display_wnd->Create(display_widget);

    // Create the window layout
    createLayout();

    setFixedSize(760, 420);

    receiver = new Receiver(display_wnd);
    if(receiver->isConnected())
    {
        updateParameters();
    }
}

void Gui::closeEvent(QCloseEvent *event)
{
    quit();
}

bool Gui::isInitialised()
{
    return receiver->isConnected();
}

bool Gui::handleSignal(int signal)
{
    quit();
    
    return true;
}

void Gui::quit()
{
    receiver->quit();
    delete receiver;
    display_widget->close();
    display_wnd->Close();
}

void Gui::setImagePath(const std::string& path)
{
    receiver->setSavingPath(path);
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
    main_layout->setStretch(0, 3);
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
    gain_field->setReadOnly( false );
    gain_field->setEnabled( true );

    // Parameter field Measuring Exposure
    QLabel* m_exp_label = new QLabel(tr( "Meas Exp (us)" ));
    m_exp_field = new QLineEdit;
    m_exp_field->setReadOnly( false );
    m_exp_field->setEnabled( true );

    QLabel* bin_label = new QLabel(tr( "Pixel Binning" ));
    bin_field = new QRadioButton;
    bin_field->setEnabled( true );

    QLabel* width_label = new QLabel(tr( "Width" ));
    width_field = new QLineEdit;
    width_field->setReadOnly( true );
    width_field->setEnabled( false );

    QLabel* height_label = new QLabel(tr( "Height" ));
    height_field = new QLineEdit;
    height_field->setReadOnly( true );
    height_field->setEnabled( false );

    QLabel* torch_label = new QLabel(tr( "Torch" ));
    torch_button = new QToolButton();
    torch_button->setCheckable(true);
    torch_button->setText(tr("Torch ON/OFF"));
    torch_slider = new QSlider(Qt::Horizontal);
    torch_slider->setMinimum(0);
    torch_slider->setMaximum(22);
    torch_slider->setSingleStep(1);

    QLabel* command_label = new QLabel(tr( "Command" ));
    command_field = new QLineEdit;
    command_button = new QToolButton();
    command_button->setText(tr("Send->"));

    // Add fields to grid
    int row = 0;
    QGridLayout* grid_layout = new QGridLayout;
    grid_layout->addWidget(name_label, row, 0);
    grid_layout->addWidget(name_field, row, 1); row++;
    grid_layout->addWidget(ip_label, row, 0);
    grid_layout->addWidget(ip_field, row, 1); row++;
    grid_layout->addWidget(mac_label, row, 0); 
    grid_layout->addWidget(mac_field, row, 1); row++;
    grid_layout->addWidget(gain_label, row, 0); 
    grid_layout->addWidget(gain_field, row, 1); row++;
    grid_layout->addWidget(m_exp_label, row, 0); 
    grid_layout->addWidget(m_exp_field, row, 1); row++;
    grid_layout->addWidget(bin_label, row, 0); ;
    grid_layout->addWidget(bin_field, row, 1); row++;
    grid_layout->addWidget(width_label, row, 0); 
    grid_layout->addWidget(width_field, row, 1); row++;
    grid_layout->addWidget(height_label, row, 0);
    grid_layout->addWidget(height_field, row, 1); row++;
    grid_layout->addWidget(torch_label, row, 0);
    grid_layout->addWidget(torch_slider, row, 1); row++;
    grid_layout->addWidget(torch_button, row, 1); row++;
    grid_layout->addWidget(command_label, row, 0);
    grid_layout->addWidget(command_field, row, 1); row++;
    grid_layout->addWidget(command_button, row, 1);

    QVBoxLayout* menu_layout = new QVBoxLayout;
    menu_layout->addLayout(grid_layout);

    // Stretch element at the bottom of the layout files the remaining vertical space
    menu_layout->addStretch();

    connect(m_exp_field, SIGNAL(editingFinished()), this, SLOT(onExposureEdit()));
    connect(bin_field, SIGNAL(clicked()), this, SLOT(onBinningEdit()));
    connect(gain_field, SIGNAL(editingFinished()), this, SLOT(onGainEdit()));
    connect(command_field, SIGNAL(editingFinished()), this, SLOT(onCommandEdit()));
    connect(torch_button, SIGNAL(released()), this, SLOT(onTorchClick()));
    connect(command_button, SIGNAL(released()), this, SLOT(onCommandClick()));


    return menu_layout;
}

void Gui::onCommandEdit()
{
    setFocus(Qt::OtherFocusReason);
}

void Gui::onCommandClick()
{
    QString str = command_field->text();
    Tools::sendSerialString(str.toStdString());
    setFocus(Qt::OtherFocusReason);
}

void Gui::onTorchClick()
{
    if (torch_button->isChecked())
    {
        int val = torch_slider->value() * 100;
        std::stringstream ss;
        ss << "0 0 " << val;
        Tools::sendSerialString(ss.str());
    }
    else
    {
        Tools::sendSerialString("0 0 0");
    }
    setFocus(Qt::OtherFocusReason);
}

void Gui::onExposureEdit()
{
    if (receiver->isConnected())
    {
        QString str = m_exp_field->text();
        receiver->setExposure(str.toInt());
    }
    updateParameters();
    setFocus(Qt::OtherFocusReason);
}

void Gui::onBinningEdit()
{
    if (receiver->isConnected())
    {
        receiver->setBinning(bin_field->isChecked());
    }
    updateParameters();
    setFocus(Qt::OtherFocusReason);
}

void Gui::onGainEdit()
{
    if (receiver->isConnected())
    {
        QString str = gain_field->text();
        receiver->setGain(str.toInt());
    }
    setFocus(Qt::OtherFocusReason);
    updateParameters();
}

// Keypress event
void Gui::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space)
    {
        // start/stop stream
            if (receiver->isConnected())
            {
                receiver->setState();
            }
    }
}