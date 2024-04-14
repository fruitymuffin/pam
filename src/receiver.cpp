#include "receiver.h"
#include "stringtools.h"
#include <PvDecompressionFilter.h>

Receiver::Receiver(PvDisplayWnd* _display_wnd) :
    display_wnd(_display_wnd),
    device(nullptr),
    stream(nullptr),
    pipeline(nullptr),
    display_thread(nullptr)
{
    if (selectDevice() )
    {
        connectToDevice();
        if (device != NULL)
        {
            openStream();
            if (stream != NULL)
            {
                configureStream();
                device->StreamEnable();

                PvGenParameterArray* params = device->GetParameters();
                PvGenCommand *start_cmd = dynamic_cast<PvGenCommand *>( params->Get( "AcquisitionStart" ) );
                start_cmd->Execute();

                display_thread = new DisplayThread(display_wnd);
                

                pipeline = new PvPipeline(stream);
                
                display_thread->Start(pipeline, params);
                pipeline->Start();

                display_thread->SetPriority(50);
            }
        }
    }
}

DeviceParams Receiver::getDeviceParams()
{
    // If the camera is connected
    if(isConnected())
    {
        // Get the device's parameters array. It is built from the 
        // GenICam XML file provided by the device itself.
        PvGenParameterArray* params = device->GetParameters();

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
    }

    return device_params;
}

