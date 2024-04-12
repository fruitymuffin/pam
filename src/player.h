// *****************************************************************************
//
// Player.h
// Player will interact with a pipeline and the PvDisplayHwnd/PvDisplayThread
// to output images to the screen
//
// *****************************************************************************


#ifndef __PLAYER_H__
#define __PLAYER_H__

#include <PvDisplayThread.h>

class Player
{
    public:
        Player();

    private:
        PvDisplayThread *display_thread;
        
};


#endif // __PLAYER_H__