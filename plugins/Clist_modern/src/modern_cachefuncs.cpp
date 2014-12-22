/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_cache_funcs.h"
#include "newpluginapi.h"
#include "./hdr/modern_gettextasync.h"
#include "hdr/modern_sync.h"

typedef BOOL(*ExecuteOnAllContactsFuncPtr) (ClcContact *contact, BOOL subcontact, void *param);

/////////////////////////////////////////////////////////////////////////////////////////
// Module static declarations

static int CopySkipUnprintableChars(TCHAR *to, TCHAR * buf, DWORD size);

static BOOL ExecuteOnAllContacts(ClcData *dat, ExecuteOnAllContactsFuncPtr func, void *param);
static BOOL ExecuteOnAllContactsOfGroup(ClcGroup *group, ExecuteOnAllContactsFuncPtr func, void *param);
int CLUI_SyncGetShortData(WPARAM wParam, LPARAM lParam);
void CListSettings_FreeCacheItemData(ClcCacheEntry *pDst);
void CListSettings_FreeCacheItemDataOption(ClcCacheEntry *pDst, DWORD flag);

/////////////////////////////////////////////////////////////////////////////////////////
// Get time zone for contact

void Cache_GetTimezone(ClcData *dat, MCONTACT hContact)
{
	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(hContact);
	if (dat == NULL && pcli->hwndContactTree)
		dat = (ClcData *)GetWindowLongPtr(pcli->hwndContactTree, 0);

	if (dat && dat->hWnd == pcli->hwndContactTree) {
		DWORD flags = dat->contact_time_show_only_if_different ? TZF_DIFONLY : 0;
		pdnce->hTimeZone = tmi.createByContact(hContact, 0, flags);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// Get all lines of text

void Cache_GetText(ClcData *dat, ClcContact *contact, BOOL forceRenew)
{
	Cache_GetFirstLineText(dat, contact);
	if (!dat->force_in_dialog) {
		ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(contact->hContact);
		if ((dat->second_line_show && (forceRenew || pdnce->szSecondLineText == NULL)) || (dat->third_line_show && (forceRenew || pdnce->szThirdLineText == NULL)))
			gtaAddRequest(dat, contact->hContact);
	}
}

void CSmileyString::AddListeningToIcon(SHORTDATA *dat, TCHAR *szText)
{
	iMaxSmileyHeight = 0;
	DestroySmileyList();

	if (szText == NULL)
		return;

	int text_size = (int)_tcslen(szText);

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
	//	ASSERT( plText == NULL );

	if (!plInput || plInput->realCount == 0) return;
	plText = List_Create(0, 1);
	for (int i = 0; i < plInput->realCount; i++)
	{
		ClcContactTextPiece *pieceFrom = (ClcContactTextPiece *)plInput->items[i];
		if (pieceFrom != NULL)
		{
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
	//ASSERT( plText == NULL );

	if (plText == NULL) return;

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

/*
* Parsing of text for smiley
*/

void CSmileyString::ReplaceSmileys(SHORTDATA *dat, ClcCacheEntry *pdnce, TCHAR * szText, BOOL replace_smileys)
{
	int last_pos = 0;
	iMaxSmileyHeight = 0;

	DestroySmileyList();

	if (!dat->text_replace_smileys || !replace_smileys || szText == NULL)
		return;

	int text_size = (int)_tcslen(szText);

	// Call service for the first time to see if needs to be used...
	SMADD_BATCHPARSE2 sp = { 0 };
	sp.cbSize = sizeof(sp);

	if (dat->text_use_protocol_smileys) {
		sp.Protocolname = pdnce->m_cache_cszProto;

		if (db_get_b(NULL, "CLC", "Meta", SETTING_USEMETAICON_DEFAULT) != 1 && pdnce->m_cache_cszProto != NULL && mir_strcmp(pdnce->m_cache_cszProto, META_PROTO) == 0) {
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

/*
*	Getting Status name
*  -1 for XStatus, 1 for Status
*/
int GetStatusName(TCHAR *text, int text_size, ClcCacheEntry *pdnce, BOOL xstatus_has_priority)
{
	BOOL noAwayMsg = FALSE;
	BOOL noXstatus = FALSE;
	// Hide status text if Offline  /// no offline
	WORD nStatus = pdnce___GetStatus(pdnce);
	if ((nStatus == ID_STATUS_OFFLINE || nStatus == 0) && g_CluiData.bRemoveAwayMessageForOffline) noAwayMsg = TRUE;
	if (nStatus == ID_STATUS_OFFLINE || nStatus == 0) noXstatus = TRUE;
	text[0] = '\0';
	// Get XStatusName
	if (!noAwayMsg && !noXstatus &&  xstatus_has_priority && pdnce->hContact && pdnce->m_cache_cszProto) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusName", &dbv)) {
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
		_tcsncpy_s(text, text_size, tmp, _TRUNCATE);
		return 1;
	}

	// Get XStatusName
	if (!noAwayMsg && !noXstatus && !xstatus_has_priority && pdnce->hContact && pdnce->m_cache_cszProto) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusName", &dbv)) {
			CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return -1;
		}
	}

	return 1;
}

/*
* Get Listening to information
*/

void GetListeningTo(TCHAR *text, int text_size, ClcCacheEntry *pdnce)
{
	DBVARIANT dbv = { 0 };
	WORD wStatus = pdnce___GetStatus(pdnce);
	text[0] = _T('\0');

	if (wStatus == ID_STATUS_OFFLINE || wStatus == 0)
		return;

	if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "ListeningTo", &dbv)) {
		CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
		db_free(&dbv);
	}
}

/*
*	Getting Status message (Away message)
*  -1 for XStatus, 1 for Status
*/

int GetStatusMessage(TCHAR *text, int text_size, ClcCacheEntry *pdnce, BOOL xstatus_has_priority)
{
	DBVARIANT dbv = { 0 };
	BOOL noAwayMsg = FALSE;
	WORD wStatus = pdnce___GetStatus(pdnce);
	text[0] = '\0';
	// Hide status text if Offline  /// no offline

	if (wStatus == ID_STATUS_OFFLINE || wStatus == 0) noAwayMsg = TRUE;
	// Get XStatusMsg
	if (!noAwayMsg  && xstatus_has_priority && pdnce->hContact && pdnce->m_cache_cszProto) {
		// Try to get XStatusMsg
		if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusMsg", &dbv)) {
			//mir_tstrncpy(text, dbv.pszVal, text_size);
			CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return -1;
		}
	}

	// Get StatusMsg
	if (pdnce->hContact && text[0] == '\0') {
		if (!db_get_ts(pdnce->hContact, "CList", "StatusMsg", &dbv)) {
			//mir_tstrncpy(text, dbv.pszVal, text_size);
			CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return 1;
		}
	}

	// Get XStatusMsg
	if (!noAwayMsg && !xstatus_has_priority && pdnce->hContact && pdnce->m_cache_cszProto && text[0] == '\0') {
		// Try to get XStatusMsg
		if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusMsg", &dbv)) {
			//mir_tstrncpy(text, dbv.pszVal, text_size);
			CopySkipUnprintableChars(text, dbv.ptszVal, text_size - 1);
			db_free(&dbv);

			if (text[0] != '\0')
				return -1;
		}
	}

	return 1;
}


/*
*	Get the text for specified lines
*/
int Cache_GetLineText(
	ClcCacheEntry *pdnce, int type, LPTSTR text, int text_size, TCHAR *variable_text, BOOL xstatus_has_priority,
	BOOL show_status_if_no_away, BOOL show_listening_if_no_away, BOOL use_name_and_message_for_xstatus,
	BOOL pdnce_time_show_only_if_different)
{

	if (text == NULL)
		return TEXT_EMPTY;
	text[0] = '\0';

	switch (type) {
	case TEXT_STATUS:
		if (GetStatusName(text, text_size, pdnce, xstatus_has_priority) == -1 && use_name_and_message_for_xstatus) {
			DBVARIANT dbv = { 0 };

			// Try to get XStatusMsg
			if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusMsg", &dbv)) {
				if (dbv.ptszVal != NULL && dbv.ptszVal[0] != 0) {
					TCHAR *tmp = NEWTSTR_ALLOCA(text);
					mir_sntprintf(text, text_size, _T("%s: %s"), tmp, dbv.ptszVal);
					CopySkipUnprintableChars(text, text, text_size - 1);
				}
				db_free(&dbv);
			}
		}

		return TEXT_STATUS;

	case TEXT_NICKNAME:
		if (pdnce->hContact && pdnce->m_cache_cszProto) {
			DBVARIANT dbv = { 0 };
			if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "Nick", &dbv)) {
				mir_tstrncpy(text, dbv.ptszVal, text_size);
				db_free(&dbv);
				CopySkipUnprintableChars(text, text, text_size - 1);
			}
		}

		return TEXT_NICKNAME;

	case TEXT_STATUS_MESSAGE:
		if (GetStatusMessage(text, text_size, pdnce, xstatus_has_priority) == -1 && use_name_and_message_for_xstatus) {
			DBVARIANT dbv = { 0 };

			// Try to get XStatusName
			if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusName", &dbv)) {
				if (dbv.pszVal != NULL && dbv.pszVal[0] != 0) {
					TCHAR *tmp = NEWTSTR_ALLOCA(text);
					mir_sntprintf(text, text_size, _T("%s: %s"), dbv.pszVal, tmp);
				}
				CopySkipUnprintableChars(text, text, text_size - 1);
				db_free(&dbv);
			}
		}
		else if (use_name_and_message_for_xstatus && xstatus_has_priority) {
			DBVARIANT dbv = { 0 };
			// Try to get XStatusName
			if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "XStatusName", &dbv)) {
				if (dbv.ptszVal != NULL && dbv.ptszVal[0] != 0)
					_tcsncpy_s(text, text_size, dbv.ptszVal, _TRUNCATE);
				CopySkipUnprintableChars(text, text, text_size - 1);
				db_free(&dbv);
			}
		}

		if (text[0] == '\0') {
			if (show_listening_if_no_away) {
				Cache_GetLineText(pdnce, TEXT_LISTENING_TO, text, text_size, variable_text, xstatus_has_priority, 0, 0, use_name_and_message_for_xstatus, pdnce_time_show_only_if_different);
				if (text[0] != '\0')
					return TEXT_LISTENING_TO;
			}

			if (show_status_if_no_away) {
				//re-request status if no away
				return Cache_GetLineText(pdnce, TEXT_STATUS, text, text_size, variable_text, xstatus_has_priority, 0, 0, use_name_and_message_for_xstatus, pdnce_time_show_only_if_different);
			}
		}
		return TEXT_STATUS_MESSAGE;

	case TEXT_LISTENING_TO:
		GetListeningTo(text, text_size, pdnce);
		return TEXT_LISTENING_TO;

	case TEXT_TEXT:
	{
		TCHAR *tmp = variables_parsedup(variable_text, pdnce->tszName, pdnce->hContact);
		mir_tstrncpy(text, tmp, text_size);
		mir_free(tmp);
		CopySkipUnprintableChars(text, text, text_size - 1);
	}
	return TEXT_TEXT;

	case TEXT_CONTACT_TIME:
		if (pdnce->hTimeZone) {
			// Get pdnce time
			text[0] = 0;
			tmi.printDateTime(pdnce->hTimeZone, _T("t"), text, text_size, 0);
		}

		return TEXT_CONTACT_TIME;
	}

	return TEXT_EMPTY;
}

