#ifndef guard_speak_dialog_miranda_dialog_h
#define guard_speak_dialog_miranda_dialog_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include <windows.h>

#include <map>

class MirandaDialog
{
  public:
//	MirandaDialog(HWND window);
	MirandaDialog();
	virtual ~MirandaDialog() = 0;

  protected:
	//--------------------------------------------------------------------------
	// Description : set the window the dialog events are coming from
	//--------------------------------------------------------------------------
/*    void setWindow(HWND window)
    {
        m_window = window;
    }

    HWND getWindow()
    {
        return m_window;
    }*/

	//--------------------------------------------------------------------------
	// Description : return a pointer to the class object
	//--------------------------------------------------------------------------
//    MirandaDialog * instance()
//    {
//        return m_instance;
//    }
//    MirandaDialog * instance(HWND window);

	//--------------------------------------------------------------------------
	// Description : flag that a dialog control has changed
	//--------------------------------------------------------------------------
	void changed(HWND window);

//    static std::map<HWND, MirandaDialog *> m_window_map;

//    HWND m_window;
};

//==============================================================================
//
//  Summary     : Abstract base class for the dialog forms
//
//  Description : Implement common code
//
//==============================================================================

#endif