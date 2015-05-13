#include "stdafx.h"

char* CDropbox::HttpStatusToText(HTTP_STATUS status)
{
	switch (status)
	{
	case HTTP_STATUS_ERROR:
		return "Server does not respond";
	case HTTP_STATUS_OK:
		return "OK";
	case HTTP_STATUS_BAD_REQUEST:
		return "Bad input parameter. Error message should indicate which one and why";
	case HTTP_STATUS_UNAUTHORIZED:
		return "Bad or expired token. This can happen if the user or Dropbox revoked or expired an access token. To fix, you should re-authenticate the user";
	case HTTP_STATUS_FORBIDDEN:
		return "Bad OAuth request (wrong consumer key, bad nonce, expired timestamp...). Unfortunately, re-authenticating the user won't help here";
	case HTTP_STATUS_NOT_FOUND:
		return "File or folder not found at the specified path";
	case HTTP_STATUS_METHOD_NOT_ALLOWED:
		return "Request method not expected (generally should be GET or POST)";
	case HTTP_STATUS_TOO_MANY_REQUESTS:
		return "Your app is making too many requests and is being rate limited. 429s can trigger on a per-app or per-user basis";
	case HTTP_STATUS_SERVICE_UNAVAILABLE:
		return "If the response includes the Retry-After header, this means your OAuth 1.0 app is being rate limited. Otherwise, this indicates a transient server error, and your app should retry its request.";
	case HTTP_STATUS_INSUFICIENTE_STORAGE:
		return "User is over Dropbox storage quota";
	}

	return "Unknown error";
}

void CDropbox::HandleHttpResponseError(NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		throw TransferException(HttpStatusToText(HTTP_STATUS_ERROR));

	if (response->resultCode != HTTP_STATUS_OK)
		throw TransferException(HttpStatusToText((HTTP_STATUS)response->resultCode));
}
