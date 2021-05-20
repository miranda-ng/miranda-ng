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
public:
	CAdvProviderSettings(const ICurrencyRatesProvider *pCurrencyRatesProvider);
	~CAdvProviderSettings();

	void SaveToDb() const;

	const ICurrencyRatesProvider* GetProviderPtr() const;

	WORD GetLogMode() const;
	void SetLogMode(WORD wMode);
	std::wstring GetHistoryFormat() const;
	void SetHistoryFormat(const std::wstring& rsFormat);
	bool GetHistoryOnlyChangedFlag() const;
	void SetHistoryOnlyChangedFlag(bool bMode);

	std::wstring GetLogFileName() const;
	void SetLogFileName(const std::wstring& rsFile);
	std::wstring GetLogFormat() const;
	void SetLogFormat(const std::wstring& rsFormat);
	bool GetLogOnlyChangedFlag() const;
	void SetLogOnlyChangedFlag(bool bMode);

	const std::wstring& GetPopupFormat() const;
	void SetPopupFormat(const std::wstring& val);

	bool GetShowPopupIfValueChangedFlag() const;
	void SetShowPopupIfValueChangedFlag(bool val);

	CPopupSettings* GetPopupSettingsPtr() const;

private:
	const ICurrencyRatesProvider *m_pCurrencyRatesProvider;
	WORD m_wLogMode;
	std::wstring m_sFormatHistory;
	bool m_bIsOnlyChangedHistory;
	std::wstring m_sLogFileName;
	std::wstring m_sFormatLogFile;
	bool m_bIsOnlyChangedLogFile;
	std::wstring m_sPopupFormat;
	bool m_bShowPopupIfValueChanged;
	mutable CPopupSettings* m_pPopupSettings;
};

void ShowSettingsDlg(MCONTACT hContact);
bool ShowSettingsDlg(HWND hWndParent, CAdvProviderSettings* pAdvSettings);

enum
{
	glfnResolveCurrencyRateName = 0x0001,
	glfnResolveUserProfile = 0x0002,
	glfnResolveAll = glfnResolveCurrencyRateName | glfnResolveUserProfile,
};
std::wstring GenerateLogFileName(const std::wstring& rsLogFilePattern, const std::wstring& rsCurrencyRateSymbol, int nFlags = glfnResolveAll);
std::wstring GetContactLogFileName(MCONTACT hContact);
std::wstring GetContactName(MCONTACT hContact);

#endif //__E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

