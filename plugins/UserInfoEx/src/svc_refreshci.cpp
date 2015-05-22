/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "commonheaders.h"

#define HM_PROTOACK	(WM_USER+100)

typedef INT_PTR	(*PUpdCallback) (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, PVOID UserData);

/***********************************************************************************************************
 * class CUpdProgress
 ***********************************************************************************************************/

class CUpdProgress
{
protected:
	BYTE			_bBBCode;		// TRUE if text renderer can handle BBCodes
	BYTE			_bIsCanceled;	// is set to TRUE upon click on the CANCEL button
	PUpdCallback	_pFnCallBack;	// a pointer to a callback function, which can be used 
									// to catch several messages by the caller.
	PVOID			_pData;			// application defined data
	HWND			_hWnd;			// window handle of the progress dialog/popup

	/**
	 * This is the default window procedure, which is called for both progress dialog and popup.
	 * It handles some common messages and calls the user defined callback function if defined.
	 *
	 * @param	pProgress		- This is the pointer to the object of CUpdProgress.
	 * @param	hWnd			- HWND window handle of the progress dialog
	 * @param	uMsg			- message sent to the dialog
	 * @param	wParam			- message specific parameter
	 * @param	lParam			- message specific parameter
	 *
	 * @return	This method returns 0.
	 **/
	static INT_PTR CALLBACK DefWndProc(CUpdProgress *pProgress, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
	{
		__try
		{
			if (PtrIsValid(pProgress))
			{
				switch (uMsg)
				{
				case UM_POPUPACTION:
				case WM_COMMAND:
					{
						if (wParam == MAKEWORD(IDSKIP, BN_CLICKED))
						{
							pProgress->Destroy();
						}
						else						
						if (wParam == MAKEWORD(IDCANCEL, BN_CLICKED))
						{
							pProgress->_bIsCanceled = TRUE;
						}
					}
				}
				if (PtrIsValid(pProgress->_pFnCallBack))
				{
					pProgress->_pFnCallBack(hWnd, uMsg, wParam, lParam, pProgress->_pData);
				}
			}
		}
		__except(GetExceptionCode()==EXCEPTION_ACCESS_VIOLATION ? 
						EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) 
		{ // code to handle exception
			puts("Exception Occurred");
		}
		return 0;
	}

public:

	virtual HWND	Create		(LPCTSTR szTitle, PUpdCallback pFnCallBack) = 0;
	virtual void	Destroy		(void) {};
	virtual void	SetTitle	(LPCTSTR szText) = 0;
	virtual void	SetText		(LPCTSTR szText) = 0;

	BYTE IsVisible() const
	{
		return _hWnd != NULL;
	}
	/**
	 *
	 *
	 **/
	BYTE IsCanceled() const
	{
		return _bIsCanceled;
	}

	/**
	 *
	 *
	 **/
	void SetTitleParam(LPCTSTR szText, ...)
	{
		if (szText)
		{
			TCHAR buf[MAXDATASIZE];
			va_list vl;
			
			va_start(vl, szText);
			if (mir_vsntprintf(buf, SIZEOF(buf), szText, vl) != -1)
			{
				SetTitle(buf);	 
			}
			va_end(vl);
		}
	}

	/**
	 * This method is used to set the popups or dialogs message text.
	 * It takes text with parameters as sprintf does. If bbcodes are
	 * disabled this method automatically deletes them from the text. 
	 *
	 * @param	szText			- the text to display. Can contain formats like
	 *							  sprintf does.
	 * @param	...				- a list of parameters, which depends on the
	 *							  format of szText.
	 *
	 * @return	nothing
	 **/
	void SetTextParam(LPCTSTR szText, ...)
	{
		if (szText)
		{
			INT_PTR cch = mir_tstrlen(szText);
			LPTSTR	fmt = (LPTSTR) mir_alloc((cch + 1) * sizeof(TCHAR));
			
			if (fmt)
			{
				TCHAR buf[MAXDATASIZE];
				va_list vl;

				mir_tstrcpy(fmt, szText);

				// delete bbcodes
				if (!_bBBCode)
				{
					LPTSTR s, e;

					for (s = fmt, e = fmt + cch; s[0] != 0; s++)
					{
						if (s[0] == '[')
						{
							// leading bbcode tag (e.g.: [b], [u], [i])
							if ((s[1] == 'b' || s[1] == 'u' || s[1] == 'i') && s[2] == ']')
							{
								memmove(s, s + 3, (e - s - 2) * sizeof(TCHAR));
								e -= 3;
							}
							// ending bbcode tag (e.g.: [/b], [/u], [/i])
							else if (s[1] == '/' && (s[2] == 'b' || s[2] == 'u' || s[2] == 'i') && s[3] == ']')
							{
								memmove(s, s + 4, (e - s - 3) * sizeof(TCHAR));
								e -= 4;
							}
						}
					}
				}
			
				va_start(vl, szText);
				if (mir_vsntprintf(buf, SIZEOF(buf), fmt, vl) != -1)
				{
					SetText(buf);	 
				}
				va_end(vl);
				mir_free(fmt);
			}
		}
	}

	/**
	 *
	 *
	 **/
	CUpdProgress()
	{
		_hWnd = NULL;
		_pFnCallBack = NULL;
		_pData = NULL;
		_bIsCanceled = FALSE;
		_bBBCode = FALSE;
	}

	/**
	 *
	 *
	 **/
	CUpdProgress(PVOID data)
	{
		_hWnd = NULL;
		_pFnCallBack = NULL;
		_pData = data;
		_bIsCanceled = FALSE;
		_bBBCode = FALSE;
	}

	~CUpdProgress()
	{
	}

};

/***********************************************************************************************************
 * class CDlgUpdProgress
 ***********************************************************************************************************/

class CDlgUpdProgress : public CUpdProgress
{
	/**
	 *
	 *
	 **/
	static INT_PTR CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:
			{
				const ICONCTRL idIcon[] = {
					{ ICO_BTN_UPDATE,		WM_SETICON,		NULL		},
					{ ICO_BTN_DOWNARROW,	BM_SETIMAGE,	IDSKIP		},
					{ ICO_BTN_CANCEL,		BM_SETIMAGE,	IDCANCEL	}
				};
				IcoLib_SetCtrlIcons(hWnd, idIcon, db_get_b(NULL, MODNAME, SET_ICONS_BUTTONS, 1) ? 2 : 1);
	
				SendDlgItemMessage(hWnd, IDCANCEL,	BUTTONTRANSLATE, NULL, NULL);
				SendDlgItemMessage(hWnd, IDSKIP,	BUTTONTRANSLATE, NULL, NULL);
				SetUserData(hWnd, lParam);
				
				TranslateDialogDefault(hWnd);
			}
			return TRUE;

