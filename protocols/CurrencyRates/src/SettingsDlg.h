#ifndef __E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__
#define __E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

class CPopupSettings
{
public:
	enum EColourMode
	{
		colourDefault,
		colourUserDefined,
	};

	enum EDelayMode
	{
		delayFromPopup,
		delayCustom,
		delayPermanent
	};

public:
	CPopupSettings();

	static COLORREF GetDefColourBk();
	static COLORREF GetDefColourText();

	void InitForContact(MCONTACT hContact);
	void SaveForContact(MCONTACT hContact) const;

	EColourMode GetColourMode() const;
	void SetColourMode(EColourMode nMode);

	COLORREF GetColourBk() const;
	void SetColourBk(COLORREF rgb);

	COLORREF GetColourText() const;
	void SetColourText(COLORREF rgb);

	EDelayMode GetDelayMode() const;
	void SetDelayMode(EDelayMode nMode);

	WORD GetDelayTimeout() const;
	void SetDelayTimeout(WORD delay);

	bool GetHistoryFlag() const;
	void SetHistoryFlag(bool flag);

private:
	EColourMode m_modeColour;
	EDelayMode m_modeDelay;
	COLORREF m_rgbBkg;
	COLORREF m_rgbText;
	WORD m_wDelay;
	bool m_bUseHistory;
};


class CAdvProviderSettings
{
	const ICurrencyRatesProvider *m_pCurrencyRatesProvider;
	WORD m_wLogMode;
	bool m_bIsOnlyChangedHistory;
	bool m_bIsOnlyChangedLogFile;
	bool m_bShowPopupIfValueChanged;
	CMStringW m_sFormatHistory;
	CMStringW m_sLogFileName;
	CMStringW m_sFormatLogFile;
	CMStringW m_sPopupFormat;
	mutable CPopupSettings* m_pPopupSettings;

public:
	CAdvProviderSettings(const ICurrencyRatesProvider *pCurrencyRatesProvider);
	~CAdvProviderSettings();

	void SaveToDb() const;

	__forceinline WORD GetLogMode() const { return m_wLogMode; }
	__forceinline void SetLogMode(WORD wMode) { m_wLogMode = wMode; }

	__forceinline const CMStringW& GetHistoryFormat() const { return m_sFormatHistory; }
	__forceinline void SetHistoryFormat(const CMStringW &rsFormat) { m_sFormatHistory = rsFormat; }

	__forceinline bool GetHistoryOnlyChangedFlag() const { return m_bIsOnlyChangedHistory; }
	__forceinline void SetHistoryOnlyChangedFlag(bool bMode) { m_bIsOnlyChangedHistory = bMode; }

	__forceinline const CMStringW& GetLogFileName() const { return m_sLogFileName; }
	__forceinline void SetLogFileName(const CMStringW &rsFile) { m_sLogFileName = rsFile; }

	__forceinline const CMStringW& GetLogFormat() const { return m_sFormatLogFile; }
	__forceinline void SetLogFormat(const CMStringW &rsFormat) { m_sFormatLogFile = rsFormat; }

	__forceinline bool GetLogOnlyChangedFlag() const { return m_bIsOnlyChangedLogFile; }
	__forceinline void SetLogOnlyChangedFlag(bool bMode) { m_bIsOnlyChangedLogFile = bMode; }

	__forceinline const CMStringW& GetPopupFormat() const { return m_sPopupFormat; }
	__forceinline void SetPopupFormat(const CMStringW &val) { m_sPopupFormat = val; }

	__forceinline bool GetShowPopupIfValueChangedFlag() const { return m_bShowPopupIfValueChanged; }
	__forceinline void SetShowPopupIfValueChangedFlag(bool val) { m_bShowPopupIfValueChanged = val; }

	const ICurrencyRatesProvider* GetProviderPtr() const;
	CPopupSettings* GetPopupSettingsPtr() const;
};

void ShowSettingsDlg(MCONTACT hContact);
bool ShowSettingsDlg(HWND hWndParent, CAdvProviderSettings* pAdvSettings);

enum
{
	glfnResolveCurrencyRateName = 0x0001,
	glfnResolveUserProfile = 0x0002,
	glfnResolveAll = glfnResolveCurrencyRateName | glfnResolveUserProfile,
};

CMStringW GenerateLogFileName(const CMStringW &rsLogFilePattern, const CMStringW &rsCurrencyRateSymbol, int nFlags = glfnResolveAll);
CMStringW GetContactLogFileName(MCONTACT hContact);
CMStringW GetContactName(MCONTACT hContact);

#endif //__E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

