// TooltipNotify.h: interface for the CTooltipNotify class.
//
//////////////////////////////////////////////////////////////////////

class CTooltip;

class CTooltipNotify  
{
private:
	struct STooltipData;

public:
	CTooltipNotify(HINSTANCE hinstDLL);
	virtual ~CTooltipNotify();

	// exceptions
	class EAlreadyExists {};

	BOOL EndNotify(STooltipData* pTooltipData);
	VOID EndNotifyAll();
	CTooltip *BeginNotify(STooltipData *pTooltipData);
	VOID OnTooltipDblClicked(CTooltip *pTooltip);
	BOOL OnTooltipBeginMove(CTooltip *pTooltip);
	VOID OnTooltipEndMove(CTooltip *pTooltip);
	int InitializeOptions(WPARAM wParam, LPARAM lParam);
	int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
	int ProtoAck(WPARAM wParam, LPARAM lParam);
	int ModulesLoaded(WPARAM wParam,LPARAM lParam);
	int ProtoContactIsTyping(WPARAM wParam, LPARAM lParam);
	
	static CTooltipNotify *GetObjInstance() { return s_pInstance; }
	HINSTANCE GetDllInstance() const { return m_hDllInstance; }

private:
	// prohibit copying
	CTooltipNotify(const CTooltipNotify& rhs);
	CTooltipNotify& operator= (const CTooltipNotify& rhs);

private:
	static CTooltipNotify *s_pInstance;
	static const char *s_szModuleNameOld;
	static const char *s_szModuleName;
	
	const HINSTANCE m_hDllInstance;
	const BOOL m_bNt50;

	struct SOptions {
		BYTE  bFirstRun;
		BYTE  bOffline;
		BYTE  bOnline;
		BYTE  bOther;
		BYTE  bTyping;
		BYTE  bIdle;
		BYTE  bConjSOLN;
		BYTE  bX2;
		BYTE  bAutoPos;
		BYTE  bBallonTip;
		BYTE  bTransp;
		BYTE  bTranspInput;
		BYTE  bAlpha;
		BYTE  bLDblClick;
		BYTE  bPrefixProto;
		WORD  wDuration;
		WORD  wXPos;
		WORD  wYPos;
		WORD  wStartupDelay;
		BYTE  bIgnoreNew;
		BYTE  bIgnoreUnknown;
	} m_sOptions;

	struct STooltipData	{
		CTooltip *pTooltip;
		UINT_PTR idTimer;
		UINT uiTimeout;
		HANDLE hContact;
		int iStatus;
	};

	struct ProtoData {
		const char* proto;
		UINT_PTR timerId; 
	};

	typedef std::vector<STooltipData*> TooltipsList;
	typedef TooltipsList::iterator TooltipsListIter;
	typedef TooltipsList::reverse_iterator TooltipsListRevIter;
	TooltipsList m_TooltipsList;

	typedef std::vector<ProtoData> MapTimerIdProto;
	typedef MapTimerIdProto::iterator MapTimerIdProtoIter;
	MapTimerIdProto m_mapTimerIdProto;

	MapTimerIdProtoIter FindProtoByTimer(UINT idTimer);
	template<typename T> TooltipsListIter FindBy(T STooltipData::* m, const T& value);
	TCHAR *StatusToString(int iStatus, TCHAR *szStatus, int iBufSize);
	TCHAR *MakeTooltipString(HANDLE hContact, int iStatus, TCHAR *szString, int iBufSize);
	VOID MigrateSettings();
	void RegisterFonts();
	void GetFont(int iStatus, LOGFONT* lf, COLORREF* text, COLORREF* bg);
	void ResetCList(HWND hwndDlg);
	void LoadList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown);
	void SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown);
	VOID LoadSettings();
	VOID SaveSettings();
	VOID ValidateSettings();
	VOID ReadSettingsFromDlg(HWND hDlg);
	VOID WriteSettingsToDlg(HWND hDlg);
	
	VOID SuspendTimer(CTooltip *pTooltip);
	VOID ResumeTimer(CTooltip *pTooltip);
	VOID OnConnectionTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	VOID OnTooltipTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	// Dialog procedures
	BOOL OptionsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL ProtosDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL ContactsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static VOID CALLBACK ConnectionTimerProcWrapper(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		CTooltipNotify::GetObjInstance()->OnConnectionTimer(hwnd, uMsg, idEvent, dwTime);
	}
	static VOID CALLBACK TooltipTimerProcWrapper(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		CTooltipNotify::GetObjInstance()->OnTooltipTimer(hwnd, uMsg, idEvent, dwTime);
	}
	static INT_PTR CALLBACK OptionsDlgProcWrapper(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return CTooltipNotify::GetObjInstance()->OptionsDlgProc(hDlg, msg, wParam, lParam);
	}
	static INT_PTR CALLBACK ProtosDlgProcWrapper(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return CTooltipNotify::GetObjInstance()->ProtosDlgProc(hDlg, msg, wParam, lParam);
	}

	static INT_PTR CALLBACK ContactsDlgProcWrapper(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		return CTooltipNotify::GetObjInstance()->ContactsDlgProc(hDlg, msg, wParam, lParam);
	}

};
