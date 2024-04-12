#include "receiver.h"

Receiver::Receiver()
{
    //selectDevice();
}

bool Receiver::selectDevice()
{
    PvResult result;
    PvSystem system;
    const PvDeviceInfo *device_info = nullptr;

    system.Find();

    // Detect, select device.
    for (int i = 0; i < system.GetInterfaceCount(); i++)
    {
        // For each detected interface
        const PvInterface* interface = dynamic_cast<const PvInterface *>(system.GetInterface(i));
        if (interface != nullptr)
        {
            // For each detected device
            for (int j = 0; j < interface->GetDeviceCount(); j++)
            {
                const PvDeviceInfo *di = dynamic_cast<const PvDeviceInfo *>(interface->GetDeviceInfo(j));
                if (di != nullptr)
                {
                    // If the device is GigE compliant, use it
                    if (di->GetType() == PvDeviceInfoTypeGEV)
                    {
                        device_info = di;
                        std::cout << interface->GetDisplayID().GetAscii() << std::endl;
                        std::cout << "\t" << di->GetDisplayID().GetAscii() << std::endl;
                        break;
                    }
                
                }					
            }
        }
    }

    // Did we find a good device?
    if (device_info == nullptr)
    {
        return false;
    }

    // If the IP Address valid?
    if (device_info->IsConfigurationValid())
    {
        connection_id = device_info->GetConnectionID();

        return true;
    }

    return false;
}

bool Receiver::connectToDevice()
{
    PvResult result;

    // Connect to the GigE Vision device
    device = PvDevice::CreateAndConnect(connection_id, &result );
    if (!result.IsOK())
    {
        PvDevice::Free(device);
        return false;
    }
    return true;
}

bool Receiver::openStream()
{
    PvResult result;

    // Open stream to the GigE Vision device
    stream = PvStream::CreateAndOpen(connection_id, &result );
    if ((stream == nullptr) || !result.IsOK())
    {
        PvStream::Free(stream);
        return false;
    }
    return true;
}