#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

class HttpRequestException
{
	CMStringA message;

public:
	HttpRequestException(const char *message) :
		message(message)
	{
	}

	const char* what() const throw()
	{
		return message.c_str();
	}
};

class HttpRequest : public MHttpRequest
{
private:
	void Init()
	{
		flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMP;
	}

protected:
	void AddBasicAuthHeader(LPCSTR szLogin, LPCSTR szPassword)
	{
		size_t length = mir_strlen(szLogin) + mir_strlen(szPassword) + 1;
		ptrA cPair((char*)mir_calloc(length + 1));
		mir_snprintf(
			cPair,
			length,
			"%s:%s",
			szLogin,
			szPassword);

		ptrA ePair(mir_base64_encode(cPair, length));
		AddHeader("Authorization", CMStringA(FORMAT, "Basic %s", ePair.get()));
	}

	void AddBearerAuthHeader(LPCSTR szValue)
	{
		AddHeader("Authorization", CMStringA(FORMAT, "Bearer %s", szValue));
	}

	void AddOAuthHeader(LPCSTR szValue)
	{
		AddHeader("Authorization", CMStringA(FORMAT, "OAuth %s", szValue));
	}

	void AddUrlParameter(const char *urlFormat, ...)
	{
		va_list urlArgs;
		va_start(urlArgs, urlFormat);
		m_szUrl += m_szUrl.Find('?') == -1 ? '?' : '&';
		m_szUrl.AppendFormatV(urlFormat, urlArgs);
		va_end(urlArgs);
	}

	void AddUrlParameterWithEncode(const char *name, const char *valueFormat, ...)
	{
		va_list valueArgs;
		va_start(valueArgs, valueFormat);
		m_szUrl += m_szUrl.Find('?') == -1 ? '?' : '&';
		m_szUrl.AppendFormat("%s=", name);
		CMStringA value;
		value.AppendFormatV(valueFormat, valueArgs);
		m_szUrl += mir_urlEncode(value);
		va_end(valueArgs);
	}

public:
	HttpRequest(int type, LPCSTR url) :
		MHttpRequest(type)
	{
		Init();

		m_szUrl = url;
	}

	HttpRequest(int type, CMStringDataFormat, LPCSTR urlFormat, ...) :
		MHttpRequest(type)
	{
		Init();

		va_list formatArgs;
		va_start(formatArgs, urlFormat);
		m_szUrl.AppendFormatV(urlFormat, formatArgs);
		va_end(formatArgs);
	}

	MHttpResponse* Send(HNETLIBUSER hConnection)
	{
		m_szUrl.Replace('\\', '/');
		return Netlib_HttpTransaction(hConnection, this);
	}
};

#endif //_HTTP_REQUEST_H_