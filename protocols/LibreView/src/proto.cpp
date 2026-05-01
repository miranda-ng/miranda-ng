#include "stdafx.h"

static void AddLibreHeaders(MHttpRequest &request, const CLibreViewProto *pAcc = nullptr)
{
	request.flags = NLHRF_HTTP11 | NLHRF_DUMPASTEXT | NLHRF_REDIRECT | NLHRF_SSL;
	request.AddHeader("Accept-Encoding", "gzip");
	request.AddHeader("Cache-Control", "no-cache");
	request.AddHeader("Connection", "Keep-Alive");
	request.AddHeader("Content-Type", "application/json");
	request.AddHeader("Product", "llu.android");
	request.AddHeader("Version", (pAcc && !pAcc->szMinVersion.IsEmpty()) ? pAcc->szMinVersion : DEFAULT_API_VERSION);

	if (pAcc && !pAcc->szToken.IsEmpty()) {
		request.AddHeader("Authorization", "Bearer " + pAcc->szToken);
		request.AddHeader("Account-Id", pAcc->szAccountHash);
	}
}

static CMStringA JsonBody(JSONNode &root)
{
	ptrW wszText(json_write(&root));
	CMStringA result(ptrA(mir_utf8encodeW(wszText)));
	return result;
}

static bool JsonTransaction(MHttpRequest &request, JSONNode &root)
{
	NLHR_PTR response(Netlib_HttpTransaction(hNetlibUser, &request));
	if (response == nullptr || response->body.IsEmpty())
		return false;

	root = JSONNode::parse(response->body);
	return !root.empty();
}

static CMStringA Sha256Hex(const CMStringA &src)
{
	uint8_t hash[MIR_SHA256_HASH_SIZE];
	mir_sha256_hash(src.c_str(), src.GetLength(), hash);

	CMStringA result;
	for (auto b : hash)
		result.AppendFormat("%02x", b);
	return result;
}

static CMStringA NormalizeBaseUrl(const CMStringA &url)
{
	CMStringA result(url);
	result.Trim();
	while (!result.IsEmpty() && result[result.GetLength() - 1] == '/')
		result.Truncate(result.GetLength() - 1);
	if (result.IsEmpty())
		result = DEFAULT_API_URL;
	return result;
}

static const wchar_t* GetLocalizedUnit(bool bUseMgdl)
{
	return bUseMgdl ? TranslateT("mg/dL") : TranslateT("mmol/L");
}

static bool IsMgdlUnitToken(const CMStringW &unit)
{
	return !mir_wstrcmpi(unit, L"mg/dL");
}

static bool IsMmolUnitToken(const CMStringW &unit)
{
	return !mir_wstrcmpi(unit, L"mmol/L");
}

CLibreViewProto::CLibreViewProto(const char *protoName, const wchar_t *userName) :
	PROTO<CLibreViewProto>(protoName, userName),
	UpdateInterval(m_szModuleName, "UpdateInterval", db_get_dw(0, MODULENAME, "UpdateInterval", 5)),
	DisplayUnits(m_szModuleName, "DisplayUnits", db_get_dw(0, MODULENAME, "DisplayUnits", 0)),
	WriteHistory(m_szModuleName, "WriteHistory", db_get_b(0, MODULENAME, "WriteHistory", 0) != 0)
{
	m_hProtoIcon = Skin_LoadProtoIcon(MODULENAME, ID_STATUS_ONLINE);
	EnsureAccount();

	// Check if login/password exist but contact doesn't - create contact
	if (!m_hContact) {
		ptrW wszEmail(getWStringA("Email"));
		ptrW wszPassword(getWStringA("Password"));
		if (mir_wstrlen(wszEmail) && mir_wstrlen(wszPassword)) {
			EnsureAccountContact();
		}
	}

	if (m_hContact) {
		szMinVersion = getMStringA(m_hContact, "MinVersion");
		Ignore_Ignore(m_hContact, IGNOREEVENT_USERONLINE);
	}

	szApiUrl = NormalizeBaseUrl(getMStringA("ApiUrl"));

	if (szMinVersion.IsEmpty())
		szMinVersion = DEFAULT_API_VERSION;

	CreateProtoService("/Update", &CLibreViewProto::Update);
	HookProtoEvent(ME_OPT_INITIALISE, &CLibreViewProto::OptInit);
}

