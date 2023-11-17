#include "StdAfx.h"

extern bool g_bAutoUpdate;
extern HANDLE g_hEventWorkThreadStop;

struct CXMLFileInfo
{
	CXMLFileInfo() : m_qs(L"Unknown") {}
	ICurrencyRatesProvider::CProviderInfo m_pi;
	CCurrencyRateSection m_qs;
	CMStringW m_sURL;
};

bool parse_currencyrate(const TiXmlNode *pTop, CCurrencyRate &q)
{
	CMStringW sSymbol, sDescription, sID;

	for (auto *pNode : TiXmlEnum(pTop)) {
		const char *sName = pNode->Value();
		if (!mir_strcmpi(sName, "symbol")) {
			sSymbol = GetNodeText(pNode);
			if (sSymbol.IsEmpty())
				return false;
		}
		else if (!mir_strcmpi(sName, "description")) {
			sDescription = GetNodeText(pNode);
		}
		else if (!mir_strcmpi(sName, "id")) {
			sID = GetNodeText(pNode);
			if (sID.IsEmpty())
				return false;
		}
	}

	q = CCurrencyRate(sID, TranslateW(sSymbol.c_str()), TranslateW(sDescription.c_str()));
	return true;
}

bool parse_section(const TiXmlNode *pTop, CCurrencyRateSection &qs)
{
	CCurrencyRateSection::TCurrencyRates aCurrencyRates;
	CMStringW sSectionName;

	for (auto *pNode : TiXmlEnum(pTop)) {
		const char *sName = pNode->Value();
		if (!mir_strcmpi(sName, "currencyrate")) {
			CCurrencyRate q;
			if (true == parse_currencyrate(pNode, q))
				aCurrencyRates.push_back(q);
		}
		else if (!mir_strcmpi(sName, "name")) {
			sSectionName = GetNodeText(pNode);
			if (sSectionName.IsEmpty())
				return false;
		}
	}

	qs = CCurrencyRateSection(TranslateW(sSectionName.c_str()), aCurrencyRates);
	return true;
}

const TiXmlNode *find_provider(const TiXmlNode *pRoot)
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

CXMLFileInfo parse_ini_file(const CMStringW &rsXMLFile, bool &rbSucceded)
{
	CXMLFileInfo res;
	
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
						res.m_qs = qs;
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

	return res;
}

CXMLFileInfo init_xml_info(LPCTSTR pszFileName, bool &rbSucceded)
{
	rbSucceded = false;
	CMStringW sIniFile = CreateFilePath(pszFileName);
	return parse_ini_file(sIniFile, rbSucceded);
}

