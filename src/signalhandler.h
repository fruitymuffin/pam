#ifndef __SIGNALHANDLER_H__
#define __SIGNALHANDLER_H__
/*
* Interface class for handling signals.
* Qt application can inherit from this class and implement handleSingle().
*
* Credit to user948581 - https://stackoverflow.com/questions/7581343
*/
class SignalHandler
{
    public:
        SignalHandler(int mask = DEFAULT_SIGNALS);
        ~SignalHandler();

        enum SIGNALS
        {
            SIG_UNHANDLED = 0,
            SIG_NOOP      = 1,
            SIG_INT       = 2,
            SIG_TERM      = 4,
            SIG_CLOSE     = 8,
            SIG_RELOAD    = 16,
            DEFAULT_SIGNALS = SIG_INT | SIG_TERM | SIG_CLOSE
        };
        static const int num_signals = 6;

        // Implement this is child
        virtual bool handleSignal(int signal) = 0;

    private:
        int mask;
};

#endif // __SIGNALHANDLER_H__