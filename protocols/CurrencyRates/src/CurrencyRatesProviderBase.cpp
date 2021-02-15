#include "StdAfx.h"

extern bool g_bAutoUpdate;
extern HANDLE g_hEventWorkThreadStop;

struct CXMLFileInfo
{
	CXMLFileInfo() : m_qs(L"Unknown") {}
	ICurrencyRatesProvider::CProviderInfo m_pi;
	CCurrencyRateSection m_qs;
	tstring m_sURL;
};

inline tstring get_ini_file_name(LPCTSTR pszFileName)
{
	return CreateFilePath(pszFileName);
}

bool parse_currencyrate(const TiXmlNode *pTop, CCurrencyRate &q)
{
	tstring sSymbol, sDescription, sID;

	for (auto *pNode : TiXmlEnum(pTop)) {
		const char *sName = pNode->Value();
		if (!mir_strcmpi(sName, "symbol")) {
			sSymbol = GetNodeText(pNode);
			if (sSymbol.empty())
				return false;
		}
		else if (!mir_strcmpi(sName, "description")) {
			sDescription = GetNodeText(pNode);
		}
		else if (!mir_strcmpi(sName, "id")) {
			sID = GetNodeText(pNode);
			if (sID.empty())
				return false;
		}
	}

	q = CCurrencyRate(sID, TranslateW(sSymbol.c_str()), TranslateW(sDescription.c_str()));
	return true;
}

bool parse_section(const TiXmlNode *pTop, CCurrencyRateSection &qs)
{
	CCurrencyRateSection::TSections aSections;
	CCurrencyRateSection::TCurrencyRates aCurrencyRates;
	tstring sSectionName;

	for (auto *pNode : TiXmlEnum(pTop)) {
		const char *sName = pNode->Value();
		if (!mir_strcmpi(sName, "section")) {
			CCurrencyRateSection qs1;
			if (true == parse_section(pNode, qs1))
				aSections.push_back(qs1);
		}
		else if (!mir_strcmpi(sName, "currencyrate")) {
			CCurrencyRate q;
			if (true == parse_currencyrate(pNode, q))
				aCurrencyRates.push_back(q);
		}
		else if (!mir_strcmpi(sName, "name")) {
			sSectionName = GetNodeText(pNode);
			if (sSectionName.empty())
				return false;
		}
	}

	qs = CCurrencyRateSection(TranslateW(sSectionName.c_str()), aSections, aCurrencyRates);
	return true;
}

const TiXmlNode* find_provider(const TiXmlNode *pRoot)
{
	for (auto *pNode : TiXmlEnum(pRoot)) {
		const char *sName = pNode->Value();
		if (!mir_strcmpi(sName, "Provider"))
			return pNode;

		if (auto *pProvider = find_provider(pNode))
			return pProvider;
	}

	return nullptr;
}

CXMLFileInfo parse_ini_file(const tstring &rsXMLFile, bool &rbSucceded)
{
	CXMLFileInfo res;
	CCurrencyRateSection::TSections aSections;

	TiXmlDocument doc;
	if (doc.LoadFile(_T2A(rsXMLFile.c_str())) == tinyxml2::XML_SUCCESS) {
		const TiXmlNode *pProvider = find_provider(&doc);
		if (pProvider) {
			rbSucceded = true;
			for (auto *pNode : TiXmlEnum(pProvider)) {
				const char *sName = pNode->Value();
				if (!mir_strcmpi(sName, "section")) {
					CCurrencyRateSection qs;
					if (parse_section(pNode, qs))
						aSections.push_back(qs);
				}
				else if (!mir_strcmpi(sName, "Name"))
					res.m_pi.m_sName = GetNodeText(pNode);
				else if (!mir_strcmpi(sName, "ref"))
					res.m_pi.m_sURL = GetNodeText(pNode);
				else if (!mir_strcmpi(sName, "url"))
					res.m_sURL = GetNodeText(pNode);
			}
		}
	}

	res.m_qs = CCurrencyRateSection(res.m_pi.m_sName, aSections);
	return res;
}

CXMLFileInfo init_xml_info(LPCTSTR pszFileName, bool& rbSucceded)
{
	rbSucceded = false;
	tstring sIniFile = get_ini_file_name(pszFileName);
	return parse_ini_file(sIniFile, rbSucceded);
}

