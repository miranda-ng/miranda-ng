/*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

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

static void eraseOldValue(JSONNode &json, const char *pszName)
{
	if (pszName && *pszName != 0) {
		auto n = json.find(pszName);
		if (n != json.end())
			json.erase(n);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const INT_PARAM &param)
{
	eraseOldValue(json, param.szName);
	json.push_back(JSONNode(param.szName, param.iValue));
	return json;
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const INT64_PARAM &param)
{
	eraseOldValue(json, param.szName);
	json.push_back(JSONNode(param.szName, param.iValue));
	return json;
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const SINT64_PARAM &param)
{
	eraseOldValue(json, param.szName);

	char str[40];
	_i64toa(param.iValue, str, 10);
	return json << CHAR_PARAM(param.szName, str);
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const BOOL_PARAM &param)
{
	eraseOldValue(json, param.szName);
	json.push_back(JSONNode(param.szName, param.bValue));
	return json;
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const CHAR_PARAM &param)
{
	eraseOldValue(json, param.szName);

	if (param.szValue == nullptr) {
		JSONNode tmp(JSON_NULL); tmp.set_name(param.szName);
		json.push_back(tmp);
	}
	else if (param.szName == nullptr) {
		JSONNode tmp; tmp.set_name(param.szValue);
		json.push_back(tmp);
	}
	else json.push_back(JSONNode(param.szName, param.szValue));
	
	return json;
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const WCHAR_PARAM &param)
{
	eraseOldValue(json, param.szName);
	json.push_back(JSONNode(param.szName, ptrA(mir_utf8encodeW(param.wszValue)).get()));
	return json;
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const NULL_PARAM &param)
{
	eraseOldValue(json, param.szName);

	JSONNode newOne(JSON_NULL);
	newOne.set_name(param.szName);
	json.push_back(newOne);
	return json;
}

LIBJSON_DLL(JSONNode&) operator<<(JSONNode &json, const JSON_PARAM &param)
{
	eraseOldValue(json, param.szName);

	JSONNode newOne(param.node);
	newOne.set_name(param.szName);
	json.push_back(newOne);
	return json;
}