/*
*	Get the text for First Line
*/
void Cache_GetFirstLineText(ClcData *dat, ClcContact *contact)
{
	if (GetCurrentThreadId() != g_dwMainThreadID)
		return;

	ClcCacheEntry *pdnce = pcli->pfnGetCacheEntry(contact->hContact);
	TCHAR *name = pcli->pfnGetContactDisplayName(contact->hContact, 0);
	if (dat->first_line_append_nick && (!dat->force_in_dialog)) {
		DBVARIANT dbv = { 0 };
		if (!db_get_ts(pdnce->hContact, pdnce->m_cache_cszProto, "Nick", &dbv)) {
			TCHAR nick[SIZEOF(contact->szText)];
			mir_tstrncpy(nick, dbv.ptszVal, SIZEOF(contact->szText));
			db_free(&dbv);

			// They are the same -> use the name to keep the case
			if (_tcsicmp(name, nick) == 0)
				mir_tstrncpy(contact->szText, name, SIZEOF(contact->szText));
			else
				// Append then
				mir_sntprintf(contact->szText, SIZEOF(contact->szText), _T("%s - %s"), name, nick);
		}
		else mir_tstrncpy(contact->szText, name, SIZEOF(contact->szText));
	}
	else mir_tstrncpy(contact->szText, name, SIZEOF(contact->szText));

	if (!dat->force_in_dialog) {
		SHORTDATA data = { 0 };
		Sync(CLUI_SyncGetShortData, (WPARAM)pcli->hwndContactTree, (LPARAM)&data);
		contact->ssText.ReplaceSmileys(&data, pdnce, contact->szText, dat->first_line_draw_smileys);
	}
}

