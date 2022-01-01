/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

Created by Pescuma
Modified by FYR
*/

/////////////////////////////////////////////////////////////////////////////////////////
// Module for working with lines text and avatars

#include "stdafx.h"
#include "modern_sync.h"

typedef BOOL(*ExecuteOnAllContactsFuncPtr) (ClcContact *contact, BOOL subcontact, void *param);

/////////////////////////////////////////////////////////////////////////////////////////
// Module static declarations

static int CopySkipUnprintableChars(wchar_t *to, wchar_t * buf, uint32_t size);

static BOOL ExecuteOnAllContacts(ClcData *dat, ExecuteOnAllContactsFuncPtr func, void *param);
static BOOL ExecuteOnAllContactsOfGroup(ClcGroup *group, ExecuteOnAllContactsFuncPtr func, void *param);

/////////////////////////////////////////////////////////////////////////////////////////
// Get time zone for contact
//
void Cache_GetTimezone(ClcData *dat, MCONTACT hContact)
{
	ClcCacheEntry *pdnce = Clist_GetCacheEntry(hContact);
	if (dat == nullptr && g_clistApi.hwndContactTree)
		dat = (ClcData *)GetWindowLongPtr(g_clistApi.hwndContactTree, 0);

	if (dat && dat->hWnd == g_clistApi.hwndContactTree) {
		uint32_t flags = dat->contact_time_show_only_if_different ? TZF_DIFONLY : 0;
		pdnce->hTimeZone = TimeZone_CreateByContact(hContact, nullptr, flags);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get all lines of text
//
void Cache_GetText(ClcData *dat, ClcContact *contact)
{
	Cache_GetFirstLineText(dat, contact);

	if (!dat->bForceInDialog) {
		if (g_plugin.secondLine.bActive)
			Cache_GetNthLineText(dat, contact->pce, 2);
		if (g_plugin.thirdLine.bActive)
			Cache_GetNthLineText(dat, contact->pce, 3);
	}
}

void CSmileyString::AddListeningToIcon(ClcData *dat, wchar_t *szText)
{
	iMaxSmileyHeight = 0;
	DestroySmileyList();

	if (szText == nullptr)
		return;

	int text_size = (int)mir_wstrlen(szText);

	plText = List_Create(0, 1);

	// Add Icon
	{
		ClcContactTextPiece *piece = (ClcContactTextPiece *)mir_alloc(sizeof(ClcContactTextPiece));
		piece->type = TEXT_PIECE_TYPE_SMILEY;
		piece->len = 0;
		piece->smiley = g_hListeningToIcon;
		piece->smiley_width = 16;
		piece->smiley_height = 16;

		ICONINFO icon;
		if (GetIconInfo(piece->smiley, &icon)) {
			BITMAP bm;
			if (GetObject(icon.hbmColor, sizeof(BITMAP), &bm)) {
				piece->smiley_width = bm.bmWidth;
				piece->smiley_height = bm.bmHeight;
			}

			DeleteObject(icon.hbmMask);
			DeleteObject(icon.hbmColor);
		}

		dat->text_smiley_height = max(piece->smiley_height, dat->text_smiley_height);
		iMaxSmileyHeight = max(piece->smiley_height, iMaxSmileyHeight);

		List_Insert(plText, piece, plText->realCount);
	}

	// Add text
	{
		ClcContactTextPiece *piece = (ClcContactTextPiece *)mir_alloc(sizeof(ClcContactTextPiece));
		piece->type = TEXT_PIECE_TYPE_TEXT;
		piece->start_pos = 0;
		piece->len = text_size;
		List_Insert(plText, piece, plText->realCount);
	}
}

void CSmileyString::_CopySmileyList(SortedList *plInput)
{
	if (!plInput || plInput->realCount == 0)
		return;
	
	plText = List_Create(0, 1);
	for (int i = 0; i < plInput->realCount; i++) {
		ClcContactTextPiece *pieceFrom = (ClcContactTextPiece *)plInput->items[i];
		if (pieceFrom != nullptr) {
			ClcContactTextPiece *piece = (ClcContactTextPiece *)mir_alloc(sizeof(ClcContactTextPiece));
			*piece = *pieceFrom;
			if (pieceFrom->type == TEXT_PIECE_TYPE_SMILEY)
				piece->smiley = CopyIcon(pieceFrom->smiley);
			List_Insert(plText, piece, plText->realCount);
		}
	}
}

void CSmileyString::DestroySmileyList()
{
	if (plText == nullptr)
		return;

	if (IsBadReadPtr(plText, sizeof(SortedList))) {
		plText = nullptr;
		return;
	}

	if (plText->realCount != 0) {
		for (int i = 0; i < plText->realCount; i++) {
			if (plText->items[i] != nullptr) {
				ClcContactTextPiece *piece = (ClcContactTextPiece *)plText->items[i];

				if (!IsBadWritePtr(piece, sizeof(ClcContactTextPiece))) {
					if (piece->type == TEXT_PIECE_TYPE_SMILEY && piece->smiley != g_hListeningToIcon)
						DestroyIcon_protect(piece->smiley);
					mir_free(piece);
				}
			}
		}
		List_Destroy(plText);
	}
	mir_free(plText);

	plText = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Parsing of text for smiley
//
void CSmileyString::ReplaceSmileys(ClcData *dat, ClcCacheEntry *pdnce, wchar_t *szText, BOOL replace_smileys)
{
	int last_pos = 0;
	iMaxSmileyHeight = 0;

	DestroySmileyList();

	if (!dat->text_replace_smileys || !replace_smileys || szText == nullptr)
		return;

	int text_size = (int)mir_wstrlen(szText);

	// Call service for the first time to see if needs to be used...
	SMADD_BATCHPARSE2 sp = {};
	sp.cbSize = sizeof(sp);
	sp.hContact = pdnce->hContact;

	if (dat->text_use_protocol_smileys) {
		sp.Protocolname = pdnce->szProto;

		if (db_get_b(0, "CLC", "Meta", SETTING_USEMETAICON_DEFAULT) != 1 && pdnce->szProto != nullptr && mir_strcmp(pdnce->szProto, META_PROTO) == 0) {
			MCONTACT hContact = db_mc_getMostOnline(pdnce->hContact);
			if (hContact != 0)
				sp.Protocolname = Proto_GetBaseAccountName(hContact);
		}
	}
	else sp.Protocolname = "clist";

	sp.str = szText;
	sp.flag = SAFL_TCHAR;

	SMADD_BATCHPARSERES *spr = (SMADD_BATCHPARSERES*)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);

	// Did not find a simley
	if (spr == nullptr || (INT_PTR)spr == CALLSERVICE_NOTFOUND)
		return;

	// Lets add smileys
	plText = List_Create(0, 1);

	for (unsigned i = 0; i < sp.numSmileys; ++i) {
		if (spr[i].hIcon != nullptr) { // For deffective smileypacks
			// Add text
			if (spr[i].startChar - last_pos > 0) {
				ClcContactTextPiece *piece = (ClcContactTextPiece *)mir_alloc(sizeof(ClcContactTextPiece));

				piece->type = TEXT_PIECE_TYPE_TEXT;
				piece->start_pos = last_pos;//sp.str - text;
				piece->len = spr[i].startChar - last_pos;
				List_Insert(plText, piece, plText->realCount);
			}

			// Add smiley
			{
				BITMAP bm;
				ICONINFO icon;
				ClcContactTextPiece *piece = (ClcContactTextPiece *)mir_alloc(sizeof(ClcContactTextPiece));

				piece->type = TEXT_PIECE_TYPE_SMILEY;
				piece->len = spr[i].size;
				piece->smiley = spr[i].hIcon;

				piece->smiley_width = 16;
				piece->smiley_height = 16;
				if (GetIconInfo(piece->smiley, &icon)) {
					if (GetObject(icon.hbmColor, sizeof(BITMAP), &bm)) {
						piece->smiley_width = bm.bmWidth;
						piece->smiley_height = bm.bmHeight;
					}

					DeleteObject(icon.hbmMask);
					DeleteObject(icon.hbmColor);
				}

				dat->text_smiley_height = max(piece->smiley_height, dat->text_smiley_height);
				iMaxSmileyHeight = max(piece->smiley_height, iMaxSmileyHeight);

				List_Insert(plText, piece, plText->realCount);
			}
		}
		// Get next
		last_pos = spr[i].startChar + spr[i].size;
	}
	CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)spr);

	// Add rest of text
	if (last_pos < text_size) {
		ClcContactTextPiece *piece = (ClcContactTextPiece *)mir_alloc(sizeof(ClcContactTextPiece));

		piece->type = TEXT_PIECE_TYPE_TEXT;
		piece->start_pos = last_pos;
		piece->len = text_size - last_pos;

		List_Insert(plText, piece, plText->realCount);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Getting Status name
// returns -1 for XStatus, 1 for Status
//
int GetStatusName(wchar_t *text, int text_size, ClcCacheEntry *pdnce, BOOL bXstatusHasPriority)
{
	BOOL noAwayMsg = FALSE;
	BOOL noXstatus = FALSE;
	// Hide status text if Offline  /// no offline
	uint16_t nStatus = pdnce->getStatus();
	if ((nStatus == ID_STATUS_OFFLINE || nStatus == 0) && g_CluiData.bRemoveAwayMessageForOffline) noAwayMsg = TRUE;
	if (nStatus == ID_STATUS_OFFLINE || nStatus == 0) noXstatus = TRUE;
	text[0] = '\0';
	// Get XStatusName
	if (!noAwayMsg && !noXstatus &&  bXstatusHasPriority && pdnce->hContact && pdnce->szProto) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ws(pdnce->hContact, pdnce->szProto, "XStatusName", &dbv)) {
			CopySkipUnprintableChars(text, dbv.pwszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return -1;
		}
	}

	// Get Status name
	wchar_t *tmp = Clist_GetStatusModeDescription(nStatus, 0);
	if (tmp && *tmp) {
		mir_wstrncpy(text, tmp, text_size);
		return 1;
	}

	// Get XStatusName
	if (!noAwayMsg && !noXstatus && !bXstatusHasPriority && pdnce->hContact && pdnce->szProto) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ws(pdnce->hContact, pdnce->szProto, "XStatusName", &dbv)) {
			CopySkipUnprintableChars(text, dbv.pwszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return -1;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get Listening to information
//
void GetListeningTo(wchar_t *text, int text_size, ClcCacheEntry *pdnce)
{
	*text = '\0';

	if (pdnce->m_iStatus == ID_STATUS_OFFLINE || pdnce->m_iStatus == 0)
		return;

	ptrW tszValue(db_get_wsa(pdnce->hContact, pdnce->szProto, "ListeningTo"));
	if (tszValue != nullptr)
		CopySkipUnprintableChars(text, tszValue, text_size - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Getting Status message(Away message)
// returns -1 for XStatus, 1 for Status
//
int GetStatusMessage(wchar_t *text, int text_size, ClcCacheEntry *pdnce, BOOL bXstatusHasPriority)
{
	BOOL noAwayMsg = FALSE;
	uint16_t wStatus = pdnce->getStatus();
	*text = '\0';

	// Hide status text if Offline  /// no offline
	if (wStatus == ID_STATUS_OFFLINE || wStatus == 0)
		noAwayMsg = TRUE;

	// Get XStatusMsg
	if (!noAwayMsg  && bXstatusHasPriority && pdnce->hContact && pdnce->szProto) {
		ptrW tszXStatusMsg(db_get_wsa(pdnce->hContact, pdnce->szProto, "XStatusMsg"));
		if (tszXStatusMsg != nullptr) {
			CopySkipUnprintableChars(text, tszXStatusMsg, text_size - 1);
			if (text[0] != '\0')
				return -1;
		}
	}

	// Get StatusMsg
	if (pdnce->hContact && text[0] == '\0') {
		if (noAwayMsg && ServiceExists(MS_LASTSEEN_GET)) {
			ptrW pwszLastSeen((LPWSTR)CallService(MS_LASTSEEN_GET, (WPARAM)pdnce->hContact));
			if (pwszLastSeen) {
				CMStringW wszLastSeen(FORMAT, L"%s: %s", TranslateT("Last seen"),  pwszLastSeen);
				CopySkipUnprintableChars(text, (wchar_t*)wszLastSeen.c_str(), text_size - 1);
				if (text[0] != '\0')
					return 1;
			}
		}

		ptrW tszStatusMsg(g_plugin.getWStringA(pdnce->hContact, "StatusMsg"));
		if (tszStatusMsg != nullptr) {
			CopySkipUnprintableChars(text, tszStatusMsg, text_size - 1);
			if (text[0] != '\0')
				return 1;
		}

	}

	// Get XStatusMsg
	if (!noAwayMsg && !bXstatusHasPriority && pdnce->hContact && pdnce->szProto && text[0] == '\0') {
		// Try to get XStatusMsg
		ptrW tszXStatusMsg(db_get_wsa(pdnce->hContact, pdnce->szProto, "XStatusMsg"));
		if (tszXStatusMsg != nullptr) {
			CopySkipUnprintableChars(text, tszXStatusMsg, text_size - 1);
			if (text[0] != '\0')
				return -1;
		}
	}

	return 1;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	Get the text for specified lines

int Cache_GetLineText(ClcCacheEntry *pdnce, int type, LPTSTR text, int text_size, ClcLineInfo &line)
{
	if (text == nullptr)
		return TEXT_EMPTY;
	text[0] = '\0';

	switch (type) {
	case TEXT_STATUS:
LBL_Status:
		if (GetStatusName(text, text_size, pdnce, line.bXstatusHasPriority) == -1 && line.bUseNameAndMessageForXstatus) {
			// Try to get XStatusMsg
			ptrW tszXStatusMsg(db_get_wsa(pdnce->hContact, pdnce->szProto, "XStatusMsg"));
			if (tszXStatusMsg != nullptr && tszXStatusMsg[0] != 0) {
				wchar_t *tmp = NEWWSTR_ALLOCA(text);
				mir_snwprintf(text, text_size, L"%s: %s", tmp, tszXStatusMsg.get());
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}
		return TEXT_STATUS;

	case TEXT_NICKNAME:
		if (pdnce->hContact && pdnce->szProto) {
			ptrW tszNick(db_get_wsa(pdnce->hContact, pdnce->szProto, "Nick"));
			if (tszNick != nullptr) {
				mir_wstrncpy(text, tszNick, text_size);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}
		return TEXT_NICKNAME;

	case TEXT_STATUS_MESSAGE:
		if (GetStatusMessage(text, text_size, pdnce, line.bXstatusHasPriority) == -1 && line.bUseNameAndMessageForXstatus) {
			// Try to get XStatusName
			ptrW tszXStatusName(db_get_wsa(pdnce->hContact, pdnce->szProto, "XStatusName"));
			if (tszXStatusName != nullptr && tszXStatusName[0] != 0) {
				wchar_t *tmp = NEWWSTR_ALLOCA(text);
				mir_snwprintf(text, text_size, L"%s: %s", tszXStatusName.get(), tmp);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}
		else if (line.bUseNameAndMessageForXstatus && line.bXstatusHasPriority) {
			// Try to get XStatusName
			ptrW tszXStatusName(db_get_wsa(pdnce->hContact, pdnce->szProto, "XStatusName"));
			if (tszXStatusName != nullptr && tszXStatusName[0] != 0) {
				mir_wstrncpy(text, tszXStatusName, text_size);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}

		if (text[0] == '\0') {
			if (line.bShowListeningIfNoAway) {
				GetListeningTo(text, text_size, pdnce);
				if (text[0] != '\0')
					return TEXT_LISTENING_TO;
			}

			if (line.bShowStatusIfNoAway) // re-request status if no away
				goto LBL_Status;
		}
		return TEXT_STATUS_MESSAGE;

	case TEXT_LISTENING_TO:
		GetListeningTo(text, text_size, pdnce);
		return TEXT_LISTENING_TO;

	case TEXT_TEXT:
		{
			ptrW tmp(variables_parsedup(line.text, pdnce->tszName, pdnce->hContact));
			mir_wstrncpy(text, tmp, text_size);
			CopySkipUnprintableChars(text, text, text_size - 1);
		}
		return TEXT_TEXT;

	case TEXT_CONTACT_TIME:
		if (pdnce->hTimeZone) {
			// Get pdnce time
			text[0] = 0;
			TimeZone_PrintDateTime(pdnce->hTimeZone, L"t", text, text_size, 0);
		}
		return TEXT_CONTACT_TIME;
	}

	return TEXT_EMPTY;
}

/////////////////////////////////////////////////////////////////////////////////////////
//	Get the text for First Line

void Cache_GetFirstLineText(ClcData *dat, ClcContact *contact)
{
	if (GetCurrentThreadId() != g_dwMainThreadID)
		return;

	ClcCacheEntry *pdnce = contact->pce;
	wchar_t *name = Clist_GetContactDisplayName(contact->hContact);
	if (dat->first_line_append_nick && !dat->bForceInDialog) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ws(pdnce->hContact, pdnce->szProto, "Nick", &dbv)) {
			wchar_t nick[_countof(contact->szText)];
			wcsncpy_s(nick, dbv.pwszVal, _TRUNCATE);
			db_free(&dbv);

			// They are the same -> use the name to keep the case
			if (mir_wstrcmpi(name, nick) == 0)
				wcsncpy_s(contact->szText, name, _TRUNCATE);
			else // Append then
				mir_snwprintf(contact->szText, L"%s - %s", name, nick);
		}
		else wcsncpy_s(contact->szText, name, _TRUNCATE);
	}
	else wcsncpy_s(contact->szText, name, _TRUNCATE);

	if (!dat->bForceInDialog)
		contact->ssText.ReplaceSmileys(dat, pdnce, contact->szText, dat->first_line_draw_smileys);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get the text for Second Line

void Cache_GetNthLineText(ClcData *dat, ClcCacheEntry *pdnce, int n)
{
	if (pdnce == nullptr)
		return;

	wchar_t Text[240 - EXTRA_ICON_COUNT]; Text[0] = 0;
	ClcLineInfo &line = (n == 2) ? g_plugin.secondLine : g_plugin.thirdLine;
	wchar_t* &szText = (n == 2) ? pdnce->szSecondLineText : pdnce->szThirdLineText;

	// in most cases replaceStrW does nothing
	if (!line.bActive) {
		replaceStrW(szText, nullptr);
		return;
	}
	
	int type = Cache_GetLineText(pdnce, line.iType, Text, _countof(Text), line);
	if (Text[0] == 0) {
		replaceStrW(szText, nullptr);
		return;
	}
	
	Text[_countof(Text) - 1] = 0; //to be sure that it is null terminated string
	replaceStrW(szText, Text);

	CSmileyString &ss = (n == 2) ? pdnce->ssSecondLine : pdnce->ssThirdLine;
	if (type == TEXT_LISTENING_TO && szText[0] != '\0')
		ss.AddListeningToIcon(dat, szText);
	else
		ss.ReplaceSmileys(dat, pdnce, szText, line.bDrawSmilies);
}

/////////////////////////////////////////////////////////////////////////////////////////

void RemoveTag(wchar_t *to, wchar_t *tag)
{
	wchar_t *st = to;
	int len = (int)mir_wstrlen(tag);
	int lastsize = (int)mir_wstrlen(to) + 1;
	while (st = wcsstr(st, tag)) {
		lastsize -= len;
		memmove((void*)st, (void*)(st + len), (lastsize)*sizeof(wchar_t));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Copy string with removing Escape chars from text and BBcodes

static int CopySkipUnprintableChars(wchar_t *to, wchar_t * buf, uint32_t size)
{
	uint32_t i;
	BOOL keep = 0;
	wchar_t * cp = to;
	if (!to) return 0;
	if (!buf) {
		to[0] = '\0';
		return 0;
	}

	for (i = 0; i < size; i++) {
		if (buf[i] == 0) break;
		if (buf[i] > 0 && buf[i] < ' ') {
			*cp = ' ';
			if (!keep) cp++;
			keep = 1;
		}
		else {
			keep = 0;
			*cp = buf[i];
			cp++;
		}
	}
	*cp = 0;

	//remove bbcodes: [b] [i] [u]  <b> <i> <u>
	RemoveTag(to, L"[b]"); RemoveTag(to, L"[/b]");
	RemoveTag(to, L"[u]"); RemoveTag(to, L"[/u]");
	RemoveTag(to, L"[i]"); RemoveTag(to, L"[/i]");

	RemoveTag(to, L"<b>"); RemoveTag(to, L"</b>");
	RemoveTag(to, L"<u>"); RemoveTag(to, L"</u>");
	RemoveTag(to, L"<i>"); RemoveTag(to, L"</i>");

	RemoveTag(to, L"[B]"); RemoveTag(to, L"[/b]");
	RemoveTag(to, L"[U]"); RemoveTag(to, L"[/u]");
	RemoveTag(to, L"[I]"); RemoveTag(to, L"[/i]");

	RemoveTag(to, L"<B>"); RemoveTag(to, L"</B>");
	RemoveTag(to, L"<U>"); RemoveTag(to, L"</U>");
	RemoveTag(to, L"<I>"); RemoveTag(to, L"</I>");
	return i;
}

/////////////////////////////////////////////////////////////////////////////////////////
// If ExecuteOnAllContactsFuncPtr returns FALSE, stop loop
// Return TRUE if finished, FALSE if was stoped
//
static BOOL ExecuteOnAllContacts(ClcData *dat, ExecuteOnAllContactsFuncPtr func, void *param)
{
	return ExecuteOnAllContactsOfGroup(&dat->list, func, param);
}

static BOOL ExecuteOnAllContactsOfGroup(ClcGroup *group, ExecuteOnAllContactsFuncPtr func, void *param)
{
	if (!group)
		return TRUE;

	for (auto &it : group->cl) {
		if (it->type == CLCIT_CONTACT) {
			if (!func(it, FALSE, param))
				return FALSE;

			if (it->iSubAllocated > 0) {
				for (int i = 0; i < it->iSubAllocated; i++)
					if (!func(&it->subcontacts[i], TRUE, param))
						return FALSE;
			}
		}
		else if (it->type == CLCIT_GROUP)
			if (!ExecuteOnAllContactsOfGroup(it->group, func, param))
				return FALSE;
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Avatar working routines
//
BOOL UpdateAllAvatarsProxy(ClcContact *contact, BOOL, void *param)
{
	Cache_GetAvatar((ClcData *)param, contact);
	return TRUE;
}

void UpdateAllAvatars(ClcData *dat)
{
	ExecuteOnAllContacts(dat, UpdateAllAvatarsProxy, dat);
}

BOOL ReduceAvatarPosition(ClcContact *contact, BOOL, void *param)
{
	if (contact->avatar_pos >= *((int *)param))
		contact->avatar_pos--;

	return TRUE;
}

void Cache_ProceedAvatarInList(ClcData *dat, ClcContact *contact)
{
	AVATARCACHEENTRY *ace = contact->avatar_data;
	int old_pos = contact->avatar_pos;

	if (ace == nullptr || ace->dwFlags == AVS_BITMAP_EXPIRED || ace->hbmPic == nullptr) {
		// Avatar was not ready or removed - need to remove it from cache
		if (old_pos >= 0) {
			ImageArray_RemoveImage(&dat->avatar_cache, old_pos);

			// Update all items
			ExecuteOnAllContacts(dat, ReduceAvatarPosition, (void *)&old_pos);
			contact->avatar_pos = AVATAR_POS_DONT_HAVE;
			return;
		}
	}
	else if (contact->avatar_data->hbmPic != nullptr) { // let's add it
		// Clipping width and height
		LONG width_clip = dat->avatars_maxwidth_size ? dat->avatars_maxwidth_size : dat->avatars_maxheight_size;
		LONG height_clip = dat->avatars_maxheight_size;

		if (height_clip * ace->bmWidth / ace->bmHeight <= width_clip)
			width_clip = height_clip * ace->bmWidth / ace->bmHeight;
		else
			height_clip = width_clip * ace->bmHeight / ace->bmWidth;

		if (wildcmpiw(contact->avatar_data->szFilename, L"*.gif")) {
			if (old_pos == AVATAR_POS_ANIMATED)
				AniAva_RemoveAvatar(contact->hContact);

			int res = AniAva_AddAvatar(contact->hContact, contact->avatar_data->szFilename, width_clip, height_clip);
			if (res) {
				contact->avatar_pos = AVATAR_POS_ANIMATED;
				contact->avatar_size.cy = HIWORD(res);
				contact->avatar_size.cx = LOWORD(res);
				return;
			}
		}

		// Create objs
		HDC hdc = CreateCompatibleDC(dat->avatar_cache.hdc);

		void *pt;
		HBITMAP hDrawBmp = ske_CreateDIB32Point(width_clip, height_clip, &pt);
		HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hDrawBmp);

		// need to draw avatar bitmap here
		DrawAvatarImageWithGDIp(hdc, 0, 0, width_clip, height_clip, ace->hbmPic, 0, 0, ace->bmWidth, ace->bmHeight, ace->dwFlags, 255);
		SelectObject(hdc, oldBmp);
		DeleteDC(hdc);

		// Add to list
		if (old_pos >= 0) {
			ImageArray_ChangeImage(&dat->avatar_cache, hDrawBmp, old_pos);
			contact->avatar_pos = old_pos;
		}
		else contact->avatar_pos = ImageArray_AddImage(&dat->avatar_cache, hDrawBmp, -1);

		if (old_pos == AVATAR_POS_ANIMATED && contact->avatar_pos != AVATAR_POS_ANIMATED)
			AniAva_RemoveAvatar(contact->hContact);

		DeleteObject(hDrawBmp);
	}
}

void Cache_GetAvatar(ClcData *dat, ClcContact *contact)
{
	// workaround for avatar service
	if (g_CluiData.bSTATE != STATE_NORMAL) {
		contact->avatar_pos = AVATAR_POS_DONT_HAVE;
		contact->avatar_data = nullptr;
		return;
	}

	if (dat->avatars_show && !g_plugin.getByte(contact->hContact, "HideContactAvatar", 0)) {
		contact->avatar_data = (AVATARCACHEENTRY*)CallService(MS_AV_GETAVATARBITMAP, contact->hContact, 0);
		if (contact->avatar_data == nullptr || contact->avatar_data->dwFlags == AVS_BITMAP_EXPIRED)
			contact->avatar_data = nullptr;

		if (contact->avatar_data != nullptr)
			contact->avatar_data->t_lastAccess = (uint32_t)time(0);
	}
	else contact->avatar_data = nullptr;

	Cache_ProceedAvatarInList(dat, contact);
}
