/*

Miranda IM: the free IM client for Microsoft* Windows*

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
#include "m_clc.h"
#include "hdr/modern_clc.h"
#include "hdr/modern_commonprototypes.h"
#include "hdr/modern_defsettings.h"
#include "hdr/modern_effectenum.h"
#include "hdr/modern_sync.h"
#include <m_modernopt.h>
#include "m_fontservice.h"

#define FONTF_NORMAL 0
#define FONTF_BOLD   1
#define FONTF_ITALIC 2
#define FONTF_UNDERLINE  4

struct FontOptionsList
{
	DWORD    dwFlags;
	int		 fontID;
	TCHAR*   szGroup;
	TCHAR*   szDescr;
	COLORREF defColour;
	TCHAR*   szDefFace;
	BYTE     defCharset, defStyle;
	char     defSize;
	FONTEFFECT  defeffect;

	COLORREF colour;
	TCHAR    szFace[LF_FACESIZE];
	BYTE     charset, style;
	char     size;
};

#define CLCGROUP			LPGENT("Contact List/Contact names")
#define CLCLINESGROUP		LPGENT("Contact List/Row Items")
#define CLCFRAMESGROUP		LPGENT("Contact List/Frame texts")
#define CLCCOLOURSGROUP     LPGENT("Contact List/Special colours")

#define DEFAULT_COLOUR		RGB(0, 0, 0)
#define DEFAULT_GREYCOLOUR	RGB(128, 128, 128)
#define DEFAULT_BACKCOLOUR	RGB(255, 255, 255)

#define DEFAULT_FAMILY		_T("Arial")
#define DEFAULT_EFFECT		{ 0, 0x00000000, 0x00000000 }



#define DEFAULT_SIZE		-11
#define DEFAULT_SMALLSIZE	-8

static struct FontOptionsList fontOptionsList[] = {

		{ FIDF_CLASSGENERAL, FONTID_CONTACTS,              CLCGROUP,       LPGENT( "Standard contacts"),                                           DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_AWAY,                  CLCGROUP,       LPGENT( "Away contacts"),                                               DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_DND,                   CLCGROUP,       LPGENT( "DND contacts"),                                                DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_NA,                    CLCGROUP,       LPGENT( "NA contacts"),                                                 DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_OCCUPIED,              CLCGROUP,       LPGENT( "Occupied contacts"),                                           DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_CHAT,                  CLCGROUP,       LPGENT( "Free for chat contacts"),                                      DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_INVISIBLE,             CLCGROUP,       LPGENT( "Invisible contacts"),                                          DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_PHONE,                 CLCGROUP,       LPGENT( "On the phone contacts"),                                       DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_LUNCH,                 CLCGROUP,       LPGENT( "Out to lunch contacts"),                                       DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_OFFLINE,               CLCGROUP,       LPGENT( "Offline contacts"),                                            DEFAULT_GREYCOLOUR, DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_INVIS,                 CLCGROUP,       LPGENT( "Online contacts to whom you have a different visibility"),     DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },		
		{ FIDF_CLASSGENERAL, FONTID_OFFINVIS,              CLCGROUP,       LPGENT( "Offline contacts to whom you have a different visibility"),    DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_NOTONLIST,             CLCGROUP,       LPGENT( "Contacts who are 'not on list'"),                              DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSHEADER,  FONTID_OPENGROUPS,            CLCGROUP,       LPGENT( "Open groups"),                                                 DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_BOLD, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSHEADER,  FONTID_OPENGROUPCOUNTS,       CLCGROUP,       LPGENT( "Open group member counts"),                                    DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSHEADER,  FONTID_CLOSEDGROUPS,          CLCGROUP,       LPGENT( "Closed groups"),                                               DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_BOLD, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSHEADER,  FONTID_CLOSEDGROUPCOUNTS,     CLCGROUP,       LPGENT( "Closed group member counts"),                                  DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSSMALL,   FONTID_DIVIDERS,              CLCGROUP,       LPGENT( "Dividers"),                                                    DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		
        { FIDF_CLASSSMALL,   FONTID_SECONDLINE,            CLCLINESGROUP,  LPGENT( "Second line"),                                                 DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSSMALL,   FONTID_THIRDLINE,             CLCLINESGROUP,  LPGENT( "Third line"),                                                  DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSSMALL,   FONTID_CONTACT_TIME,          CLCLINESGROUP,  LPGENT( "Contact time"),                                                DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SMALLSIZE, DEFAULT_EFFECT },
		
        { FIDF_CLASSGENERAL, FONTID_STATUSBAR_PROTONAME,   CLCFRAMESGROUP, LPGENT( "Status bar text"),                                             DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_EVENTAREA,             CLCFRAMESGROUP, LPGENT( "Event area text"),                                             DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
		{ FIDF_CLASSGENERAL, FONTID_VIEMODES,              CLCFRAMESGROUP, LPGENT( "Current view mode text"),                                      DEFAULT_COLOUR,     DEFAULT_FAMILY, DEFAULT_CHARSET, FONTF_NORMAL, DEFAULT_SIZE, DEFAULT_EFFECT },
};

struct ColourOptionsList
{
	char *   chGroup;
	char *   chName;
	TCHAR*   szGroup;
	TCHAR*   szDescr;
	COLORREF defColour;
};

static struct ColourOptionsList colourOptionsList[] = {

	{ "CLC",			"BkColour",           CLCGROUP,				LPGENT( "Background"),									DEFAULT_BACKCOLOUR },
	{ "CLC",			"Rows_BkColour",      CLCLINESGROUP,		LPGENT( "Background"),									DEFAULT_BACKCOLOUR },
	{ "CLC",			"Frames_BkColour",    CLCFRAMESGROUP,		LPGENT( "Background"),									DEFAULT_BACKCOLOUR},

	{ "CLC",			"HotTextColour",      CLCCOLOURSGROUP,      LPGENT( "Hot text"),									CLCDEFAULT_MODERN_HOTTEXTCOLOUR },
	{ "CLC",			"SelTextColour",      CLCCOLOURSGROUP,      LPGENT( "Selected text"),								CLCDEFAULT_MODERN_SELTEXTCOLOUR },
	{ "CLC",			"QuickSearchColour",  CLCCOLOURSGROUP,      LPGENT( "Quick search text"),							CLCDEFAULT_MODERN_QUICKSEARCHCOLOUR},

	{ "Menu",			"TextColour",		  CLCCOLOURSGROUP,      LPGENT( "Menu text"),									CLCDEFAULT_TEXTCOLOUR},
	{ "Menu",			"SelTextColour",	  CLCCOLOURSGROUP,      LPGENT( "Selected menu text"),							CLCDEFAULT_MODERN_SELTEXTCOLOUR},
	{ "FrameTitleBar",	"TextColour",		  CLCCOLOURSGROUP,      LPGENT( "Frame title text"),							CLCDEFAULT_TEXTCOLOUR },
	{ "StatusBar",		"TextColour",		  CLCCOLOURSGROUP,      LPGENT( "Statusbar text"),								CLCDEFAULT_TEXTCOLOUR},
	{ "ModernSettings", "KeyColor",			  CLCCOLOURSGROUP,      LPGENT( "3rd party frames transparent back colour"),	SETTING_KEYCOLOR_DEFAULT},

};

void RegisterCLUIFonts( void )
{
	static bool registered = false;

	if ( registered ) return;

	FontIDT fontid = {0};
	ColourIDT colourid = {0};
	EffectIDT effectid = {0};
	char idstr[10];
	int i, index = 0;

	fontid.cbSize = FontID_SIZEOF_V2;
	strncpy(fontid.dbSettingsGroup, "CLC", SIZEOF(fontid.dbSettingsGroup));

	effectid.cbSize = sizeof( EffectIDT );
	strncpy(effectid.dbSettingsGroup, "CLC", SIZEOF(effectid.dbSettingsGroup));

	for ( i = 0; i < SIZEOF(fontOptionsList); i++, index++ )
	{
		fontid.flags =  FIDF_DEFAULTVALID | FIDF_APPENDNAME | FIDF_SAVEPOINTSIZE | FIDF_ALLOWEFFECTS | FIDF_ALLOWREREGISTER | FIDF_NOAS;
		fontid.flags |= fontOptionsList[i].dwFlags;

		_tcsncpy(fontid.group, fontOptionsList[i].szGroup, SIZEOF(fontid.group));
		_tcsncpy(fontid.name, fontOptionsList[i].szDescr, SIZEOF(fontid.name));
		sprintf(idstr, "Font%d", fontOptionsList[i].fontID);
		strncpy(fontid.prefix, idstr, SIZEOF(fontid.prefix));
		fontid.order = i+1;

		fontid.deffontsettings.charset = fontOptionsList[i].defCharset;
		fontid.deffontsettings.colour = fontOptionsList[i].defColour;
		fontid.deffontsettings.size = fontOptionsList[i].defSize;
		fontid.deffontsettings.style = fontOptionsList[i].defStyle;
		_tcsncpy(fontid.deffontsettings.szFace, fontOptionsList[i].szDefFace, SIZEOF(fontid.deffontsettings.szFace));

		CallService(MS_FONT_REGISTERT, (WPARAM)&fontid, 0);

		_tcsncpy(effectid.group, fontOptionsList[i].szGroup, SIZEOF(effectid.group));
		_tcsncpy(effectid.name, fontOptionsList[i].szDescr, SIZEOF(effectid.name));
		sprintf(idstr, "Font%d", fontOptionsList[i].fontID);
		strncpy(effectid.setting, idstr, SIZEOF(effectid.setting));
		effectid.order = i + 1;

		effectid.defeffect.effectIndex = fontOptionsList[i].defeffect.effectIndex;
		effectid.defeffect.baseColour = fontOptionsList[i].defeffect.baseColour;
		effectid.defeffect.secondaryColour = fontOptionsList[i].defeffect.secondaryColour;

		CallService(MS_EFFECT_REGISTERT, (WPARAM)&effectid, 0);

	}
	colourid.cbSize = sizeof( ColourIDT );

	for ( i = 0; i < SIZEOF( colourOptionsList); i++ )
	{
		_tcsncpy(colourid.group,          colourOptionsList[i].szGroup, SIZEOF(colourid.group));
		_tcsncpy(colourid.name,           colourOptionsList[i].szDescr, SIZEOF(colourid.group));
		strncpy(colourid.setting,         colourOptionsList[i].chName,  SIZEOF(colourid.setting));
		strncpy(colourid.dbSettingsGroup, colourOptionsList[i].chGroup, SIZEOF(colourid.dbSettingsGroup));
		colourid.defcolour =  colourOptionsList[i].defColour;
		colourid.order = i + 1;
		CallService(MS_COLOUR_REGISTERT, (WPARAM)&colourid, 0);
	}
	registered = true;
}

static INT_PTR CALLBACK DlgProcClistListOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcClistAdditionalOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcStatusBarBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


DWORD GetDefaultExStyle(void)
{
	BOOL param;
	DWORD ret=CLCDEFAULT_EXSTYLE;
	if(SystemParametersInfo(SPI_GETLISTBOXSMOOTHSCROLLING,0,&param,FALSE) && !param)
		ret|=CLS_EX_NOSMOOTHSCROLLING;
	if(SystemParametersInfo(SPI_GETHOTTRACKING,0,&param,FALSE) && !param)
		ret&=~CLS_EX_TRACKSELECT;
	return ret;
}

void GetFontSetting(int i,LOGFONT *lf,COLORREF *colour,BYTE *effect, COLORREF *eColour1,COLORREF *eColour2)
{
	DBVARIANT dbv={0};
	char idstr[32];

	int index = 0;
	for ( index = 0; index < SIZEOF(fontOptionsList); index++ )
	{
		if ( fontOptionsList[index].fontID == i )
			break;
	}
	if ( index == SIZEOF(fontOptionsList) )
		return;

	FontIDT fontid = {0};
	fontid.cbSize = FontID_SIZEOF_V2;
	_tcsncpy( fontid.group, fontOptionsList[index].szGroup, SIZEOF( fontid.group ) );
	_tcsncpy( fontid.name, fontOptionsList[index].szDescr, SIZEOF( fontid.name ) );

	COLORREF col = CallService( MS_FONT_GETT, (WPARAM)&fontid, (LPARAM)lf );

	if ( colour ) *colour = col;

	if (effect)
	{
		mir_snprintf(idstr,SIZEOF(idstr),"Font%dEffect",i);
		*effect=ModernGetSettingByte(NULL,"CLC",idstr,0);
		mir_snprintf(idstr,SIZEOF(idstr),"Font%dEffectCol1",i);
		*eColour1=ModernGetSettingDword(NULL,"CLC",idstr,0);
		mir_snprintf(idstr,SIZEOF(idstr),"Font%dEffectCol2",i);
		*eColour2=ModernGetSettingDword(NULL,"CLC",idstr,0);
	}
}


static INT_PTR CALLBACK DlgProcClistOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcTrayOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcClistWindowOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK DlgProcClistBehaviourOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcSBarOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

static UINT StatusBarExpertControls[]={IDC_STATUSBAR_PER_PROTO, IDC_STATUSBAR_PROTO_LIST, IDC_SBAR_USE_ACCOUNT_SETTINGS, IDC_SBAR_HIDE_ACCOUNT_COMPLETELY};

static struct TabItemOptionConf
{
	TCHAR *name;			// Tab name
	int id;					// Dialog id
	DLGPROC wnd_proc;		// Dialog function
	UINT* expertControls;	// Expert Controls
	int nExpertControls;
	DWORD flag;				// Expertonly
} clist_opt_items[] = 
{ 
	{ LPGENT("General"), IDD_OPT_CLIST, DlgProcClistOpts, NULL, 0, 0},
	{ LPGENT("Tray"), IDD_OPT_TRAY, DlgProcTrayOpts, NULL, 0, 0 },
	{ LPGENT("List"), IDD_OPT_CLC, DlgProcClistListOpts, NULL, 0, 0 },
	{ LPGENT("Window"), IDD_OPT_CLUI, DlgProcClistWindowOpts, NULL, 0, 0 },
	{ LPGENT("Behaviour"), IDD_OPT_CLUI_2, DlgProcClistBehaviourOpts, NULL, 0, 0 },
	{ LPGENT("Status Bar"), IDD_OPT_SBAR, DlgProcSBarOpts, StatusBarExpertControls, SIZEOF(StatusBarExpertControls), 0},	
	{ LPGENT("Additional stuff"), IDD_OPT_META_CLC, DlgProcClistAdditionalOpts, NULL, 0, 0 },
	{ 0 }
};


int ClcOptInit(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp;
	if (MirandaExiting()) return 0;
	ZeroMemory(&odp,sizeof(odp));
	odp.cbSize=sizeof(odp);
	odp.position=0;
	odp.hInstance=g_hInst;
	//odp.ptszGroup=TranslateT("Contact List");
	odp.pszTemplate=MAKEINTRESOURCEA(IDD_OPT_CLC);
	odp.ptszTitle=LPGENT("Contact List");
	odp.pfnDlgProc=DlgProcClistListOpts;
	odp.flags=ODPF_BOLDGROUPS|ODPF_TCHAR;
	//CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);  
	{
		int i;	
		for (i=0; clist_opt_items[i].id!=0; i++)
		{
			odp.pszTemplate=MAKEINTRESOURCEA(clist_opt_items[i].id);
			odp.ptszTab=clist_opt_items[i].name;
			odp.pfnDlgProc=clist_opt_items[i].wnd_proc;
			odp.flags=ODPF_BOLDGROUPS|ODPF_TCHAR|clist_opt_items[i].flag;
			odp.expertOnlyControls = clist_opt_items[i].expertControls;
			odp.nExpertOnlyControls = clist_opt_items[i].nExpertControls;
			CallService(MS_OPT_ADDPAGE,wParam,(LPARAM)&odp);
		}
	}

	if (g_CluiData.fDisableSkinEngine)
	{
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CLIST_LISTBKG);
		odp.ptszGroup = LPGENT("Skins");
		odp.ptszTitle = LPGENT("Contact List");
		odp.ptszTab  = LPGENT("List Background");
		odp.pfnDlgProc = DlgProcClcBkgOpts;
		odp.flags = ODPF_BOLDGROUPS|ODPF_TCHAR;
		CallService(MS_OPT_ADDPAGE, wParam, (LPARAM) & odp);
	}
	
	return 0;
}

struct CheckBoxToStyleEx_t 
{
	int id;
	DWORD flag;
	int neg;
};
static const struct CheckBoxToStyleEx_t checkBoxToStyleEx[]={
	{IDC_DISABLEDRAGDROP,CLS_EX_DISABLEDRAGDROP,0},
	{IDC_NOTEDITLABELS,CLS_EX_EDITLABELS,1},
	{IDC_SHOWSELALWAYS,CLS_EX_SHOWSELALWAYS,0},
	{IDC_TRACKSELECT,CLS_EX_TRACKSELECT,0},
	{IDC_SHOWGROUPCOUNTS,CLS_EX_SHOWGROUPCOUNTS,0},
	{IDC_HIDECOUNTSWHENEMPTY,CLS_EX_HIDECOUNTSWHENEMPTY,0},
	{IDC_DIVIDERONOFF,CLS_EX_DIVIDERONOFF,0},
	{IDC_NOTNOTRANSLUCENTSEL,CLS_EX_NOTRANSLUCENTSEL,1},
	{IDC_LINEWITHGROUPS,CLS_EX_LINEWITHGROUPS,0},
	{IDC_QUICKSEARCHVISONLY,CLS_EX_QUICKSEARCHVISONLY,0},
	{IDC_SORTGROUPSALPHA,CLS_EX_SORTGROUPSALPHA,0},
	{IDC_NOTNOSMOOTHSCROLLING,CLS_EX_NOSMOOTHSCROLLING,1}
};

struct CheckBoxValues_t {
	DWORD style;
	TCHAR *szDescr;
};
static const struct CheckBoxValues_t greyoutValues[]={
	{GREYF_UNFOCUS,LPGENT("Not focused")},
	{MODEF_OFFLINE,LPGENT("Offline")},
	{PF2_ONLINE,LPGENT("Online")},
	{PF2_SHORTAWAY,LPGENT("Away")},
	{PF2_LONGAWAY,LPGENT("NA")},
	{PF2_LIGHTDND,LPGENT("Occupied")},
	{PF2_HEAVYDND,LPGENT("DND")},
	{PF2_FREECHAT,LPGENT("Free for chat")},
	{PF2_INVISIBLE,LPGENT("Invisible")},
	{PF2_OUTTOLUNCH,LPGENT("Out to lunch")},
	{PF2_ONTHEPHONE,LPGENT("On the phone")}
};
static const struct CheckBoxValues_t offlineValues[]=
{
	{MODEF_OFFLINE,LPGENT("Offline")},
	{PF2_ONLINE,LPGENT("Online")},
	{PF2_SHORTAWAY,LPGENT("Away")},
	{PF2_LONGAWAY,LPGENT("NA")},
	{PF2_LIGHTDND,LPGENT("Occupied")},
	{PF2_HEAVYDND,LPGENT("DND")},
	{PF2_FREECHAT,LPGENT("Free for chat")},
	{PF2_INVISIBLE,LPGENT("Invisible")},
	{PF2_OUTTOLUNCH,LPGENT("Out to lunch")},
	{PF2_ONTHEPHONE,LPGENT("On the phone")}
};

static void FillCheckBoxTree(HWND hwndTree,const struct CheckBoxValues_t *values,int nValues,DWORD style)
{
	TVINSERTSTRUCT tvis;
	int i;

	tvis.hParent=NULL;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_STATE|TVIF_IMAGE;
	for(i=0;i<nValues;i++) {
		tvis.item.lParam=values[i].style;
		tvis.item.pszText=TranslateTS(values[i].szDescr);
		tvis.item.stateMask=TVIS_STATEIMAGEMASK;
		tvis.item.state=INDEXTOSTATEIMAGEMASK((style&tvis.item.lParam)!=0?2:1);
		tvis.item.iImage=tvis.item.iSelectedImage=(style&tvis.item.lParam)!=0?1:0;
		TreeView_InsertItem(hwndTree,&tvis);
	}
}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags=0;
	TVITEMA tvi;

	tvi.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_IMAGE;
	tvi.hItem=TreeView_GetRoot(hwndTree);
	while(tvi.hItem) {
		TreeView_GetItem(hwndTree,&tvi);
		if(tvi.iImage) flags|=tvi.lParam;
		tvi.hItem=TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
	return flags;
}

static INT_PTR CALLBACK DlgProcClistAdditionalOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR t;
	t=((LPNMHDR)lParam);
	switch (msg)
	{
	case WM_INITDIALOG:

		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_META, ModernGetSettingByte(NULL,"CLC","Meta",SETTING_USEMETAICON_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METADBLCLK, ModernGetSettingByte(NULL,"CLC","MetaDoubleClick",SETTING_METAAVOIDDBLCLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUBEXTRA, ModernGetSettingByte(NULL,"CLC","MetaHideExtra",SETTING_METAHIDEEXTRA_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUBEXTRA_IGN, ModernGetSettingByte(NULL,"CLC","MetaIgnoreEmptyExtra",SETTING_METAAVOIDDBLCLICK_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METASUB_HIDEOFFLINE, ModernGetSettingByte(NULL,"CLC","MetaHideOfflineSub",SETTING_METAHIDEOFFLINESUB_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_METAEXPAND, ModernGetSettingByte(NULL,"CLC","MetaExpanding",SETTING_METAEXPANDING_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_DISCOVER_AWAYMSG, ModernGetSettingByte(NULL,"ModernData","InternalAwayMsgDiscovery",SETTING_INTERNALAWAYMSGREQUEST_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR
		CheckDlgButton(hwndDlg, IDC_REMOVE_OFFLINE_AWAYMSG, ModernGetSettingByte(NULL,"ModernData","RemoveAwayMessageForOffline",SETTING_REMOVEAWAYMSGFOROFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED); /// by FYR

		SendDlgItemMessage(hwndDlg,IDC_SUBINDENTSPIN,UDM_SETBUDDY,(WPARAM)GetDlgItem(hwndDlg,IDC_SUBINDENT),0);	
		SendDlgItemMessage(hwndDlg,IDC_SUBINDENTSPIN,UDM_SETRANGE,0,MAKELONG(50,0));
		SendDlgItemMessage(hwndDlg,IDC_SUBINDENTSPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingByte(NULL,"CLC","SubIndent",CLCDEFAULT_GROUPINDENT),0));

		{
			BYTE t;
			t=IsDlgButtonChecked(hwndDlg,IDC_METAEXPAND);
			EnableWindow(GetDlgItem(hwndDlg,IDC_METADBLCLK),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_METASUBEXTRA),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_METASUB_HIDEOFFLINE),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SUBINDENTSPIN),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SUBINDENT),t);
		}
		{
			BYTE t;
			t=ServiceExists(MS_MC_GETMOSTONLINECONTACT);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_META),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_METADBLCLK),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_METASUB_HIDEOFFLINE),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_METAEXPAND),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_METASUBEXTRA),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_FRAME_META),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_FRAME_META_CAPT),!t); 
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_SUBINDENTSPIN),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_SUBINDENT),t);
			CLUI_ShowWindowMod(GetDlgItem(hwndDlg,IDC_SUBIDENTCAPT),t);
		}
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_METAEXPAND)
		{
			BYTE t;
			t=IsDlgButtonChecked(hwndDlg,IDC_METAEXPAND);
			EnableWindow(GetDlgItem(hwndDlg,IDC_METADBLCLK),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_METASUBEXTRA),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_METASUB_HIDEOFFLINE),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SUBINDENTSPIN),t);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SUBINDENT),t);
		}
		if((LOWORD(wParam)==IDC_SUBINDENT) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus())) return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		return TRUE;
	case WM_NOTIFY:

		switch(t->idFrom) 
		{
		case 0:
			switch (t->code)
			{
			case PSN_APPLY:
				ModernWriteSettingByte(NULL,"CLC","Meta",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_META)); // by FYR
				ModernWriteSettingByte(NULL,"CLC","MetaDoubleClick",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METADBLCLK)); // by FYR
				ModernWriteSettingByte(NULL,"CLC","MetaHideExtra",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METASUBEXTRA)); // by FYR
				ModernWriteSettingByte(NULL,"CLC","MetaIgnoreEmptyExtra",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METASUBEXTRA_IGN)); // by FYR
				ModernWriteSettingByte(NULL,"CLC","MetaHideOfflineSub",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METASUB_HIDEOFFLINE)); // by FYR					
				ModernWriteSettingByte(NULL,"CLC","MetaExpanding",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METAEXPAND));
				ModernWriteSettingByte(NULL,"ModernData","InternalAwayMsgDiscovery",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DISCOVER_AWAYMSG));
				ModernWriteSettingByte(NULL,"ModernData","RemoveAwayMessageForOffline",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_REMOVE_OFFLINE_AWAYMSG));

				ModernWriteSettingByte(NULL,"CLC","SubIndent",(BYTE)SendDlgItemMessage(hwndDlg,IDC_SUBINDENTSPIN,UDM_GETPOS,0,0));
				ClcOptionsChanged();
				CLUI_ReloadCLUIOptions();
				PostMessage(pcli->hwndContactList,WM_SIZE,0,0);

				return TRUE;
			}
			break;
		}
		break;
	}
	return FALSE;




}

static INT_PTR CALLBACK DlgProcClistListOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),GWL_STYLE,GetWindowLong(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),GWL_STYLE)|TVS_NOHSCROLL);
		SetWindowLong(GetDlgItem(hwndDlg,IDC_HIDEOFFLINEOPTS),GWL_STYLE,GetWindowLong(GetDlgItem(hwndDlg,IDC_HIDEOFFLINEOPTS),GWL_STYLE)|TVS_NOHSCROLL);
		{
			HIMAGELIST himlCheckBoxes;
			himlCheckBoxes=ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),ILC_COLOR32|ILC_MASK,2,2);
			ImageList_AddIcon(himlCheckBoxes,LoadSmallIconShared(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_NOTICK)));
			ImageList_AddIcon(himlCheckBoxes,LoadSmallIconShared(GetModuleHandle(NULL),MAKEINTRESOURCE(IDI_TICK)));
			TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),himlCheckBoxes,TVSIL_NORMAL);
			TreeView_SetImageList(GetDlgItem(hwndDlg,IDC_HIDEOFFLINEOPTS),himlCheckBoxes,TVSIL_NORMAL);
		}			
		{
			DWORD exStyle=ModernGetSettingDword(NULL,"CLC","ExStyle",GetDefaultExStyle());
			for(int i=0;i<SIZEOF(checkBoxToStyleEx);i++)
				CheckDlgButton(hwndDlg,checkBoxToStyleEx[i].id,(exStyle&checkBoxToStyleEx[i].flag)^(checkBoxToStyleEx[i].flag*checkBoxToStyleEx[i].neg)?BST_CHECKED:BST_UNCHECKED);
		}
		{
			UDACCEL accel[2]={{0,10},{2,50}};
			SendDlgItemMessage(hwndDlg,IDC_SMOOTHTIMESPIN,UDM_SETRANGE,0,MAKELONG(999,0));
			SendDlgItemMessage(hwndDlg,IDC_SMOOTHTIMESPIN,UDM_SETACCEL,SIZEOF(accel),(LPARAM)&accel);
			SendDlgItemMessage(hwndDlg,IDC_SMOOTHTIMESPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"CLC","ScrollTime",CLCDEFAULT_SCROLLTIME),0));
		}
		CheckDlgButton(hwndDlg,IDC_IDLE,ModernGetSettingByte(NULL,"CLC","ShowIdle",CLCDEFAULT_SHOWIDLE)?BST_CHECKED:BST_UNCHECKED);

		SendDlgItemMessage(hwndDlg,IDC_GROUPINDENTSPIN,UDM_SETRANGE,0,MAKELONG(50,0));
		SendDlgItemMessage(hwndDlg,IDC_GROUPINDENTSPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingByte(NULL,"CLC","GroupIndent",CLCDEFAULT_GROUPINDENT),0));
		CheckDlgButton(hwndDlg,IDC_GREYOUT,ModernGetSettingDword(NULL,"CLC","GreyoutFlags",CLCDEFAULT_GREYOUTFLAGS)?BST_CHECKED:BST_UNCHECKED);

		EnableWindow(GetDlgItem(hwndDlg,IDC_SMOOTHTIME),IsDlgButtonChecked(hwndDlg,IDC_NOTNOSMOOTHSCROLLING));
		EnableWindow(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),IsDlgButtonChecked(hwndDlg,IDC_GREYOUT));
		FillCheckBoxTree(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),greyoutValues,SIZEOF(greyoutValues),ModernGetSettingDword(NULL,"CLC","FullGreyoutFlags",CLCDEFAULT_FULLGREYOUTFLAGS));
		FillCheckBoxTree(GetDlgItem(hwndDlg,IDC_HIDEOFFLINEOPTS),offlineValues,SIZEOF(offlineValues),ModernGetSettingDword(NULL,"CLC","OfflineModes",CLCDEFAULT_OFFLINEMODES));
		CheckDlgButton(hwndDlg,IDC_NOSCROLLBAR,ModernGetSettingByte(NULL,"CLC","NoVScrollBar",CLCDEFAULT_NOVSCROLL)?BST_CHECKED:BST_UNCHECKED);
		return TRUE;
	case WM_VSCROLL:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_NOTNOSMOOTHSCROLLING)
			EnableWindow(GetDlgItem(hwndDlg,IDC_SMOOTHTIME),IsDlgButtonChecked(hwndDlg,IDC_NOTNOSMOOTHSCROLLING));
		if(LOWORD(wParam)==IDC_GREYOUT)
			EnableWindow(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),IsDlgButtonChecked(hwndDlg,IDC_GREYOUT));
		if((/*LOWORD(wParam)==IDC_LEFTMARGIN ||*/ LOWORD(wParam)==IDC_SMOOTHTIME || LOWORD(wParam)==IDC_GROUPINDENT) && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus())) return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
	case IDC_GREYOUTOPTS:
	case IDC_HIDEOFFLINEOPTS:
		if(((LPNMHDR)lParam)->code==NM_CLICK) {
			TVHITTESTINFO hti;
			hti.pt.x=(short)LOWORD(GetMessagePos());
			hti.pt.y=(short)HIWORD(GetMessagePos());
			ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
			if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti))
				if(hti.flags&TVHT_ONITEMICON) {
					TVITEMA tvi;
					tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
					tvi.hItem=hti.hItem;
					TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
					tvi.iImage=tvi.iSelectedImage=tvi.iImage=!tvi.iImage;
					//tvi.state=tvi.iImage?2:1;
					TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
				}
		}
		break;
	case 0:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			{
				DWORD exStyle=0;
				for(int i=0;i<SIZEOF(checkBoxToStyleEx);i++)
					if((IsDlgButtonChecked(hwndDlg,checkBoxToStyleEx[i].id)==0)==checkBoxToStyleEx[i].neg)
						exStyle|=checkBoxToStyleEx[i].flag;
				ModernWriteSettingDword(NULL,"CLC","ExStyle",exStyle);
			}
			{	DWORD fullGreyoutFlags=MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS));
			ModernWriteSettingDword(NULL,"CLC","FullGreyoutFlags",fullGreyoutFlags);
			if(IsDlgButtonChecked(hwndDlg,IDC_GREYOUT))
				ModernWriteSettingDword(NULL,"CLC","GreyoutFlags",fullGreyoutFlags);
			else
				ModernWriteSettingDword(NULL,"CLC","GreyoutFlags",0);
			}
			/*						DBWriteContactSettingByte(NULL,"CLC","Meta",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_META)); // by FYR
			DBWriteContactSettingByte(NULL,"CLC","MetaDoubleClick",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METADBLCLK)); // by FYR
			DBWriteContactSettingByte(NULL,"CLC","MetaHideExtra",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_METASUBEXTRA)); // by FYR

			*/						
			ModernWriteSettingByte(NULL,"CLC","ShowIdle",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_IDLE)?1:0));
			ModernWriteSettingDword(NULL,"CLC","OfflineModes",MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg,IDC_HIDEOFFLINEOPTS)));
			//						DBWriteContactSettingByte(NULL,"CLC","LeftMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_LEFTMARGINSPIN,UDM_GETPOS,0,0));
			ModernWriteSettingWord(NULL,"CLC","ScrollTime",(WORD)SendDlgItemMessage(hwndDlg,IDC_SMOOTHTIMESPIN,UDM_GETPOS,0,0));
			ModernWriteSettingByte(NULL,"CLC","GroupIndent",(BYTE)SendDlgItemMessage(hwndDlg,IDC_GROUPINDENTSPIN,UDM_GETPOS,0,0));
			ModernWriteSettingByte(NULL,"CLC","NoVScrollBar",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_NOSCROLLBAR)?1:0));


			ClcOptionsChanged();
			return TRUE;
		}
		break;
		}
		break;
	case WM_DESTROY:
		ImageList_Destroy(TreeView_GetImageList(GetDlgItem(hwndDlg,IDC_GREYOUTOPTS),TVSIL_NORMAL));
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcStatusBarBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg,IDC_BITMAP,ModernGetSettingByte(NULL,"StatusBar","UseBitmap",CLCDEFAULT_USEBITMAP)?BST_CHECKED:BST_UNCHECKED);
		SendMessage(hwndDlg,WM_USER+10,0,0);
		SendDlgItemMessage(hwndDlg,IDC_BKGCOLOUR,CPM_SETDEFAULTCOLOUR,0,CLCDEFAULT_BKCOLOUR);
		//		SendDlgItemMessage(hwndDlg,IDC_BKGCOLOUR,CPM_SETCOLOUR,0,DBGetContactSettingDword(NULL,"StatusBar","BkColour",CLCDEFAULT_BKCOLOUR));
		SendDlgItemMessage(hwndDlg,IDC_SELCOLOUR,CPM_SETDEFAULTCOLOUR,0,CLCDEFAULT_SELBKCOLOUR);
		SendDlgItemMessage(hwndDlg,IDC_SELCOLOUR,CPM_SETCOLOUR,0,ModernGetSettingDword(NULL,"StatusBar","SelBkColour",CLCDEFAULT_SELBKCOLOUR));
		{	DBVARIANT dbv={0};
		if(!ModernGetSettingString(NULL,"StatusBar","BkBitmap",&dbv)) {
			SetDlgItemTextA(hwndDlg,IDC_FILENAME,dbv.pszVal);
			if (ServiceExists(MS_UTILS_PATHTOABSOLUTE)) {
				char szPath[MAX_PATH];

				if (CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)szPath))
					SetDlgItemTextA(hwndDlg,IDC_FILENAME,szPath);
			}
			ModernDBFreeVariant(&dbv);
		}
		}

		CheckDlgButton(hwndDlg,IDC_HILIGHTMODE,ModernGetSettingByte(NULL,"StatusBar","HiLightMode",SETTING_SBHILIGHTMODE_DEFAULT)==0?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_HILIGHTMODE1,ModernGetSettingByte(NULL,"StatusBar","HiLightMode",SETTING_SBHILIGHTMODE_DEFAULT)==1?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_HILIGHTMODE2,ModernGetSettingByte(NULL,"StatusBar","HiLightMode",SETTING_SBHILIGHTMODE_DEFAULT)==2?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_HILIGHTMODE3,ModernGetSettingByte(NULL,"StatusBar","HiLightMode",SETTING_SBHILIGHTMODE_DEFAULT)==3?BST_CHECKED:BST_UNCHECKED);



		{	WORD bmpUse=ModernGetSettingWord(NULL,"StatusBar","BkBmpUse",CLCDEFAULT_BKBMPUSE);
		CheckDlgButton(hwndDlg,IDC_STRETCHH,bmpUse&CLB_STRETCHH?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_STRETCHV,bmpUse&CLB_STRETCHV?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_TILEH,bmpUse&CLBF_TILEH?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_TILEV,bmpUse&CLBF_TILEV?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_SCROLL,bmpUse&CLBF_SCROLL?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_PROPORTIONAL,bmpUse&CLBF_PROPORTIONAL?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(hwndDlg,IDC_TILEVROWH,bmpUse&CLBF_TILEVTOROWHEIGHT?BST_CHECKED:BST_UNCHECKED);

		}
		{	HRESULT (STDAPICALLTYPE *MySHAutoComplete)(HWND,DWORD);
		MySHAutoComplete=(HRESULT (STDAPICALLTYPE*)(HWND,DWORD))GetProcAddress(GetModuleHandle(TEXT("shlwapi")),"SHAutoComplete");
		if(MySHAutoComplete) MySHAutoComplete(GetDlgItem(hwndDlg,IDC_FILENAME),1);
		}
		return TRUE;
	case WM_USER+10:
		EnableWindow(GetDlgItem(hwndDlg,IDC_FILENAME),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_BROWSE),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_STRETCHH),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_STRETCHV),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_TILEH),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_TILEV),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_SCROLL),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_PROPORTIONAL),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		EnableWindow(GetDlgItem(hwndDlg,IDC_TILEVROWH),IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
		break;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_BROWSE) {
			char str[MAX_PATH];
			OPENFILENAMEA ofn={0};
			char filter[512];

			GetDlgItemTextA(hwndDlg,IDC_FILENAME,str,SIZEOF(str));
			ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
			ofn.hwndOwner = hwndDlg;
			ofn.hInstance = NULL;
			CallService(MS_UTILS_GETBITMAPFILTERSTRINGS,SIZEOF(filter),(LPARAM)filter);
			ofn.lpstrFilter = filter;
			ofn.lpstrFile = str;
			ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.nMaxFile = SIZEOF(str);
			ofn.nMaxFileTitle = MAX_PATH;
			ofn.lpstrDefExt = "bmp";
			if(!GetOpenFileNameA(&ofn)) break;
			SetDlgItemTextA(hwndDlg,IDC_FILENAME,str);
		}
		else if(LOWORD(wParam)==IDC_FILENAME && HIWORD(wParam)!=EN_CHANGE) break;
		if(LOWORD(wParam)==IDC_BITMAP) SendMessage(hwndDlg,WM_USER+10,0,0);
		if(LOWORD(wParam)==IDC_FILENAME && (HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus())) return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
	case 0:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:



			ModernWriteSettingByte(NULL,"StatusBar","UseBitmap",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_BITMAP));
			{	COLORREF col;
			col=SendDlgItemMessage(hwndDlg,IDC_BKGCOLOUR,CPM_GETCOLOUR,0,0);
			if(col==CLCDEFAULT_BKCOLOUR) ModernDeleteSetting(NULL,"StatusBar","BkColour");
			else ModernWriteSettingDword(NULL,"StatusBar","BkColour",col);
			col=SendDlgItemMessage(hwndDlg,IDC_SELCOLOUR,CPM_GETCOLOUR,0,0);
			if(col==CLCDEFAULT_SELBKCOLOUR) ModernDeleteSetting(NULL,"StatusBar","SelBkColour");
			else ModernWriteSettingDword(NULL,"StatusBar","SelBkColour",col);
			}
			{	
				char str[MAX_PATH],strrel[MAX_PATH];
				GetDlgItemTextA(hwndDlg,IDC_FILENAME,str,SIZEOF(str));
				if (ServiceExists(MS_UTILS_PATHTORELATIVE)) {
					if (CallService(MS_UTILS_PATHTORELATIVE, (WPARAM)str, (LPARAM)strrel))
						ModernWriteSettingString(NULL,"StatusBar","BkBitmap",strrel);
					else ModernWriteSettingString(NULL,"StatusBar","BkBitmap",str);
				}
				else ModernWriteSettingString(NULL,"StatusBar","BkBitmap",str);

			}
			{	WORD flags=0;
			if(IsDlgButtonChecked(hwndDlg,IDC_STRETCHH)) flags|=CLB_STRETCHH;
			if(IsDlgButtonChecked(hwndDlg,IDC_STRETCHV)) flags|=CLB_STRETCHV;
			if(IsDlgButtonChecked(hwndDlg,IDC_TILEH)) flags|=CLBF_TILEH;
			if(IsDlgButtonChecked(hwndDlg,IDC_TILEV)) flags|=CLBF_TILEV;
			if(IsDlgButtonChecked(hwndDlg,IDC_SCROLL)) flags|=CLBF_SCROLL;
			if(IsDlgButtonChecked(hwndDlg,IDC_PROPORTIONAL)) flags|=CLBF_PROPORTIONAL;
			if(IsDlgButtonChecked(hwndDlg,IDC_TILEVROWH)) flags|=CLBF_TILEVTOROWHEIGHT;

			ModernWriteSettingWord(NULL,"StatusBar","BkBmpUse",flags);
			}
			{
				int hil=0;
				if (IsDlgButtonChecked(hwndDlg,IDC_HILIGHTMODE1))  hil=1;
				if (IsDlgButtonChecked(hwndDlg,IDC_HILIGHTMODE2))  hil=2;
				if (IsDlgButtonChecked(hwndDlg,IDC_HILIGHTMODE3))  hil=3;

				ModernWriteSettingByte(NULL,"StatusBar","HiLightMode",(BYTE)hil);

			}

			ClcOptionsChanged();
			//OnStatusBarBackgroundChange();
			return TRUE;
		}
		break;
		}
		break;
	}
	return FALSE;
}

