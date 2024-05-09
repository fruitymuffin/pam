#include "displaythread.h"
#include <chrono>
#include <sstream>
#include <iostream>

void DisplayThread::setSaving(const bool& _save)
{
    is_saving = _save;
    sequence = 1;
}

std::string DisplayThread::getFileName()
{
    std::stringstream ss;

    auto duration = std::chrono::system_clock::now().time_since_epoch();
    auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
    ss << path << micros << " - " << sequence << ".png";
    std::cout << ss.str() << std::endl;
    
    return ss.str();
}

void DisplayThread::setSavingPath(const std::string& _path)
{
    path = _path;
}

DisplayThread::DisplayThread(PvDisplayWnd* _display_wnd) :
    display_wnd(_display_wnd), is_saving(false)
{
    buffer_writer = new PvBufferWriter();
}

void DisplayThread::OnBufferRetrieved (PvBuffer *_buffer)
{
    // If saving, do it here
    if (is_saving)
    {
        buffer_writer->Store(_buffer, PvString(getFileName().c_str()), PvBufferFormatType::PvBufferFormatPNG, NULL);
        sequence++;
    }
}

void DisplayThread::OnBufferDisplay (PvBuffer *_buffer)
{
    // Display
    display_wnd->Display( *_buffer, false);
}

void DisplayThread::OnBufferDone (PvBuffer *_buffer)
{

}

void DisplayThread::OnBufferLog (const PvString &_log)
{
    // Nothing right now
}