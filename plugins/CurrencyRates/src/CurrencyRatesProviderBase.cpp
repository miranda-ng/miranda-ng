#include "StdAfx.h"

extern bool g_bAutoUpdate;
extern HANDLE g_hEventWorkThreadStop;

struct CCurrencyRatesProviderBase::CXMLFileInfo
{
	CXMLFileInfo() : m_qs(L"Unknown") {}
	ICurrencyRatesProvider::CProviderInfo m_pi;
	CCurrencyRatesProviderBase::CCurrencyRateSection m_qs;
	tstring m_sURL;
};

inline tstring get_ini_file_name(LPCTSTR pszFileName)
{
	return CreateFilePath(pszFileName);
}

bool parse_currencyrate(const IXMLNode::TXMLNodePtr& pTop, CCurrencyRatesProviderBase::CCurrencyRate& q)
{
	tstring sSymbol;
	tstring sDescription;
	tstring sID;

	size_t cChild = pTop->GetChildCount();
	for (size_t i = 0; i < cChild; ++i) {
		IXMLNode::TXMLNodePtr pNode = pTop->GetChildNode(i);
		tstring sName = pNode->GetName();
		if (0 == mir_wstrcmpi(L"symbol", sName.c_str())) {
			sSymbol = pNode->GetText();
			if (true == sSymbol.empty())
				return false;
		}
		else if (0 == mir_wstrcmpi(L"description", sName.c_str())) {
			sDescription = pNode->GetText();
		}
		else if (0 == mir_wstrcmpi(L"id", sName.c_str())) {
			sID = pNode->GetText();
			if (true == sID.empty())
				return false;
		}
	}

	q = CCurrencyRatesProviderBase::CCurrencyRate(sID, TranslateW(sSymbol.c_str()), TranslateW(sDescription.c_str()));
	return true;
}

bool parse_section(const IXMLNode::TXMLNodePtr& pTop, CCurrencyRatesProviderBase::CCurrencyRateSection& qs)
{
	CCurrencyRatesProviderBase::CCurrencyRateSection::TSections aSections;
	CCurrencyRatesProviderBase::CCurrencyRateSection::TCurrencyRates aCurrencyRates;
	tstring sSectionName;

	size_t cChild = pTop->GetChildCount();
	for (size_t i = 0; i < cChild; ++i) {
		IXMLNode::TXMLNodePtr pNode = pTop->GetChildNode(i);
		tstring sName = pNode->GetName();
		if (0 == mir_wstrcmpi(L"section", sName.c_str())) {
			CCurrencyRatesProviderBase::CCurrencyRateSection qs1;
			if (true == parse_section(pNode, qs1))
				aSections.push_back(qs1);
		}
		else if (0 == mir_wstrcmpi(L"currencyrate", sName.c_str())) {
			CCurrencyRatesProviderBase::CCurrencyRate q;
			if (true == parse_currencyrate(pNode, q))
				aCurrencyRates.push_back(q);
		}
		else if (0 == mir_wstrcmpi(L"name", sName.c_str())) {
			sSectionName = pNode->GetText();
			if (true == sSectionName.empty())
				return false;
		}
	}

	qs = CCurrencyRatesProviderBase::CCurrencyRateSection(TranslateW(sSectionName.c_str()), aSections, aCurrencyRates);
	return true;
}

IXMLNode::TXMLNodePtr find_provider(const IXMLNode::TXMLNodePtr& pRoot)
{
	IXMLNode::TXMLNodePtr pProvider;
	size_t cChild = pRoot->GetChildCount();
	for (size_t i = 0; i < cChild; ++i) {
		IXMLNode::TXMLNodePtr pNode = pRoot->GetChildNode(i);
		tstring sName = pNode->GetName();
		if (0 == mir_wstrcmpi(L"Provider", sName.c_str())) {
			pProvider = pNode;
			break;
		}

		pProvider = find_provider(pNode);
		if (pProvider)
			break;
	}

	return pProvider;
}