/*
*	Get the text for Second Line
*/

void Cache_GetSecondLineText(SHORTDATA *dat, ClcCacheEntry *pdnce)
{
	TCHAR Text[240 - EXTRA_ICON_COUNT] = { 0 };
	int type = TEXT_EMPTY;

	if (dat->second_line_show)
		type = Cache_GetLineText(pdnce, dat->second_line_type, Text, SIZEOF(Text), dat->second_line_text,
			dat->second_line_xstatus_has_priority, dat->second_line_show_status_if_no_away, dat->second_line_show_listening_if_no_away,
			dat->second_line_use_name_and_message_for_xstatus, dat->contact_time_show_only_if_different);

	Text[SIZEOF(Text) - 1] = 0; //to be sure that it is null terminated string

	replaceStrT(pdnce->szSecondLineText, (dat->second_line_show) ? Text : NULL);

	if (pdnce->szSecondLineText) {
		if (type == TEXT_LISTENING_TO && pdnce->szSecondLineText[0] != _T('\0'))
			pdnce->ssSecondLine.AddListeningToIcon(dat, pdnce->szSecondLineText);
		else
			pdnce->ssSecondLine.ReplaceSmileys(dat, pdnce, pdnce->szSecondLineText, dat->second_line_draw_smileys);
	}
}