CCurrencyRatesProviderBase::CCurrencyRatesProviderBase() :
	m_hEventSettingsChanged(::CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_hEventRefreshContact(::CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_bRefreshInProgress(false)
{
}

CCurrencyRatesProviderBase::~CCurrencyRatesProviderBase()
{
	delete m_pXMLInfo;

	::CloseHandle(m_hEventSettingsChanged);
	::CloseHandle(m_hEventRefreshContact);
}

bool CCurrencyRatesProviderBase::Init()
{
	bool bSucceded = (m_pXMLInfo == nullptr);
	if (!m_pXMLInfo)
		m_pXMLInfo = new CXMLFileInfo(init_xml_info(DB_DEF_IniFileName, bSucceded));

	return bSucceded;
}

const CCurrencyRatesProviderBase::CProviderInfo& CCurrencyRatesProviderBase::GetInfo() const
{
	return m_pXMLInfo->m_pi;
}

const CCurrencyRateSection& CCurrencyRatesProviderBase::GetCurrencyRates() const
{
	return m_pXMLInfo->m_qs;
}

const tstring& CCurrencyRatesProviderBase::GetURL() const
{
	return m_pXMLInfo->m_sURL;
}

bool CCurrencyRatesProviderBase::IsOnline()
{
	return /*g_bAutoUpdate*/true;
}

void CCurrencyRatesProviderBase::AddContact(MCONTACT hContact)
{
	// 	CCritSection cs(m_cs);
	assert(m_aContacts.end() == std::find(m_aContacts.begin(), m_aContacts.end(), hContact));

	m_aContacts.push_back(hContact);
}

void CCurrencyRatesProviderBase::DeleteContact(MCONTACT hContact)
{
	mir_cslock lck(m_cs);

	TContacts::iterator i = std::find(m_aContacts.begin(), m_aContacts.end(), hContact);
	if (i != m_aContacts.end())
		m_aContacts.erase(i);
}

void CCurrencyRatesProviderBase::SetContactStatus(MCONTACT hContact, int nNewStatus)
{
	int nStatus = g_plugin.getWord(hContact, DB_STR_STATUS, ID_STATUS_OFFLINE);
	if (nNewStatus != nStatus) {
		g_plugin.setWord(hContact, DB_STR_STATUS, nNewStatus);

		if (ID_STATUS_ONLINE != nNewStatus) {
			db_unset(hContact, LIST_MODULE_NAME, STATUS_MSG_NAME);
			CMStringW sSymbol = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL);
			if (!sSymbol.IsEmpty())
				db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, sSymbol);

			SetContactExtraImage(hContact, eiEmpty);
		}
	}
}

class CTendency
{
	enum { NumValues = 2 };
	enum EComparison
	{
		NonValid,
		Greater,
		Less,
		Equal,
		GreaterOrEqual,
		LessOrEqual
	};

public:
	enum EResult
	{
		NotChanged,
		Up,
		Down
	};

public:
	CTendency() : m_nComparison(NonValid) {}

	bool Parse(CCurrencyRatesProviderBase *pProvider, const tstring& rsFrmt, MCONTACT hContact)
	{
		m_abValueFlags[0] = false;
		m_abValueFlags[1] = false;
		m_nComparison = NonValid;
		bool bValid = true;
		int nCurValue = 0;
		for (tstring::const_iterator i = rsFrmt.begin(); i != rsFrmt.end() && bValid && nCurValue < NumValues;) {
			wchar_t chr = *i;
			switch (chr) {
			default:
				if (false == std::isspace(chr))
					bValid = false;
				else
					++i;
				break;

			case '%':
				++i;
				if (i != rsFrmt.end()) {
					wchar_t t = *i;
					++i;

					double d;
					bValid = pProvider->ParseSymbol(hContact, t, d);
					if (bValid) {
						m_adValues[nCurValue] = d;
						m_abValueFlags[nCurValue] = true;
						++nCurValue;
					}
				}
				else bValid = false;
				break;
			case '>':
				m_nComparison = Greater;
				++i;
				break;
			case '<':
				m_nComparison = Less;
				++i;
				break;
			case '=':
				switch (m_nComparison) {
				default:
					bValid = false;
					break;
				case NonValid:
					m_nComparison = Equal;
					break;
				case Greater:
					m_nComparison = GreaterOrEqual;
					break;
				case Less:
					m_nComparison = LessOrEqual;
					break;
				}
				++i;
				break;
			}
		}

		return (bValid && IsValid());
	}

