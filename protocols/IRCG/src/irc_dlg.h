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

/////////////////////////////////////////////////////////////////////////////////////////
// Dialogs

struct CMessageBoxDlg : public CProtoDlgBase < CIrcProto >
{
	DCCINFO *pdci;

	CMessageBoxDlg(CIrcProto* _pro, DCCINFO* _dci);

	bool OnApply() override;
};

struct CCoolIrcDlg : public CProtoDlgBase < CIrcProto >
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

struct CListDlg : public CProtoDlgBase < CIrcProto >
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

struct CQuickDlg : public CCoolIrcDlg
{
	CQuickDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	CCtrlCombo m_serverCombo;
	void OnServerCombo(CCtrlData*);

private:
	struct SERVER_INFO* m_si;
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

/////////////////////////////////////////////////////////////////////////////////////////
// options

//---- the first property page: Account -------------------------------------------------

struct CConnectPrefsDlg : public CProtoDlgBase < CIrcProto >
{
	bool m_serverlistModified;

	CCtrlCombo   m_serverCombo;
	CCtrlEdit    m_server, m_port, m_port2, m_pass;
	CCtrlMButton m_add, m_edit, m_del;
	CCtrlEdit    m_nick, m_nick2, m_name, m_userID;

	CCtrlCheck   m_ident, m_identTimer;
	CCtrlEdit    m_identSystem, m_identPort;

	CCtrlCheck   m_forceVisible, m_rejoinOnKick, m_rejoinChannels, m_disableError,
		m_address, m_useServer, m_showServer, m_keepAlive, m_autoJoin,
		m_oldStyle, m_onlineNotif, m_channelAway, m_enableServer, m_useSasl;
	CCtrlEdit    m_onlineTimer, m_limit, m_ssl;
	CCtrlSpin    m_spin1, m_spin2;

	CConnectPrefsDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	bool OnApply() override;

	void OnServerCombo(CCtrlData*);
	void OnAddServer(CCtrlButton*);
	void OnDeleteServer(CCtrlButton*);
	void OnEditServer(CCtrlButton*);
	void OnStartup(CCtrlData*);
	void OnIdent(CCtrlData*);
	void OnUseServer(CCtrlData*);
	void OnOnlineNotif(CCtrlData*);
	void OnChannelAway(CCtrlData*);
};

//---- the second property page: DCC/CTCP -----------------------------------------------

struct CCtcpPrefsDlg : public CProtoDlgBase < CIrcProto >
{
	CCtrlCombo m_combo;
	CCtrlCheck m_slow, m_fast, m_disc, m_passive, m_sendNotice, m_enableIP, m_fromServer;
	CCtrlEdit m_ip, m_userInfo;
	CCtrlCheck m_radio1, m_radio2, m_radio3;

	CCtcpPrefsDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	bool OnApply() override;

	void OnClicked(CCtrlData*);
};

//---- the third property page: Other ---------------------------------------------------

struct COtherPrefsDlg : public CProtoDlgBase < CIrcProto >
{
	bool m_performlistModified;

	CCtrlButton  m_url;
	CCtrlMButton m_add, m_delete;
	CCtrlCombo   m_performCombo, m_codepage;
	CCtrlEdit    m_pertormEdit, m_quitMessage, m_alias;
	CCtrlCheck   m_perform, m_scripting, m_autodetect;

	COtherPrefsDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	void OnPerformCombo(CCtrlData*);
	void OnCodePage(CCtrlData*);
	void OnPerformEdit(CCtrlData*);
	void OnPerform(CCtrlData*);
	void OnAdd(CCtrlButton*);
	void OnDelete(CCtrlButton*);

	void addPerformComboValue(int idx, const char* szValueName);
};

//---- the fourth property page: Ignore -------------------------------------------------

struct CIgnorePrefsDlg : public CProtoDlgBase < CIrcProto >
{
	CCtrlMButton m_add, m_edit, m_del;
	CCtrlCheck m_enable, m_ignoreChat, m_ignoreFile, m_ignoreChannel, m_ignoreUnknown;
	CCtrlListView m_list;

	CIgnorePrefsDlg(CIrcProto* _pro);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override;

	void List_OnColumnClick(CCtrlListView::TEventInfo*);
	void OnEnableIgnore(CCtrlData*);
	void OnIgnoreChat(CCtrlData*);
	void OnAdd(CCtrlButton*);
	void OnEdit(CCtrlButton*);
	void OnDelete(CCtrlButton*);

	void FixButtons(void);
	void RebuildList(void);
	void UpdateList(void);
};

struct CAddIgnoreDlg : public CProtoDlgBase < CIrcProto >
{
	CIgnorePrefsDlg* m_owner;

	wchar_t szOldMask[500];

	CAddIgnoreDlg(CIrcProto* _pro, const wchar_t* mask, CIgnorePrefsDlg* parent);

	bool OnInitDialog() override;
	bool OnApply() override;
	void OnDestroy() override;
};
