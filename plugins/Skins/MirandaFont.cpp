#include "MirandaFont.h"

MirandaFont::MirandaFont(Field *aField, const char *description) 
		: field(aField), hFont(NULL)
{
	ZeroMemory(&fid, sizeof(fid));
	lstrcpyn(fid.name, CharToTchar(description), sizeof(fid.name));
}

MirandaFont::~MirandaFont()
{
	releaseFont();
}

void MirandaFont::registerFont(FontState *font)
{
	if (fid.cbSize != 0)
		return;

	MirandaSkinnedDialog * dlg = (MirandaSkinnedDialog *) field->getDialog();

	HDC hdc = GetDC(NULL);

	fid.cbSize = sizeof(fid);
	lstrcpyn(fid.group, CharToTchar(dlg->getDescription()), sizeof(fid.group));
	strncpy(fid.dbSettingsGroup, dlg->getModule(), sizeof(fid.dbSettingsGroup));

	char tmp[sizeof(fid.prefix)];
	mir_snprintf(tmp, sizeof(tmp), "%s%s%sFont", TcharToChar(dlg->getSkinName()), dlg->getName(), field->getName());
	strncpy(fid.prefix, tmp, sizeof(fid.prefix));

	fid.deffontsettings.colour = font->getColor();
	fid.deffontsettings.size = -MulDiv(font->getSize(), GetDeviceCaps(hdc, LOGPIXELSY), 72);
	fid.deffontsettings.style = (font->isBold() ? DBFONTF_BOLD : 0)
		| (font->isItalic() ? DBFONTF_ITALIC : 0)
		| (font->isUnderline() ? DBFONTF_UNDERLINE : 0)
		| (font->isStrikeOut() ? DBFONTF_STRIKEOUT : 0);
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	lstrcpyn(fid.deffontsettings.szFace, font->getFace(), sizeof(fid.deffontsettings.szFace));
	fid.order = dlg->getIndexOf(field);
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;

	CallService(MS_FONT_REGISTERT, (WPARAM)&fid, 0);

	ReleaseDC(NULL, hdc);

	HookEventObj(ME_FONT_RELOAD, staticReloadFont, this);

	reloadFont();
}

void MirandaFont::reloadFont()
{
	releaseFont();

	LOGFONT log_font;
	COLORREF color = (COLORREF) CallService(MS_FONT_GETT, (WPARAM) &fid, (LPARAM) &log_font);
	hFont = CreateFontIndirect(&log_font);

	switch(field->getType())
	{
		case SIMPLE_TEXT:
			((TextField *) field)->setFontColor(color);
			((TextField *) field)->setFont(hFont);
			break;
		case CONTROL_LABEL:
		case CONTROL_BUTTON:
		case CONTROL_EDIT:
			((ControlField *) field)->setFont(hFont);
			break;
	}
}

int MirandaFont::staticReloadFont(void *obj, WPARAM wParam, LPARAM lParam)
{
	((MirandaFont *) obj)->reloadFont();
	return 0;
}

void MirandaFont::releaseFont()
{
	if (hFont != NULL) 
	{
		DeleteObject(hFont);
		hFont = NULL;
	}
}