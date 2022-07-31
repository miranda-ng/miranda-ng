#include "StdAfx.h"
#include "sametime.h"
#include "version.h"

// plugin stuff


PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {F1B0BA1B-0C91-4313-85EB-225069D44D01}
	{ 0xf1b0ba1b, 0xc91, 0x4313, { 0x85, 0xeb, 0x22, 0x50, 0x69, 0xd4, 0x4d, 0x1 } } 
};

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CSametimeProto>("Sametime", pluginInfoEx)
{
	SetUniqueId("stid");
}

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin g_plugin;

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////
// protocol related services

/** Copy the name of the protocol into lParam
* @param wParam :	max size of the name
* @param lParam :	reference to a char *, which will hold the name
*/
INT_PTR CSametimeProto::GetName(WPARAM wParam, LPARAM lParam)
{
	strncpy((char*)lParam, m_szModuleName, wParam);
	return 0;
}


/** Loads the icon corresponding to the status
* Called by the CList when the status changes.
* @param wParam :	icon type
* @return			an \c HICON in which the icon has been loaded.
*/
INT_PTR CSametimeProto::SametimeLoadIcon(WPARAM wParam, LPARAM)
{
	
	UINT id;
	switch (wParam & 0xFFFF) {
	case PLI_PROTOCOL:
		id = IDI_ICON_PROTO;
		break;
	default:
		return NULL;
	}

	return (INT_PTR) LoadImage(g_plugin.getInst(), MAKEINTRESOURCE(id), IMAGE_ICON,
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON),
						GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

// icolib stuff
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
	g_plugin.registerIcon("Protocols/Sametime", iconList, "SAMETIME");
}

// Copied from MSN plugin - sent acks need to be from different thread

void __cdecl sttRecvAwayThread(TFakeAckParams *tParam)
{
	CSametimeProto* proto = tParam->proto;
	proto->debugLogW(L"sttRecvAwayThread() start");

	Sleep(100);
	proto->UserRecvAwayMessage(tParam->hContact);

	proto->debugLogW(L"sttRecvAwayThread() end");
	free(tParam);
}

int CSametimeProto::OnWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *mwed = (MessageWindowEventData*)lParam;

	if ((mwed == nullptr) || Contact::IsGroupChat(mwed->hContact, m_szModuleName))
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

void CSametimeProto::OnShutdown()
{
	if (m_iStatus != ID_STATUS_OFFLINE)
		LogOut();
}

int CSametimeProto::OnSametimeContactDeleted(WPARAM hContact, LPARAM)
{
	ContactDeleted(hContact);
	ChatDeleted(hContact);
	return 0;
}

void CSametimeProto::SetAllOffline()
{
	debugLogW(L"SetAllOffline() start");

	for (auto &hContact : AccContacts()) {
		if (Contact::IsGroupChat(hContact, m_szModuleName)) {
			db_delete_contact(hContact);
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

	debugLogW(L"BroadcastNewStatus() m_iStatus=[%d], iNewStatus=[%d]", m_iStatus, iNewStatus);

	previous_status = m_iStatus;
	m_iStatus = iNewStatus;
	ProtoBroadcastAck(NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)previous_status, m_iStatus);
}
