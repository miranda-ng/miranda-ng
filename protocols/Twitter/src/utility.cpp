/*
Copyright © 2012-24 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

#include <io.h>

twitter_id CTwitterProto::getId(const char *szSetting)
{
	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;
	if (db_get(0, m_szModuleName, szSetting, &dbv))
		return 0;

	twitter_id ret = *(twitter_id *)dbv.pbVal;
	db_free(&dbv);
	return ret;
}

void CTwitterProto::setId(const char *szSetting, twitter_id id)
{
	db_set_blob(0, m_szModuleName, szSetting, &id, sizeof(id));
}

/////////////////////////////////////////////////////////////////////////////////////////
// popups

void CTwitterProto::ShowPopup(const wchar_t *text, int Error)
{
	POPUPDATAW popup = {};
	mir_snwprintf(popup.lpwzContactName, TranslateT("%s Protocol"), m_tszUserName);
	wcsncpy_s(popup.lpwzText, text, _TRUNCATE);

	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}
	PUAddPopupW(&popup);
}

void CTwitterProto::ShowPopup(const char *text, int Error)
{
	POPUPDATAW popup = {};
	mir_snwprintf(popup.lpwzContactName, TranslateT("%s Protocol"), m_tszUserName);
	wcsncpy_s(popup.lpwzText, Utf2T(text), _TRUNCATE);
	if (Error) {
		popup.iSeconds = -1;
		popup.colorBack = 0x000000FF;
		popup.colorText = 0x00FFFFFF;
	}
	PUAddPopupW(&popup);
}

/////////////////////////////////////////////////////////////////////////////////////////

bool save_url(HNETLIBUSER hNetlib, const CMStringA &url, const CMStringW &filename)
{
	MHttpRequest req(REQUEST_GET);
	req.flags = NLHRF_HTTP11 | NLHRF_REDIRECT;
	req.m_szUrl = const_cast<char*>(url.c_str());

	NLHR_PTR resp(Netlib_HttpTransaction(hNetlib, &req));
	if (!resp)
		return false;
	
	if (resp->resultCode != 200)
		return false;
	
	// Create folder if necessary
	if (CreatePathToFileW(filename) != ERROR_SUCCESS)
		return false;

	// Write to file
	if (FILE *f = _wfopen(filename, L"wb")) {
		fwrite(resp->body, 1, resp->body.GetLength(), f);
		fclose(f);
	}
	else return false;
	
	return true;
}
