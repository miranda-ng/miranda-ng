/*

Miranda IM: the free IM client for Microsoft* Windows*


Copyright 2007 Artem Shpynov
Copyright 2000-2008 Miranda ICQ/IM project, 
all portions of this codebase are copyrighted to the people 
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "hdr/modern_commonheaders.h"
#include "hdr/modern_commonprototypes.h"
#include "./m_api/m_skinbutton.h"
#include <m_toptoolbar.h>
#include "hdr/modern_sync.h"

#define TTB_OPTDIR "TopToolBar"

#if defined(WIN64)
	static char szUrl[] = "http://nightly.miranda.im/x64/toptoolbar.zip";
#else
	static char szUrl[] = "http://nightly.miranda.im/x32/toptoolbar.zip";
#endif

static TCHAR szWarning[] = LPGENT("To view a toolbar in Clist Modern you need the TopToolBar plugin. Click Yes to download it or Cancel to continue");

static void CopySettings(const char* to, const char* from)
{
	db_set_b(NULL, TTB_OPTDIR, to, db_get_b(NULL,"ModernToolBar",from, 0));
}

static void sttRegisterToolBarButton(char * pszButtonID, char * pszButtonName, char * pszServiceName,
	char * pszTooltipUp, char * pszTooltipDn, int icoDefIdx, int defResource, int defResource2, BOOL bVisByDefault)
{
	TTBButton tbb = { 0 };
	static int defPos = 0;
	defPos += 100;
	tbb.cbSize = sizeof(tbb);
	tbb.dwFlags = TTBBF_ICONBYHANDLE;
	if (pszButtonID) {
		tbb.name = pszButtonID;
		tbb.pszService = pszServiceName;

		char buf[255];
		mir_snprintf(buf,SIZEOF(buf),"%s%s%s", "TTB_", pszButtonID, "_dn");
		tbb.hIconHandleUp = RegisterIcolibIconHandle( buf, "ToolBar", pszButtonName , _T("icons\\toolbar_icons.dll"),-icoDefIdx, g_hInst, defResource );

		if (pszTooltipDn) {
			mir_snprintf(buf,SIZEOF(buf),"%s%s%s", "TTB_", pszButtonID, "_up");
			tbb.hIconHandleUp = RegisterIcolibIconHandle( buf, "ToolBar", pszTooltipDn , _T("icons\\toolbar_icons.dll"),-(icoDefIdx+1), g_hInst, defResource2 );
		}
	}
	else tbb.dwFlags |= TTBBF_ISSEPARATOR;

	tbb.dwFlags |= (bVisByDefault ? TTBBF_VISIBLE :0 );
	CallService(MS_TTB_ADDBUTTON,0, (LPARAM)&tbb);
}

static void	ToolBar_DefaultButtonRegistration()
{
	sttRegisterToolBarButton( "MainMenu", "Main Menu", "CList/ShowMainMenu",
		"Main menu", NULL,  100 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );
	
	sttRegisterToolBarButton( "StatusMenu", "Status Menu", "CList/ShowStatusMenu",
		"Status menu", NULL,  105 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	sttRegisterToolBarButton( "AccoMgr", "Accounts", MS_PROTO_SHOWACCMGR,
		"Accounts...", NULL,  282 , IDI_ACCMGR, IDI_ACCMGR, TRUE  );
	
	sttRegisterToolBarButton( "ShowHideOffline","Show/Hide offline contacts", MS_CLIST_TOGGLEHIDEOFFLINE,
					    "Hide offline contacts", "Show offline contacts", 110, IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );
	
	sttRegisterToolBarButton( "DatabaseEditor","DBEditor++", "DBEditorpp/MenuCommand",
		"Database Editor", NULL,  130 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	sttRegisterToolBarButton( "FindUser","Find User", "FindAdd/FindAddCommand",
		"Find User", NULL,  140 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );
	
	sttRegisterToolBarButton( "Options","Options", "Options/OptionsCommand",
		"Options", NULL,  150 , IDI_RESETVIEW, IDI_RESETVIEW, TRUE  );

	sttRegisterToolBarButton( "UseGroups","Use/Disable groups", MS_CLIST_TOGGLEGROUPS,
		"Use groups", "Disable Groups", 160, IDI_RESETVIEW, IDI_RESETVIEW, FALSE  );

	sttRegisterToolBarButton( "EnableSounds","Enable/Disable sounds", MS_CLIST_TOGGLESOUNDS,
		"Enable sounds", "Disable Sounds", 170, IDI_RESETVIEW, IDI_RESETVIEW, FALSE  );
	
	sttRegisterToolBarButton( "Minimize","Minimize", "CList/ShowHide",
		"Minimize", NULL,  180 , IDI_RESETVIEW, IDI_RESETVIEW, FALSE  );
}

HRESULT ToolbarLoadModule()
{
	if ( db_get_b(NULL, "CLUI", "ShowButtonBar", -1) != -1) {
		CopySettings("BUTTWIDTH",    "option_Bar0_BtnWidth");
		CopySettings("BUTTHEIGHT",   "option_Bar0_BtnHeight");
		CopySettings("BUTTGAP",      "option_Bar0_BtnSpace");
		CopySettings("BUTTAUTOSIZE", "option_Bar0_Autosize");
		CopySettings("BUTTMULTI",    "option_Bar0_Multiline");

		db_unset(NULL, "CLUI", "ShowButtonBar");

		CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)"ModernToolBar");

		if (IDYES == MessageBox(NULL, TranslateTS(szWarning), TranslateT("Toolbar upgrade"), MB_ICONQUESTION | MB_YESNO))
			CallService(MS_UTILS_OPENURL, 0, (LPARAM)szUrl);
	}

	ToolBar_DefaultButtonRegistration();
	return S_OK;
}