Receiver::~Receiver()
{
    stream->Close();
    PvStream::Free(stream); 
    device->Disconnect();
    PvDevice::Free(device);
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

void Receiver::acquireImages()
{
    // Get device parameters need to control streaming
    PvGenParameterArray *lDeviceParams = device->GetParameters();

    // Map the GenICam AcquisitionStart and AcquisitionStop commands
    PvGenCommand *lStart = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStart" ) );
    PvGenCommand *lStop = dynamic_cast<PvGenCommand *>( lDeviceParams->Get( "AcquisitionStop" ) );

    // Get stream parameters
    PvGenParameterArray *lStreamParams = stream->GetParameters();

    // Map a few GenICam stream stats counters
    PvGenFloat *lFrameRate = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "AcquisitionRate" ) );
    PvGenFloat *lBandwidth = dynamic_cast<PvGenFloat *>( lStreamParams->Get( "Bandwidth" ) );

    // Enable streaming and send the AcquisitionStart command
    device->StreamEnable();
    lStart->Execute();

    char lDoodle[] = "|\\-|-/";
    int lDoodleIndex = 0;
    double lFrameRateVal = 0.0;
    double lBandwidthVal = 0.0;
    int lErrors = 0;

    PvDecompressionFilter lDecompressionFilter;

    // Acquire images until the user instructs us to stop.
    std::cout << "Streaming started!" << std::endl;
    while ( true)
    {
        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;

        // Retrieve next buffer
        PvResult lResult = stream->RetrieveBuffer( &lBuffer, &lOperationResult, 1000 );
        if ( lResult.IsOK() )
        {
            if ( lOperationResult.IsOK() )
            {
                //
                // We now have a valid buffer. This is where you would typically process the buffer.
                // -----------------------------------------------------------------------------------------
                // ...

                lFrameRate->GetValue( lFrameRateVal );
                lBandwidth->GetValue( lBandwidthVal );

                std::cout << std::fixed << std::setprecision( 1 );
                std::cout << lDoodle[ lDoodleIndex ];
                std::cout << " BlockID: " << std::uppercase << std::hex << std::setfill( '0' ) << std::setw( 16 ) << lBuffer->GetBlockID();

                switch ( lBuffer->GetPayloadType() )
                {
                case PvPayloadTypeImage:
                    std::cout << "  W: " << std::dec << lBuffer->GetImage()->GetWidth() << " H: " << lBuffer->GetImage()->GetHeight();
                    break;

                case PvPayloadTypeChunkData:
                    std::cout << " Chunk Data payload type" << " with " << lBuffer->GetChunkCount() << " chunks";
                    break;

                case PvPayloadTypeRawData:
                    std::cout << " Raw Data with " << lBuffer->GetRawData()->GetPayloadLength() << " bytes";
                    break;

                case PvPayloadTypeMultiPart:
                    std::cout << " Multi Part with " << lBuffer->GetMultiPartContainer()->GetPartCount() << " parts";
                    break;

                case PvPayloadTypePleoraCompressed:
                    {
                        PvPixelType lPixelType = PvPixelUndefined;
                        uint32_t lWidth = 0, lHeight = 0;
                        PvDecompressionFilter::GetOutputFormatFor( lBuffer, lPixelType, lWidth, lHeight );
                        uint32_t lCalculatedSize = PvImage::GetPixelSize( lPixelType ) * lWidth * lHeight / 8;

                        PvBuffer lDecompressedBuffer;
                        // If the buffer is compressed, start by decompressing it
                        if ( lDecompressionFilter.IsCompressed( lBuffer ) )
                        {
                            lResult = lDecompressionFilter.Execute( lBuffer, &lDecompressedBuffer );
                            if ( !lResult.IsOK() )
                            {
                                break;
                            }
                        }

                        uint32_t lDecompressedSize = lDecompressedBuffer.GetSize();
                        if ( lDecompressedSize!= lCalculatedSize )
                        {
                            lErrors++;
                        }
                        double lCompressionRatio = static_cast<double>( lDecompressedSize ) / static_cast<double>( lBuffer->GetAcquiredSize() );
                        std::cout << std::dec << " Pleora compressed.   Compression Ratio " << lCompressionRatio;
                        std::cout << " Errors: " << lErrors;
                    }
                    break;

                default:
                    std::cout << " Payload type not supported by this sample";
                    break;
                }
                std::cout << "  " << lFrameRateVal << " FPS  " << ( lBandwidthVal / 1000000.0 ) << " Mb/s   \r";
            }
            else
            {
                // Non OK operational result
                std::cout << lDoodle[ lDoodleIndex ] << " " << lOperationResult.GetCodeString().GetAscii() << "\r";
            }

            // Re-queue the buffer in the stream object
            stream->QueueBuffer( lBuffer );
        }
        else
        {
            // Retrieve buffer failure
            std::cout << lDoodle[ lDoodleIndex ] << " " << lResult.GetCodeString().GetAscii() << "\r";
        }

        ++lDoodleIndex %= 6;
    }

    std::cout << std::endl << std::endl;

    // Tell the device to stop sending images.
    std::cout << "Sending AcquisitionStop command to the device" << std::endl;
    lStop->Execute();

    // Disable streaming on the device
    std::cout << "Disable streaming on the controller." << std::endl;
    device->StreamDisable();

    // Abort all buffers from the stream and dequeue
    std::cout << "Aborting buffers still in stream" << std::endl;
    stream->AbortQueuedBuffers();
    while ( stream->GetQueuedBufferCount() > 0 )
    {
        PvBuffer *lBuffer = NULL;
        PvResult lOperationResult;

        stream->RetrieveBuffer( &lBuffer, &lOperationResult );
    }
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




