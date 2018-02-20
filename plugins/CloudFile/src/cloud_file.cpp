#include "stdafx.h"

CCloudService::CCloudService(const char *protoName, const wchar_t *userName)
	: PROTO<CCloudService>(protoName, userName)
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = (char*)protoName;
	nlu.szDescriptiveName.w = (wchar_t*)userName;
	hConnection = Netlib_RegisterUser(&nlu);
}

CCloudService::~CCloudService()
{
	Netlib_CloseHandle(hConnection);
	hConnection = nullptr;
}

const char* CCloudService::GetAccountName() const
{
	return m_szModuleName;
}

const wchar_t* CCloudService::GetUserName() const
{
	return m_tszUserName;
}

DWORD_PTR CCloudService::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_1:
		return PF1_FILESEND;
	default:
		return 0;
	}
}

void CCloudService::Report(MCONTACT hContact, const wchar_t *data)
{
	if (db_get_b(NULL, MODULE, "UrlAutoSend", 1))
		SendToContact(hContact, data);

	if (db_get_b(NULL, MODULE, "UrlPasteToMessageInputArea", 0))
		PasteToInputArea(hContact, data);

	if (db_get_b(NULL, MODULE, "UrlCopyToClipboard", 0))
		PasteToClipboard(data);
}

char* CCloudService::PreparePath(const char *oldPath, char *newPath)
{
	if (oldPath == nullptr)
		mir_strcpy(newPath, "");
	else if (*oldPath != '/')
	{
		CMStringA result("/");
		result.Append(oldPath);
		result.Replace("\\", "/");
		mir_strcpy(newPath, result);
	}
	else
		mir_strcpy(newPath, oldPath);
	return newPath;
}

char* CCloudService::HttpStatusToError(int status)
{
	switch (status) {
	case HTTP_CODE_OK:
		return "OK";
	case HTTP_CODE_BAD_REQUEST:
		return "Bad input parameter. Error message should indicate which one and why";
	case HTTP_CODE_UNAUTHORIZED:
		return "Bad or expired token. This can happen if the user or Dropbox revoked or expired an access token. To fix, you should re-authenticate the user";
	case HTTP_CODE_FORBIDDEN:
		return "Bad OAuth request (wrong consumer key, bad nonce, expired timestamp...). Unfortunately, re-authenticating the user won't help here";
	case HTTP_CODE_NOT_FOUND:
		return "File or folder not found at the specified path";
	case HTTP_CODE_METHOD_NOT_ALLOWED:
		return "Request method not expected (generally should be GET or POST)";
	case HTTP_CODE_TOO_MANY_REQUESTS:
		return "Your app is making too many requests and is being rate limited. 429s can trigger on a per-app or per-user basis";
	case HTTP_CODE_SERVICE_UNAVAILABLE:
		return "If the response includes the Retry-After header, this means your OAuth 1.0 app is being rate limited. Otherwise, this indicates a transient server error, and your app should retry its request.";
	}

	return "Unknown error";
}

void CCloudService::HttpResponseToError(NETLIBHTTPREQUEST *response)
{
	if (response->dataLength)
		throw Exception(response->pData);
	throw Exception(HttpStatusToError(response->resultCode));
}

void CCloudService::HandleHttpError(NETLIBHTTPREQUEST *response)
{
	if (response == nullptr)
		throw Exception(HttpStatusToError());

	if (!HTTP_CODE_SUCCESS(response->resultCode))
		HttpResponseToError(response);
}

JSONNode CCloudService::GetJsonResponse(NETLIBHTTPREQUEST *response)
{
	HandleHttpError(response);

	JSONNode root = JSONNode::parse(response->pData);
	if (root.isnull())
		throw Exception(HttpStatusToError());

	HandleJsonError(root);

	return root;
}
