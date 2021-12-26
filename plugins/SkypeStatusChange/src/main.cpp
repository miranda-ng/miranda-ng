#include "stdafx.h"
#include "resource.h"
#include "Version.h"

#pragma comment(lib, "Comctl32.lib")

CMPlugin g_plugin;

int SSC_OptInitialise(WPARAM wp, LPARAM lp);

UINT g_MsgIDSkypeControlAPIAttach = 0;
UINT g_MsgIDSkypeControlAPIDiscover = 0;
HWND g_wndMainWindow = nullptr;

bool g_bMirandaIsShutdown = false;

HANDLE g_hThread = nullptr;
HANDLE g_hEventShutdown = nullptr;

/////////////////////////////////////////////////////////////////////////////////////////

static PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0x2925520b, 0x6677, 0x4658, { 0x8b, 0xad, 0x56, 0x61, 0xd1, 0x3e, 0x46, 0x92 }}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("Change Skype Status", pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

enum
{
	SKYPECONTROLAPI_ATTACH_SUCCESS = 0,								// Client is successfully attached and API window handle can be found in wParam parameter
	SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION = 1,	// Skype has acknowledged connection request and is waiting for confirmation from the user.
	// The client is not yet attached and should wait for SKYPECONTROLAPI_ATTACH_SUCCESS message
	SKYPECONTROLAPI_ATTACH_REFUSED = 2,								// User has explicitly denied access to client
	SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE = 3,					// API is not available at the moment. For example, this happens when no user is currently logged in.
	// Client should wait for SKYPECONTROLAPI_ATTACH_API_AVAILABLE broadcast before making any further
	// connection attempts.
	SKYPECONTROLAPI_ATTACH_API_AVAILABLE = 0x8001
};

LPCTSTR g_pszSkypeWndClassName = L"SkypeHelperWindow{155198f0-8749-47b7-ac53-58f2ac70844c}";

const CMirandaStatus2SkypeStatus g_aStatusCode[MAX_STATUS_COUNT] =
{
	{ID_STATUS_AWAY, "AWAY",L"Away"},
	{ID_STATUS_NA, "AWAY",L"Not available"}, // removed in Skype 5
	{ID_STATUS_DND, "DND",L"Do not disturb"},
	{ID_STATUS_ONLINE, "ONLINE",L"Online"},
	{ID_STATUS_FREECHAT, "ONLINE",L"Free for chat"},	// SKYPEME status doesn't work in Skype 4!
	{ID_STATUS_OFFLINE, "OFFLINE",L"Offline"},
	{ID_STATUS_INVISIBLE, "INVISIBLE",L"Invisible"},
	{ID_STATUS_OCCUPIED,"DND",L"Occupied"}
};

enum { INVALID_INDEX = 0xFFFFFFFF };

class CStatusInfo
{
public:
	CStatusInfo() : m_nStatusIndex(INVALID_INDEX) { m_szModule[0] = '\0'; }

	size_t StatusIndex()const { return m_nStatusIndex; }
	void StatusIndex(size_t val) { m_nStatusIndex = val; }

	const char* Module()const { return m_szModule; }
	void Module(const char* val)
	{
		if (val)
			strncpy_s(m_szModule, val, mir_strlen(val));
		else
			m_szModule[0] = '\0';
	}

private:
	char m_szModule[MAXMODULELABELLENGTH];
	size_t m_nStatusIndex;
};

COptions g_Options;
CStatusInfo g_CurrStatusInfo;
mir_cs g_csStatusInfo;

int SSC_OnProtocolAck(WPARAM, LPARAM lParam)
{
	if (g_bMirandaIsShutdown)
		return 0;

	ACKDATA* pAckData = reinterpret_cast<ACKDATA*>(lParam);
	if (pAckData->type != ACKTYPE_STATUS || pAckData->result != ACKRESULT_SUCCESS || !pAckData->szModule)
		return 0;

	if (!g_Options.IsProtocolExcluded(pAckData->szModule)) {
		int nStatus = Proto_GetStatus(pAckData->szModule);
		for (size_t i = 0; i < _countof(g_aStatusCode); ++i) {
			const CMirandaStatus2SkypeStatus& ms = g_aStatusCode[i];
			if (ms.m_nMirandaStatus == nStatus) {
				int nPrevStatus;
				if ((false == g_Options.IsProtocolStatusExcluded(pAckData->szModule, nStatus))
					&& ((false == g_Options.GetSyncStatusStateFlag())
						|| (false == g_Options.GetPreviousStatus(pAckData->szModule, nPrevStatus))
						|| (nPrevStatus != nStatus))) {
							{
								mir_cslock guard(g_csStatusInfo);
								g_CurrStatusInfo.StatusIndex(i);
								g_CurrStatusInfo.Module(pAckData->szModule);
							}
							if (0 == ::PostMessage(HWND_BROADCAST, g_MsgIDSkypeControlAPIDiscover, (WPARAM)g_wndMainWindow, 0)) {
								mir_cslock guard(g_csStatusInfo);
								g_CurrStatusInfo.StatusIndex(INVALID_INDEX);
								g_CurrStatusInfo.Module(nullptr);
							}
							else g_Options.SetPreviousStatus(pAckData->szModule, nStatus);
				}
				break;
			}
		}
	}

	return 0;
}

static void ThreadFunc(void*)
{
	while (true) {
		MSG msg;
		if (TRUE == ::PeekMessage(&msg, g_wndMainWindow, 0, 0, PM_NOREMOVE)) {
			while (::GetMessage(&msg, g_wndMainWindow, 0, 0)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else {
			uint32_t dwResult = ::MsgWaitForMultipleObjects(1, &g_hEventShutdown, FALSE, INFINITE, QS_ALLEVENTS);
			assert(WAIT_FAILED != dwResult);
			if (WAIT_OBJECT_0 == dwResult)
				break;
		}
	}
}

LRESULT APIENTRY SkypeAPI_WindowProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
	LRESULT lReturnCode = 0;
	bool bIssueDefProc = false;

	switch (msg) {
	case WM_DESTROY:
		g_wndMainWindow = nullptr;
		break;

	case WM_COPYDATA:
		break;

	default:
		if (msg == g_MsgIDSkypeControlAPIAttach) {
			switch (lp) {
			case SKYPECONTROLAPI_ATTACH_SUCCESS:
				{
					CStatusInfo si;
					{
						mir_cslock guard(g_csStatusInfo);
						si = g_CurrStatusInfo;
					}
					if (INVALID_INDEX != si.StatusIndex() && si.StatusIndex() < _countof(g_aStatusCode)) {
						const CMirandaStatus2SkypeStatus& ms = g_aStatusCode[si.StatusIndex()];
						HWND wndSkypeAPIWindow = reinterpret_cast<HWND>(wp);

						enum { BUFFER_SIZE = 256 };
						char szSkypeCmd[BUFFER_SIZE];
						const char szSkypeCmdSetStatus[] = "SET USERSTATUS ";
						::strncpy_s(szSkypeCmd, szSkypeCmdSetStatus, sizeof(szSkypeCmdSetStatus) / sizeof(szSkypeCmdSetStatus[0]));
						::strncat_s(szSkypeCmd, ms.m_pszSkypeStatus, _countof(szSkypeCmd) - mir_strlen(szSkypeCmd));
						uint32_t cLength = static_cast<uint32_t>(mir_strlen(szSkypeCmd));

						COPYDATASTRUCT oCopyData;
						oCopyData.dwData = 0;
						oCopyData.lpData = szSkypeCmd;
						oCopyData.cbData = cLength + 1;
						SendMessage(wndSkypeAPIWindow, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&oCopyData);
						if (g_Options.GetSyncStatusMsgFlag()) {
							wchar_t* pszStatusMsg = nullptr;
							if (ProtoServiceExists(si.Module(), PS_GETMYAWAYMSG))
								pszStatusMsg = reinterpret_cast<wchar_t*>(CallProtoService(si.Module(), PS_GETMYAWAYMSG, (WPARAM)ms.m_nMirandaStatus, SGMA_UNICODE));

							if ((nullptr == pszStatusMsg) || (CALLSERVICE_NOTFOUND == INT_PTR(pszStatusMsg)))
								pszStatusMsg = reinterpret_cast<wchar_t*>(CallService(MS_AWAYMSG_GETSTATUSMSGW, (WPARAM)ms.m_nMirandaStatus, 0));

							if (pszStatusMsg && reinterpret_cast<LPARAM>(pszStatusMsg) != CALLSERVICE_NOTFOUND) {
								T2Utf pMsg(pszStatusMsg);
								mir_free(pszStatusMsg);

								const char szSkypeCmdSetStatusMsg[] = "SET PROFILE MOOD_TEXT ";
								::strncpy_s(szSkypeCmd, szSkypeCmdSetStatusMsg, sizeof(szSkypeCmdSetStatusMsg) / sizeof(szSkypeCmdSetStatusMsg[0]));
								::strncat_s(szSkypeCmd, pMsg, _countof(szSkypeCmd) - mir_strlen(szSkypeCmd));

								oCopyData.dwData = 0;
								oCopyData.lpData = szSkypeCmd;
								oCopyData.cbData = static_cast<uint32_t>(mir_strlen(szSkypeCmd)) + 1;
								SendMessage(wndSkypeAPIWindow, WM_COPYDATA, (WPARAM)hWnd, (LPARAM)&oCopyData);
							}
						}
					}
				}
				break;

			case SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION:
				break;

			case SKYPECONTROLAPI_ATTACH_REFUSED:
				break;

			case SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE:
				break;

			case SKYPECONTROLAPI_ATTACH_API_AVAILABLE:
				break;
			}
			lReturnCode = 1;
		}
		else bIssueDefProc = true;
		break;
	}

	if (true == bIssueDefProc)
		lReturnCode = DefWindowProc(hWnd, msg, wp, lp);

	return lReturnCode;
}

int SSC_OnPreShutdown(WPARAM/* wParam*/, LPARAM/* lParam*/)
{
	g_bMirandaIsShutdown = true;
	BOOL b = SetEvent(g_hEventShutdown);
	assert(b && "SetEvent failed");

	uint32_t dwResult = ::WaitForSingleObject(g_hThread, INFINITE);
	assert(WAIT_FAILED != dwResult);

	b = ::CloseHandle(g_hEventShutdown);
	assert(b && "CloseHandle event");

	if (g_wndMainWindow) {
		b = DestroyWindow(g_wndMainWindow);
		assert(b && "DestoryWindow");
		g_wndMainWindow = nullptr;
	}

	UnregisterClass(g_pszSkypeWndClassName, g_plugin.getInst());
	return 0;
}

/******************************* INSTALLATION PROCEDURES *****************************/

int CMPlugin::Load()
{
	g_MsgIDSkypeControlAPIAttach = ::RegisterWindowMessage(L"SkypeControlAPIAttach");
	g_MsgIDSkypeControlAPIDiscover = ::RegisterWindowMessage(L"SkypeControlAPIDiscover");
	if ((0 == g_MsgIDSkypeControlAPIAttach) || (0 == g_MsgIDSkypeControlAPIDiscover))
		return 1;

	WNDCLASS oWindowClass = { 0 };
	oWindowClass.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	oWindowClass.lpfnWndProc = (WNDPROC)&SkypeAPI_WindowProc;
	oWindowClass.hInstance = g_plugin.getInst();
	oWindowClass.lpszClassName = g_pszSkypeWndClassName;
	if (!RegisterClass(&oWindowClass))
		return 1;

	g_wndMainWindow = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_WINDOWEDGE,
		g_pszSkypeWndClassName, L"", WS_BORDER | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT, 128, 128, nullptr, nullptr, g_plugin.getInst(), nullptr);
	if (g_wndMainWindow == nullptr)
		return 1;

	g_bMirandaIsShutdown = false;
	g_hEventShutdown = ::CreateEvent(nullptr, TRUE, FALSE, nullptr);

	g_hThread = mir_forkthread(ThreadFunc);

	HookEvent(ME_PROTO_ACK, SSC_OnProtocolAck);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, SSC_OnPreShutdown);
	HookEvent(ME_OPT_INITIALISE, SSC_OptInitialise);
	return 0;
}