	bool IsValid() const { return (m_abValueFlags[0] && m_abValueFlags[1] && (m_nComparison != NonValid)); }

	EResult Compare() const
	{
		switch (m_nComparison) {
		case Greater:
			if (true == IsWithinAccuracy(m_adValues[0], m_adValues[1]))
				return NotChanged;

			if (m_adValues[0] > m_adValues[1])
				return Up;
			return Down;

		case GreaterOrEqual:
			if ((true == IsWithinAccuracy(m_adValues[0], m_adValues[1])) || (m_adValues[0] > m_adValues[1]))
				return Up;
			return Down;

		case Less:
			if (true == IsWithinAccuracy(m_adValues[0], m_adValues[1]))
				return NotChanged;

			if (m_adValues[0] < m_adValues[1])
				return Up;
			return Down;

		case LessOrEqual:
			if ((true == IsWithinAccuracy(m_adValues[0], m_adValues[1])) || (m_adValues[0] < m_adValues[1]))
				return Up;
			return Down;

		case Equal:
			if (true == IsWithinAccuracy(m_adValues[0], m_adValues[1]))
				return Up;
			return Down;
		}
		return NotChanged;
	}

private:
	double m_adValues[NumValues];
	bool m_abValueFlags[NumValues];
	EComparison m_nComparison;
};

tstring format_rate(const ICurrencyRatesProvider *pProvider, MCONTACT hContact, const tstring &rsFrmt)
{
	tstring sResult;

	for (tstring::const_iterator i = rsFrmt.begin(); i != rsFrmt.end();) {
		wchar_t chr = *i;
		switch (chr) {
		default:
			sResult += chr;
			++i;
			break;

		case '\\':
			++i;
			if (i != rsFrmt.end()) {
				wchar_t t = *i;
				switch (t) {
				case '%':  sResult += L"%"; break;
				case 't':  sResult += L"\t"; break;
				case 'n':  sResult += L"\n"; break;
				case '\\': sResult += L"\\"; break;
				default:   sResult += chr; sResult += t; break;
				}
				++i;
			}
			else sResult += chr;
			break;

		case '%':
			++i;
			if (i != rsFrmt.end()) {
				chr = *i;

				byte nWidth = 0;
				if (::isdigit(chr)) {
					nWidth = chr - 0x30;
					++i;
					if (i == rsFrmt.end()) {
						sResult += chr;
						break;
					}
					else chr = *i;
				}

				sResult += pProvider->FormatSymbol(hContact, chr, nWidth);
				++i;
			}
			else sResult += chr;
			break;
		}
	}

	return sResult;
}

void log_to_file(const ICurrencyRatesProvider *pProvider,
	MCONTACT hContact,
	const tstring& rsLogFileName,
	const tstring& rsFormat)
{
	CreatePathToFileW(rsLogFileName.c_str());

	tofstream file(rsLogFileName.c_str(), std::ios::app | std::ios::out);
	file.imbue(GetSystemLocale());
	if (file.good()) {
		tstring s = format_rate(pProvider, hContact, rsFormat);
		file << s;
	}
}

void log_to_history(const ICurrencyRatesProvider *pProvider,
	MCONTACT hContact,
	time_t nTime,
	const tstring& rsFormat)
{
	tstring s = format_rate(pProvider, hContact, rsFormat);
	T2Utf psz(s.c_str());

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.timestamp = static_cast<DWORD>(nTime);
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)::mir_strlen(psz) + 1;
	dbei.pBlob = (PBYTE)(char*)psz;
	db_event_add(hContact, &dbei);
}

bool do_set_contact_extra_icon(MCONTACT hContact, const CTendency& tendency)
{
	CTendency::EResult nComparison = tendency.Compare();

	if (CTendency::NotChanged == nComparison)
		return SetContactExtraImage(hContact, eiNotChanged);

	if (CTendency::Up == nComparison)
		return SetContactExtraImage(hContact, eiUp);

	if (CTendency::Down == nComparison)
		return SetContactExtraImage(hContact, eiDown);

	return false;
}