static int _GetNetVisibleProtoCount()
{
	int i,count,netProtoCount;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts( &count, &accs );
	for(i=0,netProtoCount=0;i<count;i++) 
	{
		if ( pcli->pfnGetProtocolVisibility(accs[i]->szModuleName)==0 ) continue;
		netProtoCount++;
	}
	return netProtoCount;
}
TCHAR *sortby[]={_T("Name"), _T("Name (use locale settings)") , _T("Status"), _T("Last message time"), _T("Account Name"), _T("Rate"), _T("-Nothing-")};
int sortbyValue[]={ SORTBY_NAME, SORTBY_NAME_LOCALE, SORTBY_STATUS, SORTBY_LASTMSG, SORTBY_PROTO ,SORTBY_RATE , SORTBY_NOTHING };
static INT_PTR CALLBACK DlgProcClistOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
        {
            TranslateDialogDefault(hwndDlg);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)HookEventMessage(ME_DB_CONTACT_SETTINGCHANGED,hwndDlg,WM_USER+1));

            CheckDlgButton(hwndDlg, IDC_HIDEOFFLINE, ModernGetSettingByte(NULL,"CList","HideOffline",SETTING_HIDEOFFLINE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_HIDEEMPTYGROUPS, ModernGetSettingByte(NULL,"CList","HideEmptyGroups",SETTING_HIDEEMPTYGROUPS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_DISABLEGROUPS, ModernGetSettingByte(NULL,"CList","UseGroups",SETTING_USEGROUPS_DEFAULT) ? BST_UNCHECKED : BST_CHECKED);
            CheckDlgButton(hwndDlg, IDC_CONFIRMDELETE, ModernGetSettingByte(NULL,"CList","ConfirmDelete",SETTING_CONFIRMDELETE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hwndDlg,IDC_GAMMACORRECT,ModernGetSettingByte(NULL,"CLC","GammaCorrect",CLCDEFAULT_GAMMACORRECT)?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hwndDlg,IDC_HILIGHTMODE,ModernGetSettingByte(NULL,"CLC","HiLightMode",SETTING_HILIGHTMODE_DEFAULT)==0?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hwndDlg,IDC_HILIGHTMODE1,ModernGetSettingByte(NULL,"CLC","HiLightMode",SETTING_HILIGHTMODE_DEFAULT)==1?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hwndDlg,IDC_HILIGHTMODE2,ModernGetSettingByte(NULL,"CLC","HiLightMode",SETTING_HILIGHTMODE_DEFAULT)==2?BST_CHECKED:BST_UNCHECKED);
            CheckDlgButton(hwndDlg,IDC_HILIGHTMODE3,ModernGetSettingByte(NULL,"CLC","HiLightMode",SETTING_HILIGHTMODE_DEFAULT)==3?BST_CHECKED:BST_UNCHECKED);

            int i, item;
            int s1, s2, s3;
            for (i=0; i<SIZEOF(sortby); i++) 
            {
                item=SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_ADDSTRING,0,(LPARAM)TranslateTS(sortby[i]));
                SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_SETITEMDATA,item,(LPARAM)0);
                item=SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_ADDSTRING,0,(LPARAM)TranslateTS(sortby[i]));
                SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_SETITEMDATA,item,(LPARAM)0);
                item=SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_ADDSTRING,0,(LPARAM)TranslateTS(sortby[i]));
                SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_SETITEMDATA,item,(LPARAM)0);

            }
            s1=ModernGetSettingByte(NULL,"CList","SortBy1",SETTING_SORTBY1_DEFAULT);
            s2=ModernGetSettingByte(NULL,"CList","SortBy2",SETTING_SORTBY2_DEFAULT);
            s3=ModernGetSettingByte(NULL,"CList","SortBy3",SETTING_SORTBY3_DEFAULT);

            for (i=0; i<SIZEOF(sortby); i++) 
            {
                if (s1==sortbyValue[i])
                    SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_SETCURSEL,i,0);
                if (s2==sortbyValue[i])
                    SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_SETCURSEL,i,0);
                if (s3==sortbyValue[i])
                    SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_SETCURSEL,i,0);		
            }

            CheckDlgButton(hwndDlg, IDC_NOOFFLINEMOVE, ModernGetSettingByte(NULL,"CList","NoOfflineBottom",SETTING_NOOFFLINEBOTTOM_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_OFFLINETOROOT, ModernGetSettingByte(NULL,"CList","PlaceOfflineToRoot",SETTING_PLACEOFFLINETOROOT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
        }
		return TRUE;
	case WM_COMMAND:
        {
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
        }
		break;
	case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->idFrom) 
            {
            case 0:
                switch (((LPNMHDR)lParam)->code)
                {
                case PSN_APPLY:
                    ModernWriteSettingByte(NULL,"CList","HideOffline",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_HIDEOFFLINE));                                    
                    ModernWriteSettingByte(NULL,"CList","HideEmptyGroups",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_HIDEEMPTYGROUPS));
                    ModernWriteSettingByte(NULL,"CList","UseGroups",(BYTE)!IsDlgButtonChecked(hwndDlg,IDC_DISABLEGROUPS));
                    ModernWriteSettingByte(NULL,"CList","ConfirmDelete",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CONFIRMDELETE));

                    ModernWriteSettingByte(NULL,"CLC","GammaCorrect",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_GAMMACORRECT));
                    int hil=0;
                    if (IsDlgButtonChecked(hwndDlg,IDC_HILIGHTMODE1))  hil=1;
                    if (IsDlgButtonChecked(hwndDlg,IDC_HILIGHTMODE2))  hil=2;
                    if (IsDlgButtonChecked(hwndDlg,IDC_HILIGHTMODE3))  hil=3;
                    ModernWriteSettingByte(NULL,"CLC","HiLightMode",(BYTE)hil);

                    int s1=SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_GETCURSEL,0,0);
                    int s2=SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_GETCURSEL,0,0);
                    int s3=SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_GETCURSEL,0,0);
                    if (s1>=0) s1=sortbyValue[s1];
                    if (s2>=0) s2=sortbyValue[s2];
                    if (s3>=0) s3=sortbyValue[s3];
                    ModernWriteSettingByte(NULL,"CList","SortBy1",(BYTE)s1);
                    ModernWriteSettingByte(NULL,"CList","SortBy2",(BYTE)s2);
                    ModernWriteSettingByte(NULL,"CList","SortBy3",(BYTE)s3);

                    ModernWriteSettingByte(NULL,"CList","NoOfflineBottom",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_NOOFFLINEMOVE));
                    ModernWriteSettingByte(NULL,"CList","PlaceOfflineToRoot",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_OFFLINETOROOT));

                    pcli->pfnLoadContactTree(); /* this won't do job properly since it only really works when changes happen */
                    SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0); /* force reshuffle */
                    ClcOptionsChanged(); // Used to force loading avatar an list height related options
                    return TRUE;
                }
                break;
            }
            break;
        }
	}
	return FALSE;
}