CCurrencyRatesProviderBase::CXMLFileInfo parse_ini_file(const tstring& rsXMLFile, bool& rbSucceded)
{
	CCurrencyRatesProviderBase::CXMLFileInfo res;
	CCurrencyRatesProviderBase::CCurrencyRateSection::TSections aSections;

	const CModuleInfo::TXMLEnginePtr& pXMLEngine = CModuleInfo::GetXMLEnginePtr();
	IXMLNode::TXMLNodePtr pRoot = pXMLEngine->LoadFile(rsXMLFile);
	if (pRoot) {
		IXMLNode::TXMLNodePtr pProvider = find_provider(pRoot);
		if (pProvider) {
			rbSucceded = true;
			size_t cChild = pProvider->GetChildCount();
			for (size_t i = 0; i < cChild; ++i) {
				IXMLNode::TXMLNodePtr pNode = pProvider->GetChildNode(i);
				tstring sName = pNode->GetName();
				if (0 == mir_wstrcmpi(L"section", sName.c_str())) {
					CCurrencyRatesProviderBase::CCurrencyRateSection qs;
					if (true == parse_section(pNode, qs))
						aSections.push_back(qs);
				}
				else if (0 == mir_wstrcmpi(L"Name", sName.c_str()))
					res.m_pi.m_sName = pNode->GetText();
				else if (0 == mir_wstrcmpi(L"ref", sName.c_str()))
					res.m_pi.m_sURL = pNode->GetText();
				else if (0 == mir_wstrcmpi(L"url", sName.c_str()))
					res.m_sURL = pNode->GetText();
			}
		}
	}

	res.m_qs = CCurrencyRatesProviderBase::CCurrencyRateSection(res.m_pi.m_sName, aSections);
	return res;
}

CCurrencyRatesProviderBase::CXMLFileInfo init_xml_info(LPCTSTR pszFileName, bool& rbSucceded)
{
	rbSucceded = false;
	tstring sIniFile = get_ini_file_name(pszFileName);
	return parse_ini_file(sIniFile, rbSucceded);
}

CCurrencyRatesProviderBase::CCurrencyRatesProviderBase()
	: m_hEventSettingsChanged(::CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_hEventRefreshContact(::CreateEvent(nullptr, FALSE, FALSE, nullptr)),
	m_bRefreshInProgress(false)
{
}

CCurrencyRatesProviderBase::~CCurrencyRatesProviderBase()
{
	::CloseHandle(m_hEventSettingsChanged);
	::CloseHandle(m_hEventRefreshContact);
}

bool CCurrencyRatesProviderBase::Init()
{
	bool bSucceded = m_pXMLInfo != nullptr;
	if (!m_pXMLInfo) {
		CCurrencyRatesProviderVisitorDbSettings visitor;
		Accept(visitor);
		assert(visitor.m_pszXMLIniFileName);

		m_pXMLInfo.reset(new CXMLFileInfo(init_xml_info(visitor.m_pszXMLIniFileName, bSucceded)));
	}

	return bSucceded;
}

CCurrencyRatesProviderBase::CXMLFileInfo* CCurrencyRatesProviderBase::GetXMLFileInfo()const
{
	// 	if(!m_pXMLInfo)
	// 	{
	// 		CCurrencyRatesProviderVisitorDbSettings visitor;
	// 		Accept(visitor);
	// 		assert(visitor.m_pszXMLIniFileName);
	// 		m_pXMLInfo.reset(new CXMLFileInfo(init_xml_info(visitor.m_pszXMLIniFileName)));
	// 	}

	return m_pXMLInfo.get();
}

const CCurrencyRatesProviderBase::CProviderInfo& CCurrencyRatesProviderBase::GetInfo()const
{
	return GetXMLFileInfo()->m_pi;
}

const CCurrencyRatesProviderBase::CCurrencyRateSection& CCurrencyRatesProviderBase::GetCurrencyRates()const
{
	return GetXMLFileInfo()->m_qs;
}

const tstring& CCurrencyRatesProviderBase::GetURL()const
{
	return GetXMLFileInfo()->m_sURL;
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

	TContracts::iterator i = std::find(m_aContacts.begin(), m_aContacts.end(), hContact);
	if (i != m_aContacts.end())
		m_aContacts.erase(i);
}

void CCurrencyRatesProviderBase::SetContactStatus(MCONTACT hContact, int nNewStatus)
{
	int nStatus = db_get_w(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_STATUS, ID_STATUS_OFFLINE);
	if (nNewStatus != nStatus) {
		db_set_w(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_STATUS, nNewStatus);

		if (ID_STATUS_ONLINE != nNewStatus) {
			db_unset(hContact, LIST_MODULE_NAME, STATUS_MSG_NAME);
			tstring sSymbol = CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_SYMBOL);
			if (false == sSymbol.empty())
				db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, sSymbol.c_str());

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

	bool Parse(const ICurrencyRatesProvider* pProvider, const tstring& rsFrmt, MCONTACT hContact)
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
					CCurrencyRatesProviderVisitorTendency visitor(hContact, t);
					pProvider->Accept(visitor);
					if (false == visitor.IsValid()) {
						bValid = false;
					}
					else {
						double d = visitor.GetResult();
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

	bool IsValid()const { return (m_abValueFlags[0] && m_abValueFlags[1] && (m_nComparison != NonValid)); }

	EResult Compare()const
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
					default:       sResult += chr; sResult += t; break;
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

				CCurrencyRatesProviderVisitorFormater visitor(hContact, chr, nWidth);
				pProvider->Accept(visitor);
				const tstring& s = visitor.GetResult();
				sResult += s;
				++i;
			}
			else sResult += chr;
			break;
		}
	}

	return sResult;
}