bool show_popup(const ICurrencyRatesProvider *pProvider,
	MCONTACT hContact,
	const CTendency& tendency,
	const tstring& rsFormat,
	const CPopupSettings& ps)
{
	POPUPDATAW ppd;
	memset(&ppd, 0, sizeof(ppd));
	ppd.lchContact = hContact;

	if (tendency.IsValid()) {
		CTendency::EResult nComparison = tendency.Compare();
		if (CTendency::NotChanged == nComparison)
			ppd.lchIcon = g_plugin.getIcon(IDI_ICON_NOTCHANGED);
		else if (CTendency::Up == nComparison)
			ppd.lchIcon = g_plugin.getIcon(IDI_ICON_UP);
		else if (CTendency::Down == nComparison)
			ppd.lchIcon = g_plugin.getIcon(IDI_ICON_DOWN);
	}

	mir_wstrncpy(ppd.lpwzContactName, pProvider->FormatSymbol(hContact, 's').c_str(), MAX_CONTACTNAME);
	{
		ptrW ss(variables_parsedup((wchar_t*)rsFormat.c_str(), nullptr, hContact));
		tstring sText = format_rate(pProvider, hContact, tstring(ss));
		mir_wstrncpy(ppd.lpwzText, sText.c_str(), MAX_SECONDLINE);
	}

	if (CPopupSettings::colourDefault == ps.GetColourMode()) {
		ppd.colorText = CPopupSettings::GetDefColourText();
		ppd.colorBack = CPopupSettings::GetDefColourBk();
	}
	else {
		ppd.colorText = ps.GetColourText();
		ppd.colorBack = ps.GetColourBk();
	}

	switch (ps.GetDelayMode()) {
	default:
		assert(!"Unknown popup delay mode");
	case CPopupSettings::delayFromPopup:
		ppd.iSeconds = 0;
		break;
	case CPopupSettings::delayPermanent:
		ppd.iSeconds = -1;
		break;
	case CPopupSettings::delayCustom:
		ppd.iSeconds = ps.GetDelayTimeout();
		break;
	}

	int lp = 0;
	if (false == ps.GetHistoryFlag())
		lp |= APF_NO_POPUP;

	return (0 == PUAddPopupW(&ppd, lp));
}

