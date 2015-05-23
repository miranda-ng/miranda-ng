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

#ifndef _UI_CTRL_BASE_INCLUDE_
#define _UI_CTRL_BASE_INCLUDE_

/***********************************************************************************************************
 * old CTRL stuff (is to replace in the future
 ***********************************************************************************************************/

// control flags
#define CTRLF_CHANGED	 1
#define CTRLF_HASPROTO	2
#define CTRLF_HASCUSTOM 4
#define CTRLF_HASMETA	 8

#define CTRLF_FIRST		16 // first free flag for derived controls

// control types
#define CTRL_ERROR					0
#define CTRL_EDIT					 1
#define CTRL_COMBOBOX_STAT	2
#define CTRL_DATEPICKER		 3
#define CTRL_LIST_PROFILE	 4
#define CTRL_LIST_ITEM			7

typedef struct TCtrlInfo {
	BYTE	nType;
	WORD	wFlags;
} CTRL, *LPCTRL;

// for compatibility with old styled controls
void							Ctrl_InitTextColours();
INT_PTR CALLBACK	Ctrl_SetTextColour(HDC hdc, WORD wFlags);

/***********************************************************************************************************
 * CBaseCtrl declaration
 ***********************************************************************************************************/

union CCtrlFlags 
{
	WORD W;
	struct CBits 
	{
		BYTE	hasChanged	: 1;
		BYTE	hasProto		: 1;
		BYTE	hasCustom	 : 1;
		BYTE	hasMeta		 : 1;
		BYTE	bit_04	: 1;
		BYTE	bit_05	: 1;
		BYTE	bit_06	: 1;
		BYTE	bit_07	: 1;
		BYTE	bit_08	: 1;
		BYTE	bit_09	: 1;
		BYTE	bit_10	: 1;
		BYTE	bit_11	: 1;
		BYTE	bit_12	: 1;
		BYTE	bit_13	: 1;
		BYTE	bit_14	: 1;
		BYTE	bit_15	: 1;
	} B;
};

/**
 * CBaseCtrl is an abstract baseic class, which provides a common
 * interface for all kinds of controls. It has the task to ease
 * up programming and avoid memory leaks.
 **/
class CBaseCtrl
{
public:
	DWORD			_cbSize;

	friend class CCtrlList;

protected:
	
	CCtrlFlags		_Flags; 
	HWND			_hwnd;
	WORD			_idCtrl;
	LPCSTR			_pszModule;
	LPCSTR			_pszSetting;
	LPTSTR			_pszValue;

	/**
	 * Private constructure is to force the class used as base class only.
	 *
	 * @param	 none
	 *
	 * @return	nothing
	 **/
	CBaseCtrl();
	CBaseCtrl(HWND hDlg, WORD idCtrl, LPCSTR pszSetting);
	CBaseCtrl(HWND hDlg, WORD idCtrl, LPCSTR pszModule, LPCSTR pszSetting);

	/**
	 * Private constructure is to force the class used as base class only.
	 *
	 * @param	 none
	 *
	 * @return	nothing
	 **/
	~CBaseCtrl();

public:

	/**
	 *
	 *
	 **/
	static FORCEINLINE CBaseCtrl* GetObj(HWND hCtrl) 
		{ return (CBaseCtrl*) GetUserData(hCtrl); }

	/**
	 *
	 *
	 **/
	static FORCEINLINE CBaseCtrl* GetObj(HWND hDlg, WORD idCtrl)
		{ return GetObj(GetDlgItem(hDlg, idCtrl)); }
	
	/**
	 *
	 *
	 **/
	FORCEINLINE CCtrlFlags Flags() const { return _Flags; }

	/**
	 * This is a pure virtual method, which is the common interface 
	 * for deleting an instance of this class.
	 *
	 * @param	 none
	 *
	 * @return	nothing
	 **/
	virtual void Release() { }

	/**
	 * This is a pure virtual method, which is the common interface 
	 * to handle database changes. It reads the new values from
	 * database and refreshes the content of the associated control.
	 *
	 * @param	 hContact	- the handle of the contact
	 * @param	 pszProto	 - the contact's protocol module
	 *
	 * @retval	TRUE	- the content was updated
	 * @retval	FALSE - content not updated
	 **/
	virtual BOOL OnInfoChanged(MCONTACT hContact, LPCSTR pszProto) { return 0; }

	/**
	 * This is a pure virtual method, which is the common interface 
	 * for applying changed content of a control to database.
	 *
	 * @param	 hContact	- the handle of the contact
	 * @param	 pszProto	 - the contact's protocol module
	 *
	 * @return	nothing
	 **/
	virtual void OnApply(MCONTACT hContact, LPCSTR pszProto) { }

	/**
	 * This is a pure virtual method, which is called to set the 
	 * changed bit. This is to indicate a setting has changed and
	 * therefore enable the apply button of the details dialog.
	 *
	 * @param	 none
	 *
	 * @return	nothing
	 **/
	virtual void OnChangedByUser(WORD wChangedMsg) { }

	virtual void OnReset() { }

	INT_PTR OnSetTextColour(HDC hdc);
};

/***********************************************************************************************************
 * CCtrlList declaration
 ***********************************************************************************************************/

/**
 * The CCtrlList class is a sorted list of all dialog controls, such as edit, combo, etc. 
 * with a common data structure and interface, described by the abstract class CBaseCtrl.
 * The CCtrlList class sends notification messages to all its or to all relevant members.
 * This reduces the risk of forgetting some message handling.
 **/
class CCtrlList : public LIST<CBaseCtrl>
{
	HWND	_hOwnerDlg;

	static int sortFunc(const CBaseCtrl *tz1, const CBaseCtrl *tz2);

	CCtrlList(HWND hOwnerDlg);
	~CCtrlList();

public:
	
	static CCtrlList* CreateObj(HWND hOwnerDlg);

	static FORCEINLINE CCtrlList* GetObj(HWND hDlg) 
		{ return (CCtrlList*)GetUserData(hDlg); }

	void		Release();
	void		OnReset();
	BOOL		OnInfoChanged(MCONTACT hContact, LPCSTR pszProto);
	void		OnApply(MCONTACT hContact, LPCSTR pszProto);
	void		OnChangedByUser(WORD idCtrl, WORD wChangedMsg);
	INT_PTR OnSetTextColour(HWND hCtrl, HDC hdc);
};

#endif /* _UI_CTRL_BASE_INCLUDE_ */
