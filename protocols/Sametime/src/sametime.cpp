#include "StdAfx.h"
#include "sametime.h"
#include "version.h"

// plugin stuff
PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0xf1b0ba1b, 0xc91, 0x4313, { 0x85, 0xeb, 0x22, 0x50, 0x69, 0xd4, 0x4d, 0x1 } } // {F1B0BA1B-0C91-4313-85EB-225069D44D01}
};


HINSTANCE hInst;
LIST<CSametimeProto> g_Instances(1, PtrKeySortT);
int hLangpack;

// sametime.cpp: Defines the entry point for the DLL application.
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

// protocol related services

/** Copy the name of the protocole into lParam
* @param wParam :	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/
INT_PTR CSametimeProto::GetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, m_szModuleName, wParam);
	return 0;
}

/*
 * Returns the current status
 */
INT_PTR CSametimeProto::GetStatus(WPARAM wParam, LPARAM lParam)
{
	return m_iStatus;
}


/** Loads the icon corresponding to the status
* Called by the CList when the status changes.
* @param wParam :	one of the following values : \n
					<tt>PLI_PROTOCOL | PLI_ONLINE | PLI_OFFLINE</tt>
* @return			an \c HICON in which the icon has been loaded.
*/
INT_PTR CSametimeProto::SametimeLoadIcon(WPARAM wParam, LPARAM lParam)
{
	
	UINT id;
	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL:
		id = IDI_ICON_PROTO;
		break;
	default:
		return NULL;
	}

	return (INT_PTR) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}


//icolib stuff
static IconItem iconList[] =
{
	{ LPGEN("Protocol icon"), "protoicon", IDI_ICON_PROTO, 0 },
	{ LPGEN("Start conference"), "leaveconference", IDI_ICON_INVITE, 0 },
	{ LPGEN("Leave conference"), "startconference", IDI_ICON_LEAVE, 0 },
	{ LPGEN("Announce"), "announce", IDI_ICON_ANNOUNCE, 0 },
	{ LPGEN("Notification"), "notify", IDI_ICON_NOTIFY, 0 },
	{ LPGEN("Error"), "error", IDI_ICON_ERROR, 0 }
};

void SametimeInitIcons(void)
{
	Icon_Register(hInst, "Protocols/Sametime", iconList, SIZEOF(iconList), "SAMETIME");
}

HANDLE GetIconHandle(int iconId)
{
	for (int i = 0; i < SIZEOF(iconList); i++)
		if (iconList[i].defIconID == iconId)
			return iconList[i].hIcolib;
	return NULL;
}

HICON LoadIconEx(const char* name, BOOL big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", "SAMETIME", name);
	return Skin_GetIcon(szSettingName, big);
}

void ReleaseIconEx(const char* name, BOOL big)
{
	char szSettingName[100];
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%s_%s", "SAMETIME", name);
	Skin_ReleaseIcon(szSettingName, big);
}

// Copied from MSN plugin - sent acks need to be from different thread
void __cdecl sttFakeAckInfoSuccessThread(void *param)
{
	TFakeAckParams* tParam = (TFakeAckParams*)param;
	CSametimeProto* proto = tParam->proto;
	proto->debugLog(_T("sttFakeAckInfoSuccessThread() start"));

	Sleep(100);
	proto->ProtoBroadcastAck(tParam->hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE)1, 0);

	proto->debugLog(_T("sttFakeAckInfoSuccessThread() end"));
	mir_free(tParam);
}

void __cdecl sttFakeAckMessageSuccessThread(void *param)
{
	TFakeAckParams* tParam = (TFakeAckParams*)param;
	CSametimeProto* proto = tParam->proto;
	proto->debugLog(_T("sttFakeAckMessageSuccessThread() start"));

	Sleep(100);
	proto->ProtoBroadcastAck(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_SUCCESS, (HANDLE)tParam->lParam, 0);

	proto->debugLog(_T("sttFakeAckMessageSuccessThread() end"));
	mir_free(tParam);
}