CCurrencyRatesProviderBase::CCurrencyRatesProviderBase() :
	m_hEventSettingsChanged(::CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_hEventRefreshContact(::CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_bRefreshInProgress(false)
{}

CCurrencyRatesProviderBase::~CCurrencyRatesProviderBase()
{
	delete m_pXMLInfo;

	for (auto &it : Contacts(MODULENAME))
		SetContactStatus(it, ID_STATUS_OFFLINE);

	::CloseHandle(m_hEventSettingsChanged);
	::CloseHandle(m_hEventRefreshContact);
}

bool CCurrencyRatesProviderBase::Init()
{
	bool bSucceded = (m_pXMLInfo == nullptr);
	if (!m_pXMLInfo)
		m_pXMLInfo = new CXMLFileInfo(init_xml_info(GetXmlFilename(), bSucceded));

	return bSucceded;
}

const CCurrencyRatesProviderBase::CProviderInfo &CCurrencyRatesProviderBase::GetInfo() const
{
	return m_pXMLInfo->m_pi;
}

const CCurrencyRateSection &CCurrencyRatesProviderBase::GetSection() const
{
	return m_pXMLInfo->m_qs;
}

const CMStringW &CCurrencyRatesProviderBase::GetURL() const
{
	return m_pXMLInfo->m_sURL;
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
		Down,
		Unknown = -1
	};

public:
	CTendency() : m_nComparison(NonValid) {}

	bool Parse(CCurrencyRatesProviderBase *pProvider, const CMStringW &rsFrmt, MCONTACT hContact)
	{
		m_abValueFlags[0] = false;
		m_abValueFlags[1] = false;
		m_nComparison = NonValid;
		bool bValid = true;
		int nCurValue = 0;
		for (int i = 0; i < rsFrmt.GetLength() && bValid && nCurValue < NumValues;) {
			wchar_t chr = rsFrmt[i];
			switch (chr) {
			default:
				if (false == std::isspace(chr))
					bValid = false;
				else
					++i;
				break;

			case '%':
				++i;
				if (i != rsFrmt.GetLength()) {
					wchar_t t = rsFrmt[i];
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

CMStringW format_rate(const ICurrencyRatesProvider *pProvider, MCONTACT hContact, const CMStringW &rsFrmt)
{
	CMStringW sResult;

	for (int i = 0; i < rsFrmt.GetLength(); ) {
		wchar_t chr = rsFrmt[i];
		switch (chr) {
		default:
			sResult += chr;
			++i;
			break;

		case '\\':
			++i;
			if (i != rsFrmt.GetLength()) {
				wchar_t t = rsFrmt[i];
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
			if (i != rsFrmt.GetLength()) {
				chr = rsFrmt[i];

				byte nWidth = 0;
				if (::isdigit(chr)) {
					nWidth = chr - 0x30;
					++i;
					if (i == rsFrmt.GetLength()) {
						sResult += chr;
						break;
					}
					else chr = rsFrmt[i];
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

void log_to_file(const ICurrencyRatesProvider *pProvider, MCONTACT hContact, const CMStringW &rsLogFileName, const CMStringW &rsFormat)
{
	CreatePathToFileW(rsLogFileName);

	std::wofstream file(rsLogFileName, std::ios::app | std::ios::out);
	file.imbue(GetSystemLocale());
	if (file.good()) {
		CMStringW s = format_rate(pProvider, hContact, rsFormat);
		file << s;
	}
}

void log_to_history(const ICurrencyRatesProvider *pProvider, MCONTACT hContact, time_t nTime, const CMStringW &rsFormat)
{
	CMStringW s = format_rate(pProvider, hContact, rsFormat);
	T2Utf psz(s.c_str());

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.timestamp = static_cast<uint32_t>(nTime);
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.cbBlob = (int)::mir_strlen(psz) + 1;
	dbei.pBlob = (uint8_t *)(char *)psz;
	db_event_add(hContact, &dbei);
}

bool do_set_contact_extra_icon(MCONTACT hContact, const CTendency &tendency)
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

bool show_popup(const ICurrencyRatesProvider *pProvider, MCONTACT hContact, int nComparison, const CMStringW &rsFormat, const CPopupSettings &ps)
{
	POPUPDATAW ppd;
	memset(&ppd, 0, sizeof(ppd));
	ppd.lchContact = hContact;

	switch (nComparison) {
	case CTendency::Up:         ppd.lchIcon = g_plugin.getIcon(IDI_ICON_UP); break;
	case CTendency::Down:       ppd.lchIcon = g_plugin.getIcon(IDI_ICON_DOWN); break;
	case CTendency::NotChanged: ppd.lchIcon = g_plugin.getIcon(IDI_ICON_NOTCHANGED); break;
	default:
		ppd.lchIcon = g_plugin.getIcon(IDI_ICON_CURRENCYRATE);
	}

	if (pProvider != nullptr) {
		if (hContact)
			mir_wstrncpy(ppd.lpwzContactName, pProvider->FormatSymbol(hContact, 's').c_str(), MAX_CONTACTNAME);
		else
			mir_wstrncpy(ppd.lpwzContactName, TranslateT("Currency Rates"), MAX_CONTACTNAME);

		ptrW ss(variables_parsedup((wchar_t *)rsFormat.c_str(), nullptr, hContact));
		mir_wstrncpy(ppd.lpwzText, format_rate(pProvider, hContact, ss.get()), MAX_SECONDLINE);
	}
	else {
		mir_wstrncpy(ppd.lpwzContactName, TranslateT("Test contact"), MAX_CONTACTNAME);
		mir_wstrncpy(ppd.lpwzText, L"1 USD = 8.4342 SMC", MAX_SECONDLINE);
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

void CCurrencyRatesProviderBase::WriteContactRate(MCONTACT hContact, double dRate, const CMStringW &rsSymbol/* = ""*/)
{
	time_t nTime = ::time(0);

	if (false == rsSymbol.IsEmpty())
		g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_SYMBOL, rsSymbol.c_str());

	double dPrev = 0.0;
	bool bValidPrev = CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, dPrev);
	if (true == bValidPrev)
		CurrencyRates_DBWriteDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, dPrev);

	CurrencyRates_DBWriteDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, dRate);
	g_plugin.setDword(hContact, DB_STR_CURRENCYRATE_FETCH_TIME, nTime);

	CMStringW sSymbol = rsSymbol;

	CMStringW wszNick;
	if (false == m_sContactListFormat.IsEmpty()) {
		wszNick = format_rate(this, hContact, m_sContactListFormat);
	}
	else {
		if (sSymbol.IsEmpty())
			sSymbol = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL);

		wszNick.Format(L"%-10s %.6lf", sSymbol.c_str(), dRate);
	}

	CTendency tendency;
	if (true == tendency.Parse(this, m_sTendencyFormat, hContact))
		do_set_contact_extra_icon(hContact, tendency);

	db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, wszNick);

	CMStringW sStatusMsg = format_rate(this, hContact, m_sStatusMsgFormat);
	if (!sStatusMsg.IsEmpty())
		db_set_ws(hContact, LIST_MODULE_NAME, STATUS_MSG_NAME, sStatusMsg);
	else
		db_unset(hContact, LIST_MODULE_NAME, STATUS_MSG_NAME);

	bool bUseContactSpecific = g_plugin.getBool(hContact, DB_STR_CONTACT_SPEC_SETTINGS);

	CAdvProviderSettings global_settings(this);

	uint16_t dwMode = (bUseContactSpecific)
		? g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG, static_cast<uint16_t>(lmDisabled))
		: global_settings.GetLogMode();
	if (dwMode & lmExternalFile) {
		bool bAdd = true;
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_LOG_FILE_CONDITION, 1) > 0)
			: global_settings.GetLogOnlyChangedFlag();
		if (true == bOnlyIfChanged) {
			bAdd = ((false == bValidPrev) || (false == IsWithinAccuracy(dRate, dPrev)));
		}
		if (true == bAdd) {
			CMStringW sLogFileName = (bUseContactSpecific)
				? g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_LOG_FILE, global_settings.GetLogFileName().c_str())
				: global_settings.GetLogFileName();

			if (true == sSymbol.IsEmpty()) {
				sSymbol = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL);
			}

			sLogFileName = GenerateLogFileName(sLogFileName, sSymbol);

			CMStringW sFormat = global_settings.GetLogFormat();
			if (bUseContactSpecific)
				sFormat = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_LOG_FILE, DB_DEF_LogFormat);

			log_to_file(this, hContact, sLogFileName, sFormat);
		}
	}
	if (dwMode & lmInternalHistory) {
		bool bAdd = true;
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (g_plugin.getWord(hContact, DB_STR_CURRENCYRATE_HISTORY_CONDITION, 1) > 0)
			: global_settings.GetHistoryOnlyChangedFlag();

		if (true == bOnlyIfChanged) {
			bAdd = ((false == bValidPrev) || (false == IsWithinAccuracy(dRate, dPrev)));
		}
		if (true == bAdd) {
			CMStringW sFormat = (bUseContactSpecific)
				? g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_HISTORY, global_settings.GetHistoryFormat().c_str())
				: global_settings.GetHistoryFormat();

			log_to_history(this, hContact, nTime, sFormat);
		}
	}

	if (dwMode & lmPopup) {
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (1 == g_plugin.getByte(hContact, DB_STR_CURRENCYRATE_POPUP_CONDITION, 1) > 0)
			: global_settings.GetShowPopupIfValueChangedFlag();
		if (!bOnlyIfChanged || (bOnlyIfChanged && bValidPrev && !IsWithinAccuracy(dRate, dPrev))) {
			CMStringW sFormat = (bUseContactSpecific)
				? g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_FORMAT_POPUP, global_settings.GetPopupFormat().c_str())
				: global_settings.GetPopupFormat();

			CPopupSettings ps = *(global_settings.GetPopupSettingsPtr());
			ps.InitForContact(hContact);

			CTendency::EResult nComparison = CTendency::NotChanged;
			if (tendency.IsValid())
				nComparison = tendency.Compare();
			show_popup(this, hContact, nComparison, sFormat, ps);
		}
	}

	SetContactStatus(hContact, ID_STATUS_ONLINE);
}

