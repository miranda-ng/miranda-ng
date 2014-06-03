#ifndef _STEAM_H_
#define _STEAM_H_

namespace SteamWebApi
{
	#define STEAM_API_URL "https://api.steampowered.com"
	#define STEAM_COM_URL "https://steamcommunity.com"

	class FriendListApi;
	class PendingApi;

	class BaseApi
	{
	public:
		class Result
		{
			friend FriendListApi;
			friend PendingApi;

		protected:
			bool success;
			HTTP_STATUS status;

		public:
			Result() : success(false), status(HTTP_STATUS_NONE) { }

			bool IsSuccess() const { return success; }
			HTTP_STATUS GetStatus() const { return status; }
		};
	};

	class HttpRequest : public NETLIBHTTPREQUEST, public MZeroedObject
	{
	public:
		std::string url;

		HttpRequest(int type, LPCSTR url)
		{
			cbSize = sizeof(NETLIBHTTPREQUEST);

			requestType = type;
			this->url = url;
			szUrl = (char*)this->url.c_str();
			timeout = 0;
			flags = NLHRF_HTTP11 | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
		}

		~HttpRequest()
		{
			for (int i = 0; i < headersCount; i++)
			{
				mir_free(headers[i].szName);
				mir_free(headers[i].szValue);
			}
			mir_free(headers);
			mir_free(pData);
		}

		void AddHeader(LPCSTR szName, LPCSTR szValue)
		{
			headers = (NETLIBHTTPHEADER*)mir_realloc(headers, sizeof(NETLIBHTTPHEADER)*(headersCount + 1));
			headers[headersCount].szName = mir_strdup(szName);
			headers[headersCount].szValue = mir_strdup(szValue);
			headersCount++;
		}

		void AddParameter(LPCSTR szName, LPCSTR szValue)
		{
			if (url.find('?') == -1)
				url.append("?").append(szName).append("=").append(szValue);
			else
				url.append("&").append(szName).append("=").append(szValue);
		}

		void AddParameter(LPCSTR szValue)
		{
			if (url.find('?') == -1)
				url.append("?").append(szValue);
			else
				url.append("&").append(szValue);
		}

		void SetData(const char *data, size_t size)
		{
			if (pData != NULL)
				mir_free(pData);

			dataLength = (int)size;
			pData = (char*)mir_alloc(size + 1);
			memcpy(pData, data, size);
			pData[size] = 0;
		}
	};

	class HttpGetRequest : public HttpRequest
	{
	public:
		HttpGetRequest(LPCSTR url) : HttpRequest(REQUEST_GET, url) { }
	};

	/*class HttpPostRequest : public HttpRequest
	{
	public:
		HttpPostRequest(LPCSTR url) : HttpRequest(REQUEST_POST, url)
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");
		}
	};*/

	class HttpsRequest : public HttpRequest
	{
	public:
		HttpsRequest(int type, LPCSTR url) : HttpRequest(type, url)
		{
			flags = NLHRF_HTTP11 | NLHRF_SSL | NLHRF_NODUMPSEND | NLHRF_DUMPASTEXT;
		}
	};

	class HttpsGetRequest : public HttpsRequest
	{
	public:
		HttpsGetRequest(LPCSTR url) : HttpsRequest(REQUEST_GET, url) { }
	};

	class HttpsPostRequest : public HttpsRequest
	{
	public:
		HttpsPostRequest(LPCSTR url) : HttpsRequest(REQUEST_POST, url)
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");
		}
	};
}

#include "Steam\rsa_key.h"
#include "Steam\authorization.h"
#include "Steam\login.h"
#include "Steam\session.h"
#include "Steam\friend_list.h"
#include "Steam\pending.h"
#include "Steam\friend.h"
#include "Steam\poll.h"
#include "Steam\message.h"
#include "Steam\search.h"
#include "Steam\avatar.h"

#endif //_STEAM_H_