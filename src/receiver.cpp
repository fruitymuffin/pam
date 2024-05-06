#include "receiver.h"
#include "stringtools.h"
#include <PvDecompressionFilter.h>

Receiver::Receiver(PvDisplayWnd* _display_wnd) :
    display_wnd(_display_wnd),
    device(nullptr),
    stream(nullptr),
    pipeline(nullptr),
    display_thread(nullptr),
    params(nullptr)
{
    if (selectDevice() )
    {
        connectToDevice();
        if (device != NULL)
        {
            params = device->GetParameters();
            openStream();
            if (stream != NULL)
            {
                configureStream();
                device->StreamEnable();

                // Start image acquisition (continuous)
                startAcquisition();

                // Start the display thread/pipeline to put images on the screen
                display_thread = new DisplayThread(display_wnd);
                pipeline = new PvPipeline(stream);
                
                display_thread->Start(pipeline, params);
                pipeline->Start();

                // Docs said to do this....I assume so that the thread gets time from OS?
                display_thread->SetPriority(50);
            }
        }
    }
}

Receiver::~Receiver()
{
    stream->Close();
    PvStream::Free(stream); 
    device->Disconnect();
    PvDevice::Free(device);
}

DeviceParams Receiver::getDeviceParams()
{
    // If the camera is connected
    if(isConnected())
    {
        // Get the device's parameters array. It is built from the 
        // GenICam XML file provided by the device itself.
        params = device->GetParameters();

        PvString val_str;
        int64_t val_int;
        double val_float;

        // Update the struct with relevant parameters.
        params->GetString("DeviceModelName")->GetValue(val_str);
        device_params.name = val_str.GetAscii();

        params->GetInteger("GevCurrentIPAddress")->GetValue(val_int);
        device_params.ip = StringTools::ipToString(val_int);

        params->GetInteger("GevMACAddress")->GetValue(val_int);
        device_params.mac = StringTools::macToString(val_int);

        params->GetFloat("Gain")->GetValue(val_float);
        device_params.gain = StringTools::doubleToString(val_float);

        params->GetFloat("ExposureTime")->GetValue(val_float);
        device_params.exposure = StringTools::doubleToString(val_float);

        params->GetInteger("BinningHorizontal")->GetValue(val_int);
        device_params.binning = std::to_string(val_int);

        params->GetInteger("Width")->GetValue(val_int);
        device_params.width = std::to_string(val_int);

        params->GetInteger("Height")->GetValue(val_int);
        device_params.height = std::to_string(val_int);
    }

    return device_params;
}


bool Receiver::isConnected()
{
    return device->IsConnected();
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

    if(connection_id != "")

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

void Receiver::configureStream()
{
    // Configuration of stream is only necessary for gigE cameras. Check type by attempting dynamic cast.
    PvDeviceGEV* device_gev = dynamic_cast<PvDeviceGEV *>(device);
    if ( device_gev != NULL )
    {
        PvStreamGEV *stream_gev = static_cast<PvStreamGEV *>(stream);

        // Negotiate packet size. Alternatively we could manually set packet size.
       device_gev->NegotiatePacketSize();

        // The streaming destination IP should be the ip of the network adapter on the up-board that the camera is conencted to.
        device_gev->SetStreamDestination(stream_gev->GetLocalIPAddress(), stream_gev->GetLocalPort() );
    }
}

bool Receiver::isAcquiring()
{
    return acquiring;
}

void Receiver::stopAcquisition()
{
    PvGenCommand *cmd = dynamic_cast<PvGenCommand *>( params->Get( "AcquisitionStop" ) );
    
    // Send stop acquisition command
    cmd->Execute();

    acquiring = false;
}

void Receiver::startAcquisition()
{
    resetStream();

    PvGenCommand *cmd = dynamic_cast<PvGenCommand *>( params->Get( "AcquisitionStart" ) );

    // Send start acquisition command
    cmd->Execute();

    acquiring = true;
}

void Receiver::createStreamBuffers()
{
    // Reading payload size from device
    uint32_t psize = device->GetPayloadSize();
    std::cout << "DEVICE PAYLOAD SIZE: " << psize << std::endl;

    // Use BUFFER_COUNT or the maximum number of buffers, whichever is smaller
    uint32_t buffer_count = (stream->GetQueuedBufferMaximum() < DEFAULT_BUFFER_COUNT ) ? stream->GetQueuedBufferMaximum() : DEFAULT_BUFFER_COUNT;
    
    std::cout << "DEVICE BUFFER COUNT: " << buffer_count << std::endl;

    // Allocate buffers
    for ( uint32_t i = 0; i < buffer_count; i++ )
    {
        // Create new buffer object
        PvBuffer *buffer = new PvBuffer;

        // Allocate memory
        buffer->Alloc( static_cast<uint32_t>(psize) );
        
        // Add to external list - used to eventually release the buffers
        buffers.push_back(buffer);
    }
    
    // Queue all buffers in the stream
    BufferList::iterator it = buffers.begin();
    while (it != buffers.end() )
    {
        stream->QueueBuffer(*it);
        it++;
    }
}

void Receiver::freeStreamBuffers()
{
    // Go through the buffer list
    BufferList::iterator it = buffers.begin();
    while (it != buffers.end())
    {
        delete *it;
        it++;
    }

    // Clear the buffer list 
    buffers.clear();
}

void Receiver::startTriggeredMultiframe(int n)
{
    stopAcquisition();
    // Settings to apply are acquisition mode and trigger enable
    PvGenEnum *cmd = dynamic_cast<PvGenEnum *>( params->Get( "AcquisitionMode" ) );
    cmd->SetValue("MultiFrame");

    cmd = dynamic_cast<PvGenEnum*>(params->Get("TriggerMode"));
    cmd->SetValue("On");

    //cmd = dynamic_cast<PvGenCommand *>( params->Get( "" ) );

}

void Receiver::toggleBinning()
{
    stopAcquisition();
    device->StreamDisable();
    // Read current binning param
    PvGenInteger* bin_h = dynamic_cast<PvGenInteger *>(params->Get("BinningHorizontal"));
    PvGenInteger* bin_v = dynamic_cast<PvGenInteger *>(params->Get("BinningVertical"));

    int64_t val;
    bin_h->GetValue(val);
    if (val == 1)
    {
        bin_v->SetValue(2);
        bin_h->SetValue(2);
    }
    else
    {
        bin_v->SetValue(1);
        bin_h->SetValue(1);
    }

    device->StreamEnable();
    startAcquisition();
}

void Receiver::resetStream()
{
    display_thread->ResetStatistics();
    uint32_t payload_size = device->GetPayloadSize();
    if (payload_size > 0)
    {
        pipeline->SetBufferSize(payload_size);
    }

    pipeline->Reset();
}