static INT_PTR CALLBACK DlgProcTrayOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        {
            TranslateDialogDefault(hwndDlg);
            CheckDlgButton(hwndDlg, IDC_NOOFFLINEMOVE, ModernGetSettingByte( NULL,"CList", "NoOfflineBottom",    SETTING_NOOFFLINEBOTTOM_DEFAULT ) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_OFFLINETOROOT, ModernGetSettingByte( NULL,"CList", "PlaceOfflineToRoot", SETTING_PLACEOFFLINETOROOT_DEFAULT ) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ONECLK,        ModernGetSettingByte( NULL,"CList", "Tray1Click",         SETTING_TRAY1CLICK_DEFAULT ) ? BST_CHECKED : BST_UNCHECKED);
            {
                BYTE trayOption=ModernGetSettingByte(NULL,"CLUI","XStatusTray",SETTING_TRAYOPTION_DEFAULT);
                CheckDlgButton(hwndDlg, IDC_SHOWXSTATUS, (trayOption&3) ? BST_CHECKED : BST_UNCHECKED);
                CheckDlgButton(hwndDlg, IDC_SHOWNORMAL,  (trayOption&2) ? BST_CHECKED : BST_UNCHECKED);
                CheckDlgButton(hwndDlg, IDC_TRANSPARENTOVERLAY, (trayOption&4) ? BST_CHECKED : BST_UNCHECKED);

                EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWNORMAL),IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS));
                EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENTOVERLAY),IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS)&&IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL));

            }
            CheckDlgButton(hwndDlg, IDC_ALWAYSSTATUS, ModernGetSettingByte(NULL,"CList","AlwaysStatus",SETTING_ALWAYSSTATUS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hwndDlg, IDC_ALWAYSPRIMARY, !ModernGetSettingByte(NULL,"CList","AlwaysPrimary",SETTING_ALWAYSPRIMARY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

            CheckDlgButton(hwndDlg, IDC_ALWAYSMULTI, !ModernGetSettingByte(NULL,"CList","AlwaysMulti",SETTING_ALWAYSMULTI_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_DONTCYCLE, ModernGetSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)==SETTING_TRAYICON_SINGLE ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_CYCLE, ModernGetSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)==SETTING_TRAYICON_CYCLE ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_MULTITRAY, ModernGetSettingByte(NULL,"CList","TrayIcon",SETTING_TRAYICON_DEFAULT)==SETTING_TRAYICON_MULTI ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_DISABLEBLINK, ModernGetSettingByte(NULL,"CList","DisableTrayFlash",SETTING_DISABLETRAYFLASH_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwndDlg, IDC_SHOW_AVATARS, ModernGetSettingByte(NULL,"CList","AvatarsShow",SETTINGS_SHOWAVATARS_DEFAULT) == 1 ? BST_CHECKED : BST_UNCHECKED );
            CheckDlgButton(hwndDlg, IDC_SHOW_ANIAVATARS, ModernGetSettingByte(NULL,"CList","AvatarsAnimated",(ServiceExists(MS_AV_GETAVATARBITMAP)&&!g_CluiData.fGDIPlusFail)) == 1 ? BST_CHECKED : BST_UNCHECKED );

            if(IsDlgButtonChecked(hwndDlg,IDC_DONTCYCLE)) {
                EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIMESPIN),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIME),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSMULTI),FALSE);
            }
            if(IsDlgButtonChecked(hwndDlg,IDC_CYCLE)) {
                EnableWindow(GetDlgItem(hwndDlg,IDC_PRIMARYSTATUS),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSMULTI),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSPRIMARY),FALSE);
            }
            if(IsDlgButtonChecked(hwndDlg,IDC_MULTITRAY)) {
                EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIMESPIN),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIME),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_PRIMARYSTATUS),FALSE);
                EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSPRIMARY),FALSE);
            }
            SendDlgItemMessage(hwndDlg,IDC_CYCLETIMESPIN,UDM_SETRANGE,0,MAKELONG(120,1));
            SendDlgItemMessage(hwndDlg,IDC_CYCLETIMESPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"CList","CycleTime",SETTING_CYCLETIME_DEFAULT),0));
            {
                int i,count,item;
                PROTOACCOUNT **accs;
                DBVARIANT dbv={DBVT_DELETED};
                ModernGetSetting(NULL,"CList","PrimaryStatus",&dbv);
                ProtoEnumAccounts( &count, &accs );
                item=SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_ADDSTRING,0,(LPARAM)TranslateT("Global"));
                SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_SETITEMDATA,item,(LPARAM)0);
                for(i=0;i<count;i++) {
                    if ( !IsAccountEnabled( accs[i] ) || CallProtoService(accs[i]->szModuleName,PS_GETCAPS,PFLAGNUM_2,0)==0)
							  continue;

						  item=SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_ADDSTRING,0,(LPARAM)accs[i]->tszAccountName);
                    SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_SETITEMDATA,item,(LPARAM)accs[i]);
                    if((dbv.type==DBVT_ASCIIZ || dbv.type==DBVT_UTF8)&& !strcmp(dbv.pszVal,accs[i]->szModuleName))
                        SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_SETCURSEL,item,0);
                }
                ModernDBFreeVariant(&dbv);
            }
            if(-1==(int)SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_GETCURSEL,0,0))
                SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_SETCURSEL,0,0);
            SendDlgItemMessage(hwndDlg,IDC_BLINKSPIN,UDM_SETBUDDY,(WPARAM)GetDlgItem(hwndDlg,IDC_BLINKTIME),0);		// set buddy			
            SendDlgItemMessage(hwndDlg,IDC_BLINKSPIN,UDM_SETRANGE,0,MAKELONG(0x3FFF,250));
            SendDlgItemMessage(hwndDlg,IDC_BLINKSPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"CList","IconFlashTime",SETTING_ICONFLASHTIME_DEFAULT),0));
            {
                int i=_GetNetVisibleProtoCount();
                if (i<2)
                {
                    EnableWindow(GetDlgItem(hwndDlg,IDC_PRIMARYSTATUS),TRUE);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIMESPIN),FALSE);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIME),FALSE);				
                    EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSPRIMARY),FALSE);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSPRIMARY),FALSE);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLE),FALSE);
                    EnableWindow(GetDlgItem(hwndDlg,IDC_MULTITRAY),FALSE);
                    CheckDlgButton(hwndDlg,IDC_DONTCYCLE,TRUE);
                }
            }
        }
        return TRUE;

    case WM_COMMAND:
        {
            if (LOWORD(wParam)==IDC_SHOWXSTATUS||LOWORD(wParam)==IDC_SHOWNORMAL)
            {
                EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWNORMAL),IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS));
                EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENTOVERLAY),IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS)&&IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL));
            }
            if(LOWORD(wParam)==IDC_DONTCYCLE || LOWORD(wParam)==IDC_CYCLE || LOWORD(wParam)==IDC_MULTITRAY) 
            {
                EnableWindow(GetDlgItem(hwndDlg,IDC_PRIMARYSTATUS),IsDlgButtonChecked(hwndDlg,IDC_DONTCYCLE));
                EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIME),IsDlgButtonChecked(hwndDlg,IDC_CYCLE));
                EnableWindow(GetDlgItem(hwndDlg,IDC_CYCLETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_CYCLE));
                EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSMULTI),IsDlgButtonChecked(hwndDlg,IDC_MULTITRAY));
                EnableWindow(GetDlgItem(hwndDlg,IDC_ALWAYSPRIMARY),IsDlgButtonChecked(hwndDlg,IDC_DONTCYCLE));
            }
            if(LOWORD(wParam)==IDC_PRIMARYSTATUS && HIWORD(wParam)!=CBN_SELCHANGE) break;
            if (LOWORD(wParam)==IDC_BLINKTIME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus())) return 0; // dont make apply enabled during buddy set crap
            SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
        }
        break;
    case WM_NOTIFY:
        {
            switch (((LPNMHDR)lParam)->idFrom) 
            {
            case 0:
                switch (((LPNMHDR)lParam)->code)
                {
                case PSN_APPLY:
                    ModernWriteSettingByte(NULL,"CList","Tray1Click",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ONECLK));
                    ModernWriteSettingByte(NULL,"CList","AlwaysStatus",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ALWAYSSTATUS));

                    ModernWriteSettingByte(NULL,"CList","AlwaysMulti",(BYTE)!IsDlgButtonChecked(hwndDlg,IDC_ALWAYSMULTI));
                    ModernWriteSettingByte(NULL,"CList","AlwaysPrimary",(BYTE)!IsDlgButtonChecked(hwndDlg,IDC_ALWAYSPRIMARY));

                    ModernWriteSettingWord(NULL,"CList","CycleTime",(WORD)SendDlgItemMessage(hwndDlg,IDC_CYCLETIMESPIN,UDM_GETPOS,0,0));
                    ModernWriteSettingWord(NULL,"CList","IconFlashTime",(WORD)SendDlgItemMessage(hwndDlg,IDC_BLINKSPIN,UDM_GETPOS,0,0));
                    ModernWriteSettingByte(NULL,"CList","DisableTrayFlash",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DISABLEBLINK));


                    if (_GetNetVisibleProtoCount()>1)
                        ModernWriteSettingByte(NULL,"CList","TrayIcon",(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_DONTCYCLE)?SETTING_TRAYICON_SINGLE:(IsDlgButtonChecked(hwndDlg,IDC_CYCLE)?SETTING_TRAYICON_CYCLE:SETTING_TRAYICON_MULTI)));

                    {
                        BYTE xOptions=0;
                        xOptions=IsDlgButtonChecked(hwndDlg,IDC_SHOWXSTATUS)?1:0;
                        xOptions|=(xOptions && IsDlgButtonChecked(hwndDlg,IDC_SHOWNORMAL))?2:0;
                        xOptions|=(xOptions && IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENTOVERLAY))?4:0;
                        ModernWriteSettingByte(NULL,"CLUI","XStatusTray",xOptions);				
                    }

                    {
                        int cursel = SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_GETCURSEL,0,0);
                        PROTOACCOUNT* pa = (PROTOACCOUNT*)SendDlgItemMessage(hwndDlg,IDC_PRIMARYSTATUS,CB_GETITEMDATA,cursel,0);
                        if ( !pa ) 
                            ModernDeleteSetting(NULL, "CList","PrimaryStatus");
                        else 
                            ModernWriteSettingString(NULL,"CList","PrimaryStatus", pa->szModuleName);
                    }
                    pcli->pfnTrayIconIconsChanged();
                    pcli->pfnLoadContactTree(); /* this won't do job properly since it only really works when changes happen */
                    SendMessage(pcli->hwndContactTree,CLM_AUTOREBUILD,0,0); /* force reshuffle */
                    ClcOptionsChanged(); // Used to force loading avatar an list height related options
                    return TRUE;
                }
                break;
            }
            break;
        }
    }
    return FALSE;
}