/*
*	Get the text for Third Line
*/
void Cache_GetThirdLineText(SHORTDATA *dat, ClcCacheEntry *pdnce)
{
	TCHAR Text[240 - EXTRA_ICON_COUNT] = { 0 };
	int type = TEXT_EMPTY;
	if (dat->third_line_show)
		type = Cache_GetLineText(pdnce, dat->third_line_type, Text, SIZEOF(Text), dat->third_line_text,
			dat->third_line_xstatus_has_priority, dat->third_line_show_status_if_no_away, dat->third_line_show_listening_if_no_away,
			dat->third_line_use_name_and_message_for_xstatus, dat->contact_time_show_only_if_different);

	Text[SIZEOF(Text) - 1] = 0; //to be sure that it is null terminated string

	replaceStrT(pdnce->szThirdLineText, (dat->third_line_show) ? Text : NULL);
	if (pdnce->szThirdLineText) {
		if (type == TEXT_LISTENING_TO && pdnce->szThirdLineText[0] != _T('\0'))
			pdnce->ssThirdLine.AddListeningToIcon(dat, pdnce->szThirdLineText);
		else
			pdnce->ssThirdLine.ReplaceSmileys(dat, pdnce, pdnce->szThirdLineText, dat->third_line_draw_smileys);
	}
}

