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

////////////////////// Account manager dialog ////////////////////////////////

INT_PTR CVkProto::SvcCreateAccMgrUI(WPARAM, LPARAM lParam)
{
	CVkAccMgrForm *dlg = new CVkAccMgrForm(this, (HWND)lParam);
	dlg->Show();
	return (INT_PTR)dlg->GetHwnd();
}

//////////////////////////////////////////////////////////////////////////////

CVkAccMgrForm::CVkAccMgrForm(CVkProto *proto, HWND hwndParent):
	CVkDlgBase(proto, IDD_ACCMGRUI, false),
	m_edtLogin(this, IDC_LOGIN),
	m_edtPassword(this, IDC_PASSWORD),
	m_hlLink(this, IDC_URL, "https://vk.com/"),
	m_proto(proto)

{
	SetParent(hwndParent);

	CreateLink(m_edtLogin, "Login", _T(""));
}

void CVkAccMgrForm::OnInitDialog()
{
	CSuper::OnInitDialog();

	m_ptszOldLogin = m_edtLogin.GetText();
	m_edtLogin.SendMsg(EM_LIMITTEXT, 1024, 0);

	m_ptszOldPass = m_proto->GetUserStoredPassword();
	m_edtPassword.SetText(m_ptszOldPass);
	m_edtPassword.SendMsg(EM_LIMITTEXT, 1024, 0);
}

void CVkAccMgrForm::OnApply()
{
	pass_ptrT ptszNewPass(m_edtPassword.GetText());
	bool bPassChanged = mir_tstrcmp(m_ptszOldPass, ptszNewPass) != 0;
	if (bPassChanged) {
		T2Utf szRawPasswd(ptszNewPass);
		m_proto->setString("Password", szRawPasswd);
		pass_ptrA pszPass(szRawPasswd);
		m_ptszOldPass = ptszNewPass;
	}

	ptrT ptszNewLogin(m_edtLogin.GetText());
	if (bPassChanged || mir_tstrcmpi(m_ptszOldLogin, ptszNewLogin))
		m_proto->ClearAccessToken();
	m_ptszOldLogin = ptszNewLogin;
}

////////////////////// Options ///////////////////////////////////////////////

int CVkProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hInstance = hInst;
	odp.ptszTitle = m_tszUserName;
	odp.dwInitParam = LPARAM(this);
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR | ODPF_DONTTRANSLATE;
	odp.ptszGroup = LPGENT("Network");

	odp.ptszTab = LPGENT("Account");
	odp.position = 1;
	odp.pDialog = new CVkOptionAccountForm(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Advanced");
	odp.position = 2;
	odp.pDialog = new CVkOptionAdvancedForm(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("News and notifications");
	odp.position = 3;
	odp.pDialog = new CVkOptionFeedsForm(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("View");
	odp.position = 4;
	odp.pDialog = new CVkOptionViewForm(this);
	Options_AddPage(wParam, &odp);

	odp.ptszTab = LPGENT("Menu");
	odp.position = 5;
	odp.pDialog = new CVkOptionMenuForm(this);
	Options_AddPage(wParam, &odp);

	return 0;
}

////////////////////// Account page //////////////////////////////////////////

static CVKLang vkLangCodes[] = {
	{ NULL, LPGENT("Account language") },
	{ _T("en"), LPGENT("English") },
	{ _T("ru"), LPGENT("Russian") },
	{ _T("be"), LPGENT("Belarusian") },
	{ _T("ua"), LPGENT("Ukrainian") },
	{ _T("es"), LPGENT("Spanish") },
	{ _T("fi"), LPGENT("Finnish") },
	{ _T("de"), LPGENT("German") },
	{ _T("it"), LPGENT("Italian") },
};

CVkOptionAccountForm::CVkOptionAccountForm(CVkProto *proto):
	CVkDlgBase(proto, IDD_OPT_MAIN, false),
	m_edtLogin(this, IDC_LOGIN),
	m_edtPassword(this, IDC_PASSWORD),
	m_hlLink(this, IDC_URL, "https://vk.com/"),
	m_edtGroupName(this, IDC_GROUPNAME),
	m_cbxVKLang(this, IDC_COMBO_LANGUAGE),
	m_cbDelivery(this, IDC_DELIVERY),
	m_cbUseLocalTime(this, IDC_USE_LOCAL_TIME),
	m_cbAutoClean(this, IDC_AUTOCLEAN),
	m_cbMarkMessageOnRead(this, IDC_ONREAD),
	m_cbMarkMessageOnReceive(this, IDC_ONRECEIVE),
	m_cbMarkMessageOnReply(this, IDC_ONREPLY),
	m_cbMarkMessageOnTyping(this, IDC_ONTYPING),
	m_cbSyncHistoryOff(this, IDC_SYNC_OFF),
	m_cbSyncHistoryAuto(this, IDC_SYNC_AUTO),
	m_cbSyncHistoryForLast1Day(this, IDC_SYNC_LAST1DAY),
	m_cbSyncHistoryForLast3Day(this, IDC_SYNC_LAST3DAY),
	m_proto(proto)

{
	CreateLink(m_edtLogin, "Login", _T(""));
	CreateLink(m_edtGroupName, "ProtoGroup", _T("VKontakte"));
	CreateLink(m_cbDelivery, m_proto->m_vkOptions.bServerDelivery);
	CreateLink(m_cbUseLocalTime, m_proto->m_vkOptions.bUseLocalTime);
	CreateLink(m_cbAutoClean, m_proto->m_vkOptions.bAutoClean);

}

void CVkOptionAccountForm::OnInitDialog()
{
	m_ptszOldLogin = m_edtLogin.GetText();
	m_edtLogin.SendMsg(EM_LIMITTEXT, 1024, 0);

	m_ptszOldPass = m_proto->GetUserStoredPassword();
	m_edtPassword.SetText(m_ptszOldPass);
	m_edtPassword.SendMsg(EM_LIMITTEXT, 1024, 0);

	m_ptszOldGroup = m_edtGroupName.GetText();

	m_cbMarkMessageOnRead.SetState(m_proto->m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnRead);
	m_cbMarkMessageOnReceive.SetState(m_proto->m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnReceive);
	m_cbMarkMessageOnReply.SetState(m_proto->m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnReply);
	m_cbMarkMessageOnTyping.SetState(m_proto->m_vkOptions.iMarkMessageReadOn == MarkMsgReadOn::markOnTyping);

	m_cbSyncHistoryOff.SetState(m_proto->m_vkOptions.iSyncHistoryMetod == SyncHistoryMetod::syncOff);
	m_cbSyncHistoryAuto.SetState(m_proto->m_vkOptions.iSyncHistoryMetod == SyncHistoryMetod::syncAuto);
	m_cbSyncHistoryForLast1Day.SetState(m_proto->m_vkOptions.iSyncHistoryMetod == SyncHistoryMetod::sync1Days);
	m_cbSyncHistoryForLast3Day.SetState(m_proto->m_vkOptions.iSyncHistoryMetod == SyncHistoryMetod::sync3Days);
	
	for (size_t i = 0; i < _countof(vkLangCodes); i++) {
		m_cbxVKLang.AddString(TranslateTS(vkLangCodes[i].szDescription), (LPARAM)vkLangCodes[i].szCode);
		if (!mir_tstrcmpi(vkLangCodes[i].szCode, m_proto->m_vkOptions.ptszVKLang))
			m_cbxVKLang.SetCurSel(i);
	}
	
}

void CVkOptionAccountForm::OnApply()
{
	if (m_cbSyncHistoryOff.GetState())
		m_proto->m_vkOptions.iSyncHistoryMetod = SyncHistoryMetod::syncOff;
	if (m_cbSyncHistoryAuto.GetState())
		m_proto->m_vkOptions.iSyncHistoryMetod = SyncHistoryMetod::syncAuto;
	if (m_cbSyncHistoryForLast1Day.GetState())
		m_proto->m_vkOptions.iSyncHistoryMetod = SyncHistoryMetod::sync1Days;
	if (m_cbSyncHistoryForLast3Day.GetState())
		m_proto->m_vkOptions.iSyncHistoryMetod = SyncHistoryMetod::sync3Days;

	if (m_cbMarkMessageOnRead.GetState())
		m_proto->m_vkOptions.iMarkMessageReadOn = MarkMsgReadOn::markOnRead;
	if (m_cbMarkMessageOnReceive.GetState())
		m_proto->m_vkOptions.iMarkMessageReadOn = MarkMsgReadOn::markOnReceive;
	if (m_cbMarkMessageOnReply.GetState())
		m_proto->m_vkOptions.iMarkMessageReadOn = MarkMsgReadOn::markOnReply;
	if (m_cbMarkMessageOnTyping.GetState())
		m_proto->m_vkOptions.iMarkMessageReadOn = MarkMsgReadOn::markOnTyping;

	m_proto->m_vkOptions.ptszVKLang = (TCHAR *)m_cbxVKLang.GetItemData(m_cbxVKLang.GetCurSel());
	if (!IsEmpty(m_proto->m_vkOptions.ptszVKLang))
		m_proto->setTString("VKLang", m_proto->m_vkOptions.ptszVKLang);
	else
		m_proto->delSetting("VKLang");
	
	ptrT ptszGroupName(m_edtGroupName.GetText());
	if (mir_tstrcmp(m_ptszOldGroup, ptszGroupName)) {
		m_proto->setGroup(ptszGroupName);
		m_ptszOldGroup = ptszGroupName;
	}

	pass_ptrT ptszNewPass(m_edtPassword.GetText());
	bool bPassChanged = mir_tstrcmp(m_ptszOldPass, ptszNewPass) != 0;
	if (bPassChanged) {
		T2Utf szRawPasswd(ptszNewPass);
		m_proto->setString("Password", szRawPasswd);
		pass_ptrA pszPass(szRawPasswd);
		m_ptszOldPass = ptszNewPass;
	}

	ptrT ptszNewLogin(m_edtLogin.GetText());
	if (bPassChanged || mir_tstrcmpi(m_ptszOldLogin, ptszNewLogin))
		m_proto->ClearAccessToken();
	m_ptszOldLogin = ptszNewLogin;
	
}

////////////////////// Advanced page /////////////////////////////////////////

CVkOptionAdvancedForm::CVkOptionAdvancedForm(CVkProto *proto):
	CVkDlgBase(proto, IDD_OPT_ADV, false),
	m_cbHideChats(this, IDC_HIDECHATS),
	m_cbSyncReadMessageStatusFromServer(this, IDC_SYNC_MSG_STATUS),
	m_cbMesAsUnread(this, IDC_MESASUREAD),
	m_edtInvInterval(this, IDC_ED_INT_INVIS),
	m_spInvInterval(this, IDC_SPIN_INT_INVIS),
	m_cbUseNonStandardNotifications(this, IDC_USENOSTDURLENCODE),
	m_cbUseNonStandardUrlEncode(this, IDC_USENOSTDURLENCODE),
	m_cbReportAbuse(this, IDC_REPORT_ABUSE),
	m_cbClearServerHistory(this, IDC_CLEAR_SERVER_HISTORY),
	m_cbRemoveFromFrendlist(this, IDC_REMOVE_FROM_FRENDLIST),
	m_cbRemoveFromCList(this, IDC_REMOVE_FROM_CLIST),
	m_cbMusicSendOff(this, IDC_SEND_MUSIC_NONE),
	m_cbMusicSendBroadcastAndStatus(this, IDC_SEND_MUSIC_BROADCAST_AND_STATUS),
	m_cbSendMetodBroadcast(this, IDC_SEND_MUSIC_BROADCAST),
	m_cbMusicSendStatus(this, IDC_SEND_MUSIC_STATUS),
	m_edtReturnChatMessage(this, IDC_RET_CHAT_MES),
	m_proto(proto)
{
	CreateLink(m_cbHideChats, m_proto->m_vkOptions.bHideChats);
	CreateLink(m_cbSyncReadMessageStatusFromServer, m_proto->m_vkOptions.bSyncReadMessageStatusFromServer);
	CreateLink(m_cbMesAsUnread, m_proto->m_vkOptions.bMesAsUnread);
	CreateLink(m_edtInvInterval, m_proto->m_vkOptions.iInvisibleInterval);

	CreateLink(m_cbUseNonStandardNotifications, m_proto->m_vkOptions.bUseNonStandardNotifications);
	CreateLink(m_cbUseNonStandardUrlEncode, m_proto->m_vkOptions.bUseNonStandardUrlEncode);
	CreateLink(m_cbReportAbuse, m_proto->m_vkOptions.bReportAbuse);
	CreateLink(m_cbClearServerHistory, m_proto->m_vkOptions.bClearServerHistory);
	CreateLink(m_cbRemoveFromFrendlist, m_proto->m_vkOptions.bRemoveFromFrendlist);
	CreateLink(m_cbRemoveFromCList, m_proto->m_vkOptions.bRemoveFromCList);

	CreateLink(m_edtReturnChatMessage, "ReturnChatMessage", TranslateT("I\'m back"));
}

void CVkOptionAdvancedForm::OnInitDialog()
{

	m_cbMusicSendOff.SetState(m_proto->m_vkOptions.iMusicSendMetod == MusicSendMetod::sendNone);
	m_cbMusicSendBroadcastAndStatus.SetState(m_proto->m_vkOptions.iMusicSendMetod == MusicSendMetod::sendBroadcastAndStatus);
	m_cbSendMetodBroadcast.SetState(m_proto->m_vkOptions.iMusicSendMetod == MusicSendMetod::sendBroadcastOnly);
	m_cbMusicSendStatus.SetState(m_proto->m_vkOptions.iMusicSendMetod == MusicSendMetod::sendStatusOnly);

	m_spInvInterval.SendMsg(UDM_SETRANGE, 0, MAKELONG(60, 0));
	m_spInvInterval.SendMsg(UDM_SETPOS, 0, m_proto->m_vkOptions.iInvisibleInterval);
}

void CVkOptionAdvancedForm::OnApply()
{
	if (m_cbMusicSendOff.GetState())
		m_proto->m_vkOptions.iMusicSendMetod = MusicSendMetod::sendNone;
	if (m_cbMusicSendBroadcastAndStatus.GetState())
		m_proto->m_vkOptions.iMusicSendMetod = MusicSendMetod::sendBroadcastAndStatus;
	if (m_cbSendMetodBroadcast.GetState())
		m_proto->m_vkOptions.iMusicSendMetod = MusicSendMetod::sendBroadcastOnly;
	if (m_cbMusicSendStatus.GetState())
		m_proto->m_vkOptions.iMusicSendMetod = MusicSendMetod::sendStatusOnly;

	m_proto->m_vkOptions.ptszReturnChatMessage = m_edtReturnChatMessage.GetText();
}

////////////////////// News and notifications ////////////////////////////////

CVkOptionFeedsForm::CVkOptionFeedsForm(CVkProto *proto):
	CVkDlgBase(proto, IDD_OPT_FEEDS, false),
	m_cbNewsEnabled(this, IDC_NEWS_ENBL),
	m_edtNewsInterval(this, IDC_ED_INT_NEWS),
	m_spNewsInterval(this, IDC_SPIN_INT_NEWS),
	m_cbNotificationsEnabled(this, IDC_NOTIF_ENBL),
	m_edtNotificationsInterval(this, IDC_ED_INT_NOTIF),
	m_spNotificationsInterval(this, IDC_SPIN_INT_NOTIF),
	m_cbNotificationsMarkAsViewed(this, IDC_NOTIF_MARK_VIEWED),
	m_cbSpecialContactAlwaysEnabled(this, IDC_SPEC_CONT_ENBL),
	m_cbNewsAutoClearHistory(this, IDC_NEWSAUTOCLEAR),
	m_cbNewsFilterPosts(this, IDC_F_POSTS),
	m_cbNewsFilterPhotos(this, IDC_F_PHOTOS),
	m_cbNewsFilterTags(this, IDC_F_TAGS),
	m_cbNewsFilterWallPhotos(this, IDC_F_WALLPHOTOS),
	m_cbNewsSourceFriends(this, IDC_S_FRIENDS),
	m_cbNewsSourceGroups(this, IDC_S_GROUPS),
	m_cbNewsSourcePages(this, IDC_S_PAGES),
	m_cbNewsSourceFollowing(this, IDC_S_FOLLOWING),
	m_cbNewsSourceIncludeBanned(this, IDC_S_BANNED),
	m_cbNewsSourceNoReposts(this, IDC_S_NOREPOSTES),
	m_cbNotificationFilterComments(this, IDC_N_COMMENTS),
	m_cbNotificationFilterLikes(this, IDC_N_LIKES),
	m_cbNotificationFilterReposts(this, IDC_N_REPOSTS),
	m_cbNotificationFilterMentions(this, IDC_N_MENTIONS),
	m_cbNotificationFilterInvites(this, IDC_N_INVITES)
{
	CreateLink(m_cbNewsEnabled, m_proto->m_vkOptions.bNewsEnabled);
	CreateLink(m_edtNewsInterval, m_proto->m_vkOptions.iNewsInterval);
	CreateLink(m_cbNotificationsEnabled, m_proto->m_vkOptions.bNotificationsEnabled);
	CreateLink(m_edtNotificationsInterval, m_proto->m_vkOptions.iNotificationsInterval);
	CreateLink(m_cbNotificationsMarkAsViewed, m_proto->m_vkOptions.bNotificationsMarkAsViewed);
	CreateLink(m_cbSpecialContactAlwaysEnabled, m_proto->m_vkOptions.bSpecialContactAlwaysEnabled);
	CreateLink(m_cbNewsAutoClearHistory, m_proto->m_vkOptions.bNewsAutoClearHistory);
	CreateLink(m_cbNewsFilterPosts, m_proto->m_vkOptions.bNewsFilterPosts);
	CreateLink(m_cbNewsFilterPhotos, m_proto->m_vkOptions.bNewsFilterPhotos);
	CreateLink(m_cbNewsFilterTags, m_proto->m_vkOptions.bNewsFilterTags);
	CreateLink(m_cbNewsFilterWallPhotos, m_proto->m_vkOptions.bNewsFilterWallPhotos);
	CreateLink(m_cbNewsSourceFriends, m_proto->m_vkOptions.bNewsSourceFriends);
	CreateLink(m_cbNewsSourceGroups, m_proto->m_vkOptions.bNewsSourceGroups);
	CreateLink(m_cbNewsSourcePages, m_proto->m_vkOptions.bNewsSourcePages);
	CreateLink(m_cbNewsSourceFollowing, m_proto->m_vkOptions.bNewsSourceFollowing);
	CreateLink(m_cbNewsSourceIncludeBanned, m_proto->m_vkOptions.bNewsSourceIncludeBanned);
	CreateLink(m_cbNewsSourceNoReposts, m_proto->m_vkOptions.bNewsSourceNoReposts);
	CreateLink(m_cbNotificationFilterComments, m_proto->m_vkOptions.bNotificationFilterComments);
	CreateLink(m_cbNotificationFilterLikes, m_proto->m_vkOptions.bNotificationFilterLikes);
	CreateLink(m_cbNotificationFilterReposts, m_proto->m_vkOptions.bNotificationFilterReposts);
	CreateLink(m_cbNotificationFilterMentions, m_proto->m_vkOptions.bNotificationFilterMentions);
	CreateLink(m_cbNotificationFilterInvites, m_proto->m_vkOptions.bNotificationFilterInvites);

	m_cbNewsEnabled.OnChange = Callback(this, &CVkOptionFeedsForm::On_cbNewsEnabledChange);
	m_cbNotificationsEnabled.OnChange = Callback(this, &CVkOptionFeedsForm::On_cbNotificationsEnabledChange);

}

void CVkOptionFeedsForm::OnInitDialog()
{
	m_spNewsInterval.SendMsg(UDM_SETRANGE, 0, MAKELONG(60 * 24, 1));
	m_spNewsInterval.SendMsg(UDM_SETPOS, 0, m_proto->m_vkOptions.iNewsInterval);

	m_spNotificationsInterval.SendMsg(UDM_SETRANGE, 0, MAKELONG(60 * 24, 1));
	m_spNotificationsInterval.SendMsg(UDM_SETPOS, 0, m_proto->m_vkOptions.iNotificationsInterval);

	On_cbNewsEnabledChange(&m_cbNewsEnabled);
	On_cbNotificationsEnabledChange(&m_cbNotificationsEnabled);
}

void CVkOptionFeedsForm::On_cbNewsEnabledChange(CCtrlCheck*)
{
	bool bState = m_cbNewsEnabled.GetState() != 0;

	m_edtNewsInterval.Enable(bState);
	m_spNewsInterval.Enable(bState);
	m_cbNewsFilterPosts.Enable(bState);
	m_cbNewsFilterPhotos.Enable(bState);
	m_cbNewsFilterTags.Enable(bState);
	m_cbNewsFilterWallPhotos.Enable(bState);
	m_cbNewsSourceFriends.Enable(bState);
	m_cbNewsSourceGroups.Enable(bState);
	m_cbNewsSourcePages.Enable(bState);
	m_cbNewsSourceFollowing.Enable(bState);
	m_cbNewsSourceIncludeBanned.Enable(bState);
	m_cbNewsSourceNoReposts.Enable(bState);

}

void CVkOptionFeedsForm::On_cbNotificationsEnabledChange(CCtrlCheck*)
{
	bool bState = m_cbNotificationsEnabled.GetState() != 0;

	m_cbNotificationsMarkAsViewed.Enable(bState);
	m_edtNotificationsInterval.Enable(bState);
	m_spNotificationsInterval.Enable(bState);
	m_cbNotificationFilterComments.Enable(bState);
	m_cbNotificationFilterLikes.Enable(bState);
	m_cbNotificationFilterReposts.Enable(bState);
	m_cbNotificationFilterMentions.Enable(bState);
	m_cbNotificationFilterInvites.Enable(bState);
}

////////////////////// View page /////////////////////////////////////////////

CVkOptionViewForm::CVkOptionViewForm(CVkProto *proto):
	CVkDlgBase(proto, IDD_OPT_VIEW, false),
	m_cbIMGBBCSupportOff(this, IDC_IMG_OFF),
	m_cbIMGBBCSupportFullSize(this, IDC_IMG_FULLSIZE),
	m_cbIMGBBCSupport130(this, IDC_IMG_130),
	m_cbIMGBBCSupport604(this, IDC_IMG_604),
	m_cbBBCForNewsOff(this, IDC_NEWSBBC_OFF),
	m_cbBBCForNewsBasic(this, IDC_NEWSBBC_BASIC),
	m_cbBBCForNewsAdvanced(this, IDC_NEWSBBC_ADV),
	m_cbUseBBCOnAttacmentsAsNews(this, IDC_BBC_ATT_NEWS),
	m_cbBBCForAttachmentsOff(this, IDC_ATTBBC_OFF),
	m_cbBBCForAttachmentsBasic(this, IDC_ATTBBC_BASIC),
	m_cbBBCForAttachmentsAdvanced(this, IDC_ATTBBC_ADV),
	m_cbStikersAsSmyles(this, IDC_STICKERS_AS_SMYLES),
	m_cbShortenLinksForAudio(this, IDC_SHOTEN_LINKS_FOR_AUDIO),
	m_proto(proto)
{
	CreateLink(m_cbUseBBCOnAttacmentsAsNews, m_proto->m_vkOptions.bUseBBCOnAttacmentsAsNews);
	CreateLink(m_cbStikersAsSmyles, m_proto->m_vkOptions.bStikersAsSmyles);
	CreateLink(m_cbShortenLinksForAudio, m_proto->m_vkOptions.bShortenLinksForAudio);
}

void CVkOptionViewForm::OnInitDialog()
{
	m_cbIMGBBCSupportOff.SetState(m_proto->m_vkOptions.iIMGBBCSupport == IMGBBCSypport::imgNo);
	m_cbIMGBBCSupportFullSize.SetState(m_proto->m_vkOptions.iIMGBBCSupport == IMGBBCSypport::imgFullSize);
	m_cbIMGBBCSupport130.SetState(m_proto->m_vkOptions.iIMGBBCSupport == IMGBBCSypport::imgPreview130);
	m_cbIMGBBCSupport604.SetState(m_proto->m_vkOptions.iIMGBBCSupport == IMGBBCSypport::imgPreview604);

	m_cbBBCForNewsOff.SetState(m_proto->m_vkOptions.iBBCForNews == BBCSupport::bbcNo);
	m_cbBBCForNewsBasic.SetState(m_proto->m_vkOptions.iBBCForNews == BBCSupport::bbcBasic);
	m_cbBBCForNewsAdvanced.SetState(m_proto->m_vkOptions.iBBCForNews == BBCSupport::bbcAdvanced);
	
	m_cbBBCForAttachmentsOff.SetState(m_proto->m_vkOptions.iBBCForAttachments == BBCSupport::bbcNo);
	m_cbBBCForAttachmentsBasic.SetState(m_proto->m_vkOptions.iBBCForAttachments == BBCSupport::bbcBasic);
	m_cbBBCForAttachmentsAdvanced.SetState(m_proto->m_vkOptions.iBBCForAttachments == BBCSupport::bbcAdvanced);
}

void CVkOptionViewForm::OnApply()
{
	if (m_cbIMGBBCSupportOff.GetState())
		m_proto->m_vkOptions.iIMGBBCSupport = IMGBBCSypport::imgNo;
	if (m_cbIMGBBCSupportFullSize.GetState())
		m_proto->m_vkOptions.iIMGBBCSupport = IMGBBCSypport::imgFullSize;
	if (m_cbIMGBBCSupport130.GetState())
		m_proto->m_vkOptions.iIMGBBCSupport = IMGBBCSypport::imgPreview130;
	if (m_cbIMGBBCSupport604.GetState())
		m_proto->m_vkOptions.iIMGBBCSupport = IMGBBCSypport::imgPreview604;

	if (m_cbBBCForNewsOff.GetState())
		m_proto->m_vkOptions.iBBCForNews = BBCSupport::bbcNo;
	if (m_cbBBCForNewsBasic.GetState())
		m_proto->m_vkOptions.iBBCForNews = BBCSupport::bbcBasic;
	if (m_cbBBCForNewsAdvanced.GetState())
		m_proto->m_vkOptions.iBBCForNews = BBCSupport::bbcAdvanced;

	if (m_cbBBCForAttachmentsOff.GetState())
		m_proto->m_vkOptions.iBBCForAttachments = BBCSupport::bbcNo;
	if (m_cbBBCForAttachmentsBasic.GetState())
		m_proto->m_vkOptions.iBBCForAttachments = BBCSupport::bbcBasic;
	if (m_cbBBCForAttachmentsAdvanced.GetState())
		m_proto->m_vkOptions.iBBCForAttachments = BBCSupport::bbcAdvanced;
}

////////////////////// Menu page /////////////////////////////////////////////

CVkOptionMenuForm::CVkOptionMenuForm(CVkProto *proto):
	CVkDlgBase(proto, IDD_OPT_MENU, false),
	m_cbMenuEnabled0(this, IDC_SHOW_MENU0),
	m_cbMenuEnabled1(this, IDC_SHOW_MENU1),
	m_cbMenuEnabled2(this, IDC_SHOW_MENU2),
	m_cbMenuEnabled3(this, IDC_SHOW_MENU3),
	m_cbMenuEnabled4(this, IDC_SHOW_MENU4),
	m_cbMenuEnabled5(this, IDC_SHOW_MENU5),
	m_cbMenuEnabled6(this, IDC_SHOW_MENU6),
	m_proto(proto)
{
	CreateLink(m_cbMenuEnabled0, m_proto->m_vkOptions.bShowProtoMenuItem0);
	CreateLink(m_cbMenuEnabled1, m_proto->m_vkOptions.bShowProtoMenuItem1);
	CreateLink(m_cbMenuEnabled2, m_proto->m_vkOptions.bShowProtoMenuItem2);
	CreateLink(m_cbMenuEnabled3, m_proto->m_vkOptions.bShowProtoMenuItem3);
	CreateLink(m_cbMenuEnabled4, m_proto->m_vkOptions.bShowProtoMenuItem4);
	CreateLink(m_cbMenuEnabled5, m_proto->m_vkOptions.bShowProtoMenuItem5);
	CreateLink(m_cbMenuEnabled6, m_proto->m_vkOptions.bShowProtoMenuItem6);
}

void CVkOptionMenuForm::OnApply()
{
	if (MessageBox(NULL, 
		TranslateT("These changes will take effect after Miranda NG restart.\nWould you like to restart it now?"),
		TranslateT("VKontakte protocol"), MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2) == IDYES) 
		CallServiceSync(MS_SYSTEM_RESTART, 1,  0);
}


