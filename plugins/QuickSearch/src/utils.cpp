/*
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)

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

#define ACF_TYPE_NUMBER  0x00 // Parameter is number
#define ACF_TYPE_STRING  0x01 // Parameter is ANSI String
#define ACF_TYPE_UNICODE 0x02 // Parameter is Unicode string
#define ACF_TYPE_STRUCT  0x03 // Parameter is (result is in) structure
#define ACF_TYPE_PARAM   0x08 // Parameter is Call parameter
#define ACF_TYPE_CURRENT 0x09 // Parameter is ignored, used current user handle from current message window
#define ACF_TYPE_RESULT  0x0A // Parameter is previous action result
#define ACF_TYPE_MASK    0x0F // parameter/result type mask

ColumnItem::ColumnItem(const wchar_t *pwszTitle, int _width, int _setting_type) :
	title(mir_wstrdup(pwszTitle)),
	width(_width),
	setting_type(_setting_type)
{
	bEnabled = true;
}

ColumnItem::ColumnItem(const ColumnItem &src)
{
	memcpy(this, &src, sizeof(ColumnItem));

	title = mir_wstrdup(title);

	switch (setting_type) {
	case QST_SETTING:
		module = mir_strdup(module);
		setting = mir_strdup(setting);
		break;

	case QST_SCRIPT:
		script = mir_wstrdup(script);
		break;

	case QST_SERVICE:
		svc.service = mir_strdup(svc.service);
		switch (svc.wFlags) {
		case ACF_TYPE_NUMBER:
		case ACF_TYPE_STRING:
		case ACF_TYPE_UNICODE:
			svc.wParam = (WPARAM)mir_wstrdup((wchar_t *)svc.wParam);
			break;
		case ACF_TYPE_STRUCT:
			svc.wParam = (WPARAM)mir_strdup((char *)svc.wParam);
			break;
		}

		switch (svc.lFlags) {
		case ACF_TYPE_NUMBER:
		case ACF_TYPE_STRING:
		case ACF_TYPE_UNICODE:
			svc.lParam = (WPARAM)mir_wstrdup((wchar_t *)svc.lParam);
			break;
		case ACF_TYPE_STRUCT:
			svc.lParam = (WPARAM)mir_strdup((char *)svc.lParam);
			break;
		}
		break;
	}
}

ColumnItem::~ColumnItem()
{
	mir_free(title);

	switch (setting_type) {
	case QST_SETTING:
		mir_free(module);
		mir_free(setting);
		break;

	case QST_SCRIPT:
		mir_free(script);
		break;

	case QST_SERVICE:
		mir_free(svc.service);
		switch (svc.wFlags) {
		case ACF_TYPE_NUMBER:
		case ACF_TYPE_STRING:
		case ACF_TYPE_UNICODE:
			mir_free((wchar_t *)svc.wParam);
			break;
		case ACF_TYPE_STRUCT:
			mir_free((char *)svc.wParam);
			break;
		}

		switch (svc.lFlags) {
		case ACF_TYPE_NUMBER:
		case ACF_TYPE_STRING:
		case ACF_TYPE_UNICODE:
			mir_free((wchar_t *)svc.lParam);
			break;
		case ACF_TYPE_STRUCT:
			mir_free((char *)svc.lParam);
			break;
		}
		break;
	}
}

void ColumnItem::SetSpecialColumns()
{
    if (setting_type == QST_SETTING) {
		 if (datatype == QSTS_STRING && !mir_strcmp(module, "CList") && !mir_strcmp(setting, "Group"))
			 isGroup = true;

		 else if (datatype == QSTS_STRING && !mir_strcmp(module, "Tab_SRMsg") && !mir_strcmp(setting, "containerW"))
			 isContainer = true;

		 else if (datatype == QSTS_BYTE && !mir_strcmpi(setting, "XStatusId"))
			 isXstatus = true;

		 else if (datatype == QSTS_STRING && !mir_strcmp(setting, "MirVer") && g_bFingerInstalled)
			 isClient = true;
	 }
	 else if (setting_type == QST_CONTACTINFO && cnftype == CNF_GENDER)
		 isGender = true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// column functions

int ListViewToColumn(int col)
{
	for (auto &it : g_plugin.m_columns) {
		if (!it->bEnabled)
			continue;

		if (col-- <= 0)
			return g_plugin.m_columns.indexOf(&it);
	}
	return -1;
}

int ColumnToListView(int col)
{
	int res = -1;
	for (auto &it : g_plugin.m_columns) {
		if (it->bEnabled)
			res++;

		if (col-- <= 0)
			break;
	}
	return res;
}

void LoadDefaultColumns(OBJLIST<ColumnItem> &dst)
{
	dst.destroy();

	auto *pNew = new ColumnItem(TranslateT("Account"), 82, QST_OTHER);
	pNew->other = QSTO_ACCOUNT;
	dst.insert(pNew);

	dst.insert(new ContactIntoColumn(TranslateT("Gender"), 20, CNF_GENDER));

	pNew = new ContactIntoColumn(TranslateT("UserID"), 80, CNF_UNIQUEID);
	pNew->bFilter = true;
	dst.insert(pNew);
	
	pNew = new ContactIntoColumn(TranslateT("Nickname"), 76, QST_OTHER);
	pNew->bFilter = true;
	pNew->other = QSTO_DISPLAYNAME;

	pNew = new ContactIntoColumn(TranslateT("First name"), 68, CNF_FIRSTNAME);
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ContactIntoColumn(TranslateT("Last name"), 66, CNF_LASTNAME);
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Group"), 80, QST_SETTING);
	pNew->datatype = QSTS_STRING;
	pNew->module = mir_strdup("CList");
	pNew->setting = mir_strdup("Group");
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Container"), 80, QST_SETTING);
	pNew->datatype = QSTS_STRING;
	pNew->module = mir_strdup("Tab_SRMsg");
	pNew->setting = mir_strdup("containerW");
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ContactIntoColumn(TranslateT("Email"), 116, CNF_EMAIL);
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Client ID"), 60, QST_SETTING);
	pNew->datatype = QSTS_STRING;
	pNew->setting = mir_strdup("MirVer");
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Last seen"), 116, QST_OTHER);
	pNew->other = QSTO_LASTSEEN;
	pNew->dwFlags = 0;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Last event"), 100, QST_OTHER);
	pNew->other = QSTO_LASTEVENT;
	pNew->dwFlags = 0;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Online since"), 100, QST_SETTING);
	pNew->datatype = QSTS_TIMESTAMP;
	pNew->setting = mir_strdup("LogonTS");
	pNew->bFilter = true;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Metacontact"), 50, QST_OTHER);
	pNew->other = QSTO_METACONTACT;
	pNew->dwFlags = 0;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Event count"), 50, QST_OTHER);
	pNew->other = QSTO_EVENTCOUNT;
	pNew->dwFlags = 0;
	dst.insert(pNew);

	pNew = new ColumnItem(TranslateT("Contact add time"), 80, QST_SETTING);
	pNew->datatype = QSTS_TIMESTAMP;
	pNew->module = mir_strdup("UserInfo");
	pNew->setting = mir_strdup("ContactAddTime");
	dst.insert(pNew);
}

/////////////////////////////////////////////////////////////////////////////////////////
// window options

void CMPlugin::LoadOptWnd()
{
	m_rect.bottom = getDword(so_mbottom);
	m_rect.right = getDword(so_mright);
	m_rect.left = getDword(so_mleft);
	m_rect.top = getDword(so_mtop);

	m_flags = getDword(so_flags, QSO_SORTBYSTATUS + QSO_DRAWGRID + QSO_CLIENTICONS + QSO_COLORIZE + QSO_SORTASC);
	m_sortOrder = getDword(so_columnsort);
}

void CMPlugin::SaveOptWnd()
{
	setDword(so_mbottom, m_rect.bottom);
	setDword(so_mright, m_rect.right);
	setDword(so_mleft, m_rect.left);
	setDword(so_mtop, m_rect.top);

	setDword(so_flags, m_flags);
	setDword(so_columnsort, m_sortOrder);
}

/////////////////////////////////////////////////////////////////////////////////////////
// load options

int CMPlugin::LoadColumns(OBJLIST<ColumnItem> &dst)
{
	m_flags = getDword(so_flags, QSO_SORTBYSTATUS + QSO_DRAWGRID + QSO_CLIENTICONS + QSO_COLORIZE + QSO_SORTASC);
	int numCols = getWord(so_numcolumns);

	for (int i = 0; i < numCols; i++) {
		auto *pNew = new ColumnItem(nullptr);
		LoadColumn(i, *pNew);
		dst.insert(pNew);
	}

	return numCols;
}

void CMPlugin::LoadColumn(int n, ColumnItem &col)
{
	char buf[127];
	int offset = mir_snprintf(buf, "%s%d_", so_item, n);

	strcpy(buf + offset, so_title); col.title = getWStringA(buf);
	strcpy(buf + offset, so_setting_type); col.setting_type = getWord(buf);
	strcpy(buf + offset, so_flags); col.dwFlags = getWord(buf);
	strcpy(buf + offset, so_width); col.width = getWord(buf);

	switch (col.setting_type) {
	case QST_SETTING:
		strcpy(buf + offset, so_datatype); col.datatype = getWord(buf);
		strcpy(buf + offset, so_module); col.module = getStringA(buf);
		strcpy(buf + offset, so_setting); col.setting = getStringA(buf);
		break;

	case QST_SCRIPT:
		strcpy(buf + offset, so_script); col.script = getWStringA(buf);
		break;

	case QST_CONTACTINFO:
		strcpy(buf + offset, so_cnftype); col.cnftype = getWord(buf);
		break;

	case QST_SERVICE: 
		offset = mir_snprintf(buf, "%s%d/service/", so_item, n);
		strcpy(buf + offset, so_service); col.svc.service = getStringA(buf);
		strcpy(buf + offset, so_restype); col.svc.flags = getDword(buf);
		if (!mir_strcmp(col.svc.service, "Proto/GetContactBaseAccount")) {
			col.setting_type = QST_OTHER;
			col.other = QSTO_ACCOUNT;
			break;
		}

		strcpy(buf + offset, so_wparam); 
		LoadParamValue(buf, col.svc.wFlags, col.svc.wParam);

		strcpy(buf + offset, so_lparam);
		LoadParamValue(buf, col.svc.lFlags, col.svc.lParam);
		break;

	case QST_OTHER:
		strcpy(buf + offset, so_other); col.other = getWord(buf);
		break;
	}
}

void CMPlugin::LoadParamValue(char *buf, uint32_t &dwFlags, LPARAM &dwWalue)
{
	char *pEnd = buf + strlen(buf);
	strcpy(pEnd, "flags"); dwFlags = getDword(buf);

	strcpy(pEnd, "value"); 
	switch (dwFlags) {
	case ACF_TYPE_NUMBER:
	case ACF_TYPE_STRING:
	case ACF_TYPE_UNICODE:
		dwWalue = LPARAM(getWStringA(buf));
		break;

	case ACF_TYPE_STRUCT:
		dwWalue = LPARAM(getStringA(buf));
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// save options

static int ListSettings(const char *szSetting, void *param)
{
	if (!memcmp(szSetting, so_item, 4)) {
		auto *pList = (LIST<char>*)param;
		pList->insert(mir_strdup(szSetting));
	}
	return 0;
}

void CMPlugin::SaveOptions()
{
	// remove old settings
	LIST<char> settings(30);
	db_enum_settings(0, ListSettings, MODULENAME, &settings);

	for (auto &it : settings) {
		delSetting(it);
		mir_free(it);
	}

	// write new settings
	setDword(so_flags, m_flags);
	setWord(so_numcolumns, m_columns.getCount());

	int i = 0;
	for (auto &it : m_columns)
		SaveColumn(i++, *it);
}

void CMPlugin::SaveColumn(int n, const ColumnItem &col)
{
	char buf[127];
	int offset = mir_snprintf(buf, "%s%d_", so_item, n);

	strcpy(buf + offset, so_title); setWString(buf, col.title);
	strcpy(buf + offset, so_setting_type); setWord(buf, col.setting_type);
	strcpy(buf + offset, so_flags); setWord(buf, col.dwFlags);
	strcpy(buf + offset, so_width); setWord(buf, col.width);

	switch (col.setting_type) {
	case QST_SETTING:
		strcpy(buf + offset, so_datatype); setWord(buf, col.datatype);
		strcpy(buf + offset, so_module); setString(buf, col.module);
		strcpy(buf + offset, so_setting); setString(buf, col.setting);
		break;

	case QST_SCRIPT:
		strcpy(buf + offset, so_script); setWString(buf, col.script);
		break;

	case QST_CONTACTINFO:
		strcpy(buf + offset, so_cnftype); setWord(buf, col.cnftype);
		break;

	case QST_SERVICE: 
		offset = mir_snprintf(buf, "%s%d/service/", so_item, n);
		strcpy(buf + offset, so_service); setString(buf, col.svc.service);
		strcpy(buf + offset, so_restype); setDword(buf, col.svc.flags);
		
		strcpy(buf + offset, so_wparam); 
		SaveParamValue(buf, col.svc.wFlags, col.svc.wParam);
		
		strcpy(buf + offset, so_lparam);
		SaveParamValue(buf, col.svc.lFlags, col.svc.lParam);
		break;

	case QST_OTHER:
		strcpy(buf + offset, so_other); setWord(buf, col.other);
		break;
	}
}

void CMPlugin::SaveParamValue(char *buf, uint32_t flags, LPARAM value)
{
	char *pEnd = buf + strlen(buf);
	strcpy(pEnd, "flags"); setDword(buf, flags);

	strcpy(pEnd, "value"); 
	switch (flags) {
	case ACF_TYPE_NUMBER:
	case ACF_TYPE_STRING:
	case ACF_TYPE_UNICODE:
		setWString(buf, (wchar_t *)value);
		break;

	case ACF_TYPE_STRUCT:
		setString(buf, (char *)value);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

const wchar_t* cnf2str(int cnf)
{
	switch (cnf) {
	case CNF_FIRSTNAME:  return TranslateT("First name");
	case CNF_LASTNAME:   return TranslateT("Last name");
	case CNF_NICK:       return TranslateT("Nick");
	case CNF_CUSTOMNICK: return TranslateT("Custom nick");
	case CNF_EMAIL:      return TranslateT("Email");
	case CNF_CITY:       return TranslateT("City");
	case CNF_STATE:      return TranslateT("State");
	case CNF_COUNTRY:    return TranslateT("Country");
	case CNF_PHONE:      return TranslateT("Phone");
	case CNF_HOMEPAGE:   return TranslateT("Homepage");
	case CNF_ABOUT:      return TranslateT("About");
	case CNF_GENDER:     return TranslateT("Gender");
	case CNF_AGE:        return TranslateT("Age");
	case CNF_FIRSTLAST:  return TranslateT("First name/Last name");
	case CNF_UNIQUEID:   return TranslateT("Unique ID");
	case CNF_FAX:        return TranslateT("Fax");
	case CNF_CELLULAR:   return TranslateT("Cellular");
	case CNF_TIMEZONE:   return TranslateT("Time zone");
	case CNF_MYNOTES:    return TranslateT("My notes");
	case CNF_BIRTHDAY:   return TranslateT("Birth day");
	case CNF_BIRTHMONTH: return TranslateT("Birth month");
	case CNF_BIRTHYEAR:  return TranslateT("Birth year");
	case CNF_STREET:     return TranslateT("Street");
	case CNF_ZIP:        return TranslateT("ZIP code");
	case CNF_LANGUAGE1:  return TranslateT("Language #1");
	case CNF_LANGUAGE2:  return TranslateT("Language #2");
	case CNF_LANGUAGE3:  return TranslateT("Language #3");
	case CNF_CONAME:     return TranslateT("Company name");
	case CNF_CODEPT:     return TranslateT("Company department");
	case CNF_COPOSITION: return TranslateT("Company position");
	case CNF_COSTREET:   return TranslateT("Company street");
	case CNF_COCITY:     return TranslateT("Company city");
	case CNF_COSTATE:    return TranslateT("Company state");
	case CNF_COZIP:      return TranslateT("Company ZIP");
	case CNF_COCOUNTRY:  return TranslateT("Company country");
	case CNF_COHOMEPAGE: return TranslateT("Company homepage");
	case CNF_DISPLAYUID: return TranslateT("Display ID");
	}
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////
// formatters

wchar_t* BuildLastSeenTime(uint32_t ts)
{
	int year = ts / (60 * 24 * 31 * 356);
	if (year == 0)
		return nullptr;

	year += 1980; ts = ts % (60 * 24 * 31 * 356);

	int month = ts / (60 * 24 * 31); ts = ts % (60 * 24 * 31);
	int day = ts / (60 * 24); ts = ts % (60 * 24);
	int hours = ts / 60;
	int mins = ts % 60;

	return CMStringW(FORMAT, L"%02d.%02d.%04d - %02d:%02d", year, month, day, hours, mins).Detach();
}

uint32_t BuildLastSeenTimeInt(MCONTACT hContact, const char *szModule)
{
	int year = db_get_w(hContact, szModule, "Year");
	if (year == 0)
		return 0;

	int day = db_get_w(hContact, szModule, "Day");
	int month = db_get_w(hContact, szModule, "Month");
	int hours = db_get_w(hContact, szModule, "Hours");
	int minutes = db_get_w(hContact, szModule, "Minutes");

	return ((((year - 1980) * 356 + month) * 31 + day) * 24 + hours) * 60 + minutes;
}

wchar_t* TimeToStrW(uint32_t timestamp)
{
	wchar_t buf[63];
	TimeZone_ToStringW(timestamp, L"d - t", buf, _countof(buf));
	return mir_wstrdup(buf);
}

/////////////////////////////////////////////////////////////////////////////////////////

void SnapToScreen(RECT &rc)
{
	int left = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int top = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int right = GetSystemMetrics(SM_CXVIRTUALSCREEN) + left;
	int bottom = GetSystemMetrics(SM_CYVIRTUALSCREEN) + top;
	if (rc.right > right)
		rc.right = right;
	if (rc.bottom > bottom)
		rc.bottom = bottom;
	if (rc.left < left)
		rc.left = left;
	if (rc.top < top)
		rc.top = top;
}