CLibreViewProto::~CLibreViewProto()
{
}

INT_PTR CLibreViewProto::GetCaps(int type, MCONTACT)
{
	switch (type) {
	case PFLAGNUM_2:
	case PFLAGNUM_5:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_HEAVYDND;
	}

	return 0;
}

int CLibreViewProto::SetStatus(int iStatus)
{
	int oldStatus = m_iStatus;
	m_iStatus = (iStatus == ID_STATUS_OFFLINE) ? ID_STATUS_OFFLINE : ID_STATUS_ONLINE;
	ProtoBroadcastAck(0, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, m_iStatus);

	if (m_hContact) {
		if (m_iStatus == ID_STATUS_OFFLINE)
			setWord(m_hContact, "Status", ID_STATUS_OFFLINE);
		else
			mir_forkthread(Check_ThreadFunc, this);
	}
	return 0;
}

void CLibreViewProto::OnModulesLoaded()
{
}

void CLibreViewProto::OnShutdown()
{
	if (m_hContact)
		setWord(m_hContact, "Status", ID_STATUS_OFFLINE);
}

/////////////////////////////////////////////////////////////////////////////////////////

void CLibreViewProto::EnsureAccount()
{
	m_hContact = 0;
	for (auto &it : AccContacts()) {
		m_hContact = it;
		break;
	}

	if (m_hContact)
		setWord(m_hContact, "Status", ID_STATUS_OFFLINE);
}

MCONTACT CLibreViewProto::EnsureAccountContact()
{
	if (m_hContact)
		return m_hContact;

	MCONTACT hContact = db_add_contact();
	if (hContact == 0)
		return 0;

	Proto_AddToContact(hContact, m_szModuleName);

	// Use account name as contact Nick
	setWString(hContact, "Nick", m_tszUserName);

		
	m_hContact = hContact;
	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
	return hContact;
}

INT_PTR CLibreViewProto::Update(WPARAM, LPARAM)
{
	mir_forkthread(Check_ThreadFunc, this);
	return 0;
}

void CLibreViewProto::ClearAuth()
{
	szToken.Empty();
	szAccountHash.Empty();
	szPatientId.Empty();
}

bool CLibreViewProto::Login()
{
	ptrW wszEmail(getWStringA("Email"));
	ptrW wszPassword(getWStringA("Password"));
	if (!mir_wstrlen(wszEmail) || !mir_wstrlen(wszPassword))
		return false;

	for (int retry = 0; retry < 3; retry++) {
		JSONNode payload;
		payload << WCHAR_PARAM("email", wszEmail) << WCHAR_PARAM("password", wszPassword);

		MHttpRequest request(REQUEST_POST);
		request.m_szUrl = szApiUrl + "/llu/auth/login";
		request.m_szParam = JsonBody(payload);
		AddLibreHeaders(request, this);

		JSONNode root;
		if (!JsonTransaction(request, root))
			return false;

		JSONNode data = root["data"];
		CMStringA minimumVersion = data["minimumVersion"].as_mstring();
		if (!minimumVersion.IsEmpty() && minimumVersion != szMinVersion) {
			szMinVersion = minimumVersion;
			setString(m_hContact, "MinVersion", szMinVersion);
			continue;
		}

		if (data["redirect"].as_bool()) {
			CMStringA region = data["region"].as_mstring();
			if (!region.IsEmpty()) {
				szApiUrl.Format("https://api-%s.libreview.io", region.c_str());
				setString("ApiUrl", szApiUrl);
				continue;
			}
		}

		szToken = data["authTicket"]["token"].as_mstring();
		CMStringA accountId = data["user"]["id"].as_mstring();
		if (szToken.IsEmpty() || accountId.IsEmpty())
			return false;

		szAccountHash = Sha256Hex(accountId);
		return FetchConnections();
	}

	return false;
}

