#include "Common.h"
#include "AnnounceDialog.h"


AnnounceDialog *AnnounceDialog::m_instance = 0;

//------------------------------------------------------------------------------
// public:
//------------------------------------------------------------------------------
AnnounceDialog::AnnounceDialog(AnnounceDatabase &db) : m_db(db)
{
	m_instance = this;
}

//------------------------------------------------------------------------------
AnnounceDialog::~AnnounceDialog()
{
	m_instance = 0;
}

//------------------------------------------------------------------------------
// private:
//------------------------------------------------------------------------------
INT_PTR CALLBACK AnnounceDialog::process(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
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

	  case WM_COMMAND:
		m_instance->command(window, wparam);
		break;

	  case WM_NOTIFY:
		if (PSN_APPLY == LPNMHDR(lparam)->code)
		{	
			m_instance->save(window);
		}
		break;
	}

	return 0;
}

//------------------------------------------------------------------------------
void AnnounceDialog::command(HWND window, int control)
{
	switch (LOWORD(control))
	{
	  case IDC_STATUS_OFFLINE:
	  case IDC_STATUS_ONLINE:
	  case IDC_STATUS_AWAY:
	  case IDC_STATUS_DND:
	  case IDC_STATUS_NA:
	  case IDC_STATUS_OCCUPIED:
	  case IDC_STATUS_FREEFORCHAT:
	  case IDC_STATUS_INVISIBLE:
	  case IDC_SPEAK_STATUS_MSG:
	  case IDC_SUPPRESS_CONNECT:
	  case IDC_EVENT_MESSAGE:
	  case IDC_EVENT_URL:
	  case IDC_EVENT_FILE:
	  case IDC_EVENT_AUTHREQUEST:
	  case IDC_EVENT_ADDED:
	  case IDC_READ_MSG_LENGTH:
	  case IDC_DIALOG_OPEN:
	  case IDC_DIALOG_FOCUSED:

		changed(window);
		break;

	  case IDC_MAX_MSG:
		if (EN_CHANGE == HIWORD(control))
		{
			changed(window);
		}
		break;
	}
}

//------------------------------------------------------------------------------
void
AnnounceDialog::load(HWND window)
{
	TranslateDialogDefault(window);

	// initialise the checkboxes
	CheckDlgButton(window, IDC_STATUS_OFFLINE, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Offline) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_ONLINE, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Online) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_AWAY, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Away) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_DND, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Dnd) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_NA, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Na) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_OCCUPIED, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Occupied) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_FREEFORCHAT, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_FreeForChat) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_STATUS_INVISIBLE, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_Invisible) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_SPEAK_STATUS_MSG, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_SpeakStatusMsg) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_SUPPRESS_CONNECT, m_db.getStatusFlag(AnnounceDatabase::StatusFlag_SuppressConnect) ? BST_CHECKED : BST_UNCHECKED);

	CheckDlgButton(window, IDC_EVENT_MESSAGE, m_db.getEventFlag(AnnounceDatabase::EventFlag_Message) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_EVENT_URL, m_db.getEventFlag(AnnounceDatabase::EventFlag_Url) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_EVENT_FILE, m_db.getEventFlag(AnnounceDatabase::EventFlag_File) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_EVENT_AUTHREQUEST, m_db.getEventFlag(AnnounceDatabase::EventFlag_AuthRequest) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_EVENT_ADDED, m_db.getEventFlag(AnnounceDatabase::EventFlag_Added) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_READ_MSG_LENGTH, m_db.getEventFlag(AnnounceDatabase::EventFlag_ReadMsgLength) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_DIALOG_OPEN, m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogOpen) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(window, IDC_DIALOG_FOCUSED, m_db.getEventFlag(AnnounceDatabase::EventFlag_DialogFocused) ? BST_CHECKED : BST_UNCHECKED);

	// initialise the welcome message box
	SetDlgItemInt(window, IDC_MAX_MSG, m_db.getMaxMsgSize(), 0);
}

//------------------------------------------------------------------------------
void
AnnounceDialog::save(HWND window)
{
	// store the checkboxes
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Offline, 		(IsDlgButtonChecked(window, IDC_STATUS_OFFLINE) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Online, 		(IsDlgButtonChecked(window, IDC_STATUS_ONLINE) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Away, 		(IsDlgButtonChecked(window, IDC_STATUS_AWAY) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Dnd,		(IsDlgButtonChecked(window, IDC_STATUS_DND) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Na,		(IsDlgButtonChecked(window, IDC_STATUS_NA) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Occupied,		(IsDlgButtonChecked(window, IDC_STATUS_OCCUPIED) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_FreeForChat,		(IsDlgButtonChecked(window, IDC_STATUS_FREEFORCHAT) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_Invisible,		(IsDlgButtonChecked(window, IDC_STATUS_INVISIBLE) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_SpeakStatusMsg,		(IsDlgButtonChecked(window, IDC_SPEAK_STATUS_MSG) != 0));
	m_db.setStatusFlag(AnnounceDatabase::StatusFlag_SuppressConnect,		(IsDlgButtonChecked(window, IDC_SUPPRESS_CONNECT) != 0));

	m_db.setEventFlag(AnnounceDatabase::EventFlag_Message,		(IsDlgButtonChecked(window, IDC_EVENT_MESSAGE) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_Url,		(IsDlgButtonChecked(window, IDC_EVENT_URL) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_File,		(IsDlgButtonChecked(window, IDC_EVENT_FILE) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_AuthRequest,		(IsDlgButtonChecked(window, IDC_EVENT_AUTHREQUEST) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_Added,		(IsDlgButtonChecked(window, IDC_EVENT_ADDED) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_ReadMsgLength,		(IsDlgButtonChecked(window, IDC_READ_MSG_LENGTH) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_DialogOpen,		(IsDlgButtonChecked(window, IDC_DIALOG_OPEN) != 0));
	m_db.setEventFlag(AnnounceDatabase::EventFlag_DialogFocused,		(IsDlgButtonChecked(window, IDC_DIALOG_FOCUSED) != 0));

	m_db.setMaxMsgSize(GetDlgItemInt(window, IDC_MAX_MSG, NULL, 0));

	m_instance->m_db.save();
}

//==============================================================================