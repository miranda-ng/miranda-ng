#include <windows.h>

#include <m_system.h>
#include <m_fontservice.h>

#include "mir_fonts.h"

int FontService_RegisterFont(const char *pszDbModule, const char *pszDbName, const wchar_t *pszSection, const wchar_t *pszDescription, const wchar_t *pszBackgroundGroup, const wchar_t *pszBackgroundName, int position, BOOL bAllowEffects, LOGFONT *plfDefault, COLORREF clrDefault)
{
	FontIDW fid = {};
	mir_strncpy(fid.dbSettingsGroup, pszDbModule, sizeof(fid.dbSettingsGroup)); /* buffer safe */
	mir_strncpy(fid.setting, pszDbName, sizeof(fid.setting)); /* buffer safe */
	mir_wstrncpy(fid.group, pszSection, _countof(fid.group)); /* buffer safe */
	mir_wstrncpy(fid.name, pszDescription, _countof(fid.name)); /* buffer safe */
	mir_wstrncpy(fid.backgroundGroup, pszBackgroundGroup, _countof(fid.backgroundGroup)); /* buffer safe */
	mir_wstrncpy(fid.backgroundName, pszBackgroundName, _countof(fid.backgroundName)); /* buffer safe */
	fid.flags = FIDF_ALLOWREREGISTER;
	if (bAllowEffects) fid.flags |= FIDF_ALLOWEFFECTS;
	fid.order = position;
	if (plfDefault != nullptr) {
		fid.flags |= FIDF_DEFAULTVALID;
		fid.deffontsettings.colour = clrDefault;
		fid.deffontsettings.size = (char)plfDefault->lfHeight;
		if (plfDefault->lfItalic) fid.deffontsettings.style |= DBFONTF_ITALIC;
		if (plfDefault->lfWeight != FW_NORMAL) fid.deffontsettings.style |= DBFONTF_BOLD;
		if (plfDefault->lfUnderline) fid.deffontsettings.style |= DBFONTF_UNDERLINE;
		if (plfDefault->lfStrikeOut) fid.deffontsettings.style |= DBFONTF_STRIKEOUT;
		fid.deffontsettings.charset = plfDefault->lfCharSet;
		mir_wstrncpy(fid.deffontsettings.szFace, plfDefault->lfFaceName, _countof(fid.deffontsettings.szFace)); /* buffer safe */
	}
	Font_RegisterW(&fid, 0);
	return 0;
}

int FontService_RegisterColor(const char *pszDbModule, const char *pszDbName, const wchar_t *pszSection, const wchar_t *pszDescription, COLORREF clrDefault)
{
	ColourIDW cid = {};
	cid.defcolour = clrDefault;
	mir_strncpy(cid.dbSettingsGroup, pszDbModule, sizeof(cid.dbSettingsGroup)); /* buffer safe */
	mir_strncpy(cid.setting, pszDbName, sizeof(cid.setting)); /* buffer safe */
	mir_wstrncpy(cid.group, pszSection, _countof(cid.group)); /* buffer safe */
	mir_wstrncpy(cid.name, pszDescription, _countof(cid.name)); /* buffer safe */
	Colour_RegisterW(&cid, 0);
	return 0;
}