MCONTACT CCurrencyRatesProviderBase::CreateNewContact(const CMStringW &rsName)
{
	MCONTACT hContact = db_add_contact();
	Proto_AddToContact(hContact, MODULENAME);

	g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_PROVIDER, GetInfo().m_sName);
	g_plugin.setWString(hContact, DB_STR_CURRENCYRATE_SYMBOL, rsName);
	db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, rsName);

	return hContact;
}

uint32_t get_refresh_timeout_miliseconds()
{
	if (!g_bAutoUpdate)
		return INFINITE;

	int nRefreshRateType = g_plugin.getWord(DB_KEY_RefreshRateType, RRT_MINUTES);
	if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
		nRefreshRateType = RRT_MINUTES;

	uint32_t nTimeout = g_plugin.getWord(DB_KEY_RefreshRateValue, 1);
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
	CBoolGuard(bool &rb) : m_b(rb) { m_b = true; }
	~CBoolGuard() { m_b = false; }

private:
	bool m_b;
};

/////////////////////////////////////////////////////////////////////////////////////////

static void refreshContacts(TContacts &list)
{
	list.clear();
	for (auto &cc : Contacts(MODULENAME))
		list.push_back(cc);
}

void CCurrencyRatesProviderBase::Run()
{
	uint32_t nTimeout = get_refresh_timeout_miliseconds();
	m_sContactListFormat = g_plugin.getMStringW(DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat);
	m_sStatusMsgFormat = g_plugin.getMStringW(DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat);
	m_sTendencyFormat = g_plugin.getMStringW(DB_KEY_TendencyFormat, DB_DEF_TendencyFormat);

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
	refreshContacts(anContacts);

	bool bGoToBed = false;

	if (g_bAutoUpdate) {
		CBoolGuard bg(m_bRefreshInProgress);
		RefreshCurrencyRates(anContacts);
	}

	while (false == bGoToBed) {
		anContacts.clear();

		uint32_t dwBegin = ::GetTickCount();
		uint32_t dwResult = ::WaitForMultipleObjects(COUNT_SYNC_OBJECTS, anEvents, FALSE, nTimeout);
		switch (dwResult) {
		case WAIT_FAILED:
			assert(!"WaitForMultipleObjects failed");
			bGoToBed = true;
			break;

		case WAIT_ABANDONED_0 + STOP_THREAD:
		case WAIT_ABANDONED_0 + SETTINGS_CHANGED:
		case WAIT_ABANDONED_0 + REFRESH_CONTACT:
			assert(!"WaitForMultipleObjects abandoned");
			__fallthrough;

		case WAIT_OBJECT_0 + STOP_THREAD:
			bGoToBed = true;
			break;

		case WAIT_OBJECT_0 + SETTINGS_CHANGED:
			nTimeout = get_refresh_timeout_miliseconds();
			m_sContactListFormat = g_plugin.getMStringW(DB_KEY_DisplayNameFormat, DB_DEF_DisplayNameFormat);
			m_sStatusMsgFormat = g_plugin.getMStringW(DB_KEY_StatusMsgFormat, DB_DEF_StatusMsgFormat);
			m_sTendencyFormat = g_plugin.getMStringW(DB_KEY_TendencyFormat, DB_DEF_TendencyFormat);

			refreshContacts(anContacts);
			break;

		case WAIT_OBJECT_0 + REFRESH_CONTACT:
			{
				uint32_t dwTimeRest = ::GetTickCount() - dwBegin;
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
			refreshContacts(anContacts);
			{
				CBoolGuard bg(m_bRefreshInProgress);
				RefreshCurrencyRates(anContacts);
			}
			break;

		default:
			assert(!"What is the hell?");
		}
	}

	mir_cslock lck(m_cs);
	m_aRefreshingContacts.clear();
}

void CCurrencyRatesProviderBase::RefreshSettings()
{
	::SetEvent(m_hEventSettingsChanged);
}

void CCurrencyRatesProviderBase::RefreshAllContacts()
{
	{	mir_cslock lck(m_cs);
		refreshContacts(m_aRefreshingContacts);
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
	array.push_back(CFormatSpecificator(L"%F", LPGENW("From Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%f", LPGENW("From Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%I", LPGENW("Into Currency Full Name")));
	array.push_back(CFormatSpecificator(L"%i", LPGENW("Into Currency Short Name")));
	array.push_back(CFormatSpecificator(L"%s", LPGENW("Short notation for \"%f/%i\"")));
	array.push_back(CFormatSpecificator(L"%S", LPGENW("Source of information")));
	array.push_back(CFormatSpecificator(L"%r", LPGENW("Rate value")));
	array.push_back(CFormatSpecificator(L"%d", LPGENW("Rate delta")));
	array.push_back(CFormatSpecificator(L"%p", LPGENW("Previous rate value")));
	array.push_back(CFormatSpecificator(L"%X", LPGENW("Fetch time")));
	array.push_back(CFormatSpecificator(L"%x", LPGENW("Fetch date")));
	array.push_back(CFormatSpecificator(L"%t", LPGENW("Fetch time and date")));
	array.push_back(CFormatSpecificator(L"\\%", LPGENW("Percent character (%)")));
	array.push_back(CFormatSpecificator(L"\\t", LPGENW("Tabulation")));
	array.push_back(CFormatSpecificator(L"\\\\", LPGENW("Left slash (\\)")));
}

bool CCurrencyRatesProviderBase::ParseSymbol(MCONTACT hContact, wchar_t c, double &d) const
{
	switch (c) {
	case 'r':
	case 'R':
		return CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, d);

	case 'p':
	case 'P':
		return CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, d);

	case 'd':
	case 'D':
		double v1, v2;
		if (CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_CURR_VALUE, v1) && CurrencyRates_DBReadDouble(hContact, MODULENAME, DB_STR_CURRENCYRATE_PREV_VALUE, v2)) {
			d = v1 - v2;
			return true;
		}
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

static CMStringW format_fetch_time(MCONTACT hContact, const wchar_t *rsFormat)
{
	time_t nTime;
	if (true == get_fetch_time(hContact, nTime)) {
		wchar_t buf[200];
		wcsftime(buf, _countof(buf), rsFormat, localtime(&nTime));
		return buf;
	}

	return CMStringW();
}

static CMStringW format_double(double dValue, int nWidth)
{
	wchar_t format[] = L"%.6lf";
	if (nWidth > 0 && nWidth <= 9)
		format[2] = '0' + nWidth;
	return CMStringW(FORMAT, format, dValue);
}

CMStringW CCurrencyRatesProviderBase::FormatSymbol(MCONTACT hContact, wchar_t c, int nWidth) const
{
	CMStringW ret;
	double d = 0.0;

	switch (c) {
	case '%':
	case '\t':
	case '\\':
		ret = c;
		break;
	case 'S':
		ret = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_PROVIDER);
		break;
	case 's':
		ret = g_plugin.getMStringW(hContact, DB_STR_CURRENCYRATE_SYMBOL);
		break;
	case 'F':
		ret = g_plugin.getMStringW(hContact, DB_STR_FROM_DESCRIPTION);
		break;
	case 'f':
		ret = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
		break;
	case 'I':
		ret = g_plugin.getMStringW(hContact, DB_STR_TO_DESCRIPTION);
		break;
	case 'i':
		ret = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
		break;
	case 'X':
		ret = format_fetch_time(hContact, CurrencyRates_GetTimeFormat(true));
		break;
	case 'x':
		ret = format_fetch_time(hContact, CurrencyRates_GetDateFormat(true));
		break;
	case 't':
		{
			CMStringW sFrmt = CurrencyRates_GetDateFormat(true);
			sFrmt += L" ";
			sFrmt += CurrencyRates_GetTimeFormat(true);
			ret = format_fetch_time(hContact, sFrmt);
		}
		break;

	default:
		if (ParseSymbol(hContact, c, d))
			ret = format_double(d, nWidth);
		else
			ret = L"-";
		break;
	}

	return ret;
}

bool CCurrencyRatesProviderBase::GetWatchedRateInfo(MCONTACT hContact, TRateInfo &rRateInfo)
{
	CMStringW sSymbolFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
	CMStringW sSymbolTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
	CMStringW sDescFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_DESCRIPTION);
	CMStringW sDescTo = g_plugin.getMStringW(hContact, DB_STR_TO_DESCRIPTION);

	rRateInfo.first = CCurrencyRate(sSymbolFrom, sSymbolFrom, sDescFrom);
	rRateInfo.second = CCurrencyRate(sSymbolTo, sSymbolTo, sDescTo);
	return true;
}

bool CCurrencyRatesProviderBase::WatchForRate(const TRateInfo &ri, bool bWatch)
{
	MCONTACT hContact = GetContactByID(ri.first.GetID(), ri.second.GetID());

	if (bWatch && hContact == 0) {
		CMStringW sName = ri.first.GetSymbol() + L"/" + ri.second.GetSymbol();
		hContact = CreateNewContact(sName);
		if (hContact) {
			g_plugin.setWString(hContact, DB_STR_FROM_ID, ri.first.GetID().c_str());
			g_plugin.setWString(hContact, DB_STR_TO_ID, ri.second.GetID().c_str());
			if (false == ri.first.GetName().IsEmpty()) {
				g_plugin.setWString(hContact, DB_STR_FROM_DESCRIPTION, ri.first.GetName().c_str());
			}
			if (false == ri.second.GetName().IsEmpty()) {
				g_plugin.setWString(hContact, DB_STR_TO_DESCRIPTION, ri.second.GetName().c_str());
			}

			return true;
		}
	}
	else if (!bWatch && hContact) {
		db_delete_contact(hContact, true);
		return true;
	}

	return false;
}

MCONTACT CCurrencyRatesProviderBase::GetContactByID(const CMStringW &rsFromID, const CMStringW &rsToID) const
{
	for (auto &hContact : Contacts(MODULENAME)) {
		CMStringW sFrom = g_plugin.getMStringW(hContact, DB_STR_FROM_ID);
		CMStringW sTo = g_plugin.getMStringW(hContact, DB_STR_TO_ID);
		if (!mir_wstrcmpi(rsFromID.c_str(), sFrom.c_str()) && !mir_wstrcmpi(rsToID.c_str(), sTo.c_str()))
			return hContact;
	}

	return 0;
}

MCONTACT CCurrencyRatesProviderBase::ImportContact(const TiXmlNode *pRoot)
{
	const char *sFromID = nullptr, *sToID = nullptr;

	for (auto *pNode : TiXmlFilter(pRoot, "Setting")) {
		TNameValue Item = parse_setting_node(pNode);
		if (!mir_strcmpi(Item.first, DB_STR_FROM_ID))
			sFromID = Item.second;
		else if (!mir_strcmpi(Item.first, DB_STR_TO_ID))
			sToID = Item.second;
	}

	if (sFromID && sToID)
		return GetContactByID(Utf2T(sFromID).get(), Utf2T(sToID).get());

	return 0;
}