void CCurrencyRatesProviderBase::WriteContactRate(MCONTACT hContact, double dRate, const tstring& rsSymbol/* = ""*/)
{
	time_t nTime = ::time(0);

	if (false == rsSymbol.empty())
		g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_SYMBOL, rsSymbol.c_str());

	double dPrev = 0.0;
	bool bValidPrev = CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, dPrev);
	if (true == bValidPrev)
		CurrencyRates_DBWriteDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, dPrev);

	CurrencyRates_DBWriteDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, dRate);
	g_plugin.setDword(hContact, DB_STR_CURRENCYRATE_FETCH_TIME, nTime);

	tstring sSymbol = rsSymbol;

	tostringstream oNick;
	oNick.imbue(GetSystemLocale());
	if (false == m_sContactListFormat.empty()) {
		tstring s = format_rate(this, hContact, m_sContactListFormat);
		oNick << s;
	}
	else {
		if (true == sSymbol.empty())
			sSymbol = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_SYMBOL);

		oNick << std::setfill(L' ') << std::setw(10) << std::left << sSymbol << std::setw(6) << std::right << dRate;
	}
	CTendency tendency;

	if (true == tendency.Parse(this, m_sTendencyFormat, hContact))
		do_set_contact_extra_icon(hContact, tendency);

	db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, oNick.str().c_str());

	tstring sStatusMsg = format_rate(this, hContact, m_sStatusMsgFormat);
	if (false == sStatusMsg.empty())
		db_set_ws(hContact, LIST_MODULE_NAME, STATUS_MSG_NAME, sStatusMsg.c_str());
	else
		db_unset(hContact, LIST_MODULE_NAME, STATUS_MSG_NAME);

	bool bUseContactSpecific = g_plugin.getBool(hContact, DB_STR_CONTACT_SPEC_SETTINGS);

	CAdvProviderSettings global_settings(this);

	WORD dwMode = (bUseContactSpecific)
		? g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG, static_cast<WORD>(lmDisabled))
		: global_settings.GetLogMode();
	if (dwMode&lmExternalFile) {
		bool bAdd = true;
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG_FILE_CONDITION, 1) > 0)
			: global_settings.GetLogOnlyChangedFlag();
		if (true == bOnlyIfChanged) {
			bAdd = ((false == bValidPrev) || (false == IsWithinAccuracy(dRate, dPrev)));
		}
		if (true == bAdd) {
			tstring sLogFileName = (bUseContactSpecific)
				? CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_LOG_FILE, global_settings.GetLogFileName().c_str())
				: global_settings.GetLogFileName();

			if (true == sSymbol.empty()) {
				sSymbol = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_SYMBOL);
			}

			sLogFileName = GenerateLogFileName(sLogFileName, sSymbol);

			tstring sFormat = global_settings.GetLogFormat();
			if (bUseContactSpecific)
				sFormat = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_FORMAT_LOG_FILE, DB_DEF_LogFormat);

			log_to_file(this, hContact, sLogFileName, sFormat);
		}
	}
	if (dwMode&lmInternalHistory) {
		bool bAdd = true;
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_HISTORY_CONDITION, 1) > 0)
			: global_settings.GetHistoryOnlyChangedFlag();

		if (true == bOnlyIfChanged) {
			bAdd = ((false == bValidPrev) || (false == IsWithinAccuracy(dRate, dPrev)));
		}
		if (true == bAdd) {
			tstring sFormat = (bUseContactSpecific)
				? CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_FORMAT_HISTORY, global_settings.GetHistoryFormat().c_str())
				: global_settings.GetHistoryFormat();

			log_to_history(this, hContact, nTime, sFormat);
		}
	}

	if (dwMode&lmPopup) {
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (1 == g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_CONDITION, 1) > 0)
			: global_settings.GetShowPopupIfValueChangedFlag();
		if ((false == bOnlyIfChanged)
			|| ((true == bOnlyIfChanged) && (true == bValidPrev) && (false == IsWithinAccuracy(dRate, dPrev)))) {
			tstring sFormat = (bUseContactSpecific)
				? CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_FORMAT_POPUP, global_settings.GetPopupFormat().c_str())
				: global_settings.GetPopupFormat();

			CPopupSettings ps = *(global_settings.GetPopupSettingsPtr());
			ps.InitForContact(hContact);
			show_popup(this, hContact, tendency, sFormat, ps);
		}
	}

	SetContactStatus(hContact, ID_STATUS_ONLINE);
}

MCONTACT CCurrencyRatesProviderBase::CreateNewContact(const tstring& rsName)
{
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, MODULENAME);

	tstring sProvName = GetInfo().m_sName;
	g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_PROVIDER, sProvName.c_str());
	g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_SYMBOL, rsName.c_str());
	db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, rsName.c_str());

	mir_cslock lck(m_cs);
	m_aContacts.push_back(hContact);
	return hContact;
}

DWORD get_refresh_timeout_miliseconds()
{
	if (!g_bAutoUpdate)
		return INFINITE;

	int nRefreshRateType = g_plugin.getWord(DB_KEY_RefreshRateType, RRT_MINUTES);
	if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
		nRefreshRateType = RRT_MINUTES;

	DWORD nTimeout = g_plugin.getWord(DB_KEY_RefreshRateValue, 1);
	switch (nRefreshRateType) {
	default:
	case RRT_SECONDS:
		if (nTimeout < 1 || nTimeout > 60)
			nTimeout = 1;

		nTimeout *= 1000;
		break;
	case RRT_MINUTES:
		if (nTimeout < 1 || nTimeout > 60)
			nTimeout = 1;

		nTimeout *= 1000 * 60;
		break;
	case RRT_HOURS:
		if (nTimeout < 1 || nTimeout > 24)
			nTimeout = 1;

		nTimeout *= 1000 * 60 * 60;
		break;
	}

	return nTimeout;
}

class CBoolGuard
{
public:
	CBoolGuard(bool& rb) : m_b(rb) { m_b = true; }
	~CBoolGuard() { m_b = false; }

private:
	bool m_b;
};