void ClcOptionsChanged(void)
{
	pcli->pfnClcBroadcast( INTM_RELOADOPTIONS,0,0);
	pcli->pfnClcBroadcast( INTM_INVALIDATE,0,0);
}

HWND g_hCLUIOptionsWnd=NULL;



static INT_PTR CALLBACK DlgProcClistBehaviourOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:

		TranslateDialogDefault(hwndDlg);
		CheckDlgButton(hwndDlg, IDC_CLIENTDRAG, ModernGetSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DRAGTOSCROLL, (ModernGetSettingByte(NULL,"CLUI","DragToScroll",SETTING_DRAGTOSCROLL_DEFAULT)&&!ModernGetSettingByte(NULL,"CLUI","ClientAreaDrag",SETTING_CLIENTDRAG_DEFAULT)) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_AUTOSIZE, g_CluiData.fAutoSize ? BST_CHECKED : BST_UNCHECKED);			
		CheckDlgButton(hwndDlg, IDC_LOCKSIZING, ModernGetSettingByte(NULL,"CLUI","LockSize",SETTING_LOCKSIZE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);			   
		CheckDlgButton(hwndDlg, IDC_BRINGTOFRONT, ModernGetSettingByte(NULL,"CList","BringToFront",SETTING_BRINGTOFRONT_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);			   		


		SendDlgItemMessage(hwndDlg,IDC_MAXSIZESPIN,UDM_SETRANGE,0,MAKELONG(100,0));
		SendDlgItemMessage(hwndDlg,IDC_MAXSIZESPIN,UDM_SETPOS,0,ModernGetSettingByte(NULL,"CLUI","MaxSizeHeight",SETTING_MAXSIZEHEIGHT_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_MINSIZESPIN,UDM_SETRANGE,0,MAKELONG(100,0));
		SendDlgItemMessage(hwndDlg,IDC_MINSIZESPIN,UDM_SETPOS,0,ModernGetSettingByte(NULL,"CLUI","MinSizeHeight",SETTING_MINSIZEHEIGHT_DEFAULT));
		CheckDlgButton(hwndDlg, IDC_AUTOSIZEUPWARD, ModernGetSettingByte(NULL,"CLUI","AutoSizeUpward",SETTING_AUTOSIZEUPWARD_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_SNAPTOEDGES, ModernGetSettingByte(NULL,"CLUI","SnapToEdges",SETTING_SNAPTOEDGES_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_DOCKTOSIDES, ModernGetSettingByte(NULL,"CLUI","DockToSides",SETTING_DOCKTOSIDES_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_EVENTAREA_NONE, ModernGetSettingByte(NULL,"CLUI","EventArea",SETTING_EVENTAREAMODE_DEFAULT)==0 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EVENTAREA, ModernGetSettingByte(NULL,"CLUI","EventArea",SETTING_EVENTAREAMODE_DEFAULT)==1 ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_EVENTAREA_ALWAYS, ModernGetSettingByte(NULL,"CLUI","EventArea",SETTING_EVENTAREAMODE_DEFAULT)==2 ? BST_CHECKED : BST_UNCHECKED);

		CheckDlgButton(hwndDlg, IDC_AUTOHIDE, ModernGetSettingByte(NULL,"CList","AutoHide",SETTING_AUTOHIDE_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_SETRANGE,0,MAKELONG(900,1));
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"CList","HideTime",SETTING_HIDETIME_DEFAULT),0));
		EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIME),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
		EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC01),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
		{
			int i, item;
			TCHAR *hidemode[]={TranslateT("Hide to tray"), TranslateT("Behind left edge"), TranslateT("Behind right edge")};
			for (i=0; i<SIZEOF(hidemode); i++) {
				item=SendDlgItemMessage(hwndDlg,IDC_HIDEMETHOD,CB_ADDSTRING,0,(LPARAM)(hidemode[i]));
				SendDlgItemMessage(hwndDlg,IDC_HIDEMETHOD,CB_SETITEMDATA,item,(LPARAM)0);
				SendDlgItemMessage(hwndDlg,IDC_HIDEMETHOD,CB_SETCURSEL,ModernGetSettingByte(NULL,"ModernData","HideBehind",SETTING_HIDEBEHIND_DEFAULT),0);
			}
		}
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN2,UDM_SETRANGE,0,MAKELONG(600,0));
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN2,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"ModernData","ShowDelay",SETTING_SHOWDELAY_DEFAULT),0));
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN3,UDM_SETRANGE,0,MAKELONG(600,0));
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN3,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"ModernData","HideDelay",SETTING_HIDEDELAY_DEFAULT),0));
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN4,UDM_SETRANGE,0,MAKELONG(50,1));
		SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN4,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"ModernData","HideBehindBorderSize",SETTING_HIDEBEHINDBORDERSIZE_DEFAULT),0));
		{
			int mode=SendDlgItemMessage(hwndDlg,IDC_HIDEMETHOD,CB_GETCURSEL,0,0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWDELAY),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDEDELAY),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN2),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN3),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN4),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDEDELAY2),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC5),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC6),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC7),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC8),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC10),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC13),mode!=0);
		}

		if(!IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE)) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC21),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC22),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZEHEIGHT),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZESPIN),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MINSIZEHEIGHT),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MINSIZESPIN),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_AUTOSIZEUPWARD),FALSE);
		}
		return TRUE;
	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_AUTOHIDE) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIME),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC01),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
		}
		else if(LOWORD(wParam)==IDC_DRAGTOSCROLL && IsDlgButtonChecked(hwndDlg,IDC_CLIENTDRAG)) {
			CheckDlgButton(hwndDlg,IDC_CLIENTDRAG,FALSE);
		}
		else if(LOWORD(wParam)==IDC_CLIENTDRAG && IsDlgButtonChecked(hwndDlg,IDC_DRAGTOSCROLL)) {
			CheckDlgButton(hwndDlg,IDC_DRAGTOSCROLL,FALSE);
		}
		else if(LOWORD(wParam)==IDC_AUTOSIZE) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC21),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC22),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZEHEIGHT),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_MAXSIZESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_MINSIZEHEIGHT),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_MINSIZESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_AUTOSIZEUPWARD),IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
		}
		else if (LOWORD(wParam)==IDC_HIDEMETHOD)
		{
			int mode=SendDlgItemMessage(hwndDlg,IDC_HIDEMETHOD,CB_GETCURSEL,0,0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWDELAY),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDEDELAY),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN2),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN3),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN4),mode!=0);     
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDEDELAY2),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC5),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC6),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC7),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC8),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC10),mode!=0);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC13),mode!=0);
		}
		if ((LOWORD(wParam)==IDC_HIDETIME || LOWORD(wParam)==IDC_HIDEDELAY2 ||LOWORD(wParam)==IDC_HIDEDELAY ||LOWORD(wParam)==IDC_SHOWDELAY || LOWORD(wParam)==IDC_MAXSIZEHEIGHT || LOWORD(wParam)==IDC_MINSIZEHEIGHT) &&
			(HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
			return 0;
		// Enable apply button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;	
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			//
			//DBWriteContactSettingByte(NULL,"CLUI","LeftClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_LEFTMARGINSPIN,UDM_GETPOS,0,0));
			//DBWriteContactSettingByte(NULL,"CLUI","RightClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_RIGHTMARGINSPIN,UDM_GETPOS,0,0));
			//DBWriteContactSettingByte(NULL,"CLUI","TopClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TOPMARGINSPIN,UDM_GETPOS,0,0));
			//DBWriteContactSettingByte(NULL,"CLUI","BottomClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_BOTTOMMARGINSPIN,UDM_GETPOS,0,0));
			//if (g_proc_UpdateLayeredWindow!=NULL && IsDlgButtonChecked(hwndDlg,IDC_LAYERENGINE)) 
			//	DBWriteContactSettingByte(NULL,"ModernData","EnableLayering",0);
			//else 
			//	DBDeleteContactSetting(NULL,"ModernData","EnableLayering");	
			ModernWriteSettingByte(NULL,"ModernData","HideBehind",(BYTE)SendDlgItemMessage(hwndDlg,IDC_HIDEMETHOD,CB_GETCURSEL,0,0));  
			ModernWriteSettingWord(NULL,"ModernData","ShowDelay",(WORD)SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN2,UDM_GETPOS,0,0));
			ModernWriteSettingWord(NULL,"ModernData","HideDelay",(WORD)SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN3,UDM_GETPOS,0,0));
			ModernWriteSettingWord(NULL,"ModernData","HideBehindBorderSize",(WORD)SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN4,UDM_GETPOS,0,0));

			ModernWriteSettingByte(NULL,"CLUI","DragToScroll",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DRAGTOSCROLL));
			ModernWriteSettingByte(NULL,"CList","BringToFront",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_BRINGTOFRONT));
			g_mutex_bChangingMode=TRUE;
			ModernWriteSettingByte(NULL,"CLUI","ClientAreaDrag",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_CLIENTDRAG));
			ModernWriteSettingByte(NULL,"CLUI","AutoSize",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZE));
			ModernWriteSettingByte(NULL,"CLUI","LockSize",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_LOCKSIZING));
			ModernWriteSettingByte(NULL,"CLUI","MaxSizeHeight",(BYTE)GetDlgItemInt(hwndDlg,IDC_MAXSIZEHEIGHT,NULL,FALSE));               
			ModernWriteSettingByte(NULL,"CLUI","MinSizeHeight",(BYTE)GetDlgItemInt(hwndDlg,IDC_MINSIZEHEIGHT,NULL,FALSE));               
			ModernWriteSettingByte(NULL,"CLUI","AutoSizeUpward",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_AUTOSIZEUPWARD));
			ModernWriteSettingByte(NULL,"CLUI","SnapToEdges",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SNAPTOEDGES));
			
			ModernWriteSettingByte(NULL,"CLUI","DockToSides",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DOCKTOSIDES));			

			ModernWriteSettingByte(NULL,"CLUI","EventArea",
				(BYTE)(IsDlgButtonChecked(hwndDlg,IDC_EVENTAREA_ALWAYS)?2:(BYTE)IsDlgButtonChecked(hwndDlg,IDC_EVENTAREA)?1:0));

			ModernWriteSettingByte(NULL,"CList","AutoHide",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			ModernWriteSettingWord(NULL,"CList","HideTime",(WORD)SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_GETPOS,0,0));			
			CLUI_ChangeWindowMode(); 
			SendMessage(pcli->hwndContactTree,WM_SIZE,0,0);	//forces it to send a cln_listsizechanged
			CLUI_ReloadCLUIOptions();
			EventArea_ConfigureEventArea();
			cliShowHide(0,1);
			g_mutex_bChangingMode=FALSE;
			return TRUE;
		}
		break;
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcClistWindowOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	BOOL fEnabled=FALSE;
	switch (msg)
	{
	case WM_INITDIALOG:		
		
		TranslateDialogDefault(hwndDlg);
		g_hCLUIOptionsWnd=hwndDlg;
		CheckDlgButton(hwndDlg, IDC_ONTOP, ModernGetSettingByte(NULL,"CList","OnTop",SETTING_ONTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);		
		{	// ====== Activate/Deactivate Non-Layered items =======
			fEnabled=!g_CluiData.fLayered || g_CluiData.fDisableSkinEngine;
			EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_BORDER),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_NOBORDERWND),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWCAPTION),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWMAINMENU),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_DROPSHADOW),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_AEROGLASS),!fEnabled && (g_proc_DWMEnableBlurBehindWindow != NULL));
			EnableWindow(GetDlgItem(hwndDlg,IDC_TITLEBAR_STATIC),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ROUNDCORNERS),fEnabled);
		}
		{   // ====== Non-Layered Mode =====
			CheckDlgButton(hwndDlg, IDC_TOOLWND, ModernGetSettingByte(NULL,"CList","ToolWindow",SETTING_TOOLWINDOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MIN2TRAY, ModernGetSettingByte(NULL,"CList","Min2Tray",SETTING_MIN2TRAY_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_BORDER, ModernGetSettingByte(NULL,"CList","ThinBorder",SETTING_THINBORDER_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOBORDERWND, ModernGetSettingByte(NULL,"CList","NoBorder",SETTING_NOBORDER_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			if(IsDlgButtonChecked(hwndDlg,IDC_TOOLWND))
				EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),FALSE);
			CheckDlgButton(hwndDlg, IDC_SHOWCAPTION, ModernGetSettingByte(NULL,"CLUI","ShowCaption",SETTING_SHOWCAPTION_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWMAINMENU, ModernGetSettingByte(NULL,"CLUI","ShowMainMenu",SETTING_SHOWMAINMENU_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			//EnableWindow(GetDlgItem(hwndDlg,IDC_CLIENTDRAG),!IsDlgButtonChecked(hwndDlg,IDC_DRAGTOSCROLL));
			if(!IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION)) 
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),FALSE);
			}
			if (IsDlgButtonChecked(hwndDlg,IDC_BORDER) || IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND))
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWCAPTION),FALSE);				
			}
			CheckDlgButton(hwndDlg, IDC_DROPSHADOW, ModernGetSettingByte(NULL,"CList","WindowShadow",SETTING_WINDOWSHADOW_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_ROUNDCORNERS, ModernGetSettingByte(NULL,"CLC","RoundCorners",SETTING_ROUNDCORNERS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);	  
		}   // ====== End of Non-Layered Mode =====

		CheckDlgButton(hwndDlg, IDC_FADEINOUT, ModernGetSettingByte(NULL,"CLUI","FadeInOut",SETTING_FADEIN_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_ONDESKTOP, ModernGetSettingByte(NULL,"CList","OnDesktop", SETTING_ONDESKTOP_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		SendDlgItemMessage(hwndDlg,IDC_FRAMESSPIN,UDM_SETRANGE,0,MAKELONG(50,0));
		SendDlgItemMessage(hwndDlg,IDC_CAPTIONSSPIN,UDM_SETRANGE,0,MAKELONG(50,0));
		SendDlgItemMessage(hwndDlg,IDC_FRAMESSPIN,UDM_SETPOS,0,ModernGetSettingDword(NULL,"CLUIFrames","GapBetweenFrames",SETTING_GAPFRAMES_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_CAPTIONSSPIN,UDM_SETPOS,0,ModernGetSettingDword(NULL,"CLUIFrames","GapBetweenTitleBar",SETTING_GAPTITLEBAR_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_LEFTMARGINSPIN,UDM_SETRANGE,0,MAKELONG(250,0));
		SendDlgItemMessage(hwndDlg,IDC_RIGHTMARGINSPIN,UDM_SETRANGE,0,MAKELONG(250,0));
		SendDlgItemMessage(hwndDlg,IDC_TOPMARGINSPIN,UDM_SETRANGE,0,MAKELONG(250,0));
		SendDlgItemMessage(hwndDlg,IDC_BOTTOMMARGINSPIN,UDM_SETRANGE,0,MAKELONG(250,0));
		SendDlgItemMessage(hwndDlg,IDC_LEFTMARGINSPIN,UDM_SETPOS,0,ModernGetSettingByte(NULL,"CLUI","LeftClientMargin",SETTING_LEFTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_RIGHTMARGINSPIN,UDM_SETPOS,0,ModernGetSettingByte(NULL,"CLUI","RightClientMargin",SETTING_RIGHTCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_TOPMARGINSPIN,UDM_SETPOS,0,ModernGetSettingByte(NULL,"CLUI","TopClientMargin",SETTING_TOPCLIENTMARIGN_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_BOTTOMMARGINSPIN,UDM_SETPOS,0,ModernGetSettingByte(NULL,"CLUI","BottomClientMargin",SETTING_BOTTOMCLIENTMARIGN_DEFAULT));
		
		CheckDlgButton(hwndDlg, IDC_DISABLEENGINE, ModernGetSettingByte(NULL,"ModernData","DisableEngine", SETTING_DISABLESKIN_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);		
		CheckDlgButton(hwndDlg, IDC_AEROGLASS, ModernGetSettingByte(NULL,"ModernData","AeroGlass",SETTING_AEROGLASS_DEFAULT) ? BST_CHECKED : BST_UNCHECKED);
		
		EnableWindow(GetDlgItem(hwndDlg,IDC_LAYERENGINE),(g_proc_UpdateLayeredWindow!=NULL && !ModernGetSettingByte(NULL,"ModernData","DisableEngine", SETTING_DISABLESKIN_DEFAULT))?TRUE:FALSE);
		CheckDlgButton(hwndDlg, IDC_LAYERENGINE, ((ModernGetSettingByte(NULL,"ModernData","EnableLayering",SETTING_ENABLELAYERING_DEFAULT)&&g_proc_UpdateLayeredWindow!=NULL) && !ModernGetSettingByte(NULL,"ModernData","DisableEngine", SETTING_DISABLESKIN_DEFAULT)) ? BST_UNCHECKED:BST_CHECKED);   

		{
			DBVARIANT dbv={0};
			TCHAR *s=NULL;
			char szUin[20];
			if(!ModernGetSettingTString(NULL,"CList","TitleText",&dbv))
				s=mir_tstrdup(dbv.ptszVal);
			else
				s=mir_tstrdup(_T(MIRANDANAME));
			//dbv.pszVal=s;
			SetDlgItemText(hwndDlg,IDC_TITLETEXT,s);
			if (s) mir_free_and_nill(s);
			ModernDBFreeVariant(&dbv);
			//if (s) mir_free_and_nill(s);
			SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)MIRANDANAME);
			sprintf(szUin,"%u",ModernGetSettingDword(NULL,"ICQ","UIN",0));
			SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)szUin);

			if(!ModernGetSettingString(NULL,"ICQ","Nick",&dbv)) {
				SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
				//mir_free_and_nill(dbv.pszVal);
				ModernDBFreeVariant(&dbv);
				dbv.pszVal=NULL;
			}
			if(!ModernGetSettingString(NULL,"ICQ","FirstName",&dbv)) {
				SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
				//mir_free_and_nill(dbv.pszVal);
				ModernDBFreeVariant(&dbv);
				dbv.pszVal=NULL;
			}
			if(!ModernGetSettingString(NULL,"ICQ","e-mail",&dbv)) {
				SendDlgItemMessage(hwndDlg,IDC_TITLETEXT,CB_ADDSTRING,0,(LPARAM)dbv.pszVal);
				//mir_free_and_nill(dbv.pszVal);
				ModernDBFreeVariant(&dbv);
				dbv.pszVal=NULL;
			}
		}
		if(!IsWinVer2000Plus()) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_FADEINOUT),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENT),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_DROPSHADOW),FALSE);
		}
		else CheckDlgButton(hwndDlg,IDC_TRANSPARENT,ModernGetSettingByte(NULL,"CList","Transparent",SETTING_TRANSPARENT_DEFAULT)?BST_CHECKED:BST_UNCHECKED);
		if(!IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT)) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC11),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC12),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSACTIVE),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSINACTIVE),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ACTIVEPERC),FALSE);
			EnableWindow(GetDlgItem(hwndDlg,IDC_INACTIVEPERC),FALSE);
		}
		SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_SETRANGE,FALSE,MAKELONG(1,255));
		SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_SETRANGE,FALSE,MAKELONG(1,255));
		SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_SETPOS,TRUE,ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT));
		SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_SETPOS,TRUE,ModernGetSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT));
		SendMessage(hwndDlg,WM_HSCROLL,0x12345678,0);
		return TRUE;

	case WM_COMMAND:
		if(LOWORD(wParam)==IDC_TRANSPARENT) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC11),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC12),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSACTIVE),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSINACTIVE),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg,IDC_ACTIVEPERC),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
			EnableWindow(GetDlgItem(hwndDlg,IDC_INACTIVEPERC),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
		}
		else if(LOWORD(wParam)==IDC_LAYERENGINE || LOWORD(wParam)==IDC_DISABLEENGINE)
		{	// ====== Activate/Deactivate Non-Layered items =======
			fEnabled=!(IsWindowEnabled(GetDlgItem(hwndDlg,IDC_LAYERENGINE)) && !IsDlgButtonChecked(hwndDlg,IDC_LAYERENGINE) && !IsDlgButtonChecked(hwndDlg,IDC_DISABLEENGINE));

			EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),fEnabled&&(IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION))&&!(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));  
			EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),fEnabled&&(IsDlgButtonChecked(hwndDlg,IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));  
			EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),fEnabled&&(IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION))&&!(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));  
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWCAPTION),fEnabled&&!(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));             
			EnableWindow(GetDlgItem(hwndDlg,IDC_BORDER),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_NOBORDERWND),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWMAINMENU),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_DROPSHADOW),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_AEROGLASS),!fEnabled && (g_proc_DWMEnableBlurBehindWindow != NULL));
			EnableWindow(GetDlgItem(hwndDlg,IDC_TITLEBAR_STATIC),fEnabled);
			EnableWindow(GetDlgItem(hwndDlg,IDC_ROUNDCORNERS),fEnabled);
			if (LOWORD(wParam)==IDC_DISABLEENGINE)
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_LAYERENGINE),!IsDlgButtonChecked(hwndDlg,IDC_DISABLEENGINE) && g_proc_UpdateLayeredWindow!=NULL);
				if (IsDlgButtonChecked(hwndDlg,IDC_DISABLEENGINE))
					CheckDlgButton(hwndDlg,IDC_LAYERENGINE,BST_CHECKED);
			}

		}
		else if(LOWORD(wParam)==IDC_ONDESKTOP && IsDlgButtonChecked(hwndDlg,IDC_ONDESKTOP)) {
			CheckDlgButton(hwndDlg, IDC_ONTOP, BST_UNCHECKED);    
		}
		else if(LOWORD(wParam)==IDC_ONTOP && IsDlgButtonChecked(hwndDlg,IDC_ONTOP)) {
			CheckDlgButton(hwndDlg, IDC_ONDESKTOP, BST_UNCHECKED);    
		}
		else if(LOWORD(wParam)==IDC_TOOLWND) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),!IsDlgButtonChecked(hwndDlg,IDC_TOOLWND));
		}
		else if(LOWORD(wParam)==IDC_SHOWCAPTION) {
			EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
			EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),!IsDlgButtonChecked(hwndDlg,IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
			EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
		}
		else if(LOWORD(wParam)==IDC_NOBORDERWND ||LOWORD(wParam)==IDC_BORDER)
		{
			EnableWindow(GetDlgItem(hwndDlg,IDC_TOOLWND),(IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION))&&!(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));  
			EnableWindow(GetDlgItem(hwndDlg,IDC_MIN2TRAY),(IsDlgButtonChecked(hwndDlg,IDC_TOOLWND) && IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION)) && !(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));  
			EnableWindow(GetDlgItem(hwndDlg,IDC_TITLETEXT),(IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION))&&!(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));  
			EnableWindow(GetDlgItem(hwndDlg,IDC_SHOWCAPTION),!(IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND)||IsDlgButtonChecked(hwndDlg,IDC_BORDER)));             
			if (LOWORD(wParam)==IDC_BORDER) CheckDlgButton(hwndDlg, IDC_NOBORDERWND,BST_UNCHECKED);
			else CheckDlgButton(hwndDlg, IDC_BORDER,BST_UNCHECKED); 

		}
		if ((LOWORD(wParam)==IDC_TITLETEXT || LOWORD(wParam)==IDC_MAXSIZEHEIGHT || LOWORD(wParam)==IDC_MINSIZEHEIGHT || LOWORD(wParam)==IDC_FRAMESGAP || LOWORD(wParam)==IDC_CAPTIONSGAP ||
			LOWORD(wParam)==IDC_LEFTMARGIN || LOWORD(wParam)==IDC_RIGHTMARGIN|| LOWORD(wParam)==IDC_TOPMARGIN || LOWORD(wParam)==IDC_BOTTOMMARGIN) 
			&&(HIWORD(wParam)!=EN_CHANGE || (HWND)lParam!=GetFocus()))
			return 0;
		// Enable apply button
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;

	case WM_HSCROLL:
		{	char str[10];
		sprintf(str,"%d%%",100*SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_GETPOS,0,0)/255);
		SetDlgItemTextA(hwndDlg,IDC_INACTIVEPERC,str);
		sprintf(str,"%d%%",100*SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_GETPOS,0,0)/255);
		SetDlgItemTextA(hwndDlg,IDC_ACTIVEPERC,str);
		}
		if(wParam!=0x12345678) SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
			{
				ModernWriteSettingByte(NULL,"CLUI","LeftClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_LEFTMARGINSPIN,UDM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"CLUI","RightClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_RIGHTMARGINSPIN,UDM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"CLUI","TopClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TOPMARGINSPIN,UDM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"CLUI","BottomClientMargin",(BYTE)SendDlgItemMessage(hwndDlg,IDC_BOTTOMMARGINSPIN,UDM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"ModernData","DisableEngine",IsDlgButtonChecked(hwndDlg,IDC_DISABLEENGINE));
				ModernWriteSettingByte(NULL,"ModernData","AeroGlass",IsDlgButtonChecked(hwndDlg,IDC_AEROGLASS));
				if (!IsDlgButtonChecked(hwndDlg,IDC_DISABLEENGINE))
				{
					if (g_proc_UpdateLayeredWindow!=NULL && IsDlgButtonChecked(hwndDlg,IDC_LAYERENGINE)) 
						ModernWriteSettingByte(NULL,"ModernData","EnableLayering",0);
					else 
						ModernDeleteSetting(NULL,"ModernData","EnableLayering");	
				}
				g_CluiData.dwKeyColor=ModernGetSettingDword(NULL,"ModernSettings","KeyColor",(DWORD)SETTING_KEYCOLOR_DEFAULT);
				ModernWriteSettingByte(NULL,"CList","OnDesktop",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ONDESKTOP));
				ModernWriteSettingByte(NULL,"CList","OnTop",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ONTOP));
				SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg,IDC_ONTOP)?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE |SWP_NOACTIVATE);
				ModernWriteSettingByte(NULL,"CLUI","DragToScroll",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DRAGTOSCROLL));

				{ //====== Non-Layered Mode ======
					ModernWriteSettingByte(NULL,"CList","ToolWindow",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_TOOLWND));
					ModernWriteSettingByte(NULL,"CLUI","ShowCaption",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWCAPTION));
					ModernWriteSettingByte(NULL,"CLUI","ShowMainMenu",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_SHOWMAINMENU));
					ModernWriteSettingByte(NULL,"CList","ThinBorder",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_BORDER));
					ModernWriteSettingByte(NULL,"CList","NoBorder",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_NOBORDERWND));
					{	
						TCHAR title[256];
						GetDlgItemText(hwndDlg,IDC_TITLETEXT,title,SIZEOF(title));
						ModernWriteSettingTString(NULL,"CList","TitleText",title);
						//			SetWindowText(pcli->hwndContactList,title);
					}
					ModernWriteSettingByte(NULL,"CList","Min2Tray",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_MIN2TRAY));
					ModernWriteSettingByte(NULL,"CList","WindowShadow",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_DROPSHADOW));
					ModernWriteSettingByte(NULL,"CLC","RoundCorners",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ROUNDCORNERS));		
				} //====== End of Non-Layered Mode ======
				g_mutex_bChangingMode=TRUE;

				if (IsDlgButtonChecked(hwndDlg,IDC_ONDESKTOP)) 
				{
					HWND hProgMan=FindWindow(TEXT("Progman"),NULL);
					if (IsWindow(hProgMan)) 
					{
						SetParent(pcli->hwndContactList,hProgMan);
						Sync( CLUIFrames_SetParentForContainers, (HWND)hProgMan );
						g_CluiData.fOnDesktop=1;
					}
				} 
				else 
				{
					if (GetParent(pcli->hwndContactList))
					{
						SetParent(pcli->hwndContactList,NULL);
						Sync( CLUIFrames_SetParentForContainers, (HWND)NULL );
					}
					g_CluiData.fOnDesktop=0;
				}
				AniAva_UpdateParent();
				ModernWriteSettingByte(NULL,"CLUI","FadeInOut",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_FADEINOUT));
				g_CluiData.fSmoothAnimation=IsWinVer2000Plus()&&(BYTE)IsDlgButtonChecked(hwndDlg,IDC_FADEINOUT);
				{
					int i1;
					int i2;
					i1=SendDlgItemMessage(hwndDlg,IDC_FRAMESSPIN,UDM_GETPOS,0,0);
					i2=SendDlgItemMessage(hwndDlg,IDC_CAPTIONSSPIN,UDM_GETPOS,0,0);   

					ModernWriteSettingDword(NULL,"CLUIFrames","GapBetweenFrames",(DWORD)i1);
					ModernWriteSettingDword(NULL,"CLUIFrames","GapBetweenTitleBar",(DWORD)i2);
					Sync(CLUIFramesOnClistResize, (WPARAM)pcli->hwndContactList,(LPARAM)0);
				}
				ModernWriteSettingByte(NULL,"CList","Transparent",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				ModernWriteSettingByte(NULL,"CList","Alpha",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"CList","AutoAlpha",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_GETPOS,0,0));
				ModernWriteSettingByte(NULL,"CList","OnDesktop",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_ONDESKTOP));

				ske_LoadSkinFromDB();
				CLUI_UpdateLayeredMode();	
				CLUI_ChangeWindowMode(); 
				SendMessage(pcli->hwndContactTree,WM_SIZE,0,0);	//forces it to send a cln_listsizechanged
				CLUI_ReloadCLUIOptions();
				cliShowHide(0,1);			
				g_mutex_bChangingMode=FALSE;
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}

