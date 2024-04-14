// *****************************************************************************
//
// Receiver.h
// Implements the specific eBUS library functionality we need for
// the pam vision system.
// Handles camera connection, configuration, and streaming.
// Displaying images and saving the raw data is handled by seperate
// classes.
//
// *****************************************************************************


#ifndef __RECEIVER_H__
#define __RECEIVER_H__

// std
#include <iostream>
#include <signal.h>
#include <vector>
#include <list>
#include <iomanip>

// eBUS SDK
#include <PvSystem.h>
#include <PvDevice.h>
#include <PvDeviceGEV.h>
#include <PvStream.h>
#include <PvStreamGEV.h>
#include <PvBuffer.h>
#include <PvPipeline.h>
#include <PvDisplayWnd.h>

// project
#include "displaythread.h"

// Default software-side params
#define DEFAULT_BUFFER_COUNT 4

// Default camera-side params
#define DEFAULT_EXPOSURE_TIME 32000
#define DEFAULT_MULTIFRAME_COUNT 8
#define DEFAULT_MULTIFRAME_RATE 8
#define DEFAULT_SENSOR_GAIN 1

// Typedefs
typedef std::list<PvBuffer *> BufferList;

struct DeviceParams
{
    std::string name;
    std::string ip;
    std::string mac;
    std::string gain;
    std::string exposure;
};

// Receiver
class Receiver
{
    public:
        Receiver(PvDisplayWnd* _display_wnd);
        ~Receiver();
        
        // Pulic functions
        bool isConnected();
        bool selectDevice();
        bool openStream();
        bool connectToDevice();
        void configureStream();
        void acquireImages();
        void createStreamBuffers();
        void freeStreamBuffers();
        bool DumpGenParameterArray(PvGenParameterArray *aArray );
        bool getDeviceSettings();
        DeviceParams getDeviceParams();

    private:
        // Reciever will own a device, connection, stream and pipeline
        PvString connection_id;
        PvDevice* device;
        PvStream* stream;
        PvPipeline* pipeline;
        BufferList buffers;
        PvDisplayWnd* display_wnd;
        DisplayThread* display_thread;
        DeviceParams device_params;
};


#endif // __RECIEVER_H__