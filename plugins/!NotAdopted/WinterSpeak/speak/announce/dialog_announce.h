#ifndef guard_speak_announce_announce_dialog_h
#define guard_speak_announce_announce_dialog_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "dialog/miranda_dialog.h"
#include "announce/announce_database.h"

#include <wtypes.h>
#include <string>

class AnnounceDialog : public MirandaDialog
{
  public:
	AnnounceDialog(AnnounceDatabase &db);
	~AnnounceDialog();

	//--------------------------------------------------------------------------
	// Description : process a dialog message
	// Return      : true - update the systems configuration
	//               false - do nothing
	//--------------------------------------------------------------------------
    static int CALLBACK process(HWND window, UINT message, WPARAM wparam, 
        LPARAM lparam);

  private:
    void command(HWND window, int control);

	//--------------------------------------------------------------------------
	// Description : load/save settings to the miranda database
	//--------------------------------------------------------------------------
	void load(HWND window);
	void save(HWND window);

    static AnnounceDialog *m_instance;
	AnnounceDatabase      &m_db;
};

//==============================================================================
//
//  Summary     : Configuration Dialog box
//
//  Description : Set up the configuration dialog box and process its input 
//
//==============================================================================

#endif