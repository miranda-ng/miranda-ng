//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include "winamp_2.h"

#include <general/debug/debug.h>

#include <frontend.h>

namespace
{
    const int WINAMP_PLAY  = WINAMP_BUTTON2;
    const int WINAMP_PAUSE = WINAMP_BUTTON3;
    const int WINAMP_STOP  = WINAMP_BUTTON4;
}

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
Winamp2::Winamp2()
    :
    m_state(PlayState_Unknown)
{
    CLASSCERR("Winamp2::Winamp2");
}

//------------------------------------------------------------------------------
Winamp2::~Winamp2()
{
    CLASSCERR("Winamp2::~Winamp2");
}

//------------------------------------------------------------------------------
void 
Winamp2::play()
{
    CLASSCERR("Winamp2::play");

    HWND handle;
    
    if (getHandle(handle))
    {
        m_state = getState();
        SendMessage(handle, WM_COMMAND, WINAMP_PLAY, 0);
    }
}

//------------------------------------------------------------------------------
void 
Winamp2::stop()
{
    CLASSCERR("Winamp2::stop");

    HWND handle;
    
    if (getHandle(handle))
    {
        m_state = getState();
        SendMessage(handle, WM_COMMAND, WINAMP_STOP, 0);
    }
}

//------------------------------------------------------------------------------
void 
Winamp2::pause()
{
    CLASSCERR("Winamp2::pause");

    HWND handle;
    
    if (getHandle(handle))
    {
        m_state = getState();

        if (PlayState_Playing == m_state)
        {
            SendMessage(handle, WM_COMMAND, WINAMP_PAUSE, 0);
        }
    }
}

//------------------------------------------------------------------------------
Winamp2::PlayState 
Winamp2::restoreState()
{
    CLASSCERR("Winamp2::restoreState");

    switch (m_state)
    {
      case PlayState_Playing:
        play();
        break;

      case PlayState_Stopped:
        stop();
        break;

      case PlayState_Paused:
        pause();
        break;
    }

    return m_state;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
bool
Winamp2::getHandle(HWND &handle)
{
    handle = FindWindow("Winamp v1.x", NULL);

    bool ret = true;

    if (NULL == handle)
    {
        ret = false;
    }

    return ret;
}

//------------------------------------------------------------------------------
Winamp2::PlayState 
Winamp2::getState()
{
    CLASSCERR("Winamp2::getState");

    HWND handle;

    if (getHandle(handle))
    {
        int state = SendMessage(handle, WM_WA_IPC, 0, IPC_ISPLAYING);

        switch (state)
        {
          case 0:
            return PlayState_Stopped;
            break;
    
          case 1:
            return PlayState_Playing;
            break;

          case 3:
            return PlayState_Paused;
            break;
        }
    }

    return PlayState_Unknown;
}

//==============================================================================

