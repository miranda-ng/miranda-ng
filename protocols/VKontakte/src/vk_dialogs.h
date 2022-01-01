/*
Copyright (c) 2013-22 Miranda NG team (https://miranda-ng.org)

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

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;
	
	void On_btnOpenInBrowser_Click(CCtrlButton*);
	void On_edtValue_Change(CCtrlEdit*);

};

////////////////////////////////// IDD_WALLPOST ///////////////////////////////////////////

struct WALLPOST_FORM_PARAMS
{
	wchar_t *pwszMsg;
	wchar_t *pwszUrl;
	wchar_t *pwszNick;
	bool bFriendsOnly;

	WALLPOST_FORM_PARAMS(wchar_t *nick) :
		pwszNick(nick),
		bFriendsOnly(false)
	{
		pwszMsg = pwszUrl = nullptr;
	}

	~WALLPOST_FORM_PARAMS()
	{
		mir_free(pwszMsg);
		mir_free(pwszUrl);
		mir_free(pwszNick);
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

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void On_edtValue_Change(CCtrlEdit*);
};

////////////////////////////////// IDD_INVITE /////////////////////////////////////////////

class CVkInviteChatForm : public CVkDlgBase
{
	CCtrlCombo m_cbxCombo;

public:
	MCONTACT m_hContact;

	CVkInviteChatForm(CVkProto *proto);

	bool OnInitDialog() override;
	bool OnApply() override;
};

////////////////////////////////// IDD_GC_CREATE //////////////////////////////////////////

class CVkGCCreateForm : public CVkDlgBase
{
	CCtrlClc m_clc;
	CCtrlEdit m_edtTitle;

public:
	CVkGCCreateForm(CVkProto *proto);
	bool OnInitDialog() override;
	bool OnApply() override;

	void FilterList(CCtrlClc*);
	void ResetListOptions();
};

////////////////////////////////// IDD_CONTACTDELETE //////////////////////////////////////

struct CONTACTDELETE_FORM_PARAMS
{
	wchar_t *pwszNick;
	bool bDeleteFromFriendlist;
	bool bEnableDeleteFromFriendlist;
	bool bDeleteDialog;

	CONTACTDELETE_FORM_PARAMS(wchar_t *nick, bool _bDeleteFromFriendlist, bool _bEnableDeleteFromFriendlist, bool _bDeleteDialog) :
		pwszNick(nick),
		bDeleteFromFriendlist(_bDeleteFromFriendlist),
		bEnableDeleteFromFriendlist(_bEnableDeleteFromFriendlist),
		bDeleteDialog(_bDeleteDialog)
	{}

};


class CVkContactDeleteForm : public CVkDlgBase
{
	CCtrlBase m_stText;
	CCtrlCheck m_cbDeleteFromFriendlist;
	CCtrlCheck m_cbDeleteDialog;

	CONTACTDELETE_FORM_PARAMS *m_param;

public:
	CVkContactDeleteForm(CVkProto *proto, CONTACTDELETE_FORM_PARAMS *param);
	bool OnInitDialog() override;
	bool OnApply() override;
};
