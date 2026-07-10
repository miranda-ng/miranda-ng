#include "stdafx.h"

static void AddLibreHeaders(MHttpRequest &request, const CLibreViewProto *pAcc = nullptr)
{
	request.flags = NLHRF_HTTP11 | NLHRF_REDIRECT | NLHRF_SSL | NLHRF_NODUMP;
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

CLibreViewProto::CLibreViewProto(const char *protoName, const wchar_t *userName) :
	PROTO<CLibreViewProto>(protoName, userName),
	UpdateInterval(m_szModuleName, "UpdateInterval", db_get_dw(0, MODULENAME, "UpdateInterval", 5)),
	DisplayUnits(m_szModuleName, "DisplayUnits", db_get_dw(0, MODULENAME, "DisplayUnits", 0)),
	WriteHistory(m_szModuleName, "WriteHistory", db_get_b(0, MODULENAME, "WriteHistory", 0) != 0),
	Offset(m_szModuleName, "Offset", 0)
{
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
		Ignore_Ignore(m_hContact, IGNOREEVENT_USERONLINE);
	}

	szMinVersion = getMStringA("MinVersion");

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
		return PF2_ONLINE;
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

		// Check for API error status
		int status = root["status"].as_int();
		if (status != 0) {
			CMStringW errorMsg = root["error"]["message"].as_mstring();
			if (errorMsg.IsEmpty())
				errorMsg = TranslateT("Login failed");

			// Show popup notification
			PUShowMessageW(CMStringW(FORMAT, L"%s: %s", TranslateT("Login error"), errorMsg.c_str()), SM_ERROR);
			return false;
		}

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
	case 1: return LPGENW("falling quickly");
	case 2: return LPGENW("falling");
	case 3: return LPGENW("steady");
	case 4: return LPGENW("rising");
	case 5: return LPGENW("rising quickly");
	default: return LPGENW("trend unknown");
	}
}

static const wchar_t* TrendToArrow(int trend)
{
	switch (trend) {
	case 1: return L"\x2193\x2193";  // falling quickly (↓↓)
	case 2: return L"\x2193";         // falling (↓)
	case 3: return L"\x2192";         // steady (→)
	case 4: return L"\x2191";         // rising (↑)
	case 5: return L"\x2191\x2191";  // rising quickly (↑↑)
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


uint32_t ParseLibreTimestamp(const CMStringW &timestamp)
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


void UpdateContactDisplay(MCONTACT hContact)
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	const bool bUseMgdl = ppro && ppro->DisplayUnits == 1;

	// Get account name from Nick (always set to account name)
	CMStringW title = ppro->getMStringW(hContact, "Nick");

	// Check sensor state
	uint32_t activation = ppro ? ppro->getDword(hContact, "SensorActivationTime", 0) : 0;
	if (IsSensorExpired(activation)) {
		CMStringW expiredMsg(FORMAT, L"%s: %s", title.c_str(), TranslateT("sensor expired"));
		db_set_ws(hContact, "CList", "MyHandle", expiredMsg);
		return;
	}
	uint32_t warmupMinutes = ppro ? ppro->getDword(hContact, "SensorWarmupMinutes", 0) : 0;
	if (IsSensorWarmingUp(activation, warmupMinutes)) {
		CMStringW warmupMsg(FORMAT, L"%s: %s", title.c_str(), TranslateT("warming up"));
		db_set_ws(hContact, "CList", "MyHandle", warmupMsg);
		return;
	}

	// Get API units from database
	const int apiUnits = ppro ? ppro->getDword(hContact, "GlucoseUnits", 0) : 0;
	const bool bApiMgdl = apiUnits == 1;
	const int offset = ppro ? ppro->Offset : 0;

	// Get stored original API value and convert for display
	CMStringW originalValueText = GetGlucoseDbText(hContact, "Value");
	CMStringW valueText = ConvertGlucoseForDisplay(originalValueText, bApiMgdl, bUseMgdl, offset);

	const wchar_t *pwszUnit = GetLocalizedUnit(bUseMgdl);
	int trendValue = ppro->getDword(hContact, "TrendArrow", 0);
	CMStringW trendArrow(TrendToArrow(trendValue));

	CMStringW clistName;
	if (!valueText.IsEmpty())
		clistName.Format(L"%s: %s %s", title.c_str(), valueText.c_str(), pwszUnit);
	else
		clistName = title;
	if (!trendArrow.IsEmpty() && !valueText.IsEmpty())
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

	CMStringW originalValueText = GetGlucoseDbText(hContact, "Value");
	if (originalValueText.IsEmpty())
		return;

	const bool bUseMgdl = ppro->DisplayUnits == 1;
	const int apiUnits = ppro->getDword(hContact, "GlucoseUnits", 0);
	const bool bApiMgdl = apiUnits == 1;
	const int offset = ppro->Offset;
	const wchar_t *pwszUnit = GetLocalizedUnit(bUseMgdl);
	int trendValue = ppro->getDword(hContact, "TrendArrow", 0);
	CMStringW trendArrow(TrendToArrow(trendValue));

	// Get original API value and convert for history
	CMStringW historyValueText = ConvertGlucoseForDisplay(originalValueText, bApiMgdl, bUseMgdl, offset);

	CMStringW message(FORMAT, L"%s %s", historyValueText.c_str(), pwszUnit);
	if (!trendArrow.IsEmpty())
		message.AppendFormat(L" (%s)", trendArrow.c_str());
	T2Utf utfMessage(message);

	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	CMStringW timestampText = GetGlucoseDbText(hContact, "Timestamp");
	dbei.iTimestamp = ParseLibreTimestamp(timestampText);
	if (!dbei.iTimestamp)
		dbei.iTimestamp = (uint32_t)time(0);
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.pBlob = utfMessage;
	dbei.cbBlob = (uint32_t)mir_strlen(utfMessage) + 1;
	db_event_add(hContact, &dbei);

	ppro->setWString(hContact, "LastHistoryTimestamp", timestamp);
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
		// Show popup for network/HTTP errors
		PUShowMessageW(TranslateT("Failed to connect to LibreView server. Please check your internet connection and try again."), SM_WARNING);
		return false;
	}

	JSONNode data = root["data"];
	CMStringA minimumVersion = data["minimumVersion"].as_mstring();
	if (!minimumVersion.IsEmpty() && minimumVersion != szMinVersion) {
		szMinVersion = minimumVersion;
		setString("MinVersion", szMinVersion);
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

	uint32_t sensorActivationTime = connection["sensor"]["a"].as_int();
	if (sensorActivationTime) {
		// API returns GMT/UTC time, store as-is
		setDword(m_hContact, "SensorActivationTime", sensorActivationTime);
	}

	uint32_t warmupMinutes = connection["sensor"]["w"].as_int();
	setDword(m_hContact, "SensorWarmupMinutes", warmupMinutes);

	JSONNode measurement = connection["glucoseMeasurement"];
	int trend = measurement["TrendArrow"].as_int();
	int glucoseUnits = measurement["GlucoseUnits"].as_int();
	bool bApiMgdl = glucoseUnits == 1;

	double valueMmol, valueMgdl;
	if (bApiMgdl) {
		valueMgdl = measurement["Value"].as_float();
		valueMmol = valueMgdl / 18.0;
	}
	else {
		valueMmol = measurement["Value"].as_float();
		valueMgdl = valueMmol * 18.0;
	}
	if (valueMmol == 0 && valueMgdl == 0) {
		ClearAuth();
		return false;
	}
	CMStringW timestamp = measurement["Timestamp"].as_mstring();

	CMStringW trendText(TranslateW(TrendToText(trend)));
	CMStringW trendArrow(TrendToArrow(trend));

	// Store original API value (in original units)
	CMStringW originalValueText;
	if (bApiMgdl)
		originalValueText = FormatGlucoseValue(valueMgdl);
	else
		originalValueText = FormatGlucoseValue(valueMmol);

	setWString(m_hContact, "Value", originalValueText);
	setDword(m_hContact, "TrendArrow", trend);
	setDword(m_hContact, "GlucoseUnits", glucoseUnits);
	setWString(m_hContact, "Timestamp", timestamp);

	// Refresh graph if open (event 2: new Value arrives)
	RefreshGraphWindow();

	// Create StatusMsg first so UpdateContactDisplay can access it
	CMStringW statusMsg;
	if (IsSensorExpired(sensorActivationTime))
		statusMsg = TranslateT("sensor expired");
	else if (IsSensorWarmingUp(sensorActivationTime, warmupMinutes))
		statusMsg = TranslateT("warming up");
	else
		statusMsg = trendText;

	// Force status update to refresh StatusMsg display
	setWord(m_hContact, "Status", ID_STATUS_ONLINE);

	UpdateContactDisplay(m_hContact);

	// Set StatusMsg at very end after all other operations
	db_set_ws(m_hContact, "CList", "StatusMsg", statusMsg);
	AddHistoryEvent(m_hContact, timestamp);

	JSONNode graphDataArray = data["graphData"];
	if (!graphDataArray.empty()) {
		setString(m_hContact, "GraphData", graphDataArray.write().c_str());
		// Refresh graph if open (event 1: new GraphData arrives)
		RefreshGraphWindow();
	}

	return true;
}
