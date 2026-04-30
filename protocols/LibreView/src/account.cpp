#include "stdafx.h"

static void AddLibreHeaders(MHttpRequest &request, const Account *pAcc = nullptr)
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

Account::Account(CLibreViewProto *_1, MCONTACT _2) :
	ppro(_1),
	hContact(_2)
{
	szApiUrl = NormalizeBaseUrl(ppro->getMStringA(hContact, "ApiUrl"));
	szMinVersion = ppro->getMStringA(hContact, "MinVersion");
	if (szMinVersion.IsEmpty())
		szMinVersion = DEFAULT_API_VERSION;

	Ignore_Ignore(hContact, IGNOREEVENT_USERONLINE);
}

void Account::ClearAuth()
{
	szToken.Empty();
	szAccountHash.Empty();
	szPatientId.Empty();
}

bool Account::Login()
{
	ptrW wszEmail(ppro->getWStringA(hContact, "Email"));
	ptrW wszPassword(ppro->getWStringA(hContact, "Password"));
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
			ppro->setString(hContact, "MinVersion", szMinVersion);
			continue;
		}

		if (data["redirect"].as_bool()) {
			CMStringA region = data["region"].as_mstring();
			if (!region.IsEmpty()) {
				szApiUrl.Format("https://api-%s.libreview.io", region.c_str());
				ppro->setString(hContact, "ApiUrl", szApiUrl);
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

bool Account::FetchConnections()
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
	ptrW value(db_get_wsa(hContact, DB_MODULE_GLUCOSE, pszSetting));
	return value ? CMStringW(value) : CMStringW();
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

	const wchar_t *pwszUnit = bUseMgdl ? L"mg/dL" : L"mmol/l";
	CMStringW trendArrow = GetGlucoseDbText(hContact, "TrendSymbol");

	db_set_ws(hContact, DB_MODULE_GLUCOSE, "Value", valueText);
	db_set_ws(hContact, DB_MODULE_GLUCOSE, "Unit", pwszUnit);

	CMStringW title = GetGlucoseDbText(hContact, "PatientName");
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
	if (timestamp.IsEmpty())
		return;

	ptrW lastTimestamp(db_get_wsa(hContact, DB_MODULE_GLUCOSE, "LastHistoryTimestamp"));
	if (lastTimestamp && !mir_wstrcmp(lastTimestamp, timestamp))
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
	dbei.iTimestamp = db_get_dw(hContact, DB_MODULE_GLUCOSE, "TimestampUnix", (uint32_t)time(0));
	dbei.flags = DBEF_READ | DBEF_UTF;
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.pBlob = utfMessage;
	dbei.cbBlob = (uint32_t)mir_strlen(utfMessage) + 1;
	db_event_add(hContact, &dbei);

	db_set_ws(hContact, DB_MODULE_GLUCOSE, "LastHistoryTimestamp", timestamp);
}

static CMStringW FormatApiTarget(double value)
{
	return CMStringW(FORMAT, L"%.1f", value / 10.0);
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

bool Account::FetchGlucose()
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
		ppro->setString(hContact, "MinVersion", szMinVersion);
		ClearAuth();
		return FetchGlucose();
	}

	JSONNode connection = data["connection"];
	CMStringW firstName = connection["firstName"].as_mstring();
	CMStringW lastName = connection["lastName"].as_mstring();
	CMStringW patientName(firstName);
	if (!lastName.IsEmpty()) {
		if (!patientName.IsEmpty())
			patientName.AppendChar(' ');
		patientName.Append(lastName);
	}
	if (!patientName.IsEmpty()) {
		ppro->setWString(hContact, "Nick", patientName);
		db_set_ws(hContact, DB_MODULE_GLUCOSE, "PatientName", patientName);
	}

	int targetLow = connection["targetLow"].as_int();
	int targetHigh = connection["targetHigh"].as_int();
	if (targetLow)
		db_set_ws(hContact, DB_MODULE_GLUCOSE, "TargetLow", FormatApiTarget(targetLow));
	if (targetHigh)
		db_set_ws(hContact, DB_MODULE_GLUCOSE, "TargetHigh", FormatApiTarget(targetHigh));

	uint32_t sensorActivation = connection["sensor"]["a"].as_int();
	if (sensorActivation) {
		db_set_dw(hContact, DB_MODULE_GLUCOSE, "SensorActivationTime", sensorActivation);
		db_set_ws(hContact, DB_MODULE_GLUCOSE, "SensorActivation", FormatUnixTime(sensorActivation));
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

	db_set_ws(hContact, DB_MODULE_GLUCOSE, "ValueMmol", valueMmolText);
	db_set_ws(hContact, DB_MODULE_GLUCOSE, "ValueMgDl", valueMgdlText);
	db_set_dw(hContact, DB_MODULE_GLUCOSE, "ValueMmolTimes10", (uint32_t)(valueMmol * 10 + 0.5));
	db_set_dw(hContact, DB_MODULE_GLUCOSE, "ValueMgDlInt", (uint32_t)(valueMgdl + 0.5));
	db_set_dw(hContact, DB_MODULE_GLUCOSE, "TrendArrow", trend);
	db_set_ws(hContact, DB_MODULE_GLUCOSE, "Trend", trendText);
	db_set_ws(hContact, DB_MODULE_GLUCOSE, "TrendSymbol", trendArrow);
	db_set_dw(hContact, DB_MODULE_GLUCOSE, "GlucoseUnits", glucoseUnits);
	db_set_ws(hContact, DB_MODULE_GLUCOSE, "Timestamp", timestamp);
	db_set_dw(hContact, DB_MODULE_GLUCOSE, "TimestampUnix", timestampUnix);
	db_set_ws(hContact, DB_MODULE_GLUCOSE, "TimestampFormatted", timestampFormatted);

	UpdateContactDisplay(hContact);
	AddHistoryEvent(hContact, timestamp);

	CMStringW statusMsg(FORMAT, L"%s, %s", trendText.c_str(), timestampFormatted.c_str());
	db_set_ws(hContact, "CList", "StatusMsg", statusMsg);
	ppro->ProtoBroadcastAck(hContact, ACKTYPE_AWAYMSG, ACKRESULT_SUCCESS, nullptr, (LPARAM)statusMsg.c_str());

	int contactStatus = ID_STATUS_ONLINE;
	if (isLow)
		contactStatus = ID_STATUS_DND;
	else if (isHigh)
		contactStatus = ID_STATUS_AWAY;
	ppro->setWord(hContact, "Status", contactStatus);
	return true;
}
