/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "..\resource.h"

#include <commctrl.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>

#include "optionsdlg.h"

/// dialog that holds all 
/// gui dialogs..
class CSettingsDlgHolder : public CDialogImpl<CSettingsDlgHolder>,
						   public CMessageFilter, 
						   public IMSingeltone<CSettingsDlgHolder>
{
public: /// ctors
	CSettingsDlgHolder(void);
		/// dctors
	~CSettingsDlgHolder(void);
public:
	enum {IDD = IDD_SDLGHOLDER};	
	BEGIN_MSG_MAP(CSettingsDlgHolder)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(PSM_CHANGED, OnSettingsChanged)
	END_MSG_MAP()
protected:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnNotify(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnSettingsChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
private:
	void OnNotifyParents(LPNMHDR lpnmhdr, int nCode);
private:
	bool m_bTabSelected;
public:
	static BOOL CALLBACK FakeDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
protected:
	CTabCtrl m_tabCtrl;			/// holder of all settings dlg
	CStatic m_statusMsg;
	COptionsDlg m_generalOpt;	/// general options dlg
public:
	bool m_bDestroying;
};