void log_to_file(const ICurrencyRatesProvider* pProvider,
	MCONTACT hContact,
	const tstring& rsLogFileName,
	const tstring& rsFormat)
{
	std::string sPath = currencyrates_t2a(rsLogFileName.c_str());

	std::string::size_type n = sPath.find_last_of("\\/");
	if (std::string::npos != n)
		sPath.erase(n);

	DWORD dwAttributes = ::GetFileAttributesA(sPath.c_str());
	if ((0xffffffff == dwAttributes) || (0 == (dwAttributes&FILE_ATTRIBUTE_DIRECTORY)))
		CreateDirectoryTree(sPath.c_str());

	tofstream file(rsLogFileName.c_str(), std::ios::app | std::ios::out);
	file.imbue(GetSystemLocale());
	if (file.good()) {
		tstring s = format_rate(pProvider, hContact, rsFormat);
		file << s;
	}
}

void log_to_history(const ICurrencyRatesProvider* pProvider,
	MCONTACT hContact,
	time_t nTime,
	const tstring& rsFormat)
{
	tstring s = format_rate(pProvider, hContact, rsFormat);
	T2Utf psz(s.c_str());

	DBEVENTINFO dbei = {};
	dbei.szModule = CURRENCYRATES_MODULE_NAME;
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

bool show_popup(const ICurrencyRatesProvider* pProvider,
	MCONTACT hContact,
	const CTendency& tendency,
	const tstring& rsFormat,
	const CPopupSettings& ps)
{
	if (!ServiceExists(MS_POPUP_ADDPOPUPT))
		return false;

	POPUPDATAT ppd;
	memset(&ppd, 0, sizeof(ppd));
	ppd.lchContact = hContact;

	if (tendency.IsValid()) {
		CTendency::EResult nComparison = tendency.Compare();
		if (CTendency::NotChanged == nComparison)
			ppd.lchIcon = CurrencyRates_LoadIconEx(IDI_ICON_NOTCHANGED);
		else if (CTendency::Up == nComparison)
			ppd.lchIcon = CurrencyRates_LoadIconEx(IDI_ICON_UP);
		else if (CTendency::Down == nComparison)
			ppd.lchIcon = CurrencyRates_LoadIconEx(IDI_ICON_DOWN);
	}

	CCurrencyRatesProviderVisitorFormater visitor(hContact, 's', 0);
	pProvider->Accept(visitor);
	const tstring& sTitle = visitor.GetResult();
	mir_wstrncpy(ppd.lptzContactName, sTitle.c_str(), MAX_CONTACTNAME);
	{
		ptrW ss(variables_parsedup((wchar_t*)rsFormat.c_str(), nullptr, hContact));
		tstring sText = format_rate(pProvider, hContact, tstring(ss));
		mir_wstrncpy(ppd.lptzText, sText.c_str(), MAX_SECONDLINE);
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

	LPARAM lp = 0;
	if (false == ps.GetHistoryFlag())
		lp |= 0x08;

	return (0 == CallService(MS_POPUP_ADDPOPUPT, reinterpret_cast<WPARAM>(&ppd), lp));
}

void CCurrencyRatesProviderBase::WriteContactRate(MCONTACT hContact, double dRate, const tstring& rsSymbol/* = ""*/)
{
	time_t nTime = ::time(0);

	if (false == rsSymbol.empty())
		db_set_ws(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_SYMBOL, rsSymbol.c_str());

	double dPrev = 0.0;
	bool bValidPrev = CurrencyRates_DBReadDouble(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_CURR_VALUE, dPrev);
	if (true == bValidPrev)
		CurrencyRates_DBWriteDouble(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_PREV_VALUE, dPrev);

	CurrencyRates_DBWriteDouble(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_CURR_VALUE, dRate);
	db_set_dw(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_FETCH_TIME, nTime);

	tstring sSymbol = rsSymbol;

	tostringstream oNick;
	oNick.imbue(GetSystemLocale());
	if (false == m_sContactListFormat.empty()) {
		tstring s = format_rate(this, hContact, m_sContactListFormat);
		oNick << s;
	}
	else {
		if (true == sSymbol.empty())
			sSymbol = CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_SYMBOL);

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

	bool bUseContactSpecific = (db_get_b(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CONTACT_SPEC_SETTINGS, 0) > 0);

	CAdvProviderSettings global_settings(this);

	WORD dwMode = (bUseContactSpecific)
		? db_get_w(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_LOG, static_cast<WORD>(lmDisabled))
		: global_settings.GetLogMode();
	if (dwMode&lmExternalFile) {
		bool bAdd = true;
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (db_get_w(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_LOG_FILE_CONDITION, 1) > 0)
			: global_settings.GetLogOnlyChangedFlag();
		if (true == bOnlyIfChanged) {
			bAdd = ((false == bValidPrev) || (false == IsWithinAccuracy(dRate, dPrev)));
		}
		if (true == bAdd) {
			tstring sLogFileName = (bUseContactSpecific)
				? CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_LOG_FILE, global_settings.GetLogFileName().c_str())
				: global_settings.GetLogFileName();

			if (true == sSymbol.empty()) {
				sSymbol = CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_SYMBOL);
			}

			sLogFileName = GenerateLogFileName(sLogFileName, sSymbol);

			tstring sFormat = global_settings.GetLogFormat();
			if (bUseContactSpecific) {
				CCurrencyRatesProviderVisitorDbSettings visitor;
				Accept(visitor);
				sFormat = CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_FORMAT_LOG_FILE, visitor.m_pszDefLogFileFormat);
			}

			log_to_file(this, hContact, sLogFileName, sFormat);
		}
	}
	if (dwMode&lmInternalHistory) {
		bool bAdd = true;
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (db_get_w(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_HISTORY_CONDITION, 1) > 0)
			: global_settings.GetHistoryOnlyChangedFlag();

		if (true == bOnlyIfChanged) {
			bAdd = ((false == bValidPrev) || (false == IsWithinAccuracy(dRate, dPrev)));
		}
		if (true == bAdd) {
			tstring sFormat = (bUseContactSpecific)
				? CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_FORMAT_HISTORY, global_settings.GetHistoryFormat().c_str())
				: global_settings.GetHistoryFormat();

			log_to_history(this, hContact, nTime, sFormat);
		}
	}

	if (dwMode&lmPopup) {
		bool bOnlyIfChanged = (bUseContactSpecific)
			? (1 == db_get_b(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_POPUP_CONDITION, 1) > 0)
			: global_settings.GetShowPopupIfValueChangedFlag();
		if ((false == bOnlyIfChanged)
			|| ((true == bOnlyIfChanged) && (true == bValidPrev) && (false == IsWithinAccuracy(dRate, dPrev)))) {
			tstring sFormat = (bUseContactSpecific)
				? CurrencyRates_DBGetStringT(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_FORMAT_POPUP, global_settings.GetPopupFormat().c_str())
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
	if (hContact) {
		if (0 == Proto_AddToContact(hContact, CURRENCYRATES_PROTOCOL_NAME)) {
			tstring sProvName = GetInfo().m_sName;
			db_set_ws(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_PROVIDER, sProvName.c_str());
			db_set_ws(hContact, CURRENCYRATES_MODULE_NAME, DB_STR_CURRENCYRATE_SYMBOL, rsName.c_str());
			db_set_ws(hContact, LIST_MODULE_NAME, CONTACT_LIST_NAME, rsName.c_str());

			mir_cslock lck(m_cs);
			m_aContacts.push_back(hContact);
		}
		else {
			db_delete_contact(hContact);
			hContact = NULL;
		}
	}

	return hContact;
}

DWORD get_refresh_timeout_miliseconds(const CCurrencyRatesProviderVisitorDbSettings& visitor)
{
	if (!g_bAutoUpdate)
		return INFINITE;

	assert(visitor.m_pszDbRefreshRateType);
	assert(visitor.m_pszDbRefreshRateValue);

	int nRefreshRateType = db_get_w(0, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbRefreshRateType, RRT_MINUTES);
	if (nRefreshRateType < RRT_SECONDS || nRefreshRateType > RRT_HOURS)
		nRefreshRateType = RRT_MINUTES;

	DWORD nTimeout = db_get_w(0, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbRefreshRateValue, 1);
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
	CCurrencyRatesProviderVisitorDbSettings visitor;
	Accept(visitor);

	DWORD nTimeout = get_refresh_timeout_miliseconds(visitor);
	m_sContactListFormat = CurrencyRates_DBGetStringT(NULL, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbDisplayNameFormat, visitor.m_pszDefDisplayFormat);
	m_sStatusMsgFormat = CurrencyRates_DBGetStringT(NULL, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbStatusMsgFormat, visitor.m_pszDefStatusMsgFormat);
	m_sTendencyFormat = CurrencyRates_DBGetStringT(NULL, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbTendencyFormat, visitor.m_pszDefTendencyFormat);

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

	TContracts anContacts;
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
			nTimeout = get_refresh_timeout_miliseconds(visitor);
			m_sContactListFormat = CurrencyRates_DBGetStringT(NULL, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbDisplayNameFormat, visitor.m_pszDefDisplayFormat);
			m_sStatusMsgFormat = CurrencyRates_DBGetStringT(NULL, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbStatusMsgFormat, visitor.m_pszDefStatusMsgFormat);
			m_sTendencyFormat = CurrencyRates_DBGetStringT(NULL, CURRENCYRATES_MODULE_NAME, visitor.m_pszDbTendencyFormat, visitor.m_pszDefTendencyFormat);
			{
				mir_cslock lck(m_cs);
				anContacts = m_aContacts;
			}
			break;
		case WAIT_OBJECT_0 + REFRESH_CONTACT:
			{
				DWORD dwTimeRest = ::GetTickCount() - dwBegin;
				if (INFINITE != nTimeout && dwTimeRest < nTimeout) {
					nTimeout -= dwTimeRest;
				}

				{
					mir_cslock lck(m_cs);
					anContacts = m_aRefreshingContacts;
					m_aRefreshingContacts.clear();
				}

				{
					CBoolGuard bg(m_bRefreshInProgress);
					RefreshCurrencyRates(anContacts);
				}
			}
			break;
		case WAIT_TIMEOUT:
			nTimeout = get_refresh_timeout_miliseconds(visitor);
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
	TContracts anContacts;
	{
		mir_cslock lck(m_cs);
		anContacts = m_aContacts;
		m_aRefreshingContacts.clear();
	}

	CBoolGuard bg(m_bRefreshInProgress);
	std::for_each(anContacts.begin(), anContacts.end(), boost::bind(&SetContactStatus, _1, ID_STATUS_OFFLINE));
}

void CCurrencyRatesProviderBase::Accept(CCurrencyRatesProviderVisitor &visitor)const
{
	visitor.Visit(*this);
}

void CCurrencyRatesProviderBase::RefreshSettings()
{
	BOOL b = ::SetEvent(m_hEventSettingsChanged);
	assert(b && "Failed to set event");
}

void CCurrencyRatesProviderBase::RefreshAllContacts()
{
	{// for CCritSection
		mir_cslock lck(m_cs);
		m_aRefreshingContacts.clear();
		std::for_each(std::begin(m_aContacts), std::end(m_aContacts), [&](MCONTACT hContact) { m_aRefreshingContacts.push_back(hContact); });
	}

	BOOL b = ::SetEvent(m_hEventRefreshContact);
	assert(b && "Failed to set event");
}

void CCurrencyRatesProviderBase::RefreshContact(MCONTACT hContact)
{
	{// for CCritSection
		mir_cslock lck(m_cs);
		m_aRefreshingContacts.push_back(hContact);
	}

	BOOL b = ::SetEvent(m_hEventRefreshContact);
	assert(b && "Failed to set event");
}