		case WM_CTLCOLORSTATIC:
			{
				switch (GetWindowLongPtr((HWND)lParam, GWLP_ID)) 
				{
					case STATIC_WHITERECT:
					case IDC_INFO:
						{
							SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
							return GetSysColor(COLOR_WINDOW);
						}
				}
			}
			return FALSE;

		}
		return CUpdProgress::DefWndProc((CUpdProgress *) GetUserData(hWnd), hWnd, uMsg, wParam, lParam);
	}

public:

	/**
	 *
	 *
	 **/
	CDlgUpdProgress(PVOID data)
		: CUpdProgress(data)
	{
	}

	/**
	 *
	 *
	 **/
	virtual HWND Create(LPCTSTR szTitle, PUpdCallback pFnCallBack)
	{
		_pFnCallBack = pFnCallBack;
		_hWnd = CreateDialogParam(ghInst, 
							MAKEINTRESOURCE(IDD_REFRESHDETAILS), 
							0, 
							CDlgUpdProgress::WndProc, 
							(LPARAM) this);
		if (_hWnd)
		{
			SetTitle(szTitle);
			ShowWindow(_hWnd, SW_SHOW);
		}
		return _hWnd;
	}

	/**
	 *
	 *
	 **/
	virtual void Destroy()
	{
		if (_hWnd)
		{
			SetUserData(_hWnd, NULL);
			EndDialog(_hWnd, IDOK);
			_hWnd = NULL;
		}
	}

	/**
	 *
	 *
	 **/
	virtual void SetTitle(LPCTSTR szText)
	{
		SetWindowText(_hWnd, szText);
	}

	/**
	 *
	 *
	 **/
	virtual void SetText(LPCTSTR szText)
	{
		SetDlgItemText(_hWnd, IDC_INFO, szText);
	}

};

