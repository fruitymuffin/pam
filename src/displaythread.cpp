#include "displaythread.h"
#include <chrono>
#include <sstream>

void DisplayThread::setSaving(const bool& _save)
{
    is_saving = _save;
    sequence = 0;
}

std::string DisplayThread::getFileName()
{
    std::stringstream ss;

    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    ss << millis << " - " << sequence << ".png";
    return ss.str();
}

DisplayThread::DisplayThread(PvDisplayWnd* _display_wnd) :
    display_wnd(_display_wnd), is_saving(false)
{
    buffer_writer = new PvBufferWriter();
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

void DisplayThread::OnBufferDone (PvBuffer *_buffer)
{
    // If saving, do it here
    if (is_saving)
    {
        buffer_writer->Store(_buffer, PvString(getFileName().c_str()), PvBufferFormatType::PvBufferFormatPNG, NULL);
        sequence++;
    }
}

void DisplayThread::OnBufferLog (const PvString &_log)
{
    // Nothing right now
}