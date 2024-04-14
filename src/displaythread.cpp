#include "displaythread.h"

DisplayThread::DisplayThread(PvDisplayWnd* _display_wnd) :
    display_wnd(_display_wnd)
{
    //
}

void DisplayThread::OnBufferRetrieved (PvBuffer *_buffer)
{
    // Nothing right now
}

void DisplayThread::OnBufferDisplay (PvBuffer *_buffer)
{
    // Display
    display_wnd->Display( *_buffer, false);
}

void DisplayThread::OnBufferDone (PvBuffer *aBuffer)
{
    // If saving, do it here
}

void DisplayThread::OnBufferLog (const PvString &aLog)
{
    // Nothing right now
}