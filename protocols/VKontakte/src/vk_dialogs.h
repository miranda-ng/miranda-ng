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

struct CAPTCHA_FORM_PARAMS
{
	HBITMAP bmp;
	int w, h;
	char Result[100];
};

class CaptchaForm : public CVkDlgBase
{
	CCtrlData m_instruction;
	CCtrlEdit m_edtValue;
	CCtrlButton m_btnOpenInBrowser;
	CCtrlButton m_btnOk;
	CAPTCHA_FORM_PARAMS* m_param;

public:
	CaptchaForm(CVkProto *proto, CAPTCHA_FORM_PARAMS* param);
	void OnInitDialog();
	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam);
	void OnDestroy();
	void On_btnOpenInBrowser_Click(CCtrlButton*);
	void On_btnOk_Click(CCtrlButton*);
	void On_edtValue_Change(CCtrlEdit*);
	
};