#include "hdr/modern_commonheaders.h"

#define CLBF_TILEVTOROWHEIGHT        0x0100


#define DEFAULT_BKCOLOUR      GetSysColor(COLOR_3DFACE)
#define DEFAULT_USEBITMAP     0
#define DEFAULT_BKBMPUSE      CLB_STRETCH
#define DEFAULT_SELBKCOLOUR   GetSysColor(COLOR_HIGHLIGHT)



extern HINSTANCE g_hInst;
extern PLUGINLINK *pluginLink;
extern struct MM_INTERFACE mmi;

char **bkgrList = NULL;
int bkgrCount = 0;

#define M_BKGR_UPDATE	(WM_USER+10)
#define M_BKGR_SETSTATE	(WM_USER+11)
#define M_BKGR_GETSTATE	(WM_USER+12)

#define M_BKGR_BACKCOLOR	0x01
#define M_BKGR_SELECTCOLOR	0x02
#define M_BKGR_ALLOWBITMAPS	0x04
#define M_BKGR_STRETCH		0x08
#define M_BKGR_TILE			0x10

static int bitmapRelatedControls[] = {
	IDC_FILENAME,IDC_BROWSE,IDC_STRETCHH,IDC_STRETCHV,IDC_TILEH,IDC_TILEV,
	IDC_SCROLL,IDC_PROPORTIONAL,IDC_TILEVROWH
};
struct BkgrItem
{
	BYTE changed;
	BYTE useBitmap;
	COLORREF bkColor, selColor;
	char filename[MAX_PATH];
	WORD flags;
	BYTE useWinColours;
};
struct BkgrData
{
	struct BkgrItem *item;
	int indx;
	int count;
};
static INT_PTR CALLBACK DlgProcClcBkgOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct BkgrData *dat = (struct BkgrData *)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			int indx;
			HWND hList = GetDlgItem(hwndDlg, IDC_BKGRLIST);
			TranslateDialogDefault(hwndDlg);

			dat=(struct BkgrData*)mir_alloc(sizeof(struct BkgrData));
			SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)dat);
			dat->count = bkgrCount;
			dat->item = (struct BkgrItem*)mir_alloc(sizeof(struct BkgrItem)*dat->count);
			dat->indx = CB_ERR;
			for(indx = 0; indx < dat->count; indx++)
			{
				char *module = bkgrList[indx] + strlen(bkgrList[indx]) + 1;
				int jndx;

				dat->item[indx].changed = FALSE;
				dat->item[indx].useBitmap = ModernGetSettingByte(NULL,module, "UseBitmap", DEFAULT_USEBITMAP);
				dat->item[indx].bkColor = ModernGetSettingDword(NULL,module, "BkColour", DEFAULT_BKCOLOUR);
				dat->item[indx].selColor = ModernGetSettingDword(NULL,module, "SelBkColour", DEFAULT_SELBKCOLOUR);
				dat->item[indx].useWinColours = ModernGetSettingByte(NULL,module, "UseWinColours", CLCDEFAULT_USEWINDOWSCOLOURS);	
				{	
					DBVARIANT dbv;
					if(!ModernGetSettingString(NULL,module,"BkBitmap",&dbv))
					{
						int retval = CallService(MS_UTILS_PATHTOABSOLUTE, (WPARAM)dbv.pszVal, (LPARAM)dat->item[indx].filename);
						if(!retval || retval == CALLSERVICE_NOTFOUND)
							lstrcpynA(dat->item[indx].filename, dbv.pszVal, MAX_PATH);
						mir_free(dbv.pszVal);
					}
					else
						*dat->item[indx].filename = 0;
				}
				dat->item[indx].flags = ModernGetSettingWord(NULL,module,"BkBmpUse", DEFAULT_BKBMPUSE);
				jndx = SendMessageA(hList, CB_ADDSTRING, 0, (LPARAM)Translate(bkgrList[indx]));
				SendMessage(hList, CB_SETITEMDATA, jndx, indx);
			}
			SendMessage(hList, CB_SETCURSEL, 0, 0);
			PostMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_BKGRLIST, CBN_SELCHANGE), 0);
			{
				HRESULT (STDAPICALLTYPE *MySHAutoComplete)(HWND,DWORD);
				MySHAutoComplete=(HRESULT (STDAPICALLTYPE*)(HWND,DWORD))GetProcAddress(GetModuleHandleA("shlwapi"),"SHAutoComplete");
				if(MySHAutoComplete) MySHAutoComplete(GetDlgItem(hwndDlg,IDC_FILENAME),1);
			}
			return TRUE;
		}
		case WM_DESTROY:
			if(dat)
			{
				if(dat->item) mir_free(dat->item);
				mir_free(dat);
			}
		
			return TRUE;

		case M_BKGR_GETSTATE:
		{
			int indx = wParam;
			if(indx == CB_ERR || indx >= dat->count) break;
			indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);

			dat->item[indx].useBitmap = IsDlgButtonChecked(hwndDlg,IDC_BITMAP);
			dat->item[indx].useWinColours = IsDlgButtonChecked(hwndDlg,IDC_USEWINCOL);
			dat->item[indx].bkColor = SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_GETCOLOUR,0,0);
			dat->item[indx].selColor = SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_GETCOLOUR,0,0);
			
			GetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename, SIZEOF(dat->item[indx].filename));
			{
				WORD flags = 0;
				if(IsDlgButtonChecked(hwndDlg,IDC_STRETCHH)) flags |= CLB_STRETCHH;
				if(IsDlgButtonChecked(hwndDlg,IDC_STRETCHV)) flags |= CLB_STRETCHV;
				if(IsDlgButtonChecked(hwndDlg,IDC_TILEH)) flags |= CLBF_TILEH;
				if(IsDlgButtonChecked(hwndDlg,IDC_TILEV)) flags |= CLBF_TILEV;
				if(IsDlgButtonChecked(hwndDlg,IDC_SCROLL)) flags |= CLBF_SCROLL;
				if(IsDlgButtonChecked(hwndDlg,IDC_PROPORTIONAL)) flags |= CLBF_PROPORTIONAL;
				if(IsDlgButtonChecked(hwndDlg,IDC_TILEVROWH)) flags |= CLBF_TILEVTOROWHEIGHT;
				dat->item[indx].flags = flags;
			}	
			break;
		}
		case M_BKGR_SETSTATE:
		{
			int flags;
			int indx = wParam;
			if (indx==-1) break;
			flags = dat->item[indx].flags;
			if(indx == CB_ERR || indx >= dat->count) break;
			indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);

			CheckDlgButton(hwndDlg, IDC_BITMAP, dat->item[indx].useBitmap?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_USEWINCOL, dat->item[indx].useWinColours?BST_CHECKED:BST_UNCHECKED);

			EnableWindow(GetDlgItem(hwndDlg,IDC_BKGCOLOUR), !dat->item[indx].useWinColours);
			EnableWindow(GetDlgItem(hwndDlg,IDC_SELCOLOUR), !dat->item[indx].useWinColours);

			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETDEFAULTCOLOUR, 0, DEFAULT_BKCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_BKGCOLOUR, CPM_SETCOLOUR, 0, dat->item[indx].bkColor);
			SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETDEFAULTCOLOUR, 0, DEFAULT_SELBKCOLOUR);
			SendDlgItemMessage(hwndDlg, IDC_SELCOLOUR, CPM_SETCOLOUR, 0, dat->item[indx].selColor);
			SetDlgItemTextA(hwndDlg, IDC_FILENAME, dat->item[indx].filename);	

			CheckDlgButton(hwndDlg,IDC_STRETCHH, flags&CLB_STRETCHH?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_STRETCHV,flags&CLB_STRETCHV?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TILEH,flags&CLBF_TILEH?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TILEV,flags&CLBF_TILEV?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_SCROLL,flags&CLBF_SCROLL?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_PROPORTIONAL,flags&CLBF_PROPORTIONAL?BST_CHECKED:BST_UNCHECKED);
			CheckDlgButton(hwndDlg,IDC_TILEVROWH,flags&CLBF_TILEVTOROWHEIGHT?BST_CHECKED:BST_UNCHECKED);
