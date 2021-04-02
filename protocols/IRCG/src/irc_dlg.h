/*
IRC plugin for Miranda IM

Copyright (C) 2003-05 Jurgen Persson
Copyright (C) 2007-09 George Hazan

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#pragma once

struct CIrcBaseDlg : public CProtoDlgBase<CIrcProto>
{
	CIrcBaseDlg(CIrcProto *ppro, int dlgId) :
		CProtoDlgBase<CIrcProto>(ppro, dlgId)
	{}

	virtual void Update() {};
};

/////////////////////////////////////////////////////////////////////////////////////////
// Dialogs

struct CMessageBoxDlg : public CIrcBaseDlg
{
	DCCINFO *pdci;

	CMessageBoxDlg(CIrcProto* _pro, DCCINFO* _dci);

	bool OnApply() override;
};

struct CCoolIrcDlg : public CIrcBaseDlg
{
	CCoolIrcDlg(CIrcProto* _pro, int dlgId);

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	bool OnInitDialog() override;
	void OnDestroy() override;
};

struct CWhoisDlg : public CCoolIrcDlg
{
	CWhoisDlg(CIrcProto* _pro);

	CCtrlCombo  m_InfoNick;
	CCtrlEdit   m_Reply;
	CCtrlBase   m_Caption, m_AwayTime;
	CCtrlBase   m_InfoName, m_InfoId, m_InfoAddress, m_InfoChannels, m_InfoAuth,
		m_InfoServer, m_InfoAway2, m_InfoOther;
	CCtrlButton m_Ping, m_Version, m_Time, m_userInfo, m_Refresh, m_Query;

	void ShowMessage(const CIrcMessage*);
	void ShowMessageNoUser(const CIrcMessage*);

	void OnGo(CCtrlButton*);
	void OnQuery(CCtrlButton*);
	void OnPing(CCtrlButton*);
	void OnUserInfo(CCtrlButton*);
	void OnTime(CCtrlButton*);
	void OnVersion(CCtrlButton*);

	bool OnInitDialog() override;
	void OnDestroy() override;
};

struct CNickDlg : public CCoolIrcDlg
{
	CNickDlg(CIrcProto* _pro);

	CCtrlCombo m_Enick;

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
};

struct CListDlg : public CIrcBaseDlg
{
	CListDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	void OnDestroy() override;
	int Resizer(UTILRESIZECONTROL *urc) override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	CCtrlListView m_list, m_list2;
	CCtrlEdit m_filter, m_status;
	UINT_PTR m_timer;

	CCtrlButton m_Join;
	void onClick_Join(CCtrlButton*);

	void onChange_Filter(CCtrlEdit *ctrl);
	void onColumnClick_List(CCtrlListView::TEventInfo* ev);

	void UpdateList(void);
};

struct CJoinDlg : public CCoolIrcDlg
{
	CJoinDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
};

struct CManagerDlg : public CCoolIrcDlg
{
	CManagerDlg(CIrcProto* _pro);

	CCtrlCheck   m_check1, m_check2, m_check3, m_check4, m_check5, m_check6, m_check7, m_check8, m_check9;
	CCtrlEdit    m_key, m_limit;
	CCtrlCombo   m_topic;
	CCtrlCheck   m_radio1, m_radio2, m_radio3;
	CCtrlMButton m_add, m_edit, m_remove, m_applyTopic, m_applyModes;
	CCtrlListBox m_list;

	bool OnInitDialog() override;
	bool OnClose() override;
	void OnDestroy() override;

	void OnCheck(CCtrlData*);
	void OnCheck5(CCtrlData*);
	void OnCheck6(CCtrlData*);
	void OnRadio(CCtrlData*);

	void OnAdd(CCtrlButton*);
	void OnEdit(CCtrlButton*);
	void OnRemove(CCtrlButton*);

	void OnListDblClick(CCtrlListBox*);
	void OnChangeList(CCtrlListBox*);
	void OnChangeModes(CCtrlData*);
	void OnChangeTopic(CCtrlData*);

	void OnApplyModes(CCtrlButton*);
	void OnApplyTopic(CCtrlButton*);

	void ApplyQuestion();
	void CloseQuestion();
	void InitManager(int mode, const wchar_t* window);
};

struct CQuestionDlg : public CCoolIrcDlg
{
	CQuestionDlg(CIrcProto* _pro, CManagerDlg* owner = nullptr);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void Activate();

private:
	CManagerDlg* m_owner;
};
