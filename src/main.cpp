#include "ebusutils.h"

const std::string CAM_NAME = "GOX-5103M-PGE";

int main()
{
    PvDevice* device = NULL;
    PvStream* stream = NULL;

    PvString connectionID;
    if ( selectDevice( &connectionID ) )
    {
        device = connectToDevice( connectionID );
        if (device != NULL)
        {
            std::cout << "Connected!" << std::endl;

            getDeviceSettings(device);

            stream = openStream(connectionID);
            if (stream != NULL)
            {
                std::cout << "Opened stream!" << std::endl;
            }



            stream->Close();
            PvStream::Free(stream); 

            device->Disconnect();
            PvDevice::Free(device);
        }
    }

    return 0;
}