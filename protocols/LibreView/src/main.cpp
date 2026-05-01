#include "stdafx.h"
#include "version.h"

CMPlugin g_plugin;
HNETLIBUSER hNetlibUser;
UINT_PTR hTimer;
OBJLIST<Account> g_accs(1);
static HGENMENU g_hContactMenuUpdate = nullptr;

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
	// {D0D8A5BE-A76A-46B1-92B6-3D69924E211D}
	{ 0xd0d8a5be, 0xa76a, 0x46b1, { 0x92, 0xb6, 0x3d, 0x69, 0x92, 0x4e, 0x21, 0x1d } }
};

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<CLibreViewProto>(MODULENAME, pluginInfoEx)
{
	SetUniqueId("PatientId");
}

CLibreViewProto::CLibreViewProto(const char *protoName, const wchar_t *userName) :
	PROTO<CLibreViewProto>(protoName, userName),
	UpdateInterval(m_szModuleName, "UpdateInterval", db_get_dw(0, MODULENAME, "UpdateInterval", 5)),
	DisplayUnits(m_szModuleName, "DisplayUnits", db_get_dw(0, MODULENAME, "DisplayUnits", 0)),
	WriteHistory(m_szModuleName, "WriteHistory", db_get_b(0, MODULENAME, "WriteHistory", 0) != 0)
{
	m_hProtoIcon = Skin_LoadProtoIcon(MODULENAME, ID_STATUS_ONLINE);
	m_account = EnsureAccount(this);

	CreateProtoService("/Update", &CLibreViewProto::Update);
	HookProtoEvent(ME_OPT_INITIALISE, &CLibreViewProto::OptInit);
}

CLibreViewProto::~CLibreViewProto()
{
	if (m_account) {
		g_accs.remove(m_account);
		m_account = nullptr;
	}
}

INT_PTR CLibreViewProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_2:
	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND;
	}

	return 0;
}

int CLibreViewProto::SetStatus(int iStatus)
{
	int oldStatus = m_iStatus;
	m_iStatus = (iStatus == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	if (m_account && m_account->hContact) {
		if (m_iStatus == ID_STATUS_OFFLINE)
			setWord(m_account->hContact, "Status", ID_STATUS_OFFLINE);
		else
			mir_forkthread(Check_ThreadFunc, m_account);
	}
	return 0;
}

void CLibreViewProto::OnModulesLoaded()
{
}

void CLibreViewProto::OnShutdown()
{
	if (m_account && m_account->hContact)
		setWord(m_account->hContact, "Status", ID_STATUS_OFFLINE);
}

INT_PTR CLibreViewProto::Update(WPARAM, LPARAM)
{
	if (m_account)
		mir_forkthread(Check_ThreadFunc, m_account);
	return 0;
}

static INT_PTR PluginMenuCommand(WPARAM hContact, LPARAM)
{
	mir_forkthread(Check_ThreadFunc, GetAccountByContact(hContact));
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(g_hContactMenuUpdate, g_plugin.getInstance(hContact) != nullptr);
	return 0;
}

Account* EnsureAccount(CLibreViewProto *ppro)
{
	for (auto &it : g_accs)
		if (it->ppro == ppro)
			return it;

	MCONTACT hContact = 0;
	for (auto &it : Contacts(ppro->m_szModuleName)) {
		hContact = it;
		break;
	}

	if (hContact != 0)
		ppro->setWord(hContact, "Status", ID_STATUS_OFFLINE);

	Account *pAcc = new Account(ppro, hContact);
	g_accs.insert(pAcc);
	return pAcc;
}

MCONTACT EnsureAccountContact(Account *pAcc)
{
	if (pAcc == nullptr)
		return 0;

	if (pAcc->hContact != 0)
		return pAcc->hContact;

	MCONTACT hContact = db_add_contact();
	if (hContact == 0)
		return 0;

	Proto_AddToContact(hContact, pAcc->ppro->m_szModuleName);

	ptrW wszEmail(pAcc->ppro->getWStringA(0, "Email"));
	if (mir_wstrlen(wszEmail))
		pAcc->ppro->setWString(hContact, "Nick", wszEmail);
	else
		pAcc->ppro->setWString(hContact, "Nick", TranslateT("LibreView"));

	CMStringW wszApiUrl(pAcc->ppro->getMStringW(0, "ApiUrl"));
	if (wszApiUrl.IsEmpty())
		wszApiUrl = _A2W(DEFAULT_API_URL);
	pAcc->ppro->setWString(hContact, "ApiUrl", wszApiUrl);
	pAcc->ppro->setWord(hContact, "Status", ID_STATUS_OFFLINE);

	pAcc->hContact = hContact;
	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
	return hContact;
}

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD)
{
	time_t now = time(0);
	for (auto &it : g_accs) {
		if (it->hContact == 0)
			continue;

		uint32_t minutes = it->ppro->UpdateInterval;
		if (minutes == 0)
			continue;

		if (it->tsLastUpdate == 0 || now - it->tsLastUpdate >= time_t(minutes * 60))
			mir_forkthread(Check_ThreadFunc, it);
	}
}

void RestartTimer()
{
	if (hTimer)
		KillTimer(nullptr, hTimer);

	hTimer = SetTimer(nullptr, 0, 60000, TimerProc);
}

int CMPlugin::Load()
{
	NETLIBUSER nlu = {};
	nlu.flags = NUF_OUTGOING | NUF_HTTPCONNS | NUF_NOHTTPSOPTION | NUF_UNICODE;
	nlu.szSettingsModule = MODULENAME;
	nlu.szDescriptiveName.w = TranslateT("LibreView");
	hNetlibUser = Netlib_RegisterUser(&nlu);

	CreateServiceFunction(MODULENAME "/Update", PluginMenuCommand);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x82c22dbb, 0x9768, 0x4c1c, 0x9d, 0x72, 0x8d, 0x63, 0x3f, 0xf0, 0xe1, 0xad);
	mi.position = -0x7FFFFFFF;
	mi.hIcolibItem = Skin_GetProtoIcon(MODULENAME, ID_STATUS_ONLINE);
	mi.name.a = LPGEN("&Update data");
	mi.pszService = MODULENAME "/Update";
	g_hContactMenuUpdate = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_USERINFO_INITIALISE, UserInfoInit);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
	RestartTimer();
	return 0;
}

int CMPlugin::Unload()
{
	if (hTimer)
		KillTimer(nullptr, hTimer);

	if (hNetlibUser)
		Netlib_CloseHandle(hNetlibUser);
	return 0;
}
