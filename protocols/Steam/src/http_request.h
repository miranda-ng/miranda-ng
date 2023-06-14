#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#define STEAM_USER_AGENT "Valve/Steam HTTP Client 1.0"

class HttpResponse
{
	NETLIBHTTPREQUEST *m_response;

public:
	HttpResponse(NETLIBHTTPREQUEST *response) :
		m_response(response)
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

	char* data() const
	{
		return (m_response) ? m_response->pData : nullptr;
	}

	unsigned length() const
	{
		return (m_response) ? m_response->dataLength : 0;
	}

	LIST<NETLIBHTTPHEADER> Headers() const;

	int GetStatusCode() const
	{
		if (m_response)
			return m_response->resultCode;
		return 500;
	}
};

#endif //_HTTP_REQUEST_H_