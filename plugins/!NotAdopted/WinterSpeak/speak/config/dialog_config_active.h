#ifndef guard_speak_config_dialog_config_active_h
#define guard_speak_config_dialog_config_active_h
//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#include "dialog/miranda_dialog.h"
#include "config/config_database.h"

#include <wtypes.h>

class DialogConfigActive : public MirandaDialog
{
  public:
    //--------------------------------------------------------------------------
	// Description : Initialise
	// Parameters  : db - reference to the database to initalise and save 
    //                    control to and from
	//--------------------------------------------------------------------------
	DialogConfigActive(ConfigDatabase &db);
	virtual ~DialogConfigActive();

    //--------------------------------------------------------------------------
	// Description : process a dialog message
	// Return      : 0 - process ok
    //               1 - error
	//--------------------------------------------------------------------------
    static int CALLBACK process(HWND window, UINT message, WPARAM wparam, 
        LPARAM lparam);

  private:
	//--------------------------------------------------------------------------
	// Description : load/save setting to the miranda database
	//--------------------------------------------------------------------------
	void load(HWND window);
	void save(HWND window);

	//--------------------------------------------------------------------------
	// Description : select/unselect all the active status checkboxes
    // Parameters  : state - the state to apply to the checkboxes
	//--------------------------------------------------------------------------
    void selectAllUsers(HWND window, bool state);
    
    static DialogConfigActive *m_instance;
    ConfigDatabase            &m_db;
};

//==============================================================================
//
//  Summary     : Configuration Engine Dialog box
//
//  Description : Set up the configuration dialog box and process its input 
//
//==============================================================================

#endif