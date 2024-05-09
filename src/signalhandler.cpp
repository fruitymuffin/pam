#include "signalhandler.h"
#include <signal.h>
#include <cassert>

// Single instance
SignalHandler* g_handler = NULL;

void POSIX_handleFunc(int);
int POSIX_physicalToLogical(int);
int POSIX_logicalToPhysical(int);

SignalHandler::SignalHandler(int _mask) : mask(_mask)
{
    // Ensure only one instance exists
    assert(g_handler == NULL);
    g_handler = this;

    // Connect signals
    for (int i = 0; i < num_signals; i++)
    {
        int logical = 0x01 << i;
        if (mask & logical)
        {
            int sig = POSIX_logicalToPhysical(logical);
            bool failed = signal(sig, POSIX_handleFunc) == SIG_ERR;
            assert(!failed);
            (void)failed;
        }
    }
}

SignalHandler::~SignalHandler()
{
    // Disconnect signals
    for (int i = 0; i < num_signals; i++)
    {
        int logical = 0x01 << i;
        if(mask & logical)
        {
            signal(POSIX_logicalToPhysical(logical), SIG_DFL);
        }
    }
}



int POSIX_logicalToPhysical(int signal)
{
    switch (signal)
    {
    case SignalHandler::SIG_INT: return SIGINT;
    case SignalHandler::SIG_TERM: return SIGTERM;
    // In case the client asks for a SIG_CLOSE handler, accept and
    // bind it to a SIGTERM. Anyway the signal will never be raised
    case SignalHandler::SIG_CLOSE: return SIGTERM;
    case SignalHandler::SIG_RELOAD: return SIGHUP;
    default: 
        return -1; // SIG_ERR = -1
    }
}

int POSIX_physicalToLogical(int signal)
{
    switch (signal)
    {
    case SIGINT: return SignalHandler::SIG_INT;
    case SIGTERM: return SignalHandler::SIG_TERM;
    case SIGHUP: return SignalHandler::SIG_RELOAD;
    default:
        return SignalHandler::SIG_UNHANDLED;
    }
}

void POSIX_handleFunc(int signal)
{
    if (g_handler)
    {
        int signo = POSIX_physicalToLogical(signal);
        g_handler->handleSignal(signo);
    }
}