/*
			{
				WORD visibility;
				int cy = 55;
				char *sz = bkgrList[indx] + strlen(bkgrList[indx]) + 1;
				sz += strlen(sz) + 1;
				visibility = (WORD)~(*(DWORD*)(sz));
//M_BKGR_BACKCOLOR,M_BKGR_SELECTCOLOR,M_BKGR_ALLOWBITMAPS,M_BKGR_STRETCH,M_BKGR_TILE}
				if(visibility & M_BKGR_BACKCOLOR)
				{
					SetWindowPos(GetDlgItem(hwndDlg, IDC_BC_STATIC), 0,
						20, cy,
						0, 0,
						SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_BKGCOLOUR), 0,
						130, cy,
						0, 0,
						SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
					cy += 25;					
				}
				if(visibility & M_BKGR_SELECTCOLOR)
				{
					SetWindowPos(GetDlgItem(hwndDlg, IDC_SC_STATIC), 0,
						20, cy,
						0, 0,
						SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
					SetWindowPos(GetDlgItem(hwndDlg, IDC_SELCOLOUR), 0,
						130, cy,
						0, 0,
						SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOSIZE);
					cy += 25;					
				}
				ShowWindow(GetDlgItem(hwndDlg,IDC_STRETCHH),     visibility&CLB_STRETCHH?SW_SHOW:SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_STRETCHV),     visibility&CLB_STRETCHV?SW_SHOW:SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_TILEH),        visibility&CLBF_TILEH?SW_SHOW:SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_TILEV),        visibility&CLBF_TILEV?SW_SHOW:SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_SCROLL),       visibility&CLBF_SCROLL?SW_SHOW:SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_PROPORTIONAL), visibility&CLBF_PROPORTIONAL?SW_SHOW:SW_HIDE);
				ShowWindow(GetDlgItem(hwndDlg,IDC_TILEVROWH),    visibility&CLBF_TILEVTOROWHEIGHT?SW_SHOW:SW_HIDE);
			}
*/

			SendMessage(hwndDlg, M_BKGR_UPDATE, 0,0);
			break;
		}
		case M_BKGR_UPDATE:
		{
			int isChecked = IsDlgButtonChecked(hwndDlg,IDC_BITMAP);
			int indx;
			for(indx = 0; indx < SIZEOF(bitmapRelatedControls); indx++)
				EnableWindow(GetDlgItem(hwndDlg, bitmapRelatedControls[indx]),isChecked);
			break;
		}
		case WM_COMMAND:
			if(LOWORD(wParam) == IDC_BROWSE)
			{
				char str[MAX_PATH];
				OPENFILENAMEA ofn={0};
				char filter[512];

				GetDlgItemTextA(hwndDlg,IDC_FILENAME, str, SIZEOF(str));
				ofn.lStructSize = OPENFILENAME_SIZE_VERSION_400;
				ofn.hwndOwner = hwndDlg;
				ofn.hInstance = NULL;
				CallService(MS_UTILS_GETBITMAPFILTERSTRINGS, SIZEOF(filter), (LPARAM)filter);
				ofn.lpstrFilter = filter;
				ofn.lpstrFile = str;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
				ofn.nMaxFile = SIZEOF(str);
				ofn.nMaxFileTitle = MAX_PATH;
				ofn.lpstrDefExt = "bmp";
				if(!GetOpenFileNameA(&ofn)) break;
				SetDlgItemTextA(hwndDlg, IDC_FILENAME, str);
			}
			else
				if(LOWORD(wParam) == IDC_FILENAME && HIWORD(wParam) != EN_CHANGE) break;
			if(LOWORD(wParam) == IDC_BITMAP)
				SendMessage(hwndDlg, M_BKGR_UPDATE, 0,0);
			if(LOWORD(wParam) == IDC_FILENAME && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
				return 0;
			if(LOWORD(wParam) == IDC_BKGRLIST)
			{
				if(HIWORD(wParam) == CBN_SELCHANGE)
				{
					SendMessage(hwndDlg, M_BKGR_GETSTATE, dat->indx, 0);
					SendMessage(hwndDlg, M_BKGR_SETSTATE, dat->indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0,0), 0);
				}
				return 0;
			}
			{
				int indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0,0);
				if(indx != CB_ERR && indx < dat->count)
				{
					indx = SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETITEMDATA, indx, 0);					
					dat->item[indx].changed = TRUE;
				
				}	
				{
					BOOL EnableColours=!IsDlgButtonChecked(hwndDlg,IDC_USEWINCOL);
					EnableWindow(GetDlgItem(hwndDlg,IDC_BKGCOLOUR), EnableColours);
					EnableWindow(GetDlgItem(hwndDlg,IDC_SELCOLOUR), EnableColours);
				}
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0,0);
			}
			break;
		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
						{
							int indx;
							SendMessage(hwndDlg, M_BKGR_GETSTATE, SendDlgItemMessage(hwndDlg, IDC_BKGRLIST, CB_GETCURSEL, 0,0), 0);
							for(indx = 0; indx < dat->count; indx++)
							if(dat->item[indx].changed)
							{
								char *module = bkgrList[indx] + strlen(bkgrList[indx]) + 1;
								ModernWriteSettingByte(NULL, module, "UseBitmap", (BYTE)dat->item[indx].useBitmap);
								{	
									COLORREF col;

									if((col = dat->item[indx].bkColor) == DEFAULT_BKCOLOUR)																		
										ModernDeleteSetting(NULL, module, "BkColour");
									else
										ModernWriteSettingDword(NULL, module, "BkColour", col);

									if((col = dat->item[indx].selColor) == DEFAULT_SELBKCOLOUR)
										ModernDeleteSetting(NULL, module, "SelBkColour");
									else
										ModernWriteSettingDword(NULL, module, "SelBkColour", col);
								}
								ModernWriteSettingByte(NULL, module, "UseWinColours", (BYTE)dat->item[indx].useWinColours);
								
								{
									char str[MAX_PATH];
									int retval = CallService(MS_UTILS_PATHTOABSOLUTE,
										(WPARAM)dat->item[indx].filename,
										(LPARAM)str);
									if(!retval || retval == CALLSERVICE_NOTFOUND)
										ModernWriteSettingString(NULL, module, "BkBitmap", dat->item[indx].filename);
									else
										ModernWriteSettingString(NULL, module, "BkBitmap", str);
								}
								ModernWriteSettingWord(NULL, module, "BkBmpUse", dat->item[indx].flags);
								dat->item[indx].changed = FALSE;
								NotifyEventHooks(g_CluiData.hEventBkgrChanged, (WPARAM)module, 0);
							}
							return TRUE;
						}
					}
					break;
			}
			break;
	}
	return FALSE;
}

