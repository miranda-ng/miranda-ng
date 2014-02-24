#include "common.h"

wchar_t *CDropbox::HttpStatusToText(HTTP_STATUS status)
{
	switch (status)
	{
	case OK:
		return TranslateT("Ok");
	case BAD_REQUEST:
		return TranslateT("Bad input parameter. Error message should indicate which one and why");
	case UNAUTHORIZED:
		return TranslateT("Bad or expired token. This can happen if the user or Dropbox revoked or expired an access token. To fix, you should re-authenticate the user");
	case FORBIDDEN:
		return TranslateT("Bad OAuth request (wrong consumer key, bad nonce, expired timestamp...). Unfortunately, re-authenticating the user won't help here");
	case NOT_FOUND:
		return TranslateT("File or folder not found at the specified path");
	case METHOD_NOT_ALLOWED:
		return TranslateT("Request method not expected (generally should be GET or POST)");
	case TOO_MANY_REQUESTS:
		return TranslateT("Your app is making too many requests and is being rate limited. 429s can trigger on a per-app or per-user basis");
	case SERVICE_UNAVAILABLE:
		return TranslateT("If the response includes the Retry-After header, this means your OAuth 1.0 app is being rate limited. Otherwise, this indicates a transient server error, and your app should retry its request.");
	case INSUFICIENTE_STORAGE:
		return TranslateT("User is over Dropbox storage quota");
	}

	return TranslateT("Unknown");
}

void CDropbox::ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (Miranda_Terminated()) return;

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAW ppd = {0};
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = Skin_GetIcon("Skype_main");

		if (!PUAddPopupW(&ppd))
			return;

	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

void CDropbox::ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(TranslateT(MODULE), message, flags, hContact);
}