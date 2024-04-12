#include <QApplication>

#include "gui.h"

// void openCam()
// {
//     PvDevice* device = NULL;
//     PvStream* stream = NULL;
//     BufferList buffers;

//     PvString connectionID;
//     if ( selectDevice( &connectionID ) )
//     {
//         device = connectToDevice( connectionID );
//         if (device != NULL)
//         {
//             std::cout << "Connected!" << std::endl;

//             getDeviceSettings(device);

//             stream = openStream(connectionID);
//             if (stream != NULL)
//             {
//                 configureStream(device, stream);
//                 createStreamBuffers(device, stream, &buffers);

//                 acquireImages(device, stream);
//             }

            

//             stream->Close();
//             PvStream::Free(stream); 

//             device->Disconnect();
//             PvDevice::Free(device);
//         }
//     }
// }


void SignalHangler(int sig)
{
	qApp->quit();
}

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

