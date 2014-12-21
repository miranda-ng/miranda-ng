#ifndef __E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__
#define __E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

class IQuotesProvider;

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
	CPopupSettings(const IQuotesProvider* pQuotesProvider);

	static COLORREF GetDefColourBk();
	static COLORREF GetDefColourText();

	void InitForContact(MCONTACT hContact);
	void SaveForContact(MCONTACT hContact)const;

	EColourMode GetColourMode()const;
	void SetColourMode(EColourMode nMode);

	COLORREF GetColourBk()const;
	void SetColourBk(COLORREF rgb);

	COLORREF GetColourText()const;
	void SetColourText(COLORREF rgb);

	EDelayMode GetDelayMode()const;
	void SetDelayMode(EDelayMode nMode);

	WORD GetDelayTimeout()const;
	void SetDelayTimeout(WORD delay);

	bool GetHistoryFlag()const;
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
	CAdvProviderSettings(const IQuotesProvider* pQuotesProvider);
	~CAdvProviderSettings();

	void SaveToDb()const;

	const IQuotesProvider* GetProviderPtr()const;

	WORD GetLogMode()const;
	void SetLogMode(WORD wMode);
	tstring GetHistoryFormat()const;
	void SetHistoryFormat(const tstring& rsFormat);
	bool GetHistoryOnlyChangedFlag()const;
	void SetHistoryOnlyChangedFlag(bool bMode);

	tstring GetLogFileName()const;
	void SetLogFileName(const tstring& rsFile);
	tstring GetLogFormat()const;
	void SetLogFormat(const tstring& rsFormat);
	bool GetLogOnlyChangedFlag()const;
	void SetLogOnlyChangedFlag(bool bMode);

	const tstring& GetPopupFormat() const;
	void SetPopupFormat(const tstring& val);

	bool GetShowPopupIfValueChangedFlag() const;
	void SetShowPopupIfValueChangedFlag(bool val);

	CPopupSettings* GetPopupSettingsPtr()const;

private:
	const IQuotesProvider* m_pQuotesProvider;
	WORD m_wLogMode;
	tstring m_sFormatHistory;
	bool m_bIsOnlyChangedHistory;
	tstring m_sLogFileName;
	tstring m_sFormatLogFile;
	bool m_bIsOnlyChangedLogFile;
	tstring m_sPopupFormat;
	bool m_bShowPopupIfValueChanged;
	mutable CPopupSettings* m_pPopupSettings;
};

void ShowSettingsDlg(MCONTACT hContact);
bool ShowSettingsDlg(HWND hWndParent, CAdvProviderSettings* pAdvSettings);

enum
{
	glfnResolveQuoteName = 0x0001,
	glfnResolveUserProfile = 0x0002,
	glfnResolveAll = glfnResolveQuoteName | glfnResolveUserProfile,
};
tstring GenerateLogFileName(const tstring& rsLogFilePattern, const tstring& rsQuoteSymbol, int nFlags = glfnResolveAll);
tstring GetContactLogFileName(MCONTACT hContact);
tstring GetContactName(MCONTACT hContact);

#endif //__E211E4D9_383C_43BE_A787_7EF1D585B90D_SettingsDlg_h__

