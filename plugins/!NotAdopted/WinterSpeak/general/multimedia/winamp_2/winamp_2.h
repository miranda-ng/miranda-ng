#ifndef guard_general_multimedia_winamp_2_winamp_2_h
#define guard_general_multimedia_winamp_2_winamp_2_h
//==============================================================================
// General Code, © 2002 Ryan Winter
//==============================================================================

#include <windows.h>

class Winamp2
{
  public:
    Winamp2();
    ~Winamp2();

    enum PlayState
    {
        PlayState_Unknown,
        PlayState_Stopped,
        PlayState_Playing,
        PlayState_Paused,
    };

	//--------------------------------------------------------------------------
	// Description : play/pause/stop the current song
	//--------------------------------------------------------------------------
    void play();
    void stop();
    void pause();

	//--------------------------------------------------------------------------
	// Description : restore winamp to its previoud play state
    // Return      : the play state winamp has been restored to
	//--------------------------------------------------------------------------
    PlayState restoreState();

  private:
	//--------------------------------------------------------------------------
	// Description : get the handle to the winamp application
    // Parameters  : handle - where to place the handle
    // Returns     : true - the handle was found
    //               false - the handle wasn't found
	//--------------------------------------------------------------------------
    bool getHandle(HWND &handle);

	//--------------------------------------------------------------------------
	// Description : get the current state of the winamp application
	//--------------------------------------------------------------------------
    PlayState getState();

    PlayState m_state;    
};

//==============================================================================
//
//  Summary     : Control Winamp 2.x
//
//  Description : Allows remote control over numberous winamp 2.x setting
//                including volume, play, pause, stop etc.
//
//==============================================================================

#endif
