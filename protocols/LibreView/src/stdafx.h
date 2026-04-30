#pragma once

#include <windows.h>
#include <time.h>

#include <newpluginapi.h>
#include <m_clc.h>
#include <m_clist.h>
#include <m_database.h>
#include <m_history.h>
#include <m_ignore.h>
#include <m_json.h>
#include <m_langpack.h>
#include <m_netlib.h>
#include <m_options.h>
#include <m_protoint.h>
#include <m_protocols.h>
#include <m_protosvc.h>
#include <m_skin.h>
#include <m_timezones.h>
#include <m_userinfo.h>
#include <m_utils.h>

#include "resource.h"

#define MODULENAME "LibreView"
#define DB_MODULE_GLUCOSE "LibreViewData"
#define DEFAULT_API_URL "https://api.libreview.ru"
#define DEFAULT_API_VERSION "4.16.0"

class CLibreViewProto;

struct Account : public MZeroedObject
{
	Account(CLibreViewProto*, MCONTACT);

	mir_cs csLock;
	CLibreViewProto *ppro;
	MCONTACT hContact;
	CMStringA szToken, szAccountHash, szPatientId, szApiUrl, szMinVersion;
	time_t tsLastUpdate = 0;
	bool bChecking = false;

	bool Login();
	bool FetchConnections();
	bool FetchGlucose();
	void ClearAuth();
};

extern HNETLIBUSER hNetlibUser;
extern UINT_PTR hTimer;
extern OBJLIST<Account> g_accs;

void CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
void Check_ThreadFunc(void *);
void CheckAccount(Account *);
Account* GetAccountByContact(MCONTACT);
Account* EnsureAccount(CLibreViewProto*);
void UpdateContactDisplay(MCONTACT);
void RestartTimer();
int UserInfoInit(WPARAM, LPARAM);

class CLibreViewProto : public PROTO<CLibreViewProto>
{
public:
	CLibreViewProto(const char*, const wchar_t*);
	~CLibreViewProto();

	CMOption<uint32_t> UpdateInterval;
	CMOption<uint32_t> DisplayUnits;
	CMOption<bool> AutoUpdate;
	Account *m_account = nullptr;

	INT_PTR __cdecl Update(WPARAM, LPARAM);
	int __cdecl OptInit(WPARAM, LPARAM);

	INT_PTR GetCaps(int type, MCONTACT hContact = 0) override;
	int SetStatus(int iNewStatus) override;
	void OnModulesLoaded() override;
	void OnShutdown() override;
};

typedef CProtoDlgBase<CLibreViewProto> CLibreViewDlgBase;

struct CMPlugin : public ACCPROTOPLUGIN<CLibreViewProto>
{
	CMPlugin();

	int Load() override;
	int Unload() override;
};

extern CMPlugin g_plugin;
