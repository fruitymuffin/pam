// *****************************************************************************
//
// displaythread.h
// Inherits from PvDisplayThread and implements virtual methods to perform
// display tasks. The PvDisplayThread doesn't know anything about the PvDisplay
// adaptor. This is implemented purely in the overriden virtual functions.
//
// *****************************************************************************


#ifndef __DISPLAYTHREAD_H__
#define __DISPLAYTHREAD_H__

#include <PvDisplayThread.h>
#include <PvDisplayWnd.h>
#include <PvBufferWriter.h>

class DisplayThread : public PvDisplayThread
{
    public:
        DisplayThread(PvDisplayWnd* _display_wnd);
        void setSaving(const bool& save);
        void setSavingPath(const std::string& _path);
        

    protected:
        // Implement PvDisplayThread callbacks
        void OnBufferRetrieved (PvBuffer *aBuffer);
        void OnBufferDisplay (PvBuffer *aBuffer);
        void OnBufferDone (PvBuffer *aBuffer);
        void OnBufferLog (const PvString &aLog);

    private:
        std::string getFileName();

    private:
        PvDisplayWnd* display_wnd;
        PvBufferWriter* buffer_writer;
        std::string path;
        bool is_saving;
        unsigned int sequence;
};


#endif // __DISPLAYTHREAD_H__