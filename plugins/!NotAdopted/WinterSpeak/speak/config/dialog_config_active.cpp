//==============================================================================
// 	Miranda Speak Plugin, © 2002 Ryan Winter
//==============================================================================

#pragma warning(disable:4786)

#include "dialog_config_active.h"

#include "speak.h"
#include "resource.h"

#include <general/debug/debug.h>

#include <windows.h>
#include <commctrl.h>

//------------------------------------------------------------------------------
DialogConfigActive *DialogConfigActive::m_instance = 0;

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
DialogConfigActive::DialogConfigActive(ConfigDatabase &db)
	:
	m_db(db)
{
	CLASSCERR("DialogConfigActive::DialogConfigActive");

    m_instance = this;
}

//------------------------------------------------------------------------------
DialogConfigActive::~DialogConfigActive()
{
	CLASSCERR("DialogConfigActive::~DialogConfigActive");

    m_instance = 0;
}

//------------------------------------------------------------------------------
int CALLBACK 
DialogConfigActive::process(HWND window, UINT message, WPARAM wparam, 
    LPARAM lparam)
{
    if (!m_instance)
    {
        return 1;
    }

	switch (message)
	{
      case WM_INITDIALOG:
        m_instance->load(window);
        break;

	  case WM_NOTIFY:
        switch (reinterpret_cast<LPNMHDR>(lparam)->code)
        {
          case PSN_APPLY:
		    m_instance->save(window);
            break;

          case LVN_ITEMCHANGED:
            m_instance->changed(window);
            break;
        }
        break;

	  case WM_COMMAND:
    	switch (LOWORD(wparam))
    	{
          case IDC_ACTIVE_OFFLINE:
          case IDC_ACTIVE_ONLINE:
          case IDC_ACTIVE_AWAY:
          case IDC_ACTIVE_DND:
          case IDC_ACTIVE_NA:
          case IDC_ACTIVE_OCCUPIED:
          case IDC_ACTIVE_FREEFORCHAT:
          case IDC_ACTIVE_INVISIBLE:
    		m_instance->changed(window);
            break;

          case IDC_ACTIVE_ALL:
            m_instance->selectAllUsers(window, true);
            break;

          case IDC_ACTIVE_NONE:
            m_instance->selectAllUsers(window, false);
            break;

          case IDC_ACTIVE_USERS:
            m_instance->changed(window);
            break;
	    }
        break;
	}

    return 0;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
void
DialogConfigActive::load(HWND window)
{
	CLASSCERR("DialogConfigActive::load");

	TranslateDialogDefault(window);

	// initialise the checkboxes
    CheckDlgButton(window, IDC_ACTIVE_ONLINE, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Online));
    CheckDlgButton(window, IDC_ACTIVE_AWAY, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Away));
    CheckDlgButton(window, IDC_ACTIVE_DND, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Dnd));
    CheckDlgButton(window, IDC_ACTIVE_NA, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Na));
    CheckDlgButton(window, IDC_ACTIVE_OCCUPIED, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Occupied));
    CheckDlgButton(window, IDC_ACTIVE_FREEFORCHAT, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_FreeForChat));
    CheckDlgButton(window, IDC_ACTIVE_INVISIBLE, 
        m_db.getActiveFlag(ConfigDatabase::ActiveFlag_Invisible));

    // add all the users active states
    ConfigDatabase::ActiveUsersMap active_users = m_db.getActiveUsers();

    HWND listview = GetDlgItem(window, IDC_ACTIVE_USERS);

    // add checkboxes
    ListView_SetExtendedListViewStyle(listview, LVS_EX_CHECKBOXES);

    LV_ITEM lv_item;
	lv_item.mask = LVIF_TEXT | LVIF_PARAM;
    lv_item.iSubItem = 0;

    int index = 0;
    ConfigDatabase::ActiveUsersMap::const_iterator iter;
    for (iter = active_users.begin(); iter != active_users.end(); ++iter)
    {
		lv_item.lParam = (LPARAM)iter->first;
        std::string str = "";

        // if its index 0, the use its the unknown user
		if (0 == lv_item.lParam)
        {
            lv_item.pszText = "Unknown";
        }
        else 
        {
            char *protocol = reinterpret_cast<char *>(
                CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)iter->first,0));

            if (protocol)
            {
                str += protocol;
                str += ": ";
            }

            str += reinterpret_cast<char *>(
    		    CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)iter->first, 0));

            lv_item.pszText = const_cast<char *>(str.c_str());
        }
        lv_item.iItem = index++;

        int it = ListView_InsertItem(listview, &lv_item);
        ListView_SetCheckState(listview, it, iter->second);
    }
}

//------------------------------------------------------------------------------
void
DialogConfigActive::save(HWND window)
{
	CLASSCERR("DialogConfigActive::save");

	// store the checkboxes
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Online, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_ONLINE) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Away, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_AWAY) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Dnd, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_DND) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Na, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_NA) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Occupied, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_OCCUPIED) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_FreeForChat, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_FREEFORCHAT) != 0));
	m_db.setActiveFlag(ConfigDatabase::ActiveFlag_Invisible, 
		(IsDlgButtonChecked(window, IDC_ACTIVE_INVISIBLE) != 0));

    // look through the listview an extract the active users
    HWND listview = GetDlgItem(window, IDC_ACTIVE_USERS);

    LV_ITEM lv_item;
	lv_item.mask = LVIF_PARAM;
    lv_item.iSubItem = 0;

    for (int i = 0; i < ListView_GetItemCount(listview); ++i)
    {
        lv_item.iItem = i;

        if (TRUE == ListView_GetItem(listview, &lv_item))
        {
            m_db.setActiveUser((HANDLE)lv_item.lParam,
                ListView_GetCheckState(listview, i));
        }
    }

    m_db.save();
}

//------------------------------------------------------------------------------
void
DialogConfigActive::selectAllUsers(HWND window, bool state)
{
	CLASSCERR("DialogConfigActive::selectAllUsers");
    // look through the listview an extract the active users

    HWND listview = GetDlgItem(window, IDC_ACTIVE_USERS);

    for (int i = 0; i < ListView_GetItemCount(listview); ++i)
    {
        ListView_SetCheckState(listview, i, state);
    }

    changed(window);
}

//==============================================================================
