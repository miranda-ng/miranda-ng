#include "commonheaders.h"

HINSTANCE g_hIconInst;

char TEMP[MAX_PATH];
int  TEMP_SIZE = 0;

HANDLE   g_hEvent[2], g_hCLIcon, g_hFolders = nullptr;
HGENMENU g_hMenu[15];

int iService = 0;
int iHook = 0;

HICON g_hICO[ICO_CNT], g_hPOP[POP_CNT], g_hIEC[1 + IEC_CNT*MODE_CNT] = {};
HANDLE g_IEC[1 + IEC_CNT*MODE_CNT];

int iBmpDepth;
BOOL bPGPloaded = false, bPGPkeyrings = false, bUseKeyrings = false, bPGPprivkey = false;
BOOL bGPGloaded = false, bGPGkeyrings = false, bSavePass = false;
BOOL bSFT, bSOM, bASI, bMCD, bSCM, bDGP, bAIP, bNOL, bAAK, bMCM;
uint8_t bPGP, bGPG;
mir_cs localQueueMutex;

LPSTR myDBGetStringDecode(MCONTACT hContact, const char *szModule, const char *szSetting)
{
	char *val = db_get_sa(hContact, szModule, szSetting);
	if (!val)
		return nullptr;
	
	size_t len = mir_strlen(val) + 64;
	char *buf = (LPSTR)mir_alloc(len);
	strncpy(buf, val, len); mir_free(val);
	return buf;
}

int myDBWriteStringEncode(MCONTACT hContact, const char *szModule, const char *szSetting, const char *val)
{
	int len = (int)mir_strlen(val) + 64;
	char *buf = (LPSTR)alloca(len);
	strncpy(buf, val, len);
	int ret = db_set_s(hContact, szModule, szSetting, buf);
	return ret;
}

void GetFlags()
{
	bSFT = g_plugin.getByte("sft", 0);
	bSOM = g_plugin.getByte("som", 0);
	bASI = g_plugin.getByte("asi", 0);
	bMCD = g_plugin.getByte("mcd", 0);
	bSCM = g_plugin.getByte("scm", 0);
	bDGP = g_plugin.getByte("dgp", 0);
	bAIP = g_plugin.getByte("aip", 0);
	bNOL = g_plugin.getByte("nol", 0);
	bAAK = g_plugin.getByte("aak", 0);
	bMCM = g_plugin.getByte("mcm", 0);
}

void SetFlags()
{
	g_plugin.setByte("sft", bSFT);
	g_plugin.setByte("som", bSOM);
	g_plugin.setByte("asi", bASI);
	g_plugin.setByte("mcd", bMCD);
	g_plugin.setByte("scm", bSCM);
	g_plugin.setByte("dgp", bDGP);
	g_plugin.setByte("aip", bAIP);
	g_plugin.setByte("nol", bNOL);
	g_plugin.setByte("aak", bAAK);
	g_plugin.setByte("mcm", bMCM);
}

int msgbox(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType)
{
	LPWSTR lpwText = mir_a2u(lpText);
	LPWSTR lpwCaption = mir_a2u(lpCaption);
	int r = MessageBoxW(hWnd, TranslateW(lpwText), TranslateW(lpwCaption), uType);
	mir_free(lpwCaption);
	mir_free(lpwText);
	return r;
}

HNETLIBUSER hNetlibUser;

void InitNetlib()
{
	NETLIBUSER nl_user = {};
	nl_user.szSettingsModule = (LPSTR)MODULENAME;
	nl_user.szDescriptiveName.a = (LPSTR)MODULENAME;
	nl_user.flags = NUF_NOOPTIONS;

	hNetlibUser = Netlib_RegisterUser(&nl_user);
}

void DeinitNetlib()
{
	Netlib_CloseHandle(hNetlibUser);
}

int Sent_NetLog(const char *fmt, ...)
{
	va_list va;
	char szText[1024];

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);
	return Netlib_Log(hNetlibUser, szText);
}