/***********************************************************************************************************
 * class CPopupUpdProgress
 ***********************************************************************************************************/

class CPopupUpdProgress : public CUpdProgress
{
	LPCTSTR			_szText;
	POPUPACTION		_popupButtons[2];

	/**
	 *
	 *
	 **/
	void UpdateText()
	{
		if (_szText)
		{
			INT_PTR cb = mir_tstrlen(_szText) + 8;
			LPTSTR	pb = (LPTSTR) mir_alloc(cb * sizeof(TCHAR));

			if (pb)
			{
				mir_tstrcpy(pb, _szText);

				SendMessage(_hWnd, UM_CHANGEPOPUP, CPT_TITLET, (LPARAM) pb);
			}
		}
	}

	/**
	 * This static member is the window procedure, which is used to modify the behaviour of
	 * a popup dialog, so that it can act as a replacement for the progress dialog.
	 * The major task of this method is to filter out some messages, who would lead to a crash,
	 * if passed to the default windows procedure.
	 *
	 **/
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
	{
		// Filter out messages, which must not be passed to default windows procedure or even
		// to the callback function!
		switch (uMsg)
		{
		case UM_INITPOPUP:
		case UM_CHANGEPOPUP:
		case UM_FREEPLUGINDATA:
			break;
		default:
			CUpdProgress::DefWndProc((CUpdProgress *) PUGetPluginData(hWnd), hWnd, uMsg, wParam, lParam);
		}
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

public:

	/**
	 *
	 *
	 **/
	CPopupUpdProgress(PVOID data)
		: CUpdProgress(data)
	{
		_szText = NULL;
		_bBBCode = db_get_b(NULL, "Popup", "UseMText", FALSE);

		_popupButtons[0].cbSize = sizeof(POPUPACTION);
		_popupButtons[0].flags = PAF_ENABLED;
		_popupButtons[0].lchIcon = Skin_GetIcon(ICO_BTN_DOWNARROW);
		_popupButtons[0].wParam = MAKEWORD(IDSKIP, BN_CLICKED);
		_popupButtons[0].lParam = NULL;
		mir_strcpy(_popupButtons[0].lpzTitle, MODNAME"/Hide");

		// cancel button
		_popupButtons[1].cbSize = sizeof(POPUPACTION);
		_popupButtons[1].flags = PAF_ENABLED;
		_popupButtons[1].lchIcon = Skin_GetIcon(ICO_BTN_CANCEL);
		_popupButtons[1].wParam = MAKEWORD(IDCANCEL, BN_CLICKED);
		_popupButtons[1].lParam = NULL;
		mir_strcpy(_popupButtons[1].lpzTitle, MODNAME"/Cancel");
	}

	/**
	 *
	 *
	 **/
	virtual HWND Create(LPCTSTR szTitle, PUpdCallback pFnCallBack)
	{
		POPUPDATAT_V2 pd = { 0 };
		pd.cbSize = sizeof(pd);
		pd.lchIcon = Skin_GetIcon(ICO_BTN_UPDATE);
		pd.iSeconds = -1;
		pd.PluginData = this;
		pd.PluginWindowProc = CPopupUpdProgress::WndProc;
		pd.actionCount = SIZEOF(_popupButtons);
		pd.lpActions = _popupButtons;

		// dummy text
		_szText = mir_tstrdup(szTitle);
		mir_tstrcpy(pd.lptzContactName, _szText);
		
		mir_tstrcpy(pd.lptzText, _T(" "));
		
		_pFnCallBack = pFnCallBack;
		_hWnd = (HWND) CallService(MS_POPUP_ADDPOPUPT, (WPARAM) &pd, APF_RETURN_HWND|APF_NEWDATA);
		return _hWnd;
	}

	/**
	 *
	 *
	 **/
	virtual void Destroy()
	{
		if (_hWnd)
		{
			PUDeletePopup(_hWnd);
			_hWnd = NULL;
		}
		MIR_FREE(_szText);
	}

	/**
	 *
	 *
	 **/
	virtual void SetTitle(LPCTSTR szText)
	{
		MIR_FREE(_szText);
		_szText = mir_tstrdup(szText);
		UpdateText();
	}

	/**
	 *
	 *
	 **/
	virtual void SetText(LPCTSTR szText)
	{
		SendMessage(_hWnd, UM_CHANGEPOPUP, CPT_TEXTT, (LPARAM) mir_tstrdup(szText));
	}
};


/***********************************************************************************************************
 * class CContactUpdater
 ***********************************************************************************************************/

class CContactUpdater : public CContactQueue
{
	CUpdProgress*		_pProgress;			// pointer to the progress dialog/popup
	HANDLE				_hProtoAckEvent;	// handle to protocol ack notification
	MCONTACT				_hContact;			// current contact being refreshed
	PBYTE				_hContactAcks;		// array of acknoledgements for the current contact to wait for
	INT_PTR				_nContactAcks;		// number of acknoledgements for the current contact to wait for