void __cdecl sttFakeAckMessageFailedThread(void *param)
{
	TFakeAckParams* tParam = (TFakeAckParams*)param;
	CSametimeProto* proto = tParam->proto;
	proto->debugLog(_T("sttFakeAckMessageFailedThread() start"));

	Sleep(100);
	proto->ProtoBroadcastAck(tParam->hContact, ACKTYPE_MESSAGE, ACKRESULT_FAILED, NULL, tParam->lParam); ///TODO tParam->lParam: add error message

	proto->debugLog(_T("sttFakeAckMessageFailedThread() end"));
	mir_free(tParam);
}

void __cdecl sttRecvAwayThread(void *param)
{
	TFakeAckParams* tParam = (TFakeAckParams*)param;
	CSametimeProto* proto = tParam->proto;
	proto->debugLog(_T("sttRecvAwayThread() start"));

	Sleep(100);
	proto->UserRecvAwayMessage(tParam->hContact);

	proto->debugLog(_T("sttRecvAwayThread() end"));
	free(tParam);
}

int CSametimeProto::OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *mwed = (MessageWindowEventData*)lParam;

	if ((mwed == NULL) || db_get_b(mwed->hContact, m_szModuleName, "ChatRoom", 0))
		return 0;

	if ((mwed->uType == MSG_WINDOW_EVT_CLOSING) || (mwed->uType == MSG_WINDOW_EVT_CLOSE))
		CloseIm(mwed->hContact);

	return 0;
}

int CSametimeProto::OnIdleChanged(WPARAM, LPARAM lParam)
{
	if (!(lParam & IDF_PRIVACY)) {
		is_idle = lParam & IDF_ISIDLE ? true : false;
		SetIdle(is_idle);
	}

	return 0;
}

int CSametimeProto::OnModulesLoaded(WPARAM, LPARAM)
{
	// register with chat module
	GCREGISTER gcr = { sizeof(gcr) };
	gcr.pszModule = m_szModuleName;
	gcr.ptszDispName = m_tszUserName;
	gcr.iMaxText = MAX_MESSAGE_SIZE;
	CallService(MS_GC_REGISTER, 0, (LPARAM)&gcr);
	return 0;
}

int CSametimeProto::OnPreShutdown(WPARAM, LPARAM)
{
	if (m_iStatus != ID_STATUS_OFFLINE)
		LogOut();

	return 0;
}

int CSametimeProto::OnSametimeContactDeleted(WPARAM hContact, LPARAM)
{
	ContactDeleted(hContact);
	ChatDeleted(hContact);
	return 0;
}

void CSametimeProto::SetAllOffline()
{
	debugLog(_T("SetAllOffline() start"));

	for (MCONTACT hContact = db_find_first(m_szModuleName); hContact; hContact = db_find_next(hContact, m_szModuleName)) {
		if (db_get_b(hContact, m_szModuleName, "ChatRoom", 0)) {
			CallService(MS_DB_CONTACT_DELETE, (WPARAM)hContact, 0);
			continue;
		}

		db_set_w(hContact, m_szModuleName, "Status", ID_STATUS_OFFLINE);
		db_set_dw(hContact, m_szModuleName, "IdleTS", 0);
	}
}

void CSametimeProto::BroadcastNewStatus(int iNewStatus)
{
	if (m_iStatus == iNewStatus)
		return;

	debugLog(_T("BroadcastNewStatus() m_iStatus=[%d], iNewStatus=[%d]"), m_iStatus, iNewStatus);

	previous_status = m_iStatus;
	m_iStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)previous_status, m_iStatus);
}

static CSametimeProto* sametime_proto_init(const char* pszProtoName, const TCHAR* tszUserName)
{
	CSametimeProto* proto = new CSametimeProto(pszProtoName, tszUserName);
	g_Instances.insert(proto);
	return proto;
}

static int sametime_proto_uninit(PROTO_INTERFACE* ppro)
{
	CSametimeProto* proto = (CSametimeProto*)ppro;
	g_Instances.remove(proto);
	delete proto;
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.type = PROTOTYPE_PROTOCOL;
	pd.szName = "Sametime";
	pd.fnInit = (pfnInitProto)sametime_proto_init;
	pd.fnUninit = (pfnUninitProto)sametime_proto_uninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);
	return 0;
}

extern "C" int __declspec(dllexport) Unload()
{
	g_Instances.destroy();
	return 0;
}