void CCurrencyRatesProviderBase::Run()
{
	DWORD nTimeout = get_refresh_timeout_miliseconds();
	m_sContactListFormat = CurrencyRates_DBGetStringW(NULL, MODULENAME, DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat);
	m_sStatusMsgFormat = CurrencyRates_DBGetStringW(NULL, MODULENAME, DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat);
	m_sTendencyFormat = CurrencyRates_DBGetStringW(NULL, MODULENAME, DB_KEY_TendencyFormat, DB_DEF_TendencyFormat);

	enum
	{
		STOP_THREAD = 0,
		SETTINGS_CHANGED = 1,
		REFRESH_CONTACT = 2,
		COUNT_SYNC_OBJECTS = 3
	};

	HANDLE anEvents[COUNT_SYNC_OBJECTS];
	anEvents[STOP_THREAD] = g_hEventWorkThreadStop;
	anEvents[SETTINGS_CHANGED] = m_hEventSettingsChanged;
	anEvents[REFRESH_CONTACT] = m_hEventRefreshContact;

	TContacts anContacts;
	{
		mir_cslock lck(m_cs);
		anContacts = m_aContacts;
	}

	bool bGoToBed = false;

	if (g_bAutoUpdate) {
		CBoolGuard bg(m_bRefreshInProgress);
		RefreshCurrencyRates(anContacts);
	}

	while (false == bGoToBed) {
		anContacts.clear();

		DWORD dwBegin = ::GetTickCount();
		DWORD dwResult = ::WaitForMultipleObjects(COUNT_SYNC_OBJECTS, anEvents, FALSE, nTimeout);
		switch (dwResult) {
		case WAIT_FAILED:
			assert(!"WaitForMultipleObjects failed");
			bGoToBed = true;
			break;

		case WAIT_ABANDONED_0 + STOP_THREAD:
		case WAIT_ABANDONED_0 + SETTINGS_CHANGED:
		case WAIT_ABANDONED_0 + REFRESH_CONTACT:
			assert(!"WaitForMultipleObjects abandoned");

		case WAIT_OBJECT_0 + STOP_THREAD:
			bGoToBed = true;
			break;

		case WAIT_OBJECT_0 + SETTINGS_CHANGED:
			nTimeout = get_refresh_timeout_miliseconds();
			m_sContactListFormat = CurrencyRates_DBGetStringW(NULL, MODULENAME, DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat);
			m_sStatusMsgFormat = CurrencyRates_DBGetStringW(NULL, MODULENAME, DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat);
			m_sTendencyFormat = CurrencyRates_DBGetStringW(NULL, MODULENAME, DB_KEY_TendencyFormat, DB_DEF_TendencyFormat);
			{
				mir_cslock lck(m_cs);
				anContacts = m_aContacts;
			}
			break;

		case WAIT_OBJECT_0 + REFRESH_CONTACT:
			{
				DWORD dwTimeRest = ::GetTickCount() - dwBegin;
				if (INFINITE != nTimeout && dwTimeRest < nTimeout)
					nTimeout -= dwTimeRest;

				{
					mir_cslock lck(m_cs);
					anContacts = m_aRefreshingContacts;
					m_aRefreshingContacts.clear();
				}

				CBoolGuard bg(m_bRefreshInProgress);
				RefreshCurrencyRates(anContacts);
			}
			break;

		case WAIT_TIMEOUT:
			nTimeout = get_refresh_timeout_miliseconds();
			{
				mir_cslock lck(m_cs);
				anContacts = m_aContacts;
			}
			{
				CBoolGuard bg(m_bRefreshInProgress);
				RefreshCurrencyRates(anContacts);
			}
			break;

		default:
			assert(!"What is the hell?");
		}
	}

	OnEndRun();
}

void CCurrencyRatesProviderBase::OnEndRun()
{
	TContacts anContacts;
	{
		mir_cslock lck(m_cs);
		anContacts = m_aContacts;
		m_aRefreshingContacts.clear();
	}

	CBoolGuard bg(m_bRefreshInProgress);
	for (auto &it : anContacts)
		SetContactStatus(it, ID_STATUS_OFFLINE);
}

void CCurrencyRatesProviderBase::RefreshSettings()
{
	::SetEvent(m_hEventSettingsChanged);
}

