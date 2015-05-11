#include "stdafx.h"

wchar_t *CDropbox::HttpStatusToText(HTTP_STATUS status)
{
	switch (status) {
	case HTTP_STATUS_OK:
		return L"OK";
	case HTTP_STATUS_BAD_REQUEST:
		return L"Bad input parameter. Error message should indicate which one and why";
	case HTTP_STATUS_UNAUTHORIZED:
		return L"Bad or expired token. This can happen if the user or Dropbox revoked or expired an access token. To fix, you should re-authenticate the user";
	case HTTP_STATUS_FORBIDDEN:
		return L"Bad OAuth request (wrong consumer key, bad nonce, expired timestamp...). Unfortunately, re-authenticating the user won't help here";
	case HTTP_STATUS_NOT_FOUND:
		return L"File or folder not found at the specified path";
	case HTTP_STATUS_METHOD_NOT_ALLOWED:
		return L"Request method not expected (generally should be GET or POST)";
	case HTTP_STATUS_TOO_MANY_REQUESTS:
		return L"Your app is making too many requests and is being rate limited. 429s can trigger on a per-app or per-user basis";
	case HTTP_STATUS_SERVICE_UNAVAILABLE:
		return L"If the response includes the Retry-After header, this means your OAuth 1.0 app is being rate limited. Otherwise, this indicates a transient server error, and your app should retry its request.";
	case HTTP_STATUS_INSUFICIENTE_STORAGE:
		return L"User is over Dropbox storage quota";
	}

	return L"Unknown";
}

int CDropbox::HandleHttpResponseError(HANDLE hNetlibUser, NETLIBHTTPREQUEST *response)
{
	if (!response) {
		Netlib_Logf(hNetlibUser, "%s: %s", MODULE, "Server does not respond");
		return ACKRESULT_FAILED;
	}

	if (response->resultCode != HTTP_STATUS_OK) {
		Netlib_Logf(hNetlibUser, "%s: %s", MODULE, HttpStatusToText((HTTP_STATUS)response->resultCode));
		return response->resultCode;
	}

	return 0;
}