static INT_PTR BkgrCfg_Register(WPARAM wParam,LPARAM lParam)
{
	char *szSetting = (char *)wParam;
	char *value, *tok;
	int len = strlen(szSetting) + 1;

	value = (char *)mir_alloc(len + 4); // add room for flags (DWORD)
	memcpy(value, szSetting, len);
	tok = strchr(value, '/');
	if(tok == NULL)
	{
		mir_free(value);
		return 1;
	}
	*tok = 0;
	*(DWORD*)(value + len) = lParam;

	bkgrList = (char **)mir_realloc(bkgrList, sizeof(char*)*(bkgrCount+1));
	bkgrList[bkgrCount] = value;
	bkgrCount++;
	
	return 0;
}


HRESULT BackgroundsLoadModule()
{	
	CreateServiceFunction(MS_BACKGROUNDCONFIG_REGISTER, BkgrCfg_Register);	
	return S_OK;
}

int BackgroundsUnloadModule(void)
{
	if(bkgrList != NULL)
	{
		int indx;
		for(indx = 0; indx < bkgrCount; indx++)
			if(bkgrList[indx] != NULL)
				mir_free(bkgrList[indx]);
		mir_free(bkgrList);
	}
	DestroyHookableEvent(g_CluiData.hEventBkgrChanged);
	g_CluiData.hEventBkgrChanged=NULL;

	return 0;
}