bool CLibreViewProto::FetchConnections()
{
	MHttpRequest request(REQUEST_GET);
	request.m_szUrl = szApiUrl + "/llu/connections";
	AddLibreHeaders(request, this);

	JSONNode root;
	if (!JsonTransaction(request, root))
		return false;

	for (auto &connection : root["data"]) {
		szPatientId = connection["patientId"].as_mstring();
		if (!szPatientId.IsEmpty())
			return true;
	}

	return false;
}

static const wchar_t* TrendToText(int trend)
{
	switch (trend) {
	case 1: return L"falling quickly";
	case 2: return L"falling";
	case 3: return L"steady";
	case 4: return L"rising";
	case 5: return L"rising quickly";
	default: return L"trend unknown";
	}
}

static const wchar_t* TrendToArrow(int trend)
{
	switch (trend) {
	case 1: return L"\x2193";
	case 2: return L"\x2198";
	case 3: return L"\x2192";
	case 4: return L"\x2197";
	case 5: return L"\x2191";
	default: return L"";
	}
}

static CMStringW FormatGlucoseValue(double value)
{
	CMStringW result(FORMAT, L"%.1f", value);
	if (result.Right(2) == L".0")
		result.Truncate(result.GetLength() - 2);
	return result;
}

static CMStringW GetGlucoseDbText(MCONTACT hContact, const char *pszSetting)
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	if (!ppro)
		return CMStringW();
	return ppro->getMStringW(hContact, pszSetting);
}

static uint32_t ParseLibreTimestamp(const CMStringW &timestamp)
{
	int month = 0, day = 0, year = 0, hour = 0, minute = 0, second = 0;
	wchar_t ampm[8] = {};
	if (swscanf(timestamp.c_str(), L"%d/%d/%d %d:%d:%d %7ls", &month, &day, &year, &hour, &minute, &second, ampm) != 7)
		return 0;

	if (!mir_wstrcmpi(ampm, L"PM") && hour < 12)
		hour += 12;
	else if (!mir_wstrcmpi(ampm, L"AM") && hour == 12)
		hour = 0;

	struct tm tmLocal = {};
	tmLocal.tm_year = year - 1900;
	tmLocal.tm_mon = month - 1;
	tmLocal.tm_mday = day;
	tmLocal.tm_hour = hour;
	tmLocal.tm_min = minute;
	tmLocal.tm_sec = second;
	tmLocal.tm_isdst = -1;

	time_t result = mktime(&tmLocal);
	return result == -1 ? 0 : (uint32_t)result;
}

static CMStringW FormatMirandaTimestamp(uint32_t timestamp)
{
	if (timestamp == 0)
		return CMStringW();

	wchar_t wszTime[100];
	TimeZone_PrintTimeStamp(nullptr, timestamp, L"d t", wszTime, _countof(wszTime), 0);
	return wszTime;
}

void UpdateContactDisplay(MCONTACT hContact)
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	const bool bUseMgdl = ppro && ppro->DisplayUnits == 1;
	CMStringW valueText = GetGlucoseDbText(hContact, bUseMgdl ? "ValueMgDl" : "ValueMmol");
	if (valueText.IsEmpty())
		return;

	const wchar_t *pwszUnit = GetLocalizedUnit(bUseMgdl);
	CMStringW trendArrow = GetGlucoseDbText(hContact, "TrendSymbol");

	ppro->setWString(hContact, "Value", valueText);
	ppro->setWString(hContact, "Unit", pwszUnit);
	ppro->setWString(hContact, "TargetLow", GetGlucoseDbText(hContact, bUseMgdl ? "TargetLowMgDl" : "TargetLowMmol"));
	ppro->setWString(hContact, "TargetHigh", GetGlucoseDbText(hContact, bUseMgdl ? "TargetHighMgDl" : "TargetHighMmol"));

	// Build title from FirstName + LastName
	CMStringW firstName = GetGlucoseDbText(hContact, "FirstName");
	CMStringW lastName = GetGlucoseDbText(hContact, "LastName");
	CMStringW title(firstName);
	if (!lastName.IsEmpty()) {
		if (!title.IsEmpty())
			title.AppendChar(' ');
		title.Append(lastName);
	}
	if (title.IsEmpty()) {
		char *szProto = Proto_GetBaseAccountName(hContact);
		ptrW wszNick(szProto ? db_get_wsa(hContact, szProto, "Nick") : nullptr);
		if (mir_wstrlen(wszNick))
			title = wszNick;
		else
			title = TranslateT("LibreView");
	}

	CMStringW clistName(FORMAT, L"%s: %s %s", title.c_str(), valueText.c_str(), pwszUnit);
	if (!trendArrow.IsEmpty())
		clistName.AppendFormat(L" (%s)", trendArrow.c_str());
	db_set_ws(hContact, "CList", "MyHandle", clistName);
}

