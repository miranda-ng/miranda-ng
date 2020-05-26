#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequest;
class HttpResponse;

class HttpUri : private MNonCopyable
{
	friend class HttpRequest;

private:
	CMStringA m_uri;
	NETLIBHTTPREQUEST *m_request;

	HttpUri(NETLIBHTTPREQUEST *request, const char *uri)
		: m_request(request), m_uri(uri)
	{
		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

	HttpUri(NETLIBHTTPREQUEST *request, const char *urlFormat, va_list args)
		: m_request(request)
	{
		m_uri.AppendFormatV(urlFormat, args);
		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

	~HttpUri()
	{
		if (m_request)
			m_request->szUrl = NULL;
	}

	void FormatV(const char *urlFormat, va_list args)
	{
		m_uri.FormatV(urlFormat, args);
		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

	void AppendFormat(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		m_uri += (m_uri.Find('?') == -1) ? '?' : '&';
		m_uri.AppendFormatV(fmt, args);
		va_end(args);

		if (m_request)
			m_request->szUrl = m_uri.GetBuffer();
	}

public:
	operator const char*() const
	{
		return m_request
			? m_request->szUrl
			: NULL;
	}

	HttpUri &operator<<(const PARAM &param)
	{
		AppendFormat(param.szName);
		return *this;
	}

	HttpUri &operator<<(const INT_PARAM &param)
	{
		AppendFormat("%s=%i", param.szName, param.iValue);
		return *this;
	}

	HttpUri &operator<<(const INT64_PARAM &param)
	{
		AppendFormat("%s=%lld", param.szName, param.iValue);
		return *this;
	}

	HttpUri &operator<<(const CHAR_PARAM &param)
	{
		AppendFormat("%s=%s", param.szName, mir_urlEncode(param.szValue).c_str());
		return *this;
	}
};

class HttpHeaders : private MNonCopyable
{
	friend class HttpContent;
	friend class HttpRequest;
	friend class HttpResponse;

private:
	NETLIBHTTPREQUEST *m_request;

	HttpHeaders(NETLIBHTTPREQUEST *request)
		: m_request(request)
	{
	}

	void Set(LPCSTR szName)
	{
		Set(szName, "");
	}

	void Set(LPCSTR szName, LPCSTR szValue)
	{
		if (!m_request)
			return;

		m_request->headers = (NETLIBHTTPHEADER*)mir_realloc(m_request->headers,
			sizeof(NETLIBHTTPHEADER)*(m_request->headersCount + 1));
		m_request->headers[m_request->headersCount].szName = mir_strdup(szName);
		m_request->headers[m_request->headersCount].szValue = mir_strdup(szValue);
		m_request->headersCount++;
	}

public:
	const NETLIBHTTPHEADER* operator[](size_t idx) const
	{
		return m_request
			? &m_request->headers[idx]
			: nullptr;
	}

	size_t size() const
	{
		return m_request
			? m_request->headersCount
			: 0;
	}

	const NETLIBHTTPHEADER* begin() const
	{
		return m_request->headers;
	}

	const NETLIBHTTPHEADER* end() const
	{
		return m_request->headers + m_request->headersCount;
	}

	HttpHeaders& operator<<(const PARAM &param)
	{
		Set(param.szName);
		return *this;
	}

	HttpHeaders& operator<<(const CHAR_PARAM &param)
	{
		Set(param.szName, param.szValue);
		return *this;
	}
};

class HttpContent : private MNonCopyable
{
	friend class HttpRequest;
	friend class HttpResponse;

protected:
	HttpHeaders Headers;
	NETLIBHTTPREQUEST *m_request;

	HttpContent(NETLIBHTTPREQUEST *request)
		: Headers(request), m_request(request)
	{
	}

	virtual ~HttpContent()
	{
		if (m_request) {
			m_request->pData = nullptr;
			m_request->dataLength = 0;
		}
	}

public:
	operator bool() const
	{
		return m_request && m_request->pData && m_request->dataLength;
	}

	operator const char*() const
	{
		return m_request
			? m_request->pData
			: nullptr;
	}

	operator const uint8_t*() const
	{
		return m_request
			? (uint8_t*)m_request->pData
			: nullptr;
	}

	const uint8_t* data() const
	{
		return this->operator const uint8_t*();
	}

	size_t size() const
	{
		return m_request ? m_request->dataLength : 0;
	}
};

class FormUrlEncodedContent : public HttpContent
{
	friend FormUrlEncodedContent *operator<<(FormUrlEncodedContent *, const PARAM &);
	friend FormUrlEncodedContent *operator<<(FormUrlEncodedContent *, const BOOL_PARAM &);
	friend FormUrlEncodedContent *operator<<(FormUrlEncodedContent *, const INT_PARAM &);
	friend FormUrlEncodedContent *operator<<(FormUrlEncodedContent *, const INT64_PARAM &);
	friend FormUrlEncodedContent *operator<<(FormUrlEncodedContent *, const CHAR_PARAM &);

private:
	CMStringA m_content;

	void AppendFormat(const char *fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		if (!m_content.IsEmpty())
			m_content += '&';
		m_content.AppendFormatV(fmt, args);
		va_end(args);

		if (m_request) {
			m_request->pData = m_content.GetBuffer();
			m_request->dataLength = m_content.GetLength();
		}
	}

public:
	FormUrlEncodedContent(NETLIBHTTPREQUEST *request)
		: HttpContent(request)
	{
		Headers << CHAR_PARAM("Content-Type", "application/x-www-form-urlencoded");
	}
};

__forceinline FormUrlEncodedContent* operator<<(FormUrlEncodedContent *content, const PARAM &param)
{
	content->AppendFormat(param.szName);
	return content;
}

__forceinline FormUrlEncodedContent* operator<<(FormUrlEncodedContent *content, const BOOL_PARAM &param)
{
	content->AppendFormat("%s=%s", param.szName, param.bValue ? "true" : "false");
	return content;
}

__forceinline FormUrlEncodedContent* operator<<(FormUrlEncodedContent *content, const INT_PARAM &param)
{
	content->AppendFormat("%s=%i", param.szName, param.iValue);
	return content;
}

__forceinline FormUrlEncodedContent* operator<<(FormUrlEncodedContent *content, const INT64_PARAM &param)
{
	content->AppendFormat("%s=%lld", param.szName, param.iValue);
	return content;
}

__forceinline FormUrlEncodedContent* operator<<(FormUrlEncodedContent *content, const CHAR_PARAM &param)
{
	content->AppendFormat("%s=%s", param.szName, mir_urlEncode(param.szValue).c_str());
	return content;
}

enum HttpMethod
{
	HttpGet = 1,
	HttpPost
};

class HttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
{
	friend class HttpUri;
	friend class HttpHeaders;
	friend class HttpContent;

protected:
	enum HttpRequestUrlFormat { FORMAT };

public:
	HttpUri Uri;
	HttpHeaders Headers;
	HttpContent *Content;

	HttpRequest(HttpMethod method, const char *url)
		: Uri(this, url), Headers(this), Content(nullptr)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		requestType = method;
		flags = NLHRF_HTTP11 | NLHRF_SSL;
		timeout = 3000;

		Content = new HttpContent(this);
	}

	HttpRequest(HttpMethod method, HttpRequestUrlFormat, const char *urlFormat, ...)
		: Uri(this, urlFormat), Headers(this), Content(nullptr)
	{
		cbSize = sizeof(NETLIBHTTPREQUEST);
		requestType = method;
		flags = NLHRF_HTTP11 | NLHRF_SSL;
		timeout = 5;

		va_list formatArgs;
		va_start(formatArgs, urlFormat);
		Uri.FormatV(urlFormat, formatArgs);
		va_end(formatArgs);

		Content = new HttpContent(this);
	}

	~HttpRequest()
	{
		if (Content != nullptr) {
			delete Content;
			Content = nullptr;
		}
	}

	operator NETLIBHTTPREQUEST*()
	{
		return this;
	}
};

class HttpResponse
{
	friend class HttpRequest;

private:
	NETLIBHTTPREQUEST *m_response;

public:
	HttpRequest *Request;
	HttpHeaders Headers;
	HttpContent Content;

	HttpResponse(HttpRequest *request, NETLIBHTTPREQUEST *response) :
		Request(request),
		m_response(response),
		Headers(response),
		Content(response)
	{
	}

	~HttpResponse()
	{
		Netlib_FreeHttpRequest(m_response);
	}

	bool operator!() const
	{
		return !m_response || !m_response->pData;
	}

	operator bool() const
	{
		return m_response && m_response->pData;
	}

	bool IsSuccess() const
	{
		return m_response &&
			m_response->resultCode >= HTTP_CODE_OK &&
			m_response->resultCode <= HTTP_CODE_MULTI_STATUS;
	}

	int GetStatusCode() const
	{
		if (m_response)
			return m_response->resultCode;
		return Request ? Request->resultCode : 0;
	}
};

#endif //_HTTP_REQUEST_H_