#include <iostream>
#include <signal.h>
#include <vector>

#include <PvSystem.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvStream.h>
#include <PvStreamGEV.h>


///
/// @brief Signal handler for ctrl-c presses.
///
volatile bool s_stop = false;
void catchSigInt(int sig)
{
    s_stop = true;
}
void initSigHandler()
{
    signal(SIGINT, catchSigInt);
}

/// @brief Select a connected gigE device and establish connection.
/// @param aConnectionID 
/// @param aType 
/// @return 
inline bool selectDevice( PvString *a_connection_ID)
{
    PvResult lResult;
    const PvDeviceInfo *selected_di = NULL;
    PvSystem lSystem;

    if ( s_stop )
    { 
        return false;
    }

    lSystem.Find();

    // Detect, select device.
    for (int i = 0; i < lSystem.GetInterfaceCount(); i++)
    {
        // For each detected interface
        const PvInterface *lInterface = dynamic_cast<const PvInterface *>( lSystem.GetInterface(i) );
        if ( lInterface != NULL )
        {
            // For each detected device
            for (int j = 0; j < lInterface->GetDeviceCount(); j++ )
            {
                const PvDeviceInfo *di = dynamic_cast<const PvDeviceInfo *>( lInterface->GetDeviceInfo( j ) );
                if ( di != NULL )
                {
                    // If the device is GigE compliant, use it
                    if (di->GetType() == PvDeviceInfoTypeGEV)
                    {
                        selected_di = di;
                        std::cout << lInterface->GetDisplayID().GetAscii() << std::endl;
                        std::cout << "\t" << di->GetDisplayID().GetAscii() << std::endl;
                        break;
                    }
                
                }					
            }
        }
    }

    // Did we find a good device?
    if (selected_di == NULL)
    {
        return false;
    }

    // If the IP Address valid?
    if (selected_di->IsConfigurationValid())
    {
        *a_connection_ID = selected_di->GetConnectionID();

        return true;
    }

    return false;
}

/// @brief Connect to a gigE device
/// @param aConnectionID 
/// @return 
PvDevice *connectToDevice( const PvString &a_connectionID )
{
    PvDevice *device;
    PvResult result;

    // Connect to the GigE Vision device
    device = PvDevice::CreateAndConnect( a_connectionID, &result );
    if (!result.IsOK())
    {
        std::cout << "Unable to connect to device" << std::endl;
        PvDevice::Free(device);
        return NULL;
    }

    return device;
}

PvStream* openStream( const PvString &a_connectionID )
{
    PvStream *stream;
    PvResult result;

    // Open stream to the GigE Vision device
    stream = PvStream::CreateAndOpen( a_connectionID, &result );
    if ((stream == NULL ) || !result.IsOK())
    {
        std::cout << "Error creating and opening stream" << std::endl;
        PvStream::Free(stream);
        return NULL;
    }

    return stream;
}

bool DumpGenParameterArray( PvGenParameterArray *aArray )
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

bool getDeviceSettings(PvDevice* a_device)
{
    if (a_device == NULL)
    {
        return false;
    }
    
    // Get the device's parameters array. It is built from the 
    // GenICam XML file provided by the device itself.
    PvGenParameterArray* lParameters = a_device->GetParameters();

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