static void AddHistoryEvent(MCONTACT hContact, const CMStringW &timestamp)
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	if (ppro == nullptr || !ppro->WriteHistory)
		return;

	if (timestamp.IsEmpty())
		return;

	CMStringW lastTimestamp = ppro->getMStringW(hContact, "LastHistoryTimestamp");
	if (!lastTimestamp.IsEmpty() && !mir_wstrcmp(lastTimestamp, timestamp))
		return;

	CMStringW valueText = GetGlucoseDbText(hContact, "Value");
	if (valueText.IsEmpty())
		return;

	CMStringW unit = GetGlucoseDbText(hContact, "Unit");
	CMStringW trendArrow = GetGlucoseDbText(hContact, "TrendSymbol");
	CMStringW timestampText = GetGlucoseDbText(hContact, "TimestampFormatted");
	if (timestampText.IsEmpty())
		timestampText = timestamp;
	CMStringW message(FORMAT, L"%s: %s %s", timestampText.c_str(), valueText.c_str(), unit.c_str());
	if (!trendArrow.IsEmpty())
		message.AppendFormat(L" (%s)", trendArrow.c_str());
	T2Utf utfMessage(message);

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.iTimestamp = ppro->getDword(hContact, "TimestampUnix", (uint32_t)time(0));
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.pBlob = utfMessage;
	dbei.cbBlob = (uint32_t)mir_strlen(utfMessage) + 1;
	db_event_add(hContact, &dbei);

	ppro->setWString(hContact, "LastHistoryTimestamp", timestamp);
}

static bool IsApiUnitMgdl(const JSONNode &measurement, int glucoseUnits)
{
	CMStringW unit = measurement["Unit"].as_mstring();
	if (IsMgdlUnitToken(unit))
		return true;
	if (IsMmolUnitToken(unit))
		return false;

	return glucoseUnits == 1;
}

static CMStringW FormatTargetValue(int rawValue, bool bApiMgdl, bool bOutputMgdl)
{
	double mmolValue = bApiMgdl ? (double)rawValue / 18.0 : (double)rawValue / 10.0;
	double mgdlValue = bApiMgdl ? (double)rawValue : mmolValue * 18.0;
	return FormatGlucoseValue(bOutputMgdl ? mgdlValue : mmolValue);
}

static CMStringW FormatUnixTime(uint32_t timestamp)
{
	if (timestamp == 0)
		return CMStringW();

	time_t t = timestamp;
	struct tm tmLocal = {};
	localtime_s(&tmLocal, &t);

	wchar_t buf[64];
	wcsftime(buf, _countof(buf), L"%Y-%m-%d %H:%M:%S", &tmLocal);
	return buf;
}

