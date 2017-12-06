#ifndef _YANDEXSERVICE_API_H_
#define _YANDEXSERVICE_API_H_

namespace YandexAPI
{
#define YANDEX_OAUTH "https://oauth.yandex.ru"
#define YADISK_API "https://cloud-api.yandex.net/v1/disk/resources"

#define YANDEX_APP_ID "c311a5967cae4efa88d1af97d01ea0e8"
#include "../../../miranda-private-keys/Yandex/client_secret.h"

	class GetAccessTokenRequest : public HttpRequest
	{
	public:
		GetAccessTokenRequest(const char *code) :
			HttpRequest(REQUEST_POST, YANDEX_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data(CMStringDataFormat::FORMAT,
				"client_id=%s&client_secret=%s&grant_type=authorization_code&code=%s",
				YANDEX_APP_ID, YADISK_CLIENT_SECRET, code);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class RefreshTokenRequest : public HttpRequest
	{
	public:
		RefreshTokenRequest(const char *refreshToken) :
			HttpRequest(REQUEST_POST, YANDEX_OAUTH "/token")
		{
			AddHeader("Content-Type", "application/x-www-form-urlencoded");

			CMStringA data(CMStringDataFormat::FORMAT,
				"client_id=%s&client_secret=%s&grant_type=refresh_token&refresh_token=%s",
				YANDEX_APP_ID, YADISK_CLIENT_SECRET, refreshToken);
			SetData(data.GetBuffer(), data.GetLength());
		}
	};

	class RevokeAccessTokenRequest : public HttpRequest
	{
	public:
		RevokeAccessTokenRequest(const char *token) :
			HttpRequest(REQUEST_POST, YANDEX_OAUTH "/token/revoke")
		{
			AddOAuthHeader(token);
		}
	};

	class GetUploadUrlRequest : public HttpRequest
	{
	public:
		GetUploadUrlRequest(const char *token, const char *path, OnConflict strategy = NONE) :
			HttpRequest(REQUEST_GET, YADISK_API "/upload")
		{
			AddOAuthHeader(token);
			AddUrlParameter("path=%s", ptrA(mir_urlEncode(path)));
			if (strategy == OnConflict::REPLACE)
				AddUrlParameter("overwrite=true");
		}
	};

	class UploadFileRequest : public HttpRequest
	{
	public:
		UploadFileRequest(const char *url, const char *data, size_t size) :
			HttpRequest(REQUEST_PUT, url)
		{
			SetData(data, size);
		}
	};

	class UploadFileChunkRequest : public HttpRequest
	{
	public:
		UploadFileChunkRequest(const char *url, const char *chunk, size_t chunkSize, uint64_t offset, uint64_t fileSize) :
			HttpRequest(REQUEST_PUT, url)
		{
			uint64_t rangeMin = offset;
			uint64_t rangeMax = offset + chunkSize - 1;
			CMStringA range(CMStringDataFormat::FORMAT, "bytes %I64u-%I64u/%I64u", rangeMin, rangeMax, fileSize);
			AddHeader("Content-Range", range);

			SetData(chunk, chunkSize);
		}
	};

	class CreateFolderRequest : public HttpRequest
	{
	public:
		CreateFolderRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_PUT, YADISK_API)
		{
			AddOAuthHeader(token);
			AddUrlParameter("path=%s", ptrA(mir_urlEncode(path)));
		}
	};

	class PublishRequest : public HttpRequest
	{
	public:
		PublishRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_PUT, YADISK_API "/publish")
		{
			AddOAuthHeader(token);
			AddUrlParameter("path=%s", ptrA(mir_urlEncode(path)));
		}
	};

	class GetResourcesRequest : public HttpRequest
	{
	public:
		GetResourcesRequest(const char *token, const char *path) :
			HttpRequest(REQUEST_GET, YADISK_API)
		{
			AddOAuthHeader(token);
			AddUrlParameter("path=%s", ptrA(mir_urlEncode(path)));
		}
	};
};

#endif //_YANDEXSERVICE_API_H_
