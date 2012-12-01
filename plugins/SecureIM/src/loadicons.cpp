#include "commonheaders.h"

struct
{
	UINT key; // Resource ID
	BYTE tbl; // Table NUM
	BYTE idx; // Table IDX
	char *section;
	char *name;
	char *text;
}
static icons[] =
{
	// Contact List
	{IDI_CL_DIS,	TBL_IEC, IEC_CL_DIS,	MODULENAME"/Contact List", "sim_cl_dis", "Connection Disabled"},
	{IDI_CL_EST,	TBL_IEC, IEC_CL_EST,	MODULENAME"/Contact List", "sim_cl_est", "Connection Established"},

	// Contact Menu
	{IDI_CM_DIS,	TBL_ICO, ICO_CM_DIS,	MODULENAME"/Contact Menu", "sim_cm_dis", "Disable Secure Connection"},
	{IDI_CM_EST,	TBL_ICO, ICO_CM_EST,	MODULENAME"/Contact Menu", "sim_cm_est", "Establishe Secure Connection"},

	// Message Window
	{IDI_MW_DIS,	TBL_ICO, ICO_MW_DIS,	MODULENAME"/Message Window", "sim_mw_dis", "Connection Disabled"},
	{IDI_MW_EST,	TBL_ICO, ICO_MW_EST,	MODULENAME"/Message Window", "sim_mw_est", "Connection Established"},

	// popup's
	{IDI_PU_DIS,	TBL_POP, POP_PU_DIS,	MODULENAME"/Popups", "sim_pu_dis", "Secure Connection Disabled"},
	{IDI_PU_EST,	TBL_POP, POP_PU_EST,	MODULENAME"/Popups", "sim_pu_est", "Secure Connection Established"},
	{IDI_PU_PRC,	TBL_POP, POP_PU_PRC,	MODULENAME"/Popups", "sim_pu_prc", "Secure Connection In Process"},
	{IDI_PU_MSG,	TBL_POP, POP_PU_MSR,	MODULENAME"/Popups", "sim_pu_msr", "Recv Secured Message"},
	{IDI_PU_MSG,	TBL_POP, POP_PU_MSS,	MODULENAME"/Popups", "sim_pu_mss", "Sent Secured Message"},

	// statuses
	{IDI_ST_DIS,	TBL_ICO, ICO_ST_DIS,	MODULENAME"/Menu State", "sim_st_dis", "Disabled"},
	{IDI_ST_ENA,	TBL_ICO, ICO_ST_ENA,	MODULENAME"/Menu State", "sim_st_ena", "Enabled"},
	{IDI_ST_TRY,	TBL_ICO, ICO_ST_TRY,	MODULENAME"/Menu State", "sim_st_try", "Always Try"},

	// overlay
	{IDI_OV_NAT,	TBL_ICO, ICO_OV_NAT,	MODULENAME"/Overlays", "sim_ov_nat", "Native mode"},
	{IDI_OV_PGP,	TBL_ICO, ICO_OV_PGP,	MODULENAME"/Overlays", "sim_ov_pgp", "PGP mode"},
	{IDI_OV_GPG,	TBL_ICO, ICO_OV_GPG,	MODULENAME"/Overlays", "sim_ov_gpg", "GPG mode"},
	{IDI_OV_RSA,	TBL_ICO, ICO_OV_RSA,	MODULENAME"/Overlays", "sim_ov_rsa", "RSA/AES mode"},
};

HINSTANCE LoadIconsPack(const char* szIconsPack)
{
	HINSTANCE hNewIconInst = LoadLibrary(szIconsPack);
	if (hNewIconInst != NULL) {
		for(int i=ID_FIRSTICON; i <= ID_LASTICON; i++)
			if (LoadIcon(hNewIconInst, MAKEINTRESOURCE(i)) == NULL) {
				FreeLibrary(hNewIconInst);
				hNewIconInst = NULL;
				break;
			}
	}
	return hNewIconInst;
}

int ReloadIcons(WPARAM wParam, LPARAM lParam)
{
	for (int i=0; i < SIZEOF(icons); i++) {
		HICON hIcon = Skin_GetIcon(icons[i].name);
		if (icons[i].tbl == TBL_IEC)
			g_hIEC[icons[i].idx] = hIcon;
		else if (icons[i].tbl == TBL_ICO)
			g_hICO[icons[i].idx] = hIcon;
		else if (icons[i].tbl == TBL_POP)
			g_hPOP[icons[i].idx] = hIcon;
	}

	return 0;
}

void InitIcons(void)
{
	HINSTANCE hNewIconInst = NULL;

	if ( g_hFolders ) {
		TCHAR pathname[MAX_PATH];
		FoldersGetCustomPathExT(g_hFolders, pathname, MAX_PATH, "icons\\");
		if (hNewIconInst == NULL)
			hNewIconInst = LoadIconsPack(pathname);
	}

	if (hNewIconInst == NULL)
		hNewIconInst = LoadIconsPack("icons\\secureim_icons.dll");

	if (hNewIconInst == NULL)
		hNewIconInst = LoadIconsPack("plugins\\secureim_icons.dll");

	if (hNewIconInst == NULL)
		g_hIconInst = g_hInst;
	else
		g_hIconInst = hNewIconInst;

	TCHAR tszPath[MAX_PATH];
	GetModuleFileName(g_hIconInst, tszPath, SIZEOF(tszPath));

	SKINICONDESC sid = { sizeof(sid) };
	sid.pszSection = "SecureIM";
	sid.ptszDefaultFile = tszPath;

	for (int i=0; i < SIZEOF(icons); i++) {
		sid.pszSection = icons[i].section;
		sid.pszName = icons[i].name;
		sid.pszDescription = icons[i].text;
		sid.iDefaultIndex = -icons[i].key;
		HANDLE hIcolib = Skin_AddIcon(&sid);
		
		HICON hIcon = Skin_GetIconByHandle(hIcolib);
		if (icons[i].tbl == TBL_IEC)
			g_hIEC[icons[i].idx] = hIcon;
		else if (icons[i].tbl == TBL_ICO)
			g_hICO[icons[i].idx] = hIcon;
		else if (icons[i].tbl == TBL_POP)
			g_hPOP[icons[i].idx] = hIcon;
	}

	AddHookFunction(ME_SKIN2_ICONSCHANGED, ReloadIcons);
}

// EOF
