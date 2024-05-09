// *****************************************************************************
//
// Receiver.h
// Implements the specific eBUS library functionality we need for
// the pam vision system.
// Handles camera connection, configuration, and streaming.
// 
// This could be more tidy with some separation into more classes but for
// a draft that adds some complexity.
//
// Displaying images and saving the raw data is handled by seperate
// class
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
#include <PvAcquisitionStateManager.h>

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
    std::string binning;
    std::string width;
    std::string height;
};

// Receiver
class Receiver : public PvAcquisitionStateEventSink
{
    public:
        Receiver(PvDisplayWnd* _display_wnd);
        ~Receiver();
        
        // Pulic functions
        void quit();
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
        void startAcquisition();
        void stopAcquisition();
        bool isAcquiring();
        void startTriggeredMultiFrameMode(int n);
        void toggleBinning();
        void resetStream();
        bool isMultiFrame();
        void startViewFinderMode();
        void setSavingPath(const std::string& _path);
        DeviceParams getDeviceParams();

    protected:
        // Callback when acquisition state has changed. This function in inherited from PvAcquisitionStateEventSink.
        void OnAcquisitionStateChanged(PvDevice* _device, PvStream* _stream, uint32_t _source, PvAcquisitionState _state );

    private:
        // Reciever will own a device, connection, stream and pipeline
        PvString connection_id;
        PvDevice* device;
        PvStream* stream;
        PvPipeline* pipeline;
        BufferList buffers;
        PvDisplayWnd* display_wnd;
        DisplayThread* display_thread;
        PvGenParameterArray* params;    // Actual device params
        DeviceParams device_params;     // Struct with some params for populating gui
        PvAcquisitionStateManager* acquisition_manager;

        bool acquiring;
        bool multiframe_mode;
    };


#endif // __RECIEVER_H__