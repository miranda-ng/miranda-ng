#include "StdAfx.h"

const char g_szXmlValue[] = "Value";
const char g_szXmlName[] = "Name";
const char g_szXmlSetting[] = "Setting";
const char g_szXmlModule[] = "Module";
const char g_szXmlContact[] = "Contact";
const char g_szXmlContacts[] = "Contacts";
const char g_szXmlType[] = "type";
const char g_szXmlTypeByte[] = "byte";
const char g_szXmlTypeWord[] = "word";
const char g_szXmlTypeDword[] = "dword";
const char g_szXmlTypeAsciiz[] = "asciiz";
const char g_szXmlTypeWchar[] = "wchar";
const char g_szXmlTypeUtf8[] = "utf8";
const char g_szXmlTypeBlob[] = "blob";

struct CEnumContext
{
	CEnumContext(TiXmlDocument &doc) :
		m_xmlDoc(doc)
	{}

	TiXmlDocument &m_xmlDoc;
	TiXmlNode *m_pNode;
	MCONTACT m_hContact;
	LPCSTR m_pszModule;
};

struct mir_safety_dbvar
{
	mir_safety_dbvar(DBVARIANT* p) : m_p(p) {}
	~mir_safety_dbvar() { db_free(m_p); }
	DBVARIANT* m_p;
};

static int enum_contact_settings(const char *szSetting, void *lp)
{
	CEnumContext *ctx = reinterpret_cast<CEnumContext*>(lp);

	DBVARIANT dbv;
	if (0 == db_get(ctx->m_hContact, ctx->m_pszModule, szSetting, &dbv)) {
		mir_safety_dbvar sdbvar(&dbv);

		CMStringA sType;
		CMStringW wszValue;

		switch (dbv.type) {
		case DBVT_BYTE:
			wszValue.Format(L"%d", dbv.bVal);
			sType = g_szXmlTypeByte;
			break;
		case DBVT_WORD:
			wszValue.Format(L"%d", dbv.wVal);
			sType = g_szXmlTypeWord;
			break;
		case DBVT_DWORD:
			wszValue.Format(L"%d", dbv.dVal);
			sType = g_szXmlTypeDword;
			break;
		case DBVT_ASCIIZ:
			sType = g_szXmlTypeAsciiz;
			if (dbv.pszVal)
				wszValue = dbv.pszVal;
			break;
		case DBVT_WCHAR:
			sType = g_szXmlTypeWchar;
			if (dbv.pwszVal)
				wszValue = dbv.pwszVal;
			break;
		case DBVT_UTF8:
			sType = g_szXmlTypeUtf8;
			if (dbv.pszVal)
				wszValue = dbv.pszVal;
			break;
		case DBVT_BLOB:
			sType = g_szXmlTypeBlob;
			if (dbv.pbVal) {
				ptrA buf(mir_base64_encode(dbv.pbVal, dbv.cpbVal));
				if (buf)
					wszValue = buf;
			}
			break;
		}

		auto *pXmlName = ctx->m_xmlDoc.NewElement(g_szXmlName);
		pXmlName->SetText(szSetting);

		auto *pXmlValue = ctx->m_xmlDoc.NewElement(g_szXmlValue);
		pXmlValue->SetText(T2Utf(wszValue.c_str()).get());
		pXmlValue->SetAttribute(g_szXmlType, sType);

		auto *pXmlSet = ctx->m_xmlDoc.NewElement(g_szXmlSetting);
		pXmlSet->InsertEndChild(pXmlName);
		pXmlSet->InsertEndChild(pXmlValue);
		ctx->m_pNode->InsertEndChild(pXmlSet);
	}

	return 0;
}

int EnumDbModules(const char *szModuleName, void *lp)
{
	CEnumContext *ctx = (CEnumContext*)lp;
	auto *pXml = ctx->m_pNode;
	auto *pModule = ctx->m_xmlDoc.NewElement(g_szXmlModule);
	pModule->SetText(szModuleName);

	ctx->m_pszModule = szModuleName;
	ctx->m_pNode = pModule;
	db_enum_settings(ctx->m_hContact, &enum_contact_settings, szModuleName, ctx);

	if (pModule->FirstChildElement(g_szXmlSetting))
		pXml->InsertEndChild(pModule);

	ctx->m_pNode = pXml;
	return 0;
}

