/*
Copyright (c) 2013-16 Miranda NG project (http://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#pragma once

typedef CProtoDlgBase<CVkProto> CVkDlgBase;

////////////////////////////////// IDD_CAPTCHAFORM ////////////////////////////////////////

struct CAPTCHA_FORM_PARAMS
{
	HBITMAP bmp;
	int w, h;
	char Result[100];
};

class CVkCaptchaForm : public CVkDlgBase
{
	CCtrlData m_instruction;
	CCtrlEdit m_edtValue;
	CCtrlButton m_btnOpenInBrowser;
	CCtrlButton m_btnOk;
	CAPTCHA_FORM_PARAMS *m_param;

public:
	CVkCaptchaForm(CVkProto *proto, CAPTCHA_FORM_PARAMS *param);
	void OnInitDialog();
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void OnDestroy();
	void On_btnOpenInBrowser_Click(CCtrlButton*);
	void On_btnOk_Click(CCtrlButton*);
	void On_edtValue_Change(CCtrlEdit*);
	
};

////////////////////////////////// IDD_WALLPOST ///////////////////////////////////////////

struct WALLPOST_FORM_PARAMS
{
	wchar_t *ptszMsg;
	wchar_t *ptszUrl;
	wchar_t *ptszNick;
	bool bFriendsOnly;

	WALLPOST_FORM_PARAMS(wchar_t *nick) :
		ptszNick(nick),
		bFriendsOnly(false)
	{
		ptszMsg = ptszUrl = NULL;
	}

	~WALLPOST_FORM_PARAMS()
	{
		mir_free(ptszMsg);
		mir_free(ptszUrl);
		mir_free(ptszNick);
	}
};

class CVkWallPostForm : public CVkDlgBase
{
	CCtrlEdit m_edtMsg;
	CCtrlEdit m_edtUrl;
	CCtrlCheck m_cbOnlyForFriends;
	CCtrlButton m_btnShare;

	WALLPOST_FORM_PARAMS *m_param;

public:
	CVkWallPostForm(CVkProto *proto, WALLPOST_FORM_PARAMS *param);
	void OnInitDialog();
	void OnDestroy();
	void On_btnShare_Click(CCtrlButton*);
	void On_edtValue_Change(CCtrlEdit*);
};

////////////////////////////////// IDD_INVITE /////////////////////////////////////////////

class CVkInviteChatForm : public CVkDlgBase
{
	CCtrlButton m_btnOk;
	CCtrlCombo m_cbxCombo;

public:
	MCONTACT m_hContact;

	CVkInviteChatForm(CVkProto *proto);
	void OnInitDialog();
	void btnOk_OnOk(CCtrlButton*);
};

////////////////////////////////// IDD_GC_CREATE //////////////////////////////////////////

class CVkGCCreateForm : public CVkDlgBase
{
	CCtrlButton m_btnOk;
	CCtrlClc m_clCList;
	CCtrlEdit m_edtTitle;

public:
	CVkGCCreateForm(CVkProto *proto);
	void OnInitDialog();
	void btnOk_OnOk(CCtrlButton*);
	void FilterList(CCtrlClc*);
	void ResetListOptions(CCtrlClc*);
};