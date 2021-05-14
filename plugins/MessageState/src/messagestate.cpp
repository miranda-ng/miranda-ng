#include "stdafx.h"

static HANDLE hExtraIcon = nullptr;

int ExtraIconsApply(WPARAM hContact, LPARAM force)
{
	if (hContact != 0) {
		if (HasUnread(hContact) || force)
			ExtraIcon_SetIcon(hExtraIcon, hContact, g_plugin.getIconHandle(IDI_EXTRA));
		else
			ExtraIcon_Clear(hExtraIcon, hContact);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

void SetSRMMIcon(MCONTACT hContact, int iconId, time_t time)
{
	auto *p = FindContact(hContact);

	const wchar_t *pwszToolTip;
	switch (iconId) {
	case IDI_READ:
		// if that contact was never marked as read
		if (p->type != -1) { 
			wchar_t buf[100];
			wcsftime(buf, _countof(buf), TranslateT("Last message read at %X %x"), localtime(&time));
			pwszToolTip = buf;
		}
		else pwszToolTip = TranslateT("Last message read");
		break;

	case IDI_UNREAD: pwszToolTip = TranslateT("Last message is not read"); break;
	case IDI_FAIL:   pwszToolTip = TranslateT("Last message was not sent"); break;
	case IDI_NOSENT: pwszToolTip = TranslateT("Sending..."); break;
	default:         pwszToolTip = nullptr;
	}

	Srmm_ModifyIcon(hContact, MODULENAME, 1, g_plugin.getIcon(iconId), pwszToolTip);
}

void IconsUpdate(MCONTACT hContact)
{
	auto *p = FindContact(hContact);

	// if we've did nothing with this contact, leave its icon hidden
	if (p->type == -1)
		return;

	// if that's the first time we show an icon, unhide it first
	if (p->bHidden) {
		p->bHidden = false;
		Srmm_SetIconFlags(hContact, MODULENAME, 1, 0);
	}

	SetSRMMIcon(hContact, p->type == MRD_TYPE_DELIVERED ? IDI_UNREAD : IDI_READ, p->dwLastReadTime);

	ExtraIconsApply(hContact, 0);

	if (db_mc_isSub(hContact))
		IconsUpdate(db_mc_getMeta(hContact));
}

static int OnProtoAck(WPARAM, LPARAM lParam)
{
	ACKDATA *pAck = (ACKDATA *)lParam;
	if (pAck && (pAck->type == ACKTYPE_MESSAGE || pAck->type == ACKTYPE_FILE) && CheckProtoSupport(pAck->szModule)) {
		if (pAck->result == ACKRESULT_SUCCESS)
			SetSRMMIcon(pAck->hContact, IDI_NOSENT);
		else if (pAck->result == ACKRESULT_FAILED)
			SetSRMMIcon(pAck->hContact, IDI_FAIL);

		ExtraIconsApply(pAck->hContact, 0);
	}
	return 0;
}

static int OnEventFilterAdd(WPARAM hContact, LPARAM lParam)
{
	DBEVENTINFO *dbei = (DBEVENTINFO *)lParam;
	if ((dbei->flags & DBEF_SENT) && CheckProtoSupport(dbei->szModule)) {
		time_t dwTime = time(0);
		FindContact(hContact)->setSent(dwTime);
		if (db_mc_isSub(hContact))
			FindContact(db_mc_getMeta(hContact))->setSent(dwTime);

		SetSRMMIcon(hContact, IDI_NOSENT);
	}
	return 0;
}

static int OnSrmmWindowOpened(WPARAM, LPARAM lParam)
{
	auto *pEvent = (MessageWindowEventData*)lParam;
	if (pEvent->uType == MSG_WINDOW_EVT_OPENING)
		IconsUpdate(pEvent->hContact);
	return 0;
}

static int OnMetaChanged(WPARAM hContact, LPARAM)
{
	IconsUpdate(hContact);
	return 0;
}

int OnModulesLoaded(WPARAM, LPARAM)
{
	hExtraIcon = ExtraIcon_RegisterIcolib("messagestate_unread", LPGEN("MessageState unread extra icon"), "clist_unread_icon");

	HookEvent(ME_PROTO_ACK, OnProtoAck);
	HookEvent(ME_DB_EVENT_FILTER_ADD, OnEventFilterAdd);
	HookEvent(ME_MC_DEFAULTTCHANGED, OnMetaChanged);
	HookEvent(ME_MC_SUBCONTACTSCHANGED, OnMetaChanged);
	HookEvent(ME_MSG_WINDOWEVENT, OnSrmmWindowOpened);
	HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY, ExtraIconsApply);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.flags = MBF_HIDDEN;
	sid.dwId = 1;
	Srmm_AddIcon(&sid, &g_plugin);
	return 0;
}
