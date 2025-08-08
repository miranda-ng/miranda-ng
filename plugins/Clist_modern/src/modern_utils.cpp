#include "stdafx.h"
#include "version.h"

char * __cdecl strstri(char *a, const char *b)
{
	char * x, *y;
	if (!a || !b) return FALSE;
	x = _strdup(a);
	y = _strdup(b);
	x = _strupr(x);
	y = _strupr(y);
	char * pos = strstr(x, y);
	if (pos)
	{
		char * retval = a + (pos - x);
		free(x);
		free(y);
		return retval;
	}
	free(x);
	free(y);
	return nullptr;
}

void TRACE_ERROR()
{
	uint32_t t = GetLastError();
	LPVOID lpMsgBuf;
	if (!FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		t,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		(LPTSTR)&lpMsgBuf,
		0,
		nullptr))
	{
		// Handle the error.
		return;
	}
#ifdef _DEBUG
	MessageBox(nullptr, (LPCTSTR)lpMsgBuf, L"Error", MB_OK | MB_ICONINFORMATION);
	DebugBreak();
#endif
	LocalFree(lpMsgBuf);
}

// load small icon (not shared) it IS NEED to be destroyed
HICON LoadSmallIcon(HINSTANCE hInstance, int index)
{
	wchar_t filename[MAX_PATH] = { 0 };
	GetModuleFileName(hInstance, filename, MAX_PATH);

	HICON hIcon = nullptr;
	ExtractIconEx(filename, index, nullptr, &hIcon, 1);
	return hIcon;
}

BOOL DestroyIcon_protect(HICON icon)
{
	if (icon) return DestroyIcon(icon);
	return FALSE;
}

void GetMonitorRectFromWindow(HWND hWnd, RECT *rc)
{
	POINT pt;
	GetWindowRect(hWnd, rc);
	pt.x = rc->left;
	pt.y = rc->top;

	MONITORINFO monitorInfo;
	HMONITOR hMonitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST); // always returns a valid value
	monitorInfo.cbSize = sizeof(MONITORINFO);

	if (GetMonitorInfoW(hMonitor, &monitorInfo)) {
		memcpy(rc, &monitorInfo.rcMonitor, sizeof(RECT));
		return;
	}

	// "generic" win95/NT support, also serves as failsafe
	rc->left = 0;
	rc->top = 0;
	rc->bottom = GetSystemMetrics(SM_CYSCREEN);
	rc->right = GetSystemMetrics(SM_CXSCREEN);

}

/////////////////////////////////////////////////////////////////////////////////////////
// compatibility

struct
{
	const char *name;
	int defValue;
}
static oldSettings[] = {
	{ "SBarAccountIsCustom", 0 },
	{ "HideAccount",         0 },
	{ "SBarShow",            3 },
	{ "SBarRightClk",        0 },
	{ "UseConnectingIcon",   1 },
	{ "ShowUnreadEmails",    1 },
	{ "ShowXStatus",         6 },
	{ "PaddingLeft",         0 },
	{ "PaddingRight",        0 },
};

void StatusBarProtocolOptions::fromString(char *str)
{
	char *p = strtok(str, ";");
	int *pValue = &AccountIsCustomized;

	for (auto &it : oldSettings) {
		*pValue++ = (p) ? atoi(p) : it.defValue;
		p = strtok(0, ";");
	}
}

CMStringA StatusBarProtocolOptions::toString() const
{
	CMStringA str;
	const int *pValue = &AccountIsCustomized;
	for (int i = 0; i < _countof(oldSettings); i++) {
		if (i)
			str.AppendChar(';');
		str.AppendFormat("%d", *pValue++);
	}
	return str;
}

int sttFindProtos(const char *szSetting, void *param)
{
	auto *pArray = (OBJLIST<char>*)param;
	if (!strncmp(szSetting, "HideAccount_", 12))
		pArray->insert(newStr(szSetting + 12));
	return 0;
}

void CheckCompatibility()
{
	if (g_plugin.getBool("NoOfflineBottom")) {
		g_plugin.setByte("OfflineBottom", false);
		g_plugin.delSetting("NoOfflineBottom");
	}

	int iLevel = db_get_b(0, "Compatibility", __PLUGIN_NAME);
	if (iLevel < 1 && Modern::bDisableEngine) {
		db_unset(0, "CLUI", "LeftClientMargin");
		db_unset(0, "CLUI", "RightClientMargin");
		db_unset(0, "CLUI", "TopClientMargin");
		db_unset(0, "CLUI", "BottomClientMargin");
	}

	if (iLevel < 2) {
		OBJLIST<char> arSettings(20);
		db_enum_settings(0, &sttFindProtos, "CLUI", &arSettings);

		for (auto &szProto : arSettings) {
			StatusBarProtocolOptions tmp = { szProto };

			int *pValue = &tmp.AccountIsCustomized;
			char buf[256];
			for (auto &it : oldSettings) {
				mir_snprintf(buf, "%s_%s", it.name, szProto);
				*pValue++ = db_get_dw(0, "CLUI", buf, it.defValue);
				db_unset(0, "CLUI", buf);
			}

			ptrA szBaseProto(db_get_sa(0, szProto, "AM_BaseProto"));
			if (szBaseProto)
				db_set_s(0, szProto, "ModernSbar", tmp.toString());
		}
		db_set_b(0, "Compatibility", __PLUGIN_NAME, 2);
	}
}
