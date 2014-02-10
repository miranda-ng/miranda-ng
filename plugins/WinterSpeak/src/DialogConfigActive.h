#pragma once
#include "mirandadialog.h"

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
    static INT_PTR CALLBACK process(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

  private:
	//--------------------------------------------------------------------------
	// Description : load/save setting to the miranda database
	//--------------------------------------------------------------------------
	void load(HWND window);
	void save(HWND window);
	void notify(HWND window, LPARAM lparam);

	//--------------------------------------------------------------------------
	// Description : select/unselect all the active status checkboxes
    // Parameters  : state - the state to apply to the checkboxes
	//--------------------------------------------------------------------------
    void selectAllUsers(HWND window, bool state);
	void ResetListOptions(HWND listview);
	void InitialiseItem(HWND hwndList, HANDLE hItem, bool message, bool status);
	void SetAllContactIcons(HWND listview, HWND window);
	void SetIconsForColumn(HWND hwndList, HANDLE hItem, HANDLE hItemAll, int iColumn, int iImage);
	void SetAllChildIcons(HWND hwndList, HANDLE hFirstItem, int iColumn, int iImage);
	void SetListGroupIcons(HWND hwndList, HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount);
	void SaveItemMask(HWND hwndList, MCONTACT hContact, HANDLE hItem);
    
    static DialogConfigActive *m_instance;
    ConfigDatabase            &m_db;
	HICON hIcons[4];
	HANDLE hItemUnknown;
	HANDLE hItemAll;
};