	MIRANDA_CPP_PLUGIN_API(CContactUpdater);

	/**
	 * This is a callback dialog procedure, which is assigned the update progress dialog to
	 * gain control over certain messages.
	 *
	 * @param	hWnd		- HWND window handle of the progress dialog
	 * @param	uMsg		- message sent to the dialog
	 * @param	wParam		- message specific parameter
	 * @param	lParam		- message specific parameter
	 * @param	u			- This is a parameter assigned by the CUpdProgress' constructur.
	 *						  In this case it is a pointer to this class's object.
	 *
	 * @return	This method returns 0.
	 **/
	static int DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, CContactUpdater* u) 
	{
		switch (uMsg) 
		{

		/**
		 * User has clicked on the skip or cancel button.
		 **/
		case UM_POPUPACTION:
		case WM_COMMAND: 
			{
				if (PtrIsValid(u))
				{
					switch (LOWORD(wParam))
					{
					case IDCANCEL:
						{
							if (HIWORD(wParam) == BN_CLICKED) 
							{
								u->Cancel();
							}
						}
					}
				}
			}
		}
		return 0;
	}

	/**
	 * This method handles ack broadcasts from the protocols to determine,
	 * whether a contact's information set's update is complete to continue
	 * with the next faster. By default the queue is configured to wait
	 * about 15s between contacts. If the protocol sends a complete ack,
	 * the time is shortend to 4s.
	 *
	 * @param	wParam		- not used
	 * @param	ack			- pointer to an ACKDATA structure containing all 
	 *						  data for the acknoledgement.
	 *
	 * @return	nothing
	 **/
	int __cdecl OnProtoAck(WPARAM wParam, ACKDATA *ack)
	{
		if (ack && ack->cbSize == sizeof(ACKDATA) && ack->hContact == _hContact && ack->type == ACKTYPE_GETINFO)
		{
			if (ack->hProcess || ack->lParam) 
			{
				if (!_hContactAcks)
				{
					_nContactAcks = (INT_PTR)ack->hProcess;
					_hContactAcks = (PBYTE)mir_calloc(sizeof(BYTE) * (INT_PTR)ack->hProcess);
				}
				if (ack->result == ACKRESULT_SUCCESS || ack->result == ACKRESULT_FAILED)
				{
					_hContactAcks[ack->lParam] = 1;
				}

				for (int i = 0; i < _nContactAcks; i++)
				{
					if (_hContactAcks[i] == 0)
					{
						return 0;
					}
				}
			}
			// don't wait the full time, but continue immitiatly
			ContinueWithNext();
		}
		return 0;
	}

	/**
	 * This method is called just before the worker thread is going to suspend,
	 * if the queue is empty.
	 *
	 * @param	none
	 *
	 * @return	nothing
	 **/
	virtual void OnEmpty() 
	{
		// This was the last contact, so destroy the progress window.
		if (_hProtoAckEvent)
		{
			UnhookEvent(_hProtoAckEvent);
			_hProtoAckEvent = NULL;
		}

		// free up last ackresult array
		MIR_FREE(_hContactAcks);
		_nContactAcks = 0;
		_hContact = NULL;

		// close progress bar
		if (_pProgress)
		{
			_pProgress->Destroy();

			delete _pProgress;
			_pProgress = NULL;
		}

		// reset menu
		if (hMenuItemRefresh)
		{
			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.flags = CMIM_NAME|CMIM_ICON;
			clmi.pszName = LPGEN("Refresh contact details");
			clmi.hIcon = Skin_GetIcon(ICO_BTN_UPDATE);
			Menu_ModifyItem(hMenuItemRefresh, &clmi);
		}
	}

