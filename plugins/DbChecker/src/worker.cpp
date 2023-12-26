/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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

#include "stdafx.h"

void ProcessingDone(void);

/////////////////////////////////////////////////////////////////////////////////////////

extern HWND hwndRichEdit;

struct StringBuf
{
	char *str;
	size_t size, streamOffset;
};

DWORD CALLBACK sttStreamInCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	StringBuf *buf = (StringBuf *)dwCookie;

	if (buf->streamOffset < buf->size) {
		size_t cbLen = min(size_t(cb), buf->size - buf->streamOffset);
		memcpy(pbBuff, buf->str + buf->streamOffset, cbLen);
		buf->streamOffset += cbLen;
		*pcb = (LONG)cbLen;
	}
	else *pcb = 0;

	return 0;
}

static bool ConvertRtfEvent(DBEVENTINFO &dbei)
{
	if (mir_strncmp(dbei.pBlob, "{\\rtf1", 6))
		return false;

	StringBuf buf = {};
	buf.str = dbei.pBlob;
	buf.size = dbei.cbBlob;

	EDITSTREAM es = {};
	es.dwCookie = (DWORD_PTR)&buf;
	es.pfnCallback = sttStreamInCallback;
	SendMessage(hwndRichEdit, EM_STREAMIN, SF_RTF, (LPARAM)&es);

	GETTEXTLENGTHEX param = { GTL_NUMBYTES, CP_UTF8 };
	int iLength = SendMessage(hwndRichEdit, EM_GETTEXTLENGTHEX, WPARAM(&param), 0);

	BOOL bFlag;
	char *utf8str = (char *)mir_alloc(iLength + 1);
	GETTEXTEX param2 = { (DWORD)iLength, GT_USECRLF, CP_UTF8, "?", &bFlag };
	SendMessage(hwndRichEdit, EM_GETTEXTEX, WPARAM(&param2), LPARAM(utf8str));

	mir_free(dbei.pBlob);

	dbei.flags |= DBEF_UTF;
	dbei.cbBlob = (uint32_t)mir_strlen(utf8str);
	dbei.pBlob = utf8str;
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool ConvertOldEvent(DBEVENTINFO &dbei)
{
	if (dbei.flags & DBEF_UTF)
		return false;

	if (dbei.flags == 1) {
		dbei.flags = DBEF_UTF;
		return true;
	}

	int msglen = (int)mir_strlen(dbei.pBlob) + 1, msglenW = 0;
	if (msglen != (int)dbei.cbBlob) {
		int count = ((dbei.cbBlob - msglen) / sizeof(wchar_t));
		wchar_t *p = (wchar_t *)&dbei.pBlob[msglen];
		for (int i = 0; i < count; i++) {
			if (p[i] == 0) {
				msglenW = i;
				break;
			}
		}
	}

	char *utf8str;
	if (msglenW > 0 && msglenW <= msglen)
		utf8str = mir_utf8encodeW((wchar_t *)&dbei.pBlob[msglen]);
	else
		utf8str = mir_utf8encode((char *)dbei.pBlob);

	if (utf8str == nullptr)
		return false;

	mir_free(dbei.pBlob);

	dbei.flags |= DBEF_UTF;
	dbei.cbBlob = (uint32_t)mir_strlen(utf8str);
	dbei.pBlob = utf8str;
	return true;
}

static bool CompareContents(const DBEVENTINFO &ev1, const DBEVENTINFO &ev2)
{
	if (ev1.cbBlob == 0 && ev2.cbBlob == 0)
		return true;

	if (ev1.cbBlob == 0 && ev2.cbBlob != 0 || ev1.cbBlob != 0 && ev2.cbBlob == 0)
		return false;

	// both blobs should be compared
	if (ev1.pBlob == 0 && ev2.pBlob == 0)
		return false;

	return !memcmp(ev1.pBlob, ev2.pBlob, ev1.cbBlob);
}

void __cdecl WorkerThread(DbToolOptions *opts)
{
	time_t ts = time(nullptr);

	AddToStatus(STATUS_MESSAGE, TranslateT("Database worker thread activated"));

	uint32_t sp = 0;

	if (opts->bMarkRead || opts->bCheckRtf || opts->bCheckUtf || opts->bCheckDups || opts->bCheckServerIds) {
		int nCount = 0, nUtfCount = 0, nRtfCount = 0, nDups = 0, nIds = 0;

		for (auto &cc : Contacts()) {
			DB::ECPTR pCursor(DB::Events(cc));
			DBEVENTINFO dboldev = {};
			while (MEVENT hEvent = pCursor.FetchNext()) {
				DB::EventInfo dbei(hEvent, opts->bCheckUtf || opts->bCheckRtf || opts->bCheckDups || opts->bCheckServerIds);
				if (!dbei)
					continue;

				if (opts->bMarkRead && !dbei.markedRead()) {
					db_event_markRead(cc, hEvent);
					nCount++;
				}

				if (dbei.eventType == EVENTTYPE_MESSAGE) {
					if (opts->bCheckUtf)
						if (ConvertOldEvent(dbei)) {
							db_event_edit(hEvent, &dbei);
							nUtfCount++;
						}

					if (opts->bCheckRtf)
						if (ConvertRtfEvent(dbei)) {
							db_event_edit(hEvent, &dbei);
							nRtfCount++;
						}

					if (opts->bCheckServerIds) {
						// if a blob is longer than its text part, there's a nessage id after text
						int iMsgLen = (int)mir_strlen((char *)dbei.pBlob);
						if (dbei.cbBlob - iMsgLen > 2) {
							dbei.cbBlob = iMsgLen;
							dbei.szId = (char *)dbei.pBlob + iMsgLen + 1;
							db_event_edit(hEvent, &dbei);
							nIds++;
						}
					}
				}

				if (opts->bCheckDups) {
					if (dbei == dboldev && CompareContents(dbei, dboldev)) {
						pCursor.DeleteEvent();
						nDups++;
					}
					else {
						mir_free(dboldev.pBlob);
						dboldev = dbei;
						if (dboldev.cbBlob) {
							dboldev.pBlob = (char *)mir_alloc(dboldev.cbBlob);
							memcpy(dboldev.pBlob, dbei.pBlob, dboldev.cbBlob);
						}
						else dboldev.pBlob = nullptr;
					}
				}
			}
			mir_free(dboldev.pBlob);
		}

		if (nCount)
			AddToStatus(STATUS_MESSAGE, TranslateT("%d events marked as read"), nCount);

		if (nUtfCount)
			AddToStatus(STATUS_MESSAGE, TranslateT("UTF-8 encoding fixed in %d events"), nUtfCount);

		if (nRtfCount)
			AddToStatus(STATUS_MESSAGE, TranslateT("RTF format fixed in %d events"), nRtfCount);

		if (nDups)
			AddToStatus(STATUS_MESSAGE, TranslateT("%d duplicate events removed"), nDups);

		if (nIds)
			AddToStatus(STATUS_MESSAGE, TranslateT("%d server event IDs fixed"), nIds);
	}

	DBCHeckCallback callback;
	callback.pfnAddLogMessage = AddToStatus;
	opts->dbChecker->Start(&callback);

	for (int task = 0;; task++) {
		if (callback.spaceProcessed / (callback.spaceUsed / 1000 + 1) > sp) {
			sp = callback.spaceProcessed / (callback.spaceUsed / 1000 + 1);
			SetProgressBar(sp);
		}
		WaitForSingleObject(opts->hEventRun, INFINITE);
		if (WaitForSingleObject(opts->hEventAbort, 0) == WAIT_OBJECT_0) {
			AddToStatus(STATUS_FATAL, TranslateT("Processing aborted by user"));
			break;
		}

		int ret = opts->dbChecker->CheckDb(task);
		if (ret == ERROR_OUT_OF_PAPER) {
			opts->dbChecker->Destroy();
			opts->dbChecker = nullptr;

			AddToStatus(STATUS_MESSAGE, TranslateT("Elapsed time: %d sec"), time(nullptr) - ts);
			if (errorCount)
				AddToStatus(STATUS_SUCCESS, TranslateT("All tasks completed but with errors (%d)"), errorCount);
			else
				AddToStatus(STATUS_SUCCESS, TranslateT("All tasks completed successfully"));
			break;
		}
		else if (ret != ERROR_SUCCESS)
			break;
	}

	opts->bFinished = true;
	ProcessingDone();
}
