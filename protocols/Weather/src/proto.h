#pragma once

struct UPDATELIST
{
	MCONTACT hContact;
	UPDATELIST *next;
};

struct WEATHERINFO
{
	MCONTACT hContact;
	TCHAR id[128];
	TCHAR city[128];
	TCHAR update[64];
	TCHAR cond[128];
	TCHAR temp[16];
	TCHAR low[16];
	TCHAR high[16];
	TCHAR feel[16];
	TCHAR wind[16];
	TCHAR winddir[64];
	TCHAR dewpoint[16];
	TCHAR pressure[16];
	TCHAR humid[16];
	TCHAR vis[16];
	TCHAR sunrise[32];
	TCHAR sunset[32];
};

struct MYOPTIONS
{
	// main options
	uint8_t AutoUpdate;
	uint8_t CAutoUpdate;
	uint8_t StartupUpdate;
	uint8_t UpdateOnlyConditionChanged;
	uint8_t RemoveOldData;
	uint8_t MakeItalic;

	uint16_t UpdateTime;
	uint16_t AvatarSize;

	// units
	uint16_t tUnit;
	uint16_t wUnit;
	uint16_t vUnit;
	uint16_t pUnit;
	uint16_t dUnit;
	uint16_t eUnit;
	wchar_t DegreeSign[4];
	uint8_t DoNotAppendUnit;
	uint8_t NoFrac;

	// advanced
	uint8_t DisCondIcon;

	// popup options
	uint8_t UpdatePopup;
	uint8_t AlertPopup;
	uint8_t PopupOnChange;
	uint8_t ShowWarnings;

	// popup colors
	uint8_t UseWinColors;
	COLORREF BGColour;
	COLORREF TextColour;

	// popup actions
	uint32_t LeftClickAction;
	uint32_t RightClickAction;

	// popup delay
	uint32_t pDelay;

	// other misc stuff
	wchar_t Default[64];
	MCONTACT DefStn;
};

/////////////////////////////////////////////////////////////////////////////////////////
// Protocol class

class CWeatherProto : public PROTO<CWeatherProto>
{
	friend class CEditDlg;
	friend class COptionsDlg;
	friend class CPopupOptsDlg;
	friend class CBriefInfoDlg;
	friend class COptionsTextDlg;
	friend class WeatherUserInfoDlg;

	class CProtoImpl
	{
		friend class CWeatherProto;
		CWeatherProto &m_proto;

		CTimer m_start, m_update;

		void OnStart(CTimer *pTimer)
		{
			pTimer->Stop();
			m_proto.StartUpdate();
		}

		void OnUpdate(CTimer *)
		{
			m_proto.DoUpdate();
		}

		CProtoImpl(CWeatherProto &pro) :
			m_proto(pro),
			m_start(Miranda_GetSystemWindow(), UINT_PTR(this) + 1),
			m_update(Miranda_GetSystemWindow(), UINT_PTR(this) + 2)
		{
			m_start.OnEvent = Callback(this, &CProtoImpl::OnStart);
			m_update.OnEvent = Callback(this, &CProtoImpl::OnUpdate);
		}
	}
		m_impl;

	// avatars
	void AvatarDownloaded(MCONTACT hContact);

	INT_PTR __cdecl AdvancedStatusIconSvc(WPARAM, LPARAM);
	INT_PTR __cdecl GetAvatarInfoSvc(WPARAM, LPARAM);

	// contacts
	INT_PTR __cdecl ViewLog(WPARAM, LPARAM);
	INT_PTR __cdecl LoadForecast(WPARAM, LPARAM);
	INT_PTR __cdecl WeatherMap(WPARAM, LPARAM);
	INT_PTR __cdecl EditSettings(WPARAM, LPARAM);

	int __cdecl BriefInfoEvt(WPARAM, LPARAM);

	bool IsMyContact(MCONTACT hContact);

	// conversions
	void numToStr(double num, wchar_t *str, size_t strSize);