enum
{
	CBVT_NONE,
	CBVT_CHAR,
	CBVT_INT,
	CBVT_BYTE,
	CBVT_DWORD,
	CBVT_BOOL,
};

struct OptCheckBox
{
	UINT idc;

	DWORD defValue;		// should be full combined value for masked items!
	DWORD dwBit;

	BYTE dbType;
	char *dbModule;
	char *dbSetting;

	BYTE valueType;
	union
	{
		void *pValue;

		char *charValue;
		int *intValue;
		BYTE *byteValue;
		DWORD *dwordValue;
		BOOL *boolValue;
	};
};

DWORD OptCheckBox_LoadValue(struct OptCheckBox *cb)
{
	switch (cb->valueType)
	{
	case CBVT_NONE:
		switch (cb->dbType)
		{
		case DBVT_BYTE:
			return ModernGetSettingByte(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_WORD:
			return ModernGetSettingWord(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
		case DBVT_DWORD:
			return ModernGetSettingDword(NULL, cb->dbModule, cb->dbSetting, cb->defValue);
		}
		break;

	case CBVT_CHAR:
		return *cb->charValue;
	case CBVT_INT:
		return *cb->intValue;
	case CBVT_BYTE:
		return *cb->byteValue;
	case CBVT_DWORD:
		return *cb->dwordValue;
	case CBVT_BOOL:
		return *cb->boolValue;
	}

	return cb->defValue;
}

void OptCheckBox_Load(HWND hwnd, struct OptCheckBox *cb)
{
	DWORD value = OptCheckBox_LoadValue(cb);
	if (cb->dwBit) value &= cb->dwBit;
	CheckDlgButton(hwnd, cb->idc, value ? BST_CHECKED : BST_UNCHECKED);
}

void OptCheckBox_Save(HWND hwnd, struct OptCheckBox *cb)
{
	DWORD value = IsDlgButtonChecked(hwnd, cb->idc) == BST_CHECKED;

	if (cb->dwBit)
	{
		DWORD curValue = OptCheckBox_LoadValue(cb);
		value = value ? (curValue | cb->dwBit) : (curValue & ~cb->dwBit);
	}

	switch (cb->dbType)
	{
	case DBVT_BYTE:
		ModernWriteSettingByte(NULL, cb->dbModule, cb->dbSetting, (BYTE)value);
		break;
	case DBVT_WORD:
		ModernWriteSettingWord(NULL, cb->dbModule, cb->dbSetting, (WORD)value);
		break;
	case DBVT_DWORD:
		ModernWriteSettingDword(NULL, cb->dbModule, cb->dbSetting, (DWORD)value);
		break;
	}

	switch (cb->valueType)
	{
	case CBVT_CHAR:
		*cb->charValue = (char)value;
		break;
	case CBVT_INT:
		*cb->intValue = (int)value;
		break;
	case CBVT_BYTE:
		*cb->byteValue = (BYTE)value;
		break;
	case CBVT_DWORD:
		*cb->dwordValue = (DWORD)value;
		break;
	case CBVT_BOOL:
		*cb->boolValue = (BOOL)value;
		break;
	}
}

static INT_PTR CALLBACK DlgProcModernOptions(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	struct OptCheckBox opts[] =
	{
		//{IDC_, def, bit, dbtype, dbmodule, dbsetting, valtype, pval},
		{IDC_ONTOP, SETTING_ONTOP_DEFAULT, 0, DBVT_BYTE, "CList", "OnTop"},
		{IDC_AUTOHIDE, SETTING_AUTOHIDE_DEFAULT, 0, DBVT_BYTE, "CList", "AutoHide"},
//		{IDC_SHOWPROTO, TRUE,
//		{IDC_SHOWSTATUS, FALSE,
		{IDC_FADEINOUT, SETTING_FADEIN_DEFAULT, 0, DBVT_BYTE, "CLUI", "FadeInOut"},
		{IDC_TRANSPARENT, SETTING_TRANSPARENT_DEFAULT, 0, DBVT_BYTE, "CList", "Transparent"},
		{IDC_SHOWGROUPCOUNTS, GetDefaultExStyle(), CLS_EX_SHOWGROUPCOUNTS, DBVT_DWORD, "CLC", "ExStyle"},
		{IDC_HIDECOUNTSWHENEMPTY, GetDefaultExStyle(), CLS_EX_HIDECOUNTSWHENEMPTY, DBVT_DWORD, "CLC", "ExStyle"},
		{IDC_MINIMODE, SETTING_COMPACTMODE_DEFAULT, 0, DBVT_BYTE, "CLC", "CompactMode"},
		{IDC_SHOW_AVATARS, SETTINGS_SHOWAVATARS_DEFAULT, 0, DBVT_BYTE, "CList","AvatarsShow"},
		{IDC_SHOW_ANIAVATARS, FALSE, 0, DBVT_BYTE, "CList","AvatarsAnimated"},
		{IDC_SHOW, SETTING_SHOWTIME_DEFAULT, 0, DBVT_BYTE, "CList", "ContactTimeShow"},
	};

	static bool bInit = true;

	switch (msg)
	{
		case WM_INITDIALOG:
		{
			bInit = true;

			int i = 0;
			int item;
			int s1, s2, s3;

			TranslateDialogDefault(hwndDlg);
			for (i = 0; i < SIZEOF(opts); ++i)
				OptCheckBox_Load(hwndDlg, opts+i);

			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIME),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
			SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_SETRANGE,0,MAKELONG(900,1));
			SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_SETPOS,0,MAKELONG(ModernGetSettingWord(NULL,"CList","HideTime",SETTING_HIDETIME_DEFAULT),0));

			if(!IsWinVer2000Plus())
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_FADEINOUT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSPARENT),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_DROPSHADOW),FALSE);
			}

			if(!IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT))
			{
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC11),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC12),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSACTIVE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSINACTIVE),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_ACTIVEPERC),FALSE);
				EnableWindow(GetDlgItem(hwndDlg,IDC_INACTIVEPERC),FALSE);
			}
			SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_SETRANGE,FALSE,MAKELONG(1,255));
			SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_SETRANGE,FALSE,MAKELONG(1,255));
			SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_SETPOS,TRUE,ModernGetSettingByte(NULL,"CList","Alpha",SETTING_ALPHA_DEFAULT));
			SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_SETPOS,TRUE,ModernGetSettingByte(NULL,"CList","AutoAlpha",SETTING_AUTOALPHA_DEFAULT));
			SendMessage(hwndDlg,WM_HSCROLL,0x12345678,0);


			for (i=0; i<SIZEOF(sortby); i++) 
			{
				item=SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_ADDSTRING,0,(LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_SETITEMDATA,item,(LPARAM)0);
				item=SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_ADDSTRING,0,(LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_SETITEMDATA,item,(LPARAM)0);
				item=SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_ADDSTRING,0,(LPARAM)TranslateTS(sortby[i]));
				SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_SETITEMDATA,item,(LPARAM)0);

			}
			s1=ModernGetSettingByte(NULL,"CList","SortBy1",SETTING_SORTBY1_DEFAULT);
			s2=ModernGetSettingByte(NULL,"CList","SortBy2",SETTING_SORTBY2_DEFAULT);
			s3=ModernGetSettingByte(NULL,"CList","SortBy3",SETTING_SORTBY3_DEFAULT);

			for (i=0; i<SIZEOF(sortby); i++) 
			{
				if (s1==sortbyValue[i])
					SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_SETCURSEL,i,0);
				if (s2==sortbyValue[i])
					SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_SETCURSEL,i,0);
				if (s3==sortbyValue[i])
					SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_SETCURSEL,i,0);		
			}

			bInit = false;
			return TRUE;
		}

		case WM_DESTROY:
			bInit = true;
			break;

		case WM_HSCROLL:
		{	char str[10];
			sprintf(str,"%d%%",100*SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_GETPOS,0,0)/255);
			SetDlgItemTextA(hwndDlg,IDC_INACTIVEPERC,str);
			sprintf(str,"%d%%",100*SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_GETPOS,0,0)/255);
			SetDlgItemTextA(hwndDlg,IDC_ACTIVEPERC,str);
			if(wParam!=0x12345678)
				if (!bInit) SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);
			break;
		}

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
			case IDC_AUTOHIDE:
				EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIME),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
				EnableWindow(GetDlgItem(hwndDlg,IDC_HIDETIMESPIN),IsDlgButtonChecked(hwndDlg,IDC_AUTOHIDE));
				break;
			case IDC_TRANSPARENT:
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC11),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_STATIC12),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSACTIVE),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_TRANSINACTIVE),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_ACTIVEPERC),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				EnableWindow(GetDlgItem(hwndDlg,IDC_INACTIVEPERC),IsDlgButtonChecked(hwndDlg,IDC_TRANSPARENT));
				break;
			}
			if (!bInit) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			break;

		case WM_NOTIFY:
		{
			switch (((LPNMHDR) lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR) lParam)->code)
					{
						case PSN_APPLY:
						{
							int i;

							g_mutex_bChangingMode=TRUE;

							for (i = 0; i < SIZEOF(opts); ++i)
								OptCheckBox_Save(hwndDlg, opts+i);

							SetWindowPos(pcli->hwndContactList, IsDlgButtonChecked(hwndDlg,IDC_ONTOP)?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
							ModernWriteSettingWord(NULL,"CList","HideTime",(WORD)SendDlgItemMessage(hwndDlg,IDC_HIDETIMESPIN,UDM_GETPOS,0,0));

							ModernWriteSettingByte(NULL,"CList","Alpha",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TRANSACTIVE,TBM_GETPOS,0,0));
							ModernWriteSettingByte(NULL,"CList","AutoAlpha",(BYTE)SendDlgItemMessage(hwndDlg,IDC_TRANSINACTIVE,TBM_GETPOS,0,0));

							{
								int s1=SendDlgItemMessage(hwndDlg,IDC_CLSORT1,CB_GETCURSEL,0,0);
								int s2=SendDlgItemMessage(hwndDlg,IDC_CLSORT2,CB_GETCURSEL,0,0);
								int s3=SendDlgItemMessage(hwndDlg,IDC_CLSORT3,CB_GETCURSEL,0,0);
								if (s1>=0) s1=sortbyValue[s1];
								if (s2>=0) s2=sortbyValue[s2];
								if (s3>=0) s3=sortbyValue[s3];
								ModernWriteSettingByte(NULL,"CList","SortBy1",(BYTE)s1);
								ModernWriteSettingByte(NULL,"CList","SortBy2",(BYTE)s2);
								ModernWriteSettingByte(NULL,"CList","SortBy3",(BYTE)s3);
							}

							ClcOptionsChanged();
							AniAva_UpdateOptions();
							ske_LoadSkinFromDB();
							CLUI_UpdateLayeredMode();
							CLUI_ChangeWindowMode();
							SendMessage(pcli->hwndContactTree,WM_SIZE,0,0);	//forces it to send a cln_listsizechanged
							CLUI_ReloadCLUIOptions();
							cliShowHide(0,1);			
							g_mutex_bChangingMode=FALSE;
							return TRUE;
						}
					}
					break;
			}
			break;
		}
	}

	return FALSE;
}

int ModernOptInit(WPARAM wParam,LPARAM lParam)
{
	static int iBoldControls[] =
	{
		IDC_TXT_TITLE1, IDC_TXT_TITLE2,
		IDC_TXT_TITLE3, IDC_TXT_TITLE4,
		IDC_TXT_TITLE5,
		MODERNOPT_CTRL_LAST
	};

	MODERNOPTOBJECT obj = {0};

	obj.cbSize = sizeof(obj);
	obj.dwFlags = MODEROPT_FLG_TCHAR|MODEROPT_FLG_NORESIZE;
	obj.hIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
	obj.hInstance = g_hInst;
	obj.iSection = MODERNOPT_PAGE_CLIST;
	obj.iType = MODERNOPT_TYPE_SECTIONPAGE;
	obj.iBoldControls = iBoldControls;
	obj.lptzSubsection = _T("Fonts");
	obj.lpzClassicGroup = NULL;
	obj.lpzClassicPage = "Contact List";
	obj.lpzHelpUrl = "http://wiki.miranda-im.org/";

	obj.lpzTemplate = MAKEINTRESOURCEA(IDD_MODERNOPTS);
	obj.pfnDlgProc = DlgProcModernOptions;
	CallService(MS_MODERNOPT_ADDOBJECT, wParam, (LPARAM)&obj);
	return 0;
}