TiXmlNode* export_contact(MCONTACT hContact, TiXmlDocument &pDoc)
{
	CEnumContext ctx(pDoc);
	ctx.m_pNode = pDoc.NewElement(g_szXmlContact);
	ctx.m_hContact = hContact;
	db_enum_modules(EnumDbModules, &ctx);

	return ctx.m_pNode;
}

LPCTSTR prepare_filter(LPTSTR pszBuffer, size_t cBuffer)
{
	LPTSTR p = pszBuffer;
	LPCTSTR pszXml = TranslateT("XML File (*.xml)");
	mir_wstrncpy(p, pszXml, (int)cBuffer);
	size_t nLen = mir_wstrlen(pszXml) + 1;
	p += nLen;
	if (nLen < cBuffer) {
		mir_wstrncpy(p, L"*.xml", (int)(cBuffer - nLen));
		p += 6;
		nLen += 6;
	}

	if (nLen < cBuffer) {
		LPCTSTR pszAll = TranslateT("All files (*.*)");
		mir_wstrncpy(p, pszAll, (int)(cBuffer - nLen));
		size_t n = mir_wstrlen(pszAll) + 1;
		nLen += n;
		p += n;
	}

	if (nLen < cBuffer) {
		mir_wstrncpy(p, L"*.*", (int)(cBuffer - nLen));
		p += 4;
		nLen += 4;
	}

	if (nLen < cBuffer)
		*p = '\0';

	return pszBuffer;
}

bool show_open_file_dialog(bool bOpen, CMStringW &rsFile)
{
	wchar_t szBuffer[MAX_PATH];
	wchar_t szFilter[MAX_PATH];
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lStructSize = sizeof(OPENFILENAME);

	ofn.hwndOwner = nullptr;
	ofn.lpstrFilter = prepare_filter(szFilter, MAX_PATH);
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
	ofn.lpstrDefExt = L"xml";
	if (bOpen)
		ofn.Flags |= OFN_FILEMUSTEXIST;
	else
		ofn.Flags |= OFN_OVERWRITEPROMPT;

	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFile = szBuffer;
	ofn.lpstrFile[0] = '\0';

	if (bOpen) {
		if (FALSE == GetOpenFileName(&ofn))
			return false;
	}
	else {
		if (FALSE == GetSaveFileName(&ofn))
			return false;
	}

	rsFile = szBuffer;
	return true;
}

INT_PTR CurrencyRates_Export(WPARAM wp, LPARAM lp)
{
	CMStringW sFileName;
	const char* pszFile = reinterpret_cast<const char*>(lp);
	if (nullptr == pszFile) {
		if (false == show_open_file_dialog(false, sFileName))
			return -1;
	}
	else sFileName = _A2T(pszFile);

	TiXmlDocument doc;
	auto *pRoot = doc.NewElement(g_szXmlContacts);
	doc.InsertFirstChild(pRoot);

	MCONTACT hContact = MCONTACT(wp);
	if (hContact) {
		auto pProvider = GetContactProviderPtr(hContact);
		if (pProvider) {
			auto *pNode = export_contact(hContact, doc);
			if (pNode)
				pRoot->InsertEndChild(pNode);
		}
	}
	else {
		for (auto &cc : Contacts(MODULENAME)) {
			auto pProvider = GetContactProviderPtr(cc);
			if (pProvider) {
				auto *pNode = export_contact(cc, doc);
				if (pNode)
					pRoot->InsertEndChild(pNode);
			}
		}
	}

	return doc.SaveFile(_T2A(sFileName.c_str()), true);
}

bool set_contact_settings(MCONTACT hContact, DBCONTACTWRITESETTING& dbs)
{
	assert(DBVT_DELETED != dbs.value.type);
	return (0 == db_set(hContact, dbs.szModule, dbs.szSetting, &dbs.value));
}

bool handle_module(MCONTACT hContact, const TiXmlElement *pXmlModule)
{
	const char *szModuleName = pXmlModule->GetText();
	if (szModuleName == nullptr)
		return false;

	size_t cCreatedRecords = 0;
	bool bCListModule = !mir_strcmpi(szModuleName, "CList");

	DBCONTACTWRITESETTING dbs;
	dbs.szModule = szModuleName;
	for (auto *pSetting : TiXmlFilter(pXmlModule, g_szXmlSetting)) {
		auto *pNode = pSetting->FirstChildElement(g_szXmlName);
		if (pNode == nullptr)
			continue;
		const char *sName = pNode->GetText();

		pNode = pSetting->FirstChildElement(g_szXmlValue);
		if (pNode == nullptr)
			continue;
		const char *sValue = pNode->GetText();
		const char *sType = pNode->Attribute(g_szXmlType);

		if (sName == nullptr || sType == nullptr || sValue == nullptr)
			continue;

		dbs.szSetting = sName;
		if (!mir_strcmpi(g_szXmlTypeByte, sType)) {
			std::istringstream in(sValue);
			in.imbue(GetSystemLocale());
			dbs.value.cVal = in.get();
			if (in.good() && in.eof()) {
				dbs.value.type = DBVT_BYTE;
				if (set_contact_settings(hContact, dbs))
					++cCreatedRecords;
			}
		}
		else if (!mir_strcmpi(g_szXmlTypeWord, sType)) {
			std::istringstream in(sValue);
			in.imbue(GetSystemLocale());
			in >> dbs.value.wVal;
			if (in.good() || in.eof()) {
				dbs.value.type = DBVT_WORD;
				if (set_contact_settings(hContact, dbs))
					++cCreatedRecords;
			}
		}
		else if (!mir_strcmpi(g_szXmlTypeDword, sType)) {
			std::istringstream in(sValue);
			in.imbue(GetSystemLocale());
			in >> dbs.value.dVal;
			if (in.good() || in.eof()) {
				dbs.value.type = DBVT_DWORD;
				if (set_contact_settings(hContact, dbs))
					++cCreatedRecords;
			}
		}
		else if (!mir_strcmpi(g_szXmlTypeAsciiz, sType)) {
			dbs.value.pszVal = (char*)sValue;
			dbs.value.type = DBVT_ASCIIZ;
			if (set_contact_settings(hContact, dbs))
				++cCreatedRecords;
		}
		else if (!mir_strcmpi(g_szXmlTypeUtf8, sType)) {
			dbs.value.pszVal = (char*)sValue;
			dbs.value.type = DBVT_UTF8;
			if (set_contact_settings(hContact, dbs))
				++cCreatedRecords;
		}
		else if (!mir_strcmpi(g_szXmlTypeWchar, sType)) {
			Utf2T val(sValue);
			dbs.value.pwszVal = val;
			dbs.value.type = DBVT_WCHAR;
			if (set_contact_settings(hContact, dbs))
				++cCreatedRecords;
		}
		else if (!mir_strcmpi(g_szXmlTypeBlob, sType)) {
			size_t bufLen;
			mir_ptr<BYTE> buf((PBYTE)mir_base64_decode(sValue, &bufLen));
			if (buf) {
				dbs.value.pbVal = buf;
				dbs.value.cpbVal = (WORD)bufLen;
				dbs.value.type = DBVT_BLOB;

				if (set_contact_settings(hContact, dbs))
					++cCreatedRecords;
			}
		}

		if (bCListModule && !mir_strcmpi(sName, "Group"))
			Clist_GroupCreate(NULL, Utf2T(sValue));
	}

	return true;
}

size_t count_contacts(const TiXmlNode *pXmlRoot, bool bInContactsGroup)
{
	size_t cContacts = 0;

	for (auto *pNode : TiXmlEnum(pXmlRoot)) {
		const char *sName = pNode->Name();
		if (false == bInContactsGroup) {
			if (!mir_strcmpi(g_szXmlContacts, sName))
				cContacts += count_contacts(pNode, true);
			else
				cContacts += count_contacts(pNode, false);
		}
		else {
			if (!mir_strcmpi(g_szXmlContact, sName))
				++cContacts;
		}
	}

	return cContacts;
}

struct CImportContext
{
	CImportContext(size_t cTotalContacts) : m_cTotalContacts(cTotalContacts), m_cHandledContacts(0), m_nFlags(0) {}

	size_t m_cTotalContacts;
	size_t m_cHandledContacts;
	UINT m_nFlags;
};

struct CContactState
{
	CContactState() : m_hContact(NULL), m_bNewContact(false) {}
	MCONTACT m_hContact;
	ICurrencyRatesProvider *m_pProvider;
	bool m_bNewContact;
};

const TiXmlNode* find_currencyrates_module(const TiXmlNode *pXmlContact)
{
	for (auto *pNode : TiXmlEnum(pXmlContact))
		if ((!mir_strcmpi(g_szXmlModule, pNode->Name())) && (!mir_strcmpi(MODULENAME, pNode->GetText())))
			return pNode;

	return nullptr;
}

TNameValue parse_setting_node(const TiXmlNode *pXmlSetting)
{
	assert(pXmlSetting);

	const char *sName, *sValue;
	for (auto *pNode : TiXmlEnum(pXmlSetting)) {
		if (!mir_strcmpi(g_szXmlName, pNode->Name()))
			sName = pNode->GetText();
		else if (!mir_strcmpi(g_szXmlValue, pNode->Name()))
			sValue = pNode->GetText();
	}

	return std::make_pair(sName, sValue);
}

ICurrencyRatesProvider* find_provider(const TiXmlNode *pXmlCurrencyRatesModule)
{
	for (auto *pNode : TiXmlFilter(pXmlCurrencyRatesModule, g_szXmlSetting)) {
		TNameValue Item = parse_setting_node(pNode);
		if ((!mir_strcmpi(DB_STR_CURRENCYRATE_PROVIDER, Item.first)) && Item.second)
			return FindProvider(Utf2T(Item.second).get());
	}

	return nullptr;
}

bool get_contact_state(const TiXmlNode *pXmlContact, CContactState& cst)
{
	auto *pXmlCurrencyRates = find_currencyrates_module(pXmlContact);
	if (!pXmlCurrencyRates)
		return false;

	cst.m_pProvider = find_provider(pXmlCurrencyRates);
	if (!cst.m_pProvider)
		return false;

	cst.m_hContact = cst.m_pProvider->ImportContact(pXmlCurrencyRates);
	return true;
}

bool import_contact(const TiXmlNode *pXmlContact, CImportContext &impctx)
{
	++impctx.m_cHandledContacts;

	CContactState cst;
	if (!get_contact_state(pXmlContact, cst))
		return false;

	if (NULL == cst.m_hContact) {
		cst.m_hContact = db_add_contact();
		cst.m_bNewContact = true;
	}
	else if (impctx.m_nFlags & CURRENCYRATES_IMPORT_SKIP_EXISTING_CONTACTS)
		return true;

	if (!cst.m_hContact)
		return false;

	for (auto *pNode : TiXmlFilter(pXmlContact, g_szXmlModule))
		if (!handle_module(cst.m_hContact, pNode))
			return false;

	if (cst.m_bNewContact) {
		cst.m_pProvider->AddContact(cst.m_hContact);
		cst.m_pProvider->RefreshContact(cst.m_hContact);
	}
	return true;
}

size_t import_contacts(const TiXmlNode *pXmlContacts, CImportContext &impctx)
{
	size_t cContacts = 0;
	for (auto *pNode : TiXmlFilter(pXmlContacts, g_szXmlContact))
		if (import_contact(pNode, impctx))
			++cContacts;

	return cContacts;
}

size_t handle_contacts_node(const TiXmlNode *pXmlRoot, CImportContext& impctx)
{
	size_t cContacts = 0;
	for (auto *pNode : TiXmlEnum(pXmlRoot)) {
		if (!mir_strcmpi(g_szXmlContacts, pNode->Name()))
			cContacts += import_contacts(pNode, impctx);
		else
			cContacts += handle_contacts_node(pNode, impctx);
	}

	return cContacts;
}

bool do_import(const TiXmlNode *pXmlRoot, UINT nFlags)
{
	CImportContext imctx(count_contacts(pXmlRoot, false));
	imctx.m_cHandledContacts = 0;
	imctx.m_nFlags = nFlags;

	return (handle_contacts_node(pXmlRoot, imctx) > 0);
}

INT_PTR CurrencyRates_Import(WPARAM wp, LPARAM lp)
{
	CMStringW sFileName;
	const char* pszFile = reinterpret_cast<const char*>(lp);
	if (nullptr == pszFile) {
		if (false == show_open_file_dialog(true, sFileName))
			return -1;
	}
	else sFileName = _A2T(pszFile);

	FILE *in = _wfopen(sFileName.c_str(), L"rb");
	if (in == nullptr)
		return 1;

	TiXmlDocument doc;
	int res = doc.LoadFile(in);
	fclose(in);
	if (res)
		return 1;
	
	return (do_import(&doc, wp) ? 0 : 1);
}