	void GetTemp(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
	void GetSpeed(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
	void GetPressure(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
	void GetDist(wchar_t *tempchar, wchar_t *unit, wchar_t *str);
	void GetElev(wchar_t *tempchar, wchar_t *unit, wchar_t *str);

	// data
	void ConvertDataValue(struct WIDATAITEM *UpdateData, wchar_t *Data);
	void EraseAllInfo(void);
	void GetDataValue(WIDATAITEM *UpdateData, wchar_t *Data, wchar_t **szData);
	void GetStationID(MCONTACT hContact, wchar_t *id, int idlen);
	WEATHERINFO LoadWeatherInfo(MCONTACT hContact);

	// http
	int InternetDownloadFile(char *szUrl, char *cookie, char *userAgent, wchar_t **szData);

	// menu items
	void InitMenuItems();
	void UpdateMenu(BOOL State);

	INT_PTR __cdecl EnableDisableCmd(WPARAM, LPARAM);

	// mwin
	void AddFrameWindow(MCONTACT hContact);
	void RemoveFrameWindow(MCONTACT hContact);

	void InitMwin(void);
	void DestroyMwin(void);

	int __cdecl BuildContactMenu(WPARAM, LPARAM);

	INT_PTR __cdecl Mwin_MenuClicked(WPARAM, LPARAM);

	// options
	void LoadOptions();
	void SaveOptions();
	void RestartTimer();
	void InitPopupOptions(WPARAM);

	int __cdecl OptInit(WPARAM, LPARAM);

	CMStringW GetTextValue(int c);

	// popups
	int WPShowMessage(const wchar_t *lpzText, int kind);
	int WeatherPopup(MCONTACT hContact, bool bNew);

	// search
	bool CheckSearch();

	int IDSearch(wchar_t *id, const int searchId);
	int IDSearchProc(wchar_t *sID, const int searchId, struct WIIDSEARCH *sData, wchar_t *svc, wchar_t *svcname);

	int NameSearch(wchar_t *name, const int searchId);
	int NameSearchProc(wchar_t *name, const int searchId, struct WINAMESEARCH *sData, wchar_t *svc, wchar_t *svcname);

	void __cdecl NameSearchThread(void *);
	void __cdecl BasicSearchThread(void *);

	// update weather
	UPDATELIST *UpdateListHead = nullptr, *UpdateListTail = nullptr;

	// check if weather is currently updating
	bool m_bThreadRunning;
	mir_cs m_csUpdate;

	void DoUpdate();
	void StartUpdate();

	int  GetWeatherData(MCONTACT hContact);
	int  UpdateWeather(MCONTACT hContact);
	void UpdateListAdd(MCONTACT hContact);
	MCONTACT UpdateGetFirst();
	void DestroyUpdateList(void);

	void __cdecl UpdateThread(void *);
	void UpdateAll(BOOL AutoUpdate, BOOL RemoveOld);

	INT_PTR __cdecl UpdateSingleStation(WPARAM, LPARAM);
	INT_PTR __cdecl UpdateSingleRemove(WPARAM, LPARAM);

	INT_PTR __cdecl UpdateAllInfo(WPARAM, LPARAM);
	INT_PTR __cdecl UpdateAllRemove(WPARAM, LPARAM);

	// user info
	int __cdecl UserInfoInit(WPARAM, LPARAM);

public:
	CWeatherProto(const char *protoName, const wchar_t *userName);
	~CWeatherProto();

	MYOPTIONS opt;
	CMOption<bool> m_bPopups;

	INT_PTR __cdecl BriefInfo(WPARAM, LPARAM);

	int MapCondToStatus(MCONTACT hContact);
	HICON GetStatusIcon(MCONTACT hContact);
	HICON GetStatusIconBig(MCONTACT hContact);

	static LRESULT CALLBACK CWeatherProto::PopupWndProc(HWND hWnd, UINT uMsg, WPARAM, LPARAM);

	// PROTO_INTERFACE
	MCONTACT AddToList(int flags, PROTOSEARCHRESULT *psr) override;
	HANDLE   SearchBasic(const wchar_t *id) override;
	HANDLE   SearchAdvanced(MWindow owner) override;
	MWindow  CreateExtendedSearchUI(MWindow owner) override;

	INT_PTR  GetCaps(int type, MCONTACT hContact) override;
	int      SetStatus(int iNewStatus) override;

	void __cdecl GetAwayMsgThread(void *arg);
	HANDLE   GetAwayMsg(MCONTACT hContact) override;

	void __cdecl AckThreadProc(void *arg);
	int      GetInfo(MCONTACT hContact, int) override;

	bool     OnContactDeleted(MCONTACT, uint32_t flags) override;
	void     OnModulesLoaded() override;
	void     OnShutdown() override;

	int __cdecl OnToolbarLoaded(WPARAM, LPARAM);
};

typedef CProtoDlgBase<CWeatherProto> CWeatherDlgBase;

/////////////////////////////////////////////////////////////////////////////////////////
// Plugin class

struct CMPlugin : public ACCPROTOPLUGIN<CWeatherProto>
{
	CMPlugin();

	HINSTANCE hIconsDll = nullptr;

	int Load() override;
	int Unload() override;
};