void RemoveTag(TCHAR *to, TCHAR *tag)
{
	TCHAR *st = to;
	int len = (int)_tcslen(tag);
	int lastsize = (int)_tcslen(to) + 1;
	while (st = _tcsstr(st, tag)) {
		lastsize -= len;
		memmove((void*)st, (void*)(st + len), (lastsize)*sizeof(TCHAR));
	}
}

/*
*	Copy string with removing Escape chars from text
*   And BBcodes
*/
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

// If ExecuteOnAllContactsFuncPtr returns FALSE, stop loop
// Return TRUE if finished, FALSE if was stoped
static BOOL ExecuteOnAllContacts(ClcData *dat, ExecuteOnAllContactsFuncPtr func, void *param)
{
	BOOL res;
	res = ExecuteOnAllContactsOfGroup(&dat->list, func, param);
	return res;
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


/*
*	Avatar working routines
*/
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
	struct avatarCacheEntry * ace = contact->avatar_data;
	int old_pos = contact->avatar_pos;

	if (ace == NULL || ace->dwFlags == AVS_BITMAP_EXPIRED || ace->hbmPic == NULL) {
		//Avatar was not ready or removed - need to remove it from cache
		if (old_pos >= 0) {
			ImageArray_RemoveImage(&dat->avatar_cache, old_pos);
			// Update all items
			ExecuteOnAllContacts(dat, ReduceAvatarPosition, (void *)&old_pos);
			contact->avatar_pos = AVATAR_POS_DONT_HAVE;
			return;
		}
	}
	else if (contact->avatar_data->hbmPic != NULL) //Lets Add it
	{
		// Make bounds -> keep aspect radio
		LONG width_clip;
		LONG height_clip;

		// Clipping width and height
		width_clip = dat->avatars_maxwidth_size ? dat->avatars_maxwidth_size : dat->avatars_maxheight_size;
		height_clip = dat->avatars_maxheight_size;

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
		void * pt;
		HDC hdc = CreateCompatibleDC(dat->avatar_cache.hdc);
		HBITMAP hDrawBmp = ske_CreateDIB32Point(width_clip, height_clip, &pt);
		HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hDrawBmp);
		//need to draw avatar bitmap here
		{
			int w = width_clip;
			int h = height_clip;
			DrawAvatarImageWithGDIp(hdc, 0, 0, w, h, ace->hbmPic, 0, 0, ace->bmWidth, ace->bmHeight, ace->dwFlags, 255);
		}
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
	int old_pos = contact->avatar_pos;
	// workaround for avatar service and other wich destroys service on OK_TOEXIT
	if (g_CluiData.bSTATE != STATE_NORMAL || (dat->use_avatar_service && !ServiceExists(MS_AV_GETAVATARBITMAP))) {
		contact->avatar_pos = AVATAR_POS_DONT_HAVE;
		contact->avatar_data = NULL;
		return;
	}

	if (dat->use_avatar_service && ServiceExists(MS_AV_GETAVATARBITMAP)) {
		if (dat->avatars_show && !db_get_b(contact->hContact, "CList", "HideContactAvatar", 0)) {
			contact->avatar_data = (struct avatarCacheEntry *)CallService(MS_AV_GETAVATARBITMAP, (WPARAM)contact->hContact, 0);
			if (contact->avatar_data == NULL || contact->avatar_data->cbSize != sizeof(struct avatarCacheEntry) || contact->avatar_data->dwFlags == AVS_BITMAP_EXPIRED)
				contact->avatar_data = NULL;

			if (contact->avatar_data != NULL)
				contact->avatar_data->t_lastAccess = (DWORD)time(NULL);
		}
		else contact->avatar_data = NULL;

		Cache_ProceedAvatarInList(dat, contact);
	}
	else {
		contact->avatar_pos = AVATAR_POS_DONT_HAVE;
		if (dat->avatars_show && !db_get_b(contact->hContact, "CList", "HideContactAvatar", 0)) {
			DBVARIANT dbv;
			if (!db_get_ts(contact->hContact, "ContactPhoto", "File", &dbv)) {
				HBITMAP hBmp = (HBITMAP)CallService(MS_UTILS_LOADBITMAPT, 0, (LPARAM)dbv.ptszVal);
				if (hBmp != NULL) {
					// Make bounds
					BITMAP bm;
					if (GetObject(hBmp, sizeof(BITMAP), &bm)) {
						// Create data...
						HDC hdc;
						HBITMAP hDrawBmp, oldBmp;

						// Make bounds -> keep aspect radio
						LONG width_clip;
						LONG height_clip;
						RECT rc = { 0 };

						// Clipping width and height
						width_clip = dat->avatars_maxheight_size;
						height_clip = dat->avatars_maxheight_size;

						if (height_clip * bm.bmWidth / bm.bmHeight <= width_clip)
							width_clip = height_clip * bm.bmWidth / bm.bmHeight;
						else
							height_clip = width_clip * bm.bmHeight / bm.bmWidth;

						// Create objs
						hdc = CreateCompatibleDC(dat->avatar_cache.hdc);
						hDrawBmp = ske_CreateDIB32(width_clip, height_clip);
						oldBmp = (HBITMAP)SelectObject(hdc, hDrawBmp);
						SetBkMode(hdc, TRANSPARENT);

						POINT org;
						GetBrushOrgEx(hdc, &org);
						SetStretchBltMode(hdc, HALFTONE);
						SetBrushOrgEx(hdc, org.x, org.y, NULL);

						rc.right = width_clip - 1;
						rc.bottom = height_clip - 1;

						// Draw bitmap             8//8
						HDC dcMem = CreateCompatibleDC(hdc);
						HBITMAP obmp = (HBITMAP)SelectObject(dcMem, hBmp);
						StretchBlt(hdc, 0, 0, width_clip, height_clip, dcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
						SelectObject(dcMem, obmp);
						DeleteDC(dcMem);

						RECT rtr = { 0 };
						rtr.right = width_clip + 1;
						rtr.bottom = height_clip + 1;
						ske_SetRectOpaque(hdc, &rtr);

						hDrawBmp = (HBITMAP)GetCurrentObject(hdc, OBJ_BITMAP);
						SelectObject(hdc, oldBmp);
						DeleteDC(hdc);

						// Add to list
						if (old_pos >= 0) {
							ImageArray_ChangeImage(&dat->avatar_cache, hDrawBmp, old_pos);
							contact->avatar_pos = old_pos;
						}
						else contact->avatar_pos = ImageArray_AddImage(&dat->avatar_cache, hDrawBmp, -1);

						DeleteObject(hDrawBmp);
					} // if (GetObject(hBmp,sizeof(BITMAP),&bm))
					DeleteObject(hBmp);
				} //if (hBmp != NULL)
				db_free(&dbv);
			}
		}

		// Remove avatar if needed
		if (old_pos >= 0 && contact->avatar_pos == AVATAR_POS_DONT_HAVE) {
			ImageArray_RemoveImage(&dat->avatar_cache, old_pos);
			// Update all items
			ExecuteOnAllContacts(dat, ReduceAvatarPosition, (void *)&old_pos);
		}

		if (old_pos == AVATAR_POS_ANIMATED && contact->avatar_pos != AVATAR_POS_ANIMATED)
			AniAva_RemoveAvatar(contact->hContact);
	}
}
