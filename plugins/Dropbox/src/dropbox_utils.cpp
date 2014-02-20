#include "common.h"

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