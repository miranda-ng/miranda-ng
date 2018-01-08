/*
Copyright (c) 2015-18 Miranda NG team (https://miranda-ng.org)

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

HttpRequest::HttpRequestBody& HttpRequest::HttpRequestBody::operator<<(const char *str)
{
	AppendSeparator();
	if (str != nullptr)
		content.Append(str);
	return *this;
}

HttpRequest::HttpRequestBody& HttpRequest::HttpRequestBody::operator<<(const BOOL_PARAM &param)
{
	AppendSeparator();
	content.AppendFormat("%s=%s", param.szName, param.bValue ? "true" : "false");
	return *this;
}

HttpRequest::HttpRequestBody& HttpRequest::HttpRequestBody::operator<<(const INT_PARAM &param)
{
	AppendSeparator();
	content.AppendFormat("%s=%i", param.szName, param.iValue);
	return *this;
}

HttpRequest::HttpRequestBody& HttpRequest::HttpRequestBody::operator<<(const INT64_PARAM &param)
{
	AppendSeparator();
	content.AppendFormat("%s=%lld", param.szName, param.iValue);
	return *this;
}

HttpRequest::HttpRequestBody& HttpRequest::HttpRequestBody::operator<<(const CHAR_PARAM &param)
{
	AppendSeparator();
	content.AppendFormat("%s=%s", param.szName, utils::url::encode(param.szValue).c_str());
	return *this;
}

HttpRequest::HttpRequestUrl& HttpRequest::HttpRequestUrl::operator<<(const char *param)
{
	if (param)
		request.AddUrlParameter("%s", param);
	return *this;
}

HttpRequest::HttpRequestUrl& HttpRequest::HttpRequestUrl::operator<<(const BOOL_PARAM &param)
{
	request.AddUrlParameter("%s=%s", param.szName, param.bValue ? "true" : "false");
	return *this;
}

HttpRequest::HttpRequestUrl& HttpRequest::HttpRequestUrl::operator<<(const INT_PARAM &param)
{
	request.AddUrlParameter("%s=%i", param.szName, param.iValue);
	return *this;
}

HttpRequest::HttpRequestUrl& HttpRequest::HttpRequestUrl::operator<<(const INT64_PARAM &param)
{
	request.AddUrlParameter("%s=%lld", param.szName, param.iValue);
	return *this;
}

HttpRequest::HttpRequestUrl& HttpRequest::HttpRequestUrl::operator<<(const CHAR_PARAM &param)
{
	request.AddUrlParameter("%s=%s", param.szName, param.szValue);
	return *this;
}