bool CLibreViewProto::FetchGlucose()
{
	if (szToken.IsEmpty() || szAccountHash.IsEmpty() || szPatientId.IsEmpty())
		if (!Login())
			return false;

	MHttpRequest request(REQUEST_GET);
	request.m_szUrl.Format("%s/llu/connections/%s/graph", szApiUrl.c_str(), szPatientId.c_str());
	AddLibreHeaders(request, this);

	JSONNode root;
	if (!JsonTransaction(request, root)) {
		ClearAuth();
		return false;
	}

	JSONNode data = root["data"];
	CMStringA minimumVersion = data["minimumVersion"].as_mstring();
	if (!minimumVersion.IsEmpty() && minimumVersion != szMinVersion) {
		szMinVersion = minimumVersion;
		setString(m_hContact, "MinVersion", szMinVersion);
		ClearAuth();
		return FetchGlucose();
	}

	JSONNode connection = data["connection"];
	CMStringW firstName = connection["firstName"].as_mstring();
	CMStringW lastName = connection["lastName"].as_mstring();

	if (!firstName.IsEmpty())
		setWString(m_hContact, "FirstName", firstName);
	if (!lastName.IsEmpty())
		setWString(m_hContact, "LastName", lastName);

	uint32_t sensorActivation = connection["sensor"]["a"].as_int();
	if (sensorActivation) {
		setDword(m_hContact, "SensorActivationTime", sensorActivation);
		setWString(m_hContact, "SensorActivation", FormatUnixTime(sensorActivation));
	}

	JSONNode measurement = connection["glucoseMeasurement"];
	double valueMmol = measurement["Value"].as_float();
	double valueMgdl = measurement["ValueInMgPerDl"].as_float();
	if (valueMmol == 0 && valueMgdl == 0) {
		ClearAuth();
		return false;
	}
	if (valueMmol == 0 && valueMgdl)
		valueMmol = valueMgdl / 18.0;
	if (valueMgdl == 0 && valueMmol)
		valueMgdl = valueMmol * 18.0;

	int trend = measurement["TrendArrow"].as_int();
	int glucoseUnits = measurement["GlucoseUnits"].as_int();
	bool bApiMgdl = IsApiUnitMgdl(measurement, glucoseUnits);
	bool isLow = measurement["isLow"].as_bool();
	bool isHigh = measurement["isHigh"].as_bool();
	CMStringW timestamp = measurement["Timestamp"].as_mstring();
	uint32_t timestampUnix = ParseLibreTimestamp(timestamp);
	CMStringW timestampFormatted = FormatMirandaTimestamp(timestampUnix);
	if (timestampFormatted.IsEmpty())
		timestampFormatted = timestamp;
	CMStringW trendText(TranslateW(TrendToText(trend)));
	CMStringW trendArrow(TrendToArrow(trend));
	CMStringW valueMmolText = FormatGlucoseValue(valueMmol);
	CMStringW valueMgdlText = FormatGlucoseValue(valueMgdl);

	setWString(m_hContact, "ValueMmol", valueMmolText);
	setWString(m_hContact, "ValueMgDl", valueMgdlText);
	setDword(m_hContact, "ValueMmolTimes10", (uint32_t)(valueMmol * 10 + 0.5));
	setDword(m_hContact, "ValueMgDlInt", (uint32_t)(valueMgdl + 0.5));
	setDword(m_hContact, "TrendArrow", trend);
	setWString(m_hContact, "Trend", trendText);
	setWString(m_hContact, "TrendSymbol", trendArrow);
	setDword(m_hContact, "GlucoseUnits", glucoseUnits);
	setWString(m_hContact, "Timestamp", timestamp);
	setDword(m_hContact, "TimestampUnix", timestampUnix);
	setWString(m_hContact, "TimestampFormatted", timestampFormatted);

	int targetLow = connection["targetLow"].as_int();
	int targetHigh = connection["targetHigh"].as_int();
	if (targetLow) {
		setWString(m_hContact, "TargetLowMmol", FormatTargetValue(targetLow, bApiMgdl, false));
		setWString(m_hContact, "TargetLowMgDl", FormatTargetValue(targetLow, bApiMgdl, true));
	}
	if (targetHigh) {
		setWString(m_hContact, "TargetHighMmol", FormatTargetValue(targetHigh, bApiMgdl, false));
		setWString(m_hContact, "TargetHighMgDl", FormatTargetValue(targetHigh, bApiMgdl, true));
	}

	UpdateContactDisplay(m_hContact);
	AddHistoryEvent(m_hContact, timestamp);

	CMStringW statusMsg(FORMAT, L"%s, %s", trendText.c_str(), timestampFormatted.c_str());
	db_set_ws(m_hContact, "CList", "StatusMsg", statusMsg);
	ProtoBroadcastAck(m_hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, nullptr, (LPARAM)statusMsg.c_str());

	int contactStatus = ID_STATUS_ONLINE;
	if (isLow)
		contactStatus = ID_STATUS_DND;
	else if (isHigh)
		contactStatus = ID_STATUS_AWAY;
	setWord(m_hContact, "Status", contactStatus);
	return true;
}
