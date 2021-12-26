// TooltipNotify.h: interface for the CTooltipNotify class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CTooltip;

class CTooltipNotify : public MNonCopyable
{
private:
	struct STooltipData;

public:
	CTooltipNotify();
	virtual ~CTooltipNotify();

	// exceptions
	class EAlreadyExists {};

	BOOL EndNotify(STooltipData* pTooltipData);
	void EndNotifyAll();
	CTooltip *BeginNotify(STooltipData *pTooltipData);
	void OnTooltipDblClicked(CTooltip *pTooltip);
	BOOL OnTooltipBeginMove(CTooltip *pTooltip);
	void OnTooltipEndMove(CTooltip *pTooltip);
	int InitializeOptions(WPARAM wParam, LPARAM lParam);
	int ContactSettingChanged(WPARAM wParam, LPARAM lParam);
	int ProtoAck(WPARAM wParam, LPARAM lParam);
	int ModulesLoaded(WPARAM wParam,LPARAM lParam);
	int ProtoContactIsTyping(WPARAM wParam, LPARAM lParam);
	
	static CTooltipNotify *GetObjInstance() { return s_pInstance; }

private:
	static CTooltipNotify *s_pInstance;
	static const char *s_szModuleNameOld;
	
	struct SOptions {
		uint8_t  bFirstRun;
		uint8_t  bOffline;
		uint8_t  bOnline;
		uint8_t  bOther;
		uint8_t  bTyping;
		uint8_t  bIdle;
		uint8_t  bConjSOLN;
		uint8_t  bX2;
		uint8_t  bAutoPos;
		uint8_t  bBallonTip;
		uint8_t  bTransp;
		uint8_t  bTranspInput;
		uint8_t  bAlpha;
		uint8_t  bLDblClick;
		uint8_t  bPrefixProto;
		uint16_t  wDuration;
		uint16_t  wStartupDelay;
		uint8_t  bIgnoreNew;
		uint8_t  bIgnoreUnknown;
	} m_sOptions;

	struct STooltipData	{
		CTooltip *pTooltip;
		UINT_PTR idTimer;
		UINT uiTimeout;
		MCONTACT hContact;
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
	wchar_t *StatusToString(int iStatus, wchar_t *szStatus, int iBufSize);
	wchar_t *MakeTooltipString(MCONTACT hContact, int iStatus, wchar_t *szString, int iBufSize);
	void MigrateSettings();
	void RegisterFonts();
	void GetFont(int iStatus, LOGFONT* lf, COLORREF* text, COLORREF* bg);
	void ResetCList(HWND hwndDlg);
	void LoadList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown);
	void SaveList(HWND hwndDlg, HANDLE hItemNew, HANDLE hItemUnknown);
	void LoadSettings();
	void SaveSettings();
	void ValidateSettings();
	void ReadSettingsFromDlg(HWND hDlg);
	void WriteSettingsToDlg(HWND hDlg);
	
	void SuspendTimer(CTooltip *pTooltip);
	void ResumeTimer(CTooltip *pTooltip);
	void OnConnectionTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	void OnTooltipTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);

	// Dialog procedures
	BOOL OptionsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL ProtosDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL ContactsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static void CALLBACK ConnectionTimerProcWrapper(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
	{
		CTooltipNotify::GetObjInstance()->OnConnectionTimer(hwnd, uMsg, idEvent, dwTime);
	}
	static void CALLBACK TooltipTimerProcWrapper(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
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