	/**
	 * This virtual method is called by the derived CContactQueue class,
	 * if an action is requested for a queue item.
	 *
	 * @param	hContact	- the handle of the contact an action is requested for
	 * @param	param		- not used here
	 *
	 * @return	nothing
	 **/
	virtual void Callback(MCONTACT hContact, PVOID param)
	{
		LPSTR	pszProto	= DB::Contact::Proto(hContact);

		if (pszProto && pszProto[0])
		{
			MIR_FREE(_hContactAcks);
			_nContactAcks = 0;
			_hContact = hContact;

			if (!_hProtoAckEvent)
			{
				_hProtoAckEvent = (HANDLE) ThisHookEvent(ME_PROTO_ACK, (EVENTHOOK) &CContactUpdater::OnProtoAck);
			}

			if (_pProgress)
			{
				_pProgress->SetTextParam(TranslateT("[b]%s (%S)...[/b]\n%d Contacts remaining"),
					DB::Contact::DisplayName(_hContact), pszProto, Size());
			}
			if (IsProtoOnline(pszProto))
			{
				int i;
				for (i = 0; i < 3 && CallContactService(hContact, PSS_GETINFO, 0, 0); i++)
				{
					Sleep(3000);
				}
			}
		}
	}

public:

	/**
	 * This is the default constructor
	 *
	 **/
	CContactUpdater() : CContactQueue()
	{
		_hContactAcks	= NULL;
		_nContactAcks	= 0;
		_hContact		= NULL;
		_pProgress		= NULL;
		_hProtoAckEvent	= NULL;
	}

	/**
	 *
	 *
	 **/
	~CContactUpdater()
	{
		RemoveAll();
		OnEmpty();
	}

	/**
	 *
	 *
	 **/
	BOOL QueueAddRefreshContact(MCONTACT hContact, int iWait)
	{
		LPSTR pszProto = DB::Contact::Proto(hContact);

		if ((mir_strcmp(pszProto, "Weather") != 0) && (mir_strcmp(pszProto, META_PROTO) != 0) && IsProtoOnline(pszProto))
			return Add(iWait, hContact);

		return 0;
	}