bool Receiver::DumpGenParameterArray( PvGenParameterArray *aArray )
{
    // Getting array size
    uint32_t lParameterArrayCount = aArray->GetCount();
    std::cout << std::endl;
    std::cout << "Array has " << lParameterArrayCount << " parameters" << std::endl;

    // Traverse through Array and print out parameters available.
    for( uint32_t x = 0; x < lParameterArrayCount; x++ )
    {
        // Get a parameter
        PvGenParameter *lGenParameter = aArray->Get( x );

        // Don't show invisible parameters - display everything up to Guru.
        if ( !lGenParameter->IsVisible( PvGenVisibilityGuru ) )
        {
            continue;
        }

        // Get and print parameter's name.
        PvString lGenParameterName, lCategory;
        lGenParameter->GetCategory( lCategory );
        lGenParameter->GetName( lGenParameterName );
        std::cout << lCategory.GetAscii() << ":" << lGenParameterName.GetAscii() << ", ";

        // Parameter available?
        if ( !lGenParameter->IsAvailable() )
        {
            std::cout << "{Not Available}" << std::endl;
            continue;
        }

        // Parameter readable?
        if ( !lGenParameter->IsReadable() )
        {
            std::cout << "{Not readable}" << std::endl;
            continue;
        }
        
        // Get the parameter type
        PvGenType lType;
        lGenParameter->GetType( lType );
        switch ( lType )
        {
            case PvGenTypeInteger:
                {
                    int64_t lValue;             
                    static_cast<PvGenInteger *>( lGenParameter )->GetValue( lValue );
                    std::cout << "Integer: " << lValue;
                }
                break;
            
            case PvGenTypeEnum:
                {
                    PvString lValue;                
                    static_cast<PvGenEnum *>( lGenParameter )->GetValue( lValue );
                    std::cout << "Enum: " << lValue.GetAscii();
                }
                break;
            
            case PvGenTypeBoolean:
                {
                    bool lValue;                
                    static_cast<PvGenBoolean *>( lGenParameter )->GetValue( lValue );
                    if( lValue ) 
                    {
                        std::cout << "Boolean: TRUE";
                    }
                    else 
                    {
                        std::cout << "Boolean: FALSE";
                    }
                }
                break;

            case PvGenTypeString:
                {
                    PvString lValue;
                    static_cast<PvGenString *>( lGenParameter )->GetValue( lValue );
                    std::cout << "String: " << lValue.GetAscii();
                }
                break;

            case PvGenTypeCommand:
                std::cout << "Command";
                break;

            case PvGenTypeFloat:
                {
                    double lValue;              
                    static_cast<PvGenFloat *>( lGenParameter )->GetValue( lValue );
                    std::cout << "Float: " << lValue;
                }
                break;
                
            default:
                // Unexpected
                break;
        }
        std::cout << std::endl;
    }

    return true;
}

bool Receiver::getDeviceSettings()
{
    if (device == NULL)
    {
        return false;
    }
    
    // Get the device's parameters array. It is built from the 
    // GenICam XML file provided by the device itself.
    PvGenParameterArray* lParameters = device->GetParameters();

    // Dumping device's parameters array content.
    DumpGenParameterArray( lParameters );

    // Get width parameter - mandatory GigE Vision parameter, it should be there.
    PvGenParameter *lParameter = lParameters->Get( "Width" );

    // Converter generic parameter to width using dynamic cast. If the
    // type is right, the conversion will work otherwise lWidth will be NULL.
    PvGenInteger *lWidthParameter = dynamic_cast<PvGenInteger *>( lParameter );

    if ( lWidthParameter == NULL )
    {
        std::cout << "Unable to get the width parameter." << std::endl;
    }

    // Read current width value.
    int64_t lOriginalWidth = 0;
    if ( !(lWidthParameter->GetValue( lOriginalWidth ).IsOK()) )
    {
        std::cout << "Error retrieving width from device" << std::endl;   
    }

    // Read max.
    int64_t lMaxWidth = 0;
    if ( !(lWidthParameter->GetMax( lMaxWidth ).IsOK()) )
    {
        std::cout << "Error retrieving width max from device" << std::endl;   
    }

    // Change width value.
    if ( !lWidthParameter->SetValue( lMaxWidth ).IsOK() )
    {
        std::cout << "Error changing width on device - the device is on Read Only Mode, please change to Exclusive to change value" << std::endl; 
    } 

    // Reset width to original value.
    if ( !lWidthParameter->SetValue( lOriginalWidth ).IsOK() )
    {
        std::cout << "Error changing width on device" << std::endl;   
    }

    return true;
}

