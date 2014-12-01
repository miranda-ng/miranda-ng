#include <windows.h>

#include <m_system.h>
#include <m_fontservice.h>
#include <win2k.h>

#include "mir_fonts.h"

int FontService_RegisterFont(const char *pszDbModule,const char *pszDbName,const TCHAR *pszSection,const TCHAR *pszDescription,int position,BOOL bAllowEffects,LOGFONT *plfDefault,COLORREF clrDefault)
{
	FontIDT fid;
	memset(&fid, 0, sizeof(fid));
	fid.cbSize=sizeof(fid);
	mir_strncpy(fid.dbSettingsGroup,pszDbModule,sizeof(fid.dbSettingsGroup)); /* buffer safe */
	mir_strncpy(fid.prefix,pszDbName,sizeof(fid.prefix)); /* buffer safe */
	mir_tstrncpy(fid.group,pszSection,SIZEOF(fid.group)); /* buffer safe */
	mir_tstrncpy(fid.name,pszDescription,SIZEOF(fid.name)); /* buffer safe */
	fid.flags=FIDF_ALLOWREREGISTER;
	if(bAllowEffects) fid.flags|=FIDF_ALLOWEFFECTS;
	fid.order=position;
	if(plfDefault!=NULL) {
		fid.flags|=FIDF_DEFAULTVALID;
		fid.deffontsettings.colour=clrDefault;
		fid.deffontsettings.size=(char)plfDefault->lfHeight;
		if(plfDefault->lfItalic) fid.deffontsettings.style|=DBFONTF_ITALIC;
		if(plfDefault->lfWeight!=FW_NORMAL) fid.deffontsettings.style|=DBFONTF_BOLD;
		if(plfDefault->lfUnderline) fid.deffontsettings.style|=DBFONTF_UNDERLINE;
		if(plfDefault->lfStrikeOut) fid.deffontsettings.style|=DBFONTF_STRIKEOUT;
		fid.deffontsettings.charset=plfDefault->lfCharSet;
		mir_tstrncpy(fid.deffontsettings.szFace,plfDefault->lfFaceName,SIZEOF(fid.deffontsettings.szFace)); /* buffer safe */
	}
	FontRegisterT(&fid);
	return 0;
}

int FontService_GetFont(const TCHAR *pszSection,const TCHAR *pszDescription,COLORREF *pclr,LOGFONT *plf)
{
	FontIDT fid;
	fid.cbSize=sizeof(fid);
	mir_tstrncpy(fid.group,pszSection,SIZEOF(fid.group)); /* buffer sfae */
	mir_tstrncpy(fid.name,pszDescription,SIZEOF(fid.name)); /* buffer safe */
	*pclr=(COLORREF)CallService(MS_FONT_GETT,(WPARAM)&fid,(LPARAM)plf); /* uses fallback font on error */
	return (int)*pclr==-1;
}

int FontService_RegisterColor(const char *pszDbModule,const char *pszDbName,const TCHAR *pszSection,const TCHAR *pszDescription,COLORREF clrDefault)
{
	ColourIDT cid;
	memset(&cid, 0, sizeof(cid));
	cid.cbSize=sizeof(cid);
	cid.defcolour=clrDefault;
	mir_strncpy(cid.dbSettingsGroup,pszDbModule,sizeof(cid.dbSettingsGroup)); /* buffer safe */
	mir_strncpy(cid.setting,pszDbName,sizeof(cid.setting)); /* buffer safe */
	mir_tstrncpy(cid.group,pszSection,SIZEOF(cid.group)); /* buffer safe */
	mir_tstrncpy(cid.name,pszDescription,SIZEOF(cid.name)); /* buffer safe */
	ColourRegisterT(&cid);
	return 0;
}

int FontService_GetColor(const TCHAR *pszSection,const TCHAR *pszDescription,COLORREF *pclr)
{
	ColourIDT cid;
	memset(&cid, 0, sizeof(cid));
	cid.cbSize=sizeof(cid);
	mir_tstrncpy(cid.group,pszSection,sizeof(cid.group)); /* buffer safe */
	mir_tstrncpy(cid.name,pszDescription,sizeof(cid.name)); /* buffer safe */
	*pclr=(COLORREF)CallService(MS_COLOUR_GETT,(WPARAM)&cid,0);
	return (int)*pclr==-1;
}
