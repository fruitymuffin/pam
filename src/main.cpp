#include <QApplication>

#include "gui.h"

int main( int argc, char *argv[] )
{
	QLocale::setDefault( QLocale( QLocale::English, QLocale::Australia ) );
	QApplication app(argc, argv);


    QCoreApplication::setOrganizationName( "UTS" );
    QCoreApplication::setApplicationName( "Pam Gui" );

    Gui gui;
    gui.show();

    return app.exec();
}

