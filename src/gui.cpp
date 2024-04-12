#include "gui.h"

#include <QLabel>
#include <QLineEdit>


Gui::Gui(QWidget *parent)
    : QWidget(parent), display_wnd(nullptr)
{
    // Set window to be maximised
    setWindowState(Qt::WindowMaximized);

    display_wnd = new PvDisplayWnd;

    // Create the window layout
    createLayout();
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

    QVBoxLayout* menu_layout = createMenu();
    main_layout->addLayout(menu_layout, Qt::AlignLeft );
    main_layout->addWidget(display_wnd->GetQWidget());
}

void Gui::createDisplay()
{
    // Create some sort of display widget/context for outputting images
}

QVBoxLayout* Gui::createMenu()
{
    //  QGridLayout   
    //  ___________ 
    // |_____|_____|
    // |_____|_____|
    // |_____|_____|
    // |_____|_____|


    // Parameter field Test
    QLabel* ip_label = new QLabel(tr( "IP Address" ));
    QLineEdit* ip_field = new QLineEdit;
    ip_field->setReadOnly( true );
    ip_field->setEnabled( false );

    // Parameter field Test
    QLabel* mac_label = new QLabel(tr( "MAC Address" ));
    QLineEdit* mac_field = new QLineEdit;
    mac_field->setReadOnly( true );
    mac_field->setEnabled( false );

    // Add fields to grid
    int row = 0;
    QGridLayout* grid_layout = new QGridLayout;
    grid_layout->addWidget(ip_label, row, 0);
    grid_layout->addWidget(ip_field, row, 1); row++;
    grid_layout->addWidget(mac_label, row, 0);
    grid_layout->addWidget(mac_field, row, 1);

    QVBoxLayout* menu_layout = new QVBoxLayout;
    menu_layout->addLayout(grid_layout);
    menu_layout->addStretch();

    return menu_layout;
}