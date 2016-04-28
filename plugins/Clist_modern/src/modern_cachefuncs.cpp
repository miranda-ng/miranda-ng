/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-16 Miranda NG project (http://miranda-ng.org),
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
#include "modern_cache_funcs.h"
#include "newpluginapi.h"
#include "modern_sync.h"

typedef BOOL(*ExecuteOnAllContactsFuncPtr) (ClcContact *contact, BOOL subcontact, void *param);

/////////////////////////////////////////////////////////////////////////////////////////
// Module static declarations

static int CopySkipUnprintableChars(TCHAR *to, TCHAR * buf, DWORD size);

static BOOL ExecuteOnAllContacts(ClcData *dat, ExecuteOnAllContactsFuncPtr func, void *param);
static BOOL ExecuteOnAllContactsOfGroup(ClcGroup *group, ExecuteOnAllContactsFuncPtr func, void *param);

/////////////////////////////////////////////////////////////////////////////////////////
// Get time zone for contact
//
void Cache_GetTimezone(ClcData *dat, MCONTACT hContact)
{
	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(hContact);
	if (dat == NULL && pcli->hwndContactTree)
		dat = (ClcData *)GetWindowLongPtr(pcli->hwndContactTree, 0);

	if (dat && dat->hWnd == pcli->hwndContactTree) {
		DWORD flags = dat->contact_time_show_only_if_different ? TZF_DIFONLY : 0;
		pdnce->hTimeZone = TimeZone_CreateByContact(hContact, 0, flags);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get all lines of text
//
void Cache_GetText(ClcData *dat, ClcContact *contact)
{
	Cache_GetFirstLineText(dat, contact);

	if (!dat->force_in_dialog) {
		if (dat->secondLine.show)
			Cache_GetNthLineText(dat, contact->pce, 2);
		if (dat->thirdLine.show)
			Cache_GetNthLineText(dat, contact->pce, 3);
	}
}

void CSmileyString::AddListeningToIcon(ClcData *dat, TCHAR *szText)
{
	iMaxSmileyHeight = 0;
	DestroySmileyList();

	if (szText == NULL)
		return;

	int text_size = (int)mir_tstrlen(szText);

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
		if (pieceFrom != NULL) {
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
	if (plText == NULL)
		return;

	if (IsBadReadPtr(plText, sizeof(SortedList))) {
		plText = NULL;
		return;
	}

	if (plText->realCount != 0) {
		for (int i = 0; i < plText->realCount; i++) {
			if (plText->items[i] != NULL) {
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

	plText = NULL;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Parsing of text for smiley
//
void CSmileyString::ReplaceSmileys(ClcData *dat, ClcCacheEntry *pdnce, TCHAR * szText, BOOL replace_smileys)
{
	int last_pos = 0;
	iMaxSmileyHeight = 0;

	DestroySmileyList();

	if (!dat->text_replace_smileys || !replace_smileys || szText == NULL)
		return;

	int text_size = (int)mir_tstrlen(szText);

	// Call service for the first time to see if needs to be used...
	SMADD_BATCHPARSE2 sp = { 0 };
	sp.cbSize = sizeof(sp);

	if (dat->text_use_protocol_smileys) {
		sp.Protocolname = pdnce->m_pszProto;

		if (db_get_b(NULL, "CLC", "Meta", SETTING_USEMETAICON_DEFAULT) != 1 && pdnce->m_pszProto != NULL && mir_strcmp(pdnce->m_pszProto, META_PROTO) == 0) {
			MCONTACT hContact = db_mc_getMostOnline(pdnce->hContact);
			if (hContact != 0)
				sp.Protocolname = GetContactProto(hContact);
		}
	}
	else sp.Protocolname = "clist";

	sp.str = szText;
	sp.flag = SAFL_TCHAR;

	SMADD_BATCHPARSERES *spr = (SMADD_BATCHPARSERES*)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);

	// Did not find a simley
	if (spr == NULL || (INT_PTR)spr == CALLSERVICE_NOTFOUND)
		return;

	// Lets add smileys
	plText = List_Create(0, 1);

	for (unsigned i = 0; i < sp.numSmileys; ++i) {
		if (spr[i].hIcon != NULL) { // For deffective smileypacks
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
int GetStatusName(TCHAR *text, int text_size, ClcCacheEntry *pdnce, BOOL xstatus_has_priority)
{
	BOOL noAwayMsg = FALSE;
	BOOL noXstatus = FALSE;
	// Hide status text if Offline  /// no offline
	WORD nStatus = pdnce->getStatus();
	if ((nStatus == ID_STATUS_OFFLINE || nStatus == 0) && g_CluiData.bRemoveAwayMessageForOffline) noAwayMsg = TRUE;
	if (nStatus == ID_STATUS_OFFLINE || nStatus == 0) noXstatus = TRUE;
	text[0] = '\0';
	// Get XStatusName
	if (!noAwayMsg && !noXstatus &&  xstatus_has_priority && pdnce->hContact && pdnce->m_pszProto) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, pdnce->m_pszProto, "XStatusName", &dbv)) {
			//mir_tstrncpy(text, dbv.pszVal, text_size);
			CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return -1;
		}
	}

	// Get Status name
	TCHAR *tmp = pcli->pfnGetStatusModeDescription(nStatus, 0);
	if (tmp && *tmp) {
		mir_tstrncpy(text, tmp, text_size);
		return 1;
	}

	// Get XStatusName
	if (!noAwayMsg && !noXstatus && !xstatus_has_priority && pdnce->hContact && pdnce->m_pszProto) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, pdnce->m_pszProto, "XStatusName", &dbv)) {
			CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
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
void GetListeningTo(TCHAR *text, int text_size, ClcCacheEntry *pdnce)
{
	*text = _T('\0');

	if (pdnce->m_iStatus == ID_STATUS_OFFLINE || pdnce->m_iStatus == 0)
		return;

	ptrT tszValue(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "ListeningTo"));
	if (tszValue != NULL)
		CopySkipUnprintableChars(text, tszValue, text_size - 1);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Getting Status message(Away message)
// returns -1 for XStatus, 1 for Status
//
int GetStatusMessage(TCHAR *text, int text_size, ClcCacheEntry *pdnce, BOOL xstatus_has_priority)
{
	BOOL noAwayMsg = FALSE;
	WORD wStatus = pdnce->getStatus();
	*text = '\0';

	// Hide status text if Offline  /// no offline
	if (wStatus == ID_STATUS_OFFLINE || wStatus == 0)
		noAwayMsg = TRUE;

	// Get XStatusMsg
	if (!noAwayMsg  && xstatus_has_priority && pdnce->hContact && pdnce->m_pszProto) {
		ptrT tszXStatusMsg(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "XStatusMsg"));
		if (tszXStatusMsg != NULL) {
			CopySkipUnprintableChars(text, tszXStatusMsg, text_size - 1);
			if (text[0] != '\0')
				return -1;
		}
	}

	// Get StatusMsg
	if (pdnce->hContact && text[0] == '\0') {
		ptrT tszStatusMsg(db_get_tsa(pdnce->hContact, "CList", "StatusMsg"));
		if (tszStatusMsg != NULL) {
			CopySkipUnprintableChars(text, tszStatusMsg, text_size - 1);
			if (text[0] != '\0')
				return 1;
		}
	}

	// Get XStatusMsg
	if (!noAwayMsg && !xstatus_has_priority && pdnce->hContact && pdnce->m_pszProto && text[0] == '\0') {
		// Try to get XStatusMsg
		ptrT tszXStatusMsg(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "XStatusMsg"));
		if (tszXStatusMsg != NULL) {
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
	if (text == NULL)
		return TEXT_EMPTY;
	text[0] = '\0';

	switch (type) {
	case TEXT_STATUS:
LBL_Status:
		if (GetStatusName(text, text_size, pdnce, line.xstatus_has_priority) == -1 && line.use_name_and_message_for_xstatus) {
			// Try to get XStatusMsg
			ptrT tszXStatusMsg(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "XStatusMsg"));
			if (tszXStatusMsg != NULL && tszXStatusMsg[0] != 0) {
				TCHAR *tmp = NEWTSTR_ALLOCA(text);
				mir_sntprintf(text, text_size, _T("%s: %s"), tmp, tszXStatusMsg);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}
		return TEXT_STATUS;

	case TEXT_NICKNAME:
		if (pdnce->hContact && pdnce->m_pszProto) {
			ptrT tszNick(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "Nick"));
			if (tszNick != NULL) {
				mir_tstrncpy(text, tszNick, text_size);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}
		return TEXT_NICKNAME;

	case TEXT_STATUS_MESSAGE:
		if (GetStatusMessage(text, text_size, pdnce, line.xstatus_has_priority) == -1 && line.use_name_and_message_for_xstatus) {
			// Try to get XStatusName
			ptrT tszXStatusName(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "XStatusName"));
			if (tszXStatusName != NULL && tszXStatusName[0] != 0) {
				TCHAR *tmp = NEWTSTR_ALLOCA(text);
				mir_sntprintf(text, text_size, _T("%s: %s"), tszXStatusName, tmp);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}
		else if (line.use_name_and_message_for_xstatus && line.xstatus_has_priority) {
			// Try to get XStatusName
			ptrT tszXStatusName(db_get_tsa(pdnce->hContact, pdnce->m_pszProto, "XStatusName"));
			if (tszXStatusName != NULL && tszXStatusName[0] != 0) {
				mir_tstrncpy(text, tszXStatusName, text_size);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}

		if (text[0] == '\0') {
			if (line.show_listening_if_no_away) {
				GetListeningTo(text, text_size, pdnce);
				if (text[0] != '\0')
					return TEXT_LISTENING_TO;
			}

			if (line.show_status_if_no_away) // re-request status if no away
				goto LBL_Status;
		}
		return TEXT_STATUS_MESSAGE;

	case TEXT_LISTENING_TO:
		GetListeningTo(text, text_size, pdnce);
		return TEXT_LISTENING_TO;

	case TEXT_TEXT:
		{
			ptrT tmp(variables_parsedup(line.text, pdnce->tszName, pdnce->hContact));
			mir_tstrncpy(text, tmp, text_size);
			CopySkipUnprintableChars(text, text, text_size - 1);
		}
		return TEXT_TEXT;

	case TEXT_CONTACT_TIME:
		if (pdnce->hTimeZone) {
			// Get pdnce time
			text[0] = 0;
			TimeZone_PrintDateTime(pdnce->hTimeZone, _T("t"), text, text_size, 0);
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
	TCHAR *name = pcli->pfnGetContactDisplayName(contact->hContact, 0);
	if (dat->first_line_append_nick && !dat->force_in_dialog) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, pdnce->m_pszProto, "Nick", &dbv)) {
			TCHAR nick[_countof(contact->szText)];
			mir_tstrncpy(nick, dbv.ptszVal, _countof(contact->szText));
			db_free(&dbv);

			// They are the same -> use the name to keep the case
			if (mir_tstrcmpi(name, nick) == 0)
				mir_tstrncpy(contact->szText, name, _countof(contact->szText));
			else // Append then
				mir_sntprintf(contact->szText, _T("%s - %s"), name, nick);
		}
		else mir_tstrncpy(contact->szText, name, _countof(contact->szText));
	}
	else mir_tstrncpy(contact->szText, name, _countof(contact->szText));

	if (!dat->force_in_dialog)
		contact->ssText.ReplaceSmileys(dat, pdnce, contact->szText, dat->first_line_draw_smileys);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get the text for Second Line

void Cache_GetNthLineText(ClcData *dat, ClcCacheEntry *pdnce, int n)
{
	TCHAR Text[240 - EXTRA_ICON_COUNT]; Text[0] = 0;
	ClcLineInfo &line = (n == 2) ? dat->secondLine : dat->thirdLine;
	TCHAR* &szText = (n == 2) ? pdnce->szSecondLineText : pdnce->szThirdLineText;

	// in most cases replaceStrT does nothing
	if (!line.show) {
		replaceStrT(szText, NULL);
		return;
	}
	
	int type = Cache_GetLineText(pdnce, line.type, Text, _countof(Text), line);
	if (Text[0] == 0) {
		replaceStrT(szText, NULL);
		return;
	}
	
	Text[_countof(Text) - 1] = 0; //to be sure that it is null terminated string
	replaceStrT(szText, Text);

	CSmileyString &ss = (n == 2) ? pdnce->ssSecondLine : pdnce->ssThirdLine;
	if (type == TEXT_LISTENING_TO && szText[0] != _T('\0'))
		ss.AddListeningToIcon(dat, szText);
	else
		ss.ReplaceSmileys(dat, pdnce, szText, line.draw_smileys);
}

/////////////////////////////////////////////////////////////////////////////////////////

void RemoveTag(TCHAR *to, TCHAR *tag)
{
	TCHAR *st = to;
	int len = (int)mir_tstrlen(tag);
	int lastsize = (int)mir_tstrlen(to) + 1;
	while (st = _tcsstr(st, tag)) {
		lastsize -= len;
		memmove((void*)st, (void*)(st + len), (lastsize)*sizeof(TCHAR));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Copy string with removing Escape chars from text and BBcodes

static int CopySkipUnprintableChars(TCHAR *to, TCHAR * buf, DWORD size)
{
	DWORD i;
	BOOL keep = 0;
	TCHAR * cp = to;
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
	RemoveTag(to, _T("[b]")); RemoveTag(to, _T("[/b]"));
	RemoveTag(to, _T("[u]")); RemoveTag(to, _T("[/u]"));
	RemoveTag(to, _T("[i]")); RemoveTag(to, _T("[/i]"));

	RemoveTag(to, _T("<b>")); RemoveTag(to, _T("</b>"));
	RemoveTag(to, _T("<u>")); RemoveTag(to, _T("</u>"));
	RemoveTag(to, _T("<i>")); RemoveTag(to, _T("</i>"));

	RemoveTag(to, _T("[B]")); RemoveTag(to, _T("[/b]"));
	RemoveTag(to, _T("[U]")); RemoveTag(to, _T("[/u]"));
	RemoveTag(to, _T("[I]")); RemoveTag(to, _T("[/i]"));

	RemoveTag(to, _T("<B>")); RemoveTag(to, _T("</B>"));
	RemoveTag(to, _T("<U>")); RemoveTag(to, _T("</U>"));
	RemoveTag(to, _T("<I>")); RemoveTag(to, _T("</I>"));
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

	for (int scanIndex = 0; scanIndex < group->cl.count; scanIndex++) {
		if (group->cl.items[scanIndex]->type == CLCIT_CONTACT) {
			if (!func(group->cl.items[scanIndex], FALSE, param))
				return FALSE;

			if (group->cl.items[scanIndex]->SubAllocated > 0) {
				for (int i = 0; i < group->cl.items[scanIndex]->SubAllocated; i++)
					if (!func(&group->cl.items[scanIndex]->subcontacts[i], TRUE, param))
						return FALSE;
			}
		}
		else if (group->cl.items[scanIndex]->type == CLCIT_GROUP)
			if (!ExecuteOnAllContactsOfGroup(group->cl.items[scanIndex]->group, func, param))
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
	avatarCacheEntry *ace = contact->avatar_data;
	int old_pos = contact->avatar_pos;

	if (ace == NULL || ace->dwFlags == AVS_BITMAP_EXPIRED || ace->hbmPic == NULL) {
		// Avatar was not ready or removed - need to remove it from cache
		if (old_pos >= 0) {
			ImageArray_RemoveImage(&dat->avatar_cache, old_pos);

			// Update all items
			ExecuteOnAllContacts(dat, ReduceAvatarPosition, (void *)&old_pos);
			contact->avatar_pos = AVATAR_POS_DONT_HAVE;
			return;
		}
	}
	else if (contact->avatar_data->hbmPic != NULL) { // let's add it
		// Clipping width and height
		LONG width_clip = dat->avatars_maxwidth_size ? dat->avatars_maxwidth_size : dat->avatars_maxheight_size;
		LONG height_clip = dat->avatars_maxheight_size;

		if (height_clip * ace->bmWidth / ace->bmHeight <= width_clip)
			width_clip = height_clip * ace->bmWidth / ace->bmHeight;
		else
			height_clip = width_clip * ace->bmHeight / ace->bmWidth;

		if (wildcmpit(contact->avatar_data->szFilename, _T("*.gif"))) {
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
		contact->avatar_data = NULL;
		return;
	}

	if (dat->avatars_show && !db_get_b(contact->hContact, "CList", "HideContactAvatar", 0)) {
		contact->avatar_data = (avatarCacheEntry*)CallService(MS_AV_GETAVATARBITMAP, contact->hContact, 0);
		if (contact->avatar_data == NULL || contact->avatar_data->cbSize != sizeof(avatarCacheEntry) || contact->avatar_data->dwFlags == AVS_BITMAP_EXPIRED)
			contact->avatar_data = NULL;

		if (contact->avatar_data != NULL)
			contact->avatar_data->t_lastAccess = (DWORD)time(NULL);
	}
	else contact->avatar_data = NULL;

	Cache_ProceedAvatarInList(dat, contact);
}