	/**
	 *
	 *
	 **/
	void RefreshAll()
	{
		int iWait = 100;

		for (MCONTACT hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
			if (QueueAddRefreshContact(hContact, iWait))
				iWait += 5000;

		if (Size() && !_pProgress)
		{
			if (ServiceExists(MS_POPUP_CHANGETEXTT) && db_get_b(NULL, MODNAME, "PopupProgress", FALSE))
			{
				_pProgress = new CPopupUpdProgress(this);
			}
			else
			{
				_pProgress = new CDlgUpdProgress(this);
			}

			_pProgress->Create(TranslateT("Refresh contact details"), (PUpdCallback) CContactUpdater::DlgProc);
			_pProgress->SetText(TranslateT("Preparing..."));
		}

		// if there are contacts in the queue, change the main menu item to indicate it is meant for canceling.
		if (hMenuItemRefresh && Size() > 0)
		{
			CLISTMENUITEM clmi = { sizeof(clmi) };
			clmi.flags = CMIM_NAME|CMIM_ICON;
			clmi.pszName = LPGEN("Abort Refreshing Contact Details");
			clmi.hIcon = Skin_GetIcon(ICO_BTN_CANCEL);
			Menu_ModifyItem(hMenuItemRefresh, &clmi);
		}
	}

	/**
	 *
	 *
	 **/
	void Cancel()
	{
		RemoveAll();
		ContinueWithNext();
	}

};

static CContactUpdater	*ContactUpdater = NULL;

/***********************************************************************************************************
 * common helper functions
 ***********************************************************************************************************/

/**
 * This function checks, whether at least one protocol is online!
 *
 * @param	none
 *
 * @retval	TRUE		- At least one protocol is online.
 * @retval	FALSE		- All protocols are offline.
 **/
static BOOL IsMirandaOnline()
{
	PROTOACCOUNT **pAcc;
	int i, nAccCount;
	BOOL bIsOnline = FALSE;

	if (MIRSUCCEEDED(ProtoEnumAccounts(&nAccCount, &pAcc)))
	{
		for (i = 0; (i < nAccCount) && !bIsOnline; i++) 
		{
			bIsOnline |= (IsProtoAccountEnabled(pAcc[i]) && IsProtoOnline(pAcc[i]->szModuleName));
		}
	}
	return bIsOnline;
}

/***********************************************************************************************************
 * services
 ***********************************************************************************************************/

/**
 * This is the service function being called by MS_USERINFO_REFRESH.
 * It adds each contact, whose protocol is online, to the queue of contacts to refresh.
 * The queue is running a separate thread, which is responsible for requesting the contact information
 * one after another with a certain time to wait in between.
 *
 * @param	wParam		- not used
 * @param	lParam		- not used
 *
 * @return	This service function always returns 0.
 **/
static INT_PTR RefreshService(WPARAM wParam, LPARAM lParam)
{
	try
	{
		if (IsMirandaOnline())
		{
			if (!ContactUpdater)
			{
				ContactUpdater = new CContactUpdater();
			}

			if (ContactUpdater->Size() == 0)
			{
				ContactUpdater->RefreshAll();
			}
			else if (IDYES == MsgBox(NULL, MB_YESNO|MB_ICON_QUESTION, LPGENT("Refresh contact details"), NULL, 
				LPGENT("Do you want to cancel the current refresh procedure?")))
			{
				ContactUpdater->Cancel();
			}
		}
		else
		{
			MsgErr(NULL, LPGENT("Miranda must be online for refreshing contact information!"));
		}
	}
	catch(...)
	{
		MsgErr(NULL, LPGENT("The function caused an exception!"));
	}
	return 0;
}

/***********************************************************************************************************
 * events
 ***********************************************************************************************************/

/**
 *
 *
 **/
static int OnContactAdded(WPARAM hContact, LPARAM lParam)
{
	try
	{
		DWORD dwStmp = db_get_dw(hContact, USERINFO, SET_CONTACT_ADDEDTIME, 0);
		if (!dwStmp)
		{
			MTime mt;
			
			mt.GetLocalTime();
			mt.DBWriteStamp(hContact, USERINFO, SET_CONTACT_ADDEDTIME);

			// create updater, if not yet exists
			if (!ContactUpdater)
			{
				ContactUpdater = new CContactUpdater();
			}
			
			// add to the end of the queue
			ContactUpdater->AddIfDontHave(
				(ContactUpdater->Size() > 0) 
					? max(ContactUpdater->Get(ContactUpdater->Size() - 1)->check_time + 15000, 4000) 
					: 4000, hContact);
		}
	}
	catch(...)
	{
		MsgErr(NULL, LPGENT("The function caused an exception!"));
	}
	return 0;
}

/**
 * Miranda is going to shutdown soon, so any panding contact information refresh is to be terminated
 * and the queue object must be deleted. Further refresh requests will be ignored.
 *
 * @param	wParam		- not used
 * @param	lParam		- not used
 *
 * @return	This function always returns 0.
 **/
static int OnPreShutdown(WPARAM, LPARAM)
{
	if (ContactUpdater) {
		delete ContactUpdater;
		ContactUpdater = 0;
	}
	//MIR_DELETE(ContactUpdater);
	return 0;
}

/***********************************************************************************************************
 * initialization
 ***********************************************************************************************************/

/**
 * This function initially loads the module upon startup.
 **/
void SvcRefreshContactInfoLoadModule(void)
{
	CreateServiceFunction(MS_USERINFO_REFRESH, RefreshService);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, OnPreShutdown);
	HookEvent(ME_DB_CONTACT_ADDED, OnContactAdded);

	HOTKEYDESC hk = { 0 };
	hk.cbSize = sizeof(HOTKEYDESC);
	hk.pszSection = MODNAME;
	hk.pszName = "RefreshContactDetails";
	hk.pszDescription = LPGEN("Refresh contact details");
	hk.pszService = MS_USERINFO_REFRESH;
	Hotkey_Register(&hk);
}
