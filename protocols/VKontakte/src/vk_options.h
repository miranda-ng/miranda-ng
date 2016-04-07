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

////////////////////// Account manager dialog ////////////////////////////////

class CVkAccMgrForm : public CVkDlgBase
{
	typedef CVkDlgBase CSuper;

	CCtrlEdit m_edtLogin;
	CCtrlEdit m_edtPassword;
	CCtrlHyperlink m_hlLink;

	CVkProto *m_proto;
	
	pass_ptrT m_ptszOldPass;
	ptrT m_ptszOldLogin;

public:
	CVkAccMgrForm(CVkProto *proto, HWND hwndParent);
	void OnInitDialog();
	void OnApply();
};

////////////////////// Account page //////////////////////////////////////////

class CVkOptionAccountForm : public CVkDlgBase
{
	CCtrlEdit m_edtLogin;
	CCtrlEdit m_edtPassword;
	CCtrlHyperlink m_hlLink;
	CCtrlEdit m_edtGroupName;
	CCtrlCombo m_cbxVKLang;
	CCtrlCheck m_cbDelivery;
	CCtrlCheck m_cbUseLocalTime;
	CCtrlCheck m_cbAutoClean;
	CCtrlCheck m_cbMarkMessageOnRead;
	CCtrlCheck m_cbMarkMessageOnReceive;
	CCtrlCheck m_cbMarkMessageOnReply;
	CCtrlCheck m_cbMarkMessageOnTyping;
	CCtrlCheck m_cbSyncHistoryOff;
	CCtrlCheck m_cbSyncHistoryAuto;
	CCtrlCheck m_cbSyncHistoryForLast1Day;
	CCtrlCheck m_cbSyncHistoryForLast3Day;

	CVkProto *m_proto;

	pass_ptrT m_ptszOldPass;
	ptrT m_ptszOldLogin;
	ptrT m_ptszOldGroup;

public:
	CVkOptionAccountForm(CVkProto *proto);
	void OnInitDialog();
	void OnApply();
};

////////////////////// Advanced page /////////////////////////////////////////

class CVkOptionAdvancedForm : public CVkDlgBase
{
	CCtrlCheck m_cbHideChats;
	CCtrlCheck m_cbSyncReadMessageStatusFromServer;
	CCtrlCheck m_cbMesAsUnread;
	
	CCtrlEdit m_edtInvInterval;
	CCtrlButton m_spInvInterval;

	CCtrlCheck m_cbUseNonStandardNotifications;
	CCtrlCheck m_cbUseNonStandardUrlEncode;

	CCtrlCheck m_cbReportAbuse;
	CCtrlCheck m_cbClearServerHistory;
	CCtrlCheck m_cbRemoveFromFrendlist;
	CCtrlCheck m_cbRemoveFromCList;

	CCtrlCheck m_cbMusicSendOff;
	CCtrlCheck m_cbMusicSendBroadcastAndStatus;
	CCtrlCheck m_cbSendMetodBroadcast;
	CCtrlCheck m_cbMusicSendStatus;

	CCtrlEdit m_edtReturnChatMessage;

	CVkProto *m_proto;

public:
	CVkOptionAdvancedForm(CVkProto  *proto);
	void OnInitDialog();
	void OnApply();
};

////////////////////// News and notifications ////////////////////////////////

class CVkOptionFeedsForm : public CVkDlgBase
{
	CCtrlCheck m_cbNewsEnabled;
	
	CCtrlEdit m_edtNewsInterval;
	CCtrlButton m_spNewsInterval;

	CCtrlCheck m_cbNotificationsEnabled;

	CCtrlEdit m_edtNotificationsInterval;
	CCtrlButton m_spNotificationsInterval;

	CCtrlCheck m_cbNotificationsMarkAsViewed;
	CCtrlCheck m_cbSpecialContactAlwaysEnabled;
	CCtrlCheck m_cbNewsAutoClearHistory;

	CCtrlCheck m_cbNewsFilterPosts;
	CCtrlCheck m_cbNewsFilterPhotos;
	CCtrlCheck m_cbNewsFilterTags;
	CCtrlCheck m_cbNewsFilterWallPhotos;

	CCtrlCheck m_cbNewsSourceFriends;
	CCtrlCheck m_cbNewsSourceGroups;
	CCtrlCheck m_cbNewsSourcePages;
	CCtrlCheck m_cbNewsSourceFollowing;
	CCtrlCheck m_cbNewsSourceIncludeBanned;
	CCtrlCheck m_cbNewsSourceNoReposts;
	
	CCtrlCheck m_cbNotificationFilterComments;
	CCtrlCheck m_cbNotificationFilterLikes;
	CCtrlCheck m_cbNotificationFilterReposts;
	CCtrlCheck m_cbNotificationFilterMentions;
	CCtrlCheck m_cbNotificationFilterInvites;

public:
	CVkOptionFeedsForm(CVkProto *proto);
	void OnInitDialog();

	void On_cbNewsEnabledChange(CCtrlCheck*);
	void On_cbNotificationsEnabledChange(CCtrlCheck*);
};

////////////////////// View page /////////////////////////////////////////////

class CVkOptionViewForm : public CVkDlgBase
{
	CCtrlCheck m_cbIMGBBCSupportOff;
	CCtrlCheck m_cbIMGBBCSupportFullSize;
	CCtrlCheck m_cbIMGBBCSupport130;
	CCtrlCheck m_cbIMGBBCSupport604;
	
	CCtrlCheck m_cbBBCForNewsOff;
	CCtrlCheck m_cbBBCForNewsBasic;
	CCtrlCheck m_cbBBCForNewsAdvanced;

	CCtrlCheck m_cbUseBBCOnAttacmentsAsNews;

	CCtrlCheck m_cbBBCForAttachmentsOff;
	CCtrlCheck m_cbBBCForAttachmentsBasic;
	CCtrlCheck m_cbBBCForAttachmentsAdvanced;

	CCtrlCheck m_cbStikersAsSmyles;
	CCtrlCheck m_cbShortenLinksForAudio;
	
	CVkProto *m_proto;

public:
	CVkOptionViewForm(CVkProto *proto);
	void OnInitDialog();
	void OnApply();
};

////////////////////// Menu page /////////////////////////////////////////////

class CVkOptionMenuForm : public CVkDlgBase
{
	CCtrlCheck m_cbMenuEnabled0;
	CCtrlCheck m_cbMenuEnabled1;
	CCtrlCheck m_cbMenuEnabled2;
	CCtrlCheck m_cbMenuEnabled3;
	CCtrlCheck m_cbMenuEnabled4;
	CCtrlCheck m_cbMenuEnabled5;
	CCtrlCheck m_cbMenuEnabled6;

	CVkProto *m_proto;

public:
	CVkOptionMenuForm(CVkProto *proto);
	void OnApply();
};