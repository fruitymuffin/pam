#include <QApplication>

#include "gui.h"

const std::string saving_path = "images/";

int main( int argc, char *argv[] )
{
	QLocale::setDefault( QLocale( QLocale::English, QLocale::Australia ) );
	QApplication app(argc, argv);


    QCoreApplication::setOrganizationName( "UTS" );
    QCoreApplication::setApplicationName( "Pam Gui" );

    Gui gui;
    gui.setImagePath(saving_path);
    gui.show();

    return app.exec();
}