void CCurrencyRatesProviderBase::RefreshAllContacts()
{
	{	mir_cslock lck(m_cs);
		m_aRefreshingContacts.clear();
		for (auto &hContact : m_aContacts)
			m_aRefreshingContacts.push_back(hContact);
	}

	::SetEvent(m_hEventRefreshContact);
}

void CCurrencyRatesProviderBase::RefreshContact(MCONTACT hContact)
{
	{	mir_cslock lck(m_cs);
		m_aRefreshingContacts.push_back(hContact);
	}

	::SetEvent(m_hEventRefreshContact);
}

void CCurrencyRatesProviderBase::FillFormat(TFormatSpecificators &array) const
{
	array.push_back(CFormatSpecificator(L"%S", TranslateT("Source of Information")));
	array.push_back(CFormatSpecificator(L"%r", TranslateT("Rate Value")));
	array.push_back(CFormatSpecificator(L"%p", TranslateT("Previous Rate Value")));
	array.push_back(CFormatSpecificator(L"%X", TranslateT("Fetch Time")));
	array.push_back(CFormatSpecificator(L"%x", TranslateT("Fetch Date")));
	array.push_back(CFormatSpecificator(L"%t", TranslateT("Fetch Time and Date")));
	array.push_back(CFormatSpecificator(L"\\%", TranslateT("Percentage Character (%)")));
	array.push_back(CFormatSpecificator(L"\\t", TranslateT("Tabulation")));
	array.push_back(CFormatSpecificator(L"\\\\", TranslateT("Left slash (\\)")));
}

bool CCurrencyRatesProviderBase::ParseSymbol(MCONTACT hContact, wchar_t c, double &d)
{
	switch (c) {
	case 'r':
	case 'R':
		return CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, d);
	
	case 'p':
	case 'P':
		return CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, d);
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////

static bool get_fetch_time(MCONTACT hContact, time_t &rTime)
{
	DBVARIANT dbv;
	if (db_get(hContact, MODULENAME, DB_STR_CURRENCYRATE_FETCH_TIME, &dbv) || (DBVT_DWORD != dbv.type))
		return false;

	rTime = dbv.dVal;
	return true;
}

static tstring format_fetch_time(MCONTACT hContact, const tstring &rsFormat)
{
	time_t nTime;
	if (true == get_fetch_time(hContact, nTime)) {
		boost::posix_time::ptime time = boost::date_time::c_local_adjustor<boost::posix_time::ptime>::utc_to_local(boost::posix_time::from_time_t(nTime));
		tostringstream k;
		k.imbue(std::locale(GetSystemLocale(), new ttime_facet(rsFormat.c_str())));
		k << time;
		return k.str();
	}

	return tstring();
}

static tstring format_double(double dValue, int nWidth)
{
	wchar_t str[100], format[] = L"%.6lf";
	if (nWidth > 0 && nWidth <= 9)
		format[2] = '0' + nWidth;
	swprintf_s(str, format, dValue);
	return str;
}

tstring CCurrencyRatesProviderBase::FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth) const
{
	tstring ret;
	double d = 0.0;

	switch (c) {
	case '%':
	case '\t':
	case '\\':
		ret = c;
		break;
	case 'S':
		ret = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_PROVIDER);
		break;
	case 's':
		ret = CurrencyRates_DBGetStringW(hContact, MODULENAME, DB_STR_CURRENCYRATE_SYMBOL);
		break;
	case 'X':
		ret = format_fetch_time(hContact, CurrencyRates_GetTimeFormat(true));
		break;
	case 'x':
		ret = format_fetch_time(hContact, CurrencyRates_GetDateFormat(true));
		break;
	case 't':
		{
			tstring sFrmt = CurrencyRates_GetDateFormat(true);
			sFrmt += L" ";
			sFrmt += CurrencyRates_GetTimeFormat(true);
			ret = format_fetch_time(hContact, sFrmt);
		}
		break;
	case 'r':
	case 'R':
		if (true == CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, d))
			ret = format_double(d, nWidth);
		else
			ret = L"-";
		break;

	case 'p':
	case 'P':
		if (true == CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, d))
			ret = format_double(d, nWidth);
		else
			ret = L"-";
		break;
	}

	return ret;
}
