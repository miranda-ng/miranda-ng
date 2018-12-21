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

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequest : public MHttpRequest
{
	va_list formatArgs;

protected:
	class HttpRequestBody
	{
	private:
		CMStringA content;

		void AppendSeparator()
		{
			if (!content.IsEmpty())
				content.AppendChar('&');
		}

	public:
		HttpRequestBody() {}

		HttpRequestBody& operator<<(const char *str);
		HttpRequestBody& operator<<(const BOOL_PARAM &param);
		HttpRequestBody& operator<<(const INT_PARAM &param);
		HttpRequestBody& operator<<(const INT64_PARAM &param);
		HttpRequestBody& operator<<(const CHAR_PARAM &param);

		char* ToString()
		{
			return content.Detach();
		}
	};

public:
	HttpRequestBody Body;

	enum PersistentType { NONE, DEFAULT, CHANNEL, MESSAGES };

	bool NotifyErrors;
	PersistentType Persistent;

	HttpRequest(int type, LPCSTR url)
	{
		m_szUrl = url;
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_DUMPASTEXT;
		requestType = type;
		timeout = 600 * 1000;

		NotifyErrors = true;
		Persistent = DEFAULT;
	}

	HttpRequest(int type, CMStringDataFormat, LPCSTR urlFormat, ...)
	{
		m_szUrl.AppendFormatV(urlFormat, (va_start(formatArgs, urlFormat), formatArgs));
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_DUMPASTEXT;
		requestType = type;
		va_end(formatArgs);
		timeout = 20 * 1000;

		NotifyErrors = true;
		Persistent = DEFAULT;
	}

	virtual NETLIBHTTPREQUEST* Send(HNETLIBUSER nlu)
	{
		if (m_szUrl.Find("://") == -1)
			m_szUrl.Insert(0, ((flags & NLHRF_SSL) ? "https://" : "http://"));
		if (!m_szParam.IsEmpty()) {
			m_szUrl.AppendChar('?');
			m_szUrl += m_szParam;
		}
		szUrl = m_szUrl.GetBuffer();

		if (!pData) {
			pData = Body.ToString();
			dataLength = (int)mir_strlen(pData);
		}

		Netlib_Logf(nlu, "Send request to %s", szUrl);

		return Netlib_HttpTransaction(nlu, this);
	}
};

#endif //_HTTP_REQUEST_H_