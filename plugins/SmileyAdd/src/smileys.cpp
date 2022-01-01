/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org)
Copyright (C) 2005-11 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003-04 Rein-Peter de Boer

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

SmileyPackListType g_SmileyPacks;
SmileyCategoryListType g_SmileyCategories;

static HWND hwndHidden = nullptr;

static void CALLBACK timerProc(HWND, UINT, UINT_PTR param, DWORD)
{
	SmileyType *pType = (SmileyType*)param;
	pType->MoveToNextFrame();
}

// these two functions must be called from the main thread
static void CALLBACK sttStartTimer(PVOID obj)
{
	if (hwndHidden == nullptr)
		hwndHidden = CreateWindowEx(0, L"STATIC", nullptr, 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

	SmileyType *pType = (SmileyType*)obj;
	pType->SetFrameDelay();
}

static void CALLBACK sttStopTimer(PVOID obj)
{
	KillTimer(hwndHidden, (DWORD_PTR)obj);
}

//
// SmileyType
//

SmileyType::SmileyType(void) :
	m_arSmileys(10, PtrKeySortT)
{
	m_SmileyIcon = nullptr;
	m_xepimg = nullptr;
	m_flags = 0;
	m_index = 0;
	m_size.cx = 0;
	m_size.cy = 0;
}

SmileyType::~SmileyType()
{
	if (m_xepimg) {
		m_xepimg->Release();
		m_xepimg = nullptr;
	}

	if (m_SmileyIcon != nullptr) {
		DestroyIcon(m_SmileyIcon);
		m_SmileyIcon = nullptr;
	}
}

void SmileyType::AddObject(ISmileyBase *pObject)
{
	if (m_arSmileys.getCount() == 0) {
		if (m_xepimg == nullptr)
			m_xepimg = AddCacheImage(m_filepath, m_index);
		CallFunctionAsync(sttStartTimer, this);
	}

	m_arSmileys.insert(pObject);
}

void SmileyType::RemoveObject(ISmileyBase *pObject)
{
	int idx = m_arSmileys.getIndex(pObject);
	if (idx == -1)
		return;

	m_arSmileys.remove(idx);
	if (m_arSmileys.getCount() == 0)
		CallFunctionAsync(sttStopTimer, this);
}

void SmileyType::SetFrameDelay()
{
	int iFrameDelay = (m_xepimg == nullptr) ? 0 : m_xepimg->GetFrameDelay();
	if (iFrameDelay <= 0)
		KillTimer(hwndHidden, (DWORD_PTR)this);
	else
		SetTimer(hwndHidden, (DWORD_PTR)this, iFrameDelay*10, timerProc);
}

void SmileyType::MoveToNextFrame()
{
	m_index = m_xepimg->SelectNextFrame(m_index);

	for (auto &it : m_arSmileys.rev_iter())
		it->Draw();

	SetFrameDelay(); // reset timer
}

HICON SmileyType::GetIcon(void)
{
	if (m_SmileyIcon == nullptr) {
		ImageBase *img = CreateCachedImage();
		if (!img)
			return nullptr;
		
		img->SelectFrame(m_index);
		m_SmileyIcon = img->GetIcon();
		img->Release();
	}
	return m_SmileyIcon;
}

HICON SmileyType::GetIconDup(void)
{
	ImageBase *img = CreateCachedImage();
	img->SelectFrame(m_index);
	HICON hIcon = img->GetIcon();
	img->Release();
	return hIcon;
}

bool SmileyType::LoadFromImage(IStream *pStream)
{
	if (m_xepimg)
		m_xepimg->Release();

	CMStringW name;
	m_xepimg = new ImageType(0, name, pStream);
	return true;
}

bool SmileyType::LoadFromResource(const CMStringW &file, const int index)
{
	m_index = index;
	m_filepath = file;
	return true;
}

void SmileyType::GetSize(SIZE &size)
{
	if (m_size.cy == 0) {
		ImageBase *img = CreateCachedImage();
		if (img) {
			img->GetSize(m_size);
			img->Release();
		}
	}
	size = m_size;
}

ImageBase* SmileyType::CreateCachedImage(void)
{
	if (m_xepimg) {
		m_xepimg->AddRef();
		return m_xepimg;
	}
	return AddCacheImage(m_filepath, m_index);
}

void SmileyType::SetImList(HIMAGELIST hImLst, long i)
{
	if (m_xepimg) m_xepimg->Release();
	m_xepimg = new ImageListItemType(0, hImLst, i);
}

HBITMAP SmileyType::GetBitmap(COLORREF bkgClr, int sizeX, int sizeY)
{
	ImageBase *img = CreateCachedImage();
	if (!img) return nullptr;
	img->SelectFrame(m_index);
	HBITMAP hBmp = img->GetBitmap(bkgClr, sizeX, sizeY);
	img->Release();

	return hBmp;
}

//
// SmileyPackType
//

SmileyPackType::SmileyPackType()
{
	m_hSmList = nullptr;
	errorFound = false;
}

SmileyType* SmileyPackType::GetSmiley(unsigned index)
{
	return (index < (unsigned)m_SmileyList.getCount()) ? &m_SmileyList[index] : nullptr;
}

SmileyPackType::~SmileyPackType()
{
	if (m_hSmList != nullptr) ImageList_Destroy(m_hSmList);
}

static const wchar_t urlRegEx[] = L"(?:ftp|https|http|file|aim|webcal|irc|msnim|xmpp|gopher|mailto|news|nntp|telnet|wais|prospero)://?[\\w.?%:/$+;]*";
static const wchar_t pathRegEx[] = L"[\\s\"][a-zA-Z]:[\\\\/][\\w.\\-\\\\/]*";
static const wchar_t timeRegEx[] = L"\\d{1,2}:\\d{2}:\\d{2}|\\d{1,2}:\\d{2}";

void SmileyPackType::AddTriggersToSmileyLookup(void)
{
	CMStringW emptystr;
	m_SmileyLookup.insert(new SmileyLookup(urlRegEx, true, -1, emptystr));
	m_SmileyLookup.insert(new SmileyLookup(pathRegEx, true, -1, emptystr));
	m_SmileyLookup.insert(new SmileyLookup(timeRegEx, true, -1, emptystr));

	for (int dist = 0; dist < m_SmileyList.getCount(); dist++) {
		auto &p = m_SmileyList[dist];
		if (p.IsRegEx()) {
			SmileyLookup *dats = new SmileyLookup(p.GetTriggerText(), true, dist, GetFilename());
			if (dats->IsValid())
				m_SmileyLookup.insert(dats);
			else
				errorFound = true;
			if (p.m_InsertText.IsEmpty())
				p.m_InsertText = p.m_ToolText;
		}
		else if (!p.IsService()) {
			bool first = true;
			const CMStringW &text = p.GetTriggerText();
			int iStart = 0;
			while (true) {
				CMStringW wszWord = text.Tokenize(L" \t", iStart);
				if (iStart == -1)
					break;

				ReplaceAllSpecials(wszWord, wszWord);
				SmileyLookup *dats = new SmileyLookup(wszWord, false, dist, GetFilename());
				if (dats->IsValid()) {
					m_SmileyLookup.insert(dats);
					if (first) {
						p.m_InsertText = wszWord;
						first = false;
					}
				}
				else delete dats;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////

static MRegexp16 isCode(L"\\&\\#(\\d*)\\;");

static void replaceCodes(CMStringW &str)
{
	if (isCode.match(str) < 0)
		return;

	str.Delete(isCode.getPos(), isCode.getLength());

	uint32_t iCode = _wtoi(isCode.getGroup(1));
	wchar_t tmp[3] = { 0, 0, 0 };
	if (iCode < 0x10000)
		tmp[0] = LOWORD(iCode), tmp[1] = HIWORD(iCode);
	else {
		iCode -= 0x10000;
		tmp[0] = 0xD800 + (iCode >> 10);
		tmp[1] = 0xDC00 + (iCode & 0x3FF);
	}
	str.Insert(isCode.getPos(), tmp);
}

void SmileyPackType::ReplaceAllSpecials(const CMStringW &Input, CMStringW &Output)
{
	Output = Input;
	Output.Replace(L"%%_%%", L" ");
	Output.Replace(L"%%__%%", L" ");
	Output.Replace(L"%%''%%", L"\"");
	replaceCodes(Output);
}

void SmileyPackType::Clear(void)
{
	m_SmileyList.destroy();
	m_SmileyLookup.destroy();
	if (m_hSmList != nullptr) { ImageList_Destroy(m_hSmList); m_hSmList = nullptr; }
	m_Filename.Empty();
	m_Name.Empty();
	m_Date.Empty();
	m_Version.Empty();
	m_Author.Empty();
	m_VisibleCount = 0;
	m_ButtonSmiley.Empty();
	errorFound = false;
}

bool SmileyPackType::LoadSmileyFile(const CMStringW &filename, const CMStringW &packname, bool onlyInfo, bool noerr)
{
	Clear();

	if (filename.IsEmpty()) {
		m_Name = L"Nothing loaded";
		return false;
	}

	wchar_t wszTmp[MAX_PATH];
	CMStringW modpath = VARSW(filename);
	if (_waccess(modpath, 4) != 0) {
		PathToAbsoluteW(filename, wszTmp, g_plugin.wszDefaultPath);
		if (_waccess(wszTmp, 4) != 0) {
			if (!noerr) {
				static const wchar_t errmsg[] = LPGENW("Smiley pack %s for category \"%s\" not found.\nSelect correct smiley pack in the Options -> Customize -> Smileys.");
				wchar_t msgtxt[1024];
				mir_snwprintf(msgtxt, TranslateW(errmsg), modpath.c_str(), packname.c_str());
				ReportError(msgtxt);
			}

			m_Name = L"Nothing loaded";
			return false;
		}
	}
	else PathToAbsoluteW(modpath, wszTmp, g_plugin.wszDefaultPath);

	modpath = wszTmp;
	m_Filename = filename;

	// Load file
	bool res;
	if (filename.Find(L".xep") == -1)
		res = LoadSmileyFileMSL(modpath, onlyInfo, modpath);
	else
		res = LoadSmileyFileXEP(modpath, onlyInfo);

	if (errorFound)
		ReportError(TranslateT("There were problems loading smiley pack (it should be corrected).\nSee network log for details."));

	return res;
}

static IStream* DecodeBase64Data(const char *pString)
{
	if (pString == nullptr)
		return nullptr;

	size_t dataLen;
	ptrA data((char*)mir_base64_decode(pString, &dataLen));
	if (data == nullptr)
		return nullptr;

	// Read image list
	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, dataLen);
	if (!hBuffer)
		return nullptr;

	void *dst = GlobalLock(hBuffer);
	memcpy(dst, data, dataLen);
	GlobalUnlock(hBuffer);

	IStream *pStream = nullptr;
	CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);
	return pStream;
}

static CMStringW FilterQuotes(const char *pStr)
{
	CMStringW res(pStr);
	int iStart = res.Find('\"', 0);
	if (iStart != -1) {
		int iEnd = res.Find('\"', ++iStart);
		if (iEnd != -1)
			res = res.Mid(iStart, iEnd - iStart);
	}

	return res.Trim();
}

bool SmileyPackType::LoadSmileyFileXEP(const CMStringW &fileName, bool onlyInfo)
{
	FILE *in = _wfopen(fileName, L"rb");
	if (in == nullptr)
		return false;

	TiXmlDocument doc;
	int ret = doc.LoadFile(in);
	fclose(in);
	if (ret != 0)
		return false;

	auto *pSettings = doc.FirstChildElement("settings");
	if (pSettings != nullptr) {
		if (auto *pNode = pSettings->FirstChildElement("DataBaseName"))
			m_Name = CMStringW(Utf2T(pNode->GetText())).Trim();

		if (auto *pNode = pSettings->FirstChildElement("PackageAuthor"))
			m_Author = CMStringW(Utf2T(pNode->GetText())).Trim();
	}

	if (!onlyInfo) {
		auto *pImages = TiXmlConst(&doc)["lists"]["images"].ToElement();
		if (pImages) {
			IStream *pStream = DecodeBase64Data(pImages->GetText());
			if (pStream) {
				if (m_hSmList != nullptr)
					ImageList_Destroy(m_hSmList);
				m_hSmList = ImageList_Read(pStream);
				pStream->Release();
			}
		}

		for (auto *nRec : TiXmlFilter(doc.FirstChildElement("dataroot"), "record")) {
			int idx = nRec->IntAttribute("ImageIndex", -1);
			if (idx == -1)
				continue;

			SmileyType *dat = new SmileyType;
			dat->SetRegEx(true);
			dat->SetImList(m_hSmList, idx);
			dat->m_ToolText = FilterQuotes(nRec->GetText());

			if (auto *pNode = nRec->FirstChildElement("Expression"))
				dat->m_TriggerText = FilterQuotes(pNode->GetText());
			if (auto *pNode = nRec->FirstChildElement("PasteText"))
				dat->m_InsertText = FilterQuotes(pNode->GetText());

			dat->SetHidden(dat->m_InsertText.IsEmpty());

			if (auto *pNode = nRec->FirstChildElement("Image")) {
				IStream *pStream = DecodeBase64Data(pNode->GetText());
				if (pStream) {
					dat->LoadFromImage(pStream);
					pStream->Release();
				}
			}

			m_SmileyList.insert(dat);
		}
	}

	m_VisibleCount = m_SmileyList.getCount();
	AddTriggersToSmileyLookup();

	selec.x = selec.y = win.x = win.y = 0;
	return true;
}

bool SmileyPackType::LoadSmileyFileMSL(const CMStringW &filename, bool onlyInfo, CMStringW &modpath)
{
	int fh = _wopen(filename.c_str(), _O_BINARY | _O_RDONLY);
	if (fh == -1)
		return false;

	// Find file size
	const long flen = _filelength(fh);

	// Allocate file buffer
	char *buf = new char[flen + sizeof(wchar_t)];

	// Read file in
	int len = _read(fh, buf, flen);
	*(wchar_t*)(buf + len) = 0;

	// Close file
	_close(fh);

	CMStringW tbuf;
	if (len > 2 && *(wchar_t*)buf == 0xfeff)
		tbuf = ((wchar_t*)buf + 1);
	else if (len > 3 && buf[0] == '\xef' && buf[1] == '\xbb' && buf[2] == '\xbf')
		tbuf = _A2T(buf + 3, CP_UTF8);
	else
		tbuf = _A2T(buf);

	delete[] buf;

	CMStringW pathstr, packstr;
	{
		MRegexp16 pathsplit(L"(.*\\\\)(.*)\\.|$");
		pathsplit.match(modpath);

		pathstr = pathsplit.getGroup(1);
		packstr = pathsplit.getGroup(2);
	}

	if (!onlyInfo)
		selec.x = selec.y = win.x = win.y = 0;

	int iStart = 0;
	MRegexp16 otherf(L"^\\s*(Name|Author|Date|Version|ButtonSmiley)\\s*=\\s*\"(.*)\"");
	MRegexp16 size(L"^\\s*(Selection|Window)Size\\s*=\\s*(\\d+)\\s*,\\s*(\\d+)");
	MRegexp16 smiley(
		L"^\\s*Smiley(\\*)?\\s*="	// Is Hidden
		L"(?:\\s*\"(.*)\")"			// Smiley file name
		L"(?:[\\s,]+(\\-?\\d+))"		// Icon resource id
		L"(?:[\\s,]+(R|S)?\"(.*?)\")"	// Trigger text
		L"(?:[\\s,]+\"(.*?)\")?"		// Tooltip or insert text
		L"(?:[\\s,]+\"(.*?)\")?");		// Tooltip text

	SmileyVectorType hiddenSmileys;
	unsigned smnum = 0;

	while (true) {
		CMStringW line = tbuf.Tokenize(L"\r\n", iStart);
		if (iStart == -1)
			break;

		if (line.IsEmpty() || line[0] == ';')
			continue;

		if (otherf.match(line) >= 0) {
			CMStringW key(otherf.getGroup(1)), value(otherf.getGroup(2));
			if (key == L"Name")
				m_Name = value;
			else if (key == L"Author")
				m_Author = value;
			else if (key == L"Date")
				m_Date = value;
			else if (key == L"Version")
				m_Version = value;
			else if (key == L"ButtonSmiley")
				m_ButtonSmiley = value;
			continue;
		}

		if (onlyInfo)
			continue;

		if (size.match(line) >= 0) {
			POINT tpt;
			tpt.x = _wtol(size.getGroup(2));
			tpt.y = _wtol(size.getGroup(3));

			if (size.getGroup(1) == L"Selection")
				selec = tpt;
			else if (size.getGroup(1) == L"Window")
				win = tpt;
			continue;
		}

		if (smiley.match(line)) {
			CMStringW resname = smiley.getGroup(2);
			if (resname.Find(L"http://") != -1) {
				if (GetSmileyFile(resname, packstr))
					continue;
			}
			else if (!resname.IsEmpty())
				resname.Insert(0, pathstr);

			SmileyType *dat = new SmileyType;

			const int iconIndex = _wtol(smiley.getGroup(3));

			dat->SetHidden(!smiley.getGroup(1).IsEmpty());

			CMStringW wszGrp4(smiley.getGroup(4));
			if (!wszGrp4.IsEmpty()) {
				dat->SetRegEx(wszGrp4 == L"R");
				dat->SetService(wszGrp4 == L"S");
			}

			dat->m_TriggerText = smiley.getGroup(5);

			CMStringW wszGrp6(smiley.getGroup(6)), wszGrp7(smiley.getGroup(7));
			if (dat->IsRegEx()) {
				if (!wszGrp6.IsEmpty())
					ReplaceAllSpecials(wszGrp6, dat->m_InsertText);

				if (!wszGrp7.IsEmpty())
					ReplaceAllSpecials(wszGrp7, dat->m_ToolText);
				else
					dat->m_ToolText = dat->m_InsertText;
			}
			else {
				if (!wszGrp6.IsEmpty())
					ReplaceAllSpecials(wszGrp6, dat->m_ToolText);
				else
					ReplaceAllSpecials(dat->m_TriggerText, dat->m_ToolText);
			}

			bool noerr;
			if (resname.IsEmpty()) {
				dat->SetHidden(true);
				dat->SetText(true);
				noerr = true;
			}
			else noerr = dat->LoadFromResource(resname, iconIndex);

			if (dat->IsHidden())
				hiddenSmileys.insert(dat);
			else
				m_SmileyList.insert(dat);

			if (!noerr) {
				static const wchar_t errmsg[] = LPGENW("Smiley #%u in file %s for smiley pack %s not found.");
				wchar_t msgtxt[1024];
				mir_snwprintf(msgtxt, TranslateW(errmsg), smnum, resname.c_str(), modpath.c_str());
				Netlib_LogW(hNetlibUser, msgtxt);
				errorFound = true;
			}
			smnum++;
		}
	}

	m_VisibleCount = m_SmileyList.getCount();
	m_SmileyList.splice(hiddenSmileys);
	AddTriggersToSmileyLookup();
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
// SmileyPackListType

bool SmileyPackListType::AddSmileyPack(CMStringW &filename, CMStringW &packname)
{
	bool res = true;
	if (GetSmileyPack(filename) == nullptr) {  //not exist yet, so add
		SmileyPackType *smileyPack = new SmileyPackType;

		res = smileyPack->LoadSmileyFile(filename, packname, FALSE);
		if (res)
			m_SmileyPacks.insert(smileyPack);
		else
			delete smileyPack;
	}
	return res;
}

SmileyPackType* SmileyPackListType::GetSmileyPack(CMStringW &filename)
{
	CMStringW modpath = VARSW(filename);

	for (auto &it : m_SmileyPacks) {
		CMStringW modpath1(VARSW(it->GetFilename()));
		if (mir_wstrcmpi(modpath.c_str(), modpath1.c_str()) == 0)
			return it;
	}
	return nullptr;
}

void SmileyPackListType::ClearAndFreeAll()
{
	m_SmileyPacks.destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////
// SmileyCategoryType

SmileyCategoryType::SmileyCategoryType(SmileyPackListType *pSPS, const CMStringW &name,
	const CMStringW &displayName, const CMStringW &defaultFilename, SmcType typ)
{
	m_pSmileyPackStore = pSPS;
	type = typ;
	m_Name = name;
	m_DisplayName = displayName;
	visible = true;

	opt.ReadPackFileName(m_Filename, m_Name, defaultFilename);
}

void SmileyCategoryType::Load(void)
{
	bool bVisibleCat = opt.UsePhysProto ? !IsAcc() : !IsPhysProto();
	bool bVisible = opt.UseOneForAll ? !IsProto() : bVisibleCat;
	if (bVisible && !m_Filename.IsEmpty()) {
		bool loaded = m_pSmileyPackStore->AddSmileyPack(m_Filename, m_DisplayName);
		if (!loaded) {
			ClearFilename();
			SaveSettings();
		}
	}
}

SmileyPackType* SmileyCategoryType::GetSmileyPack(void)
{
	return m_pSmileyPackStore->GetSmileyPack(m_Filename);
}

void SmileyCategoryType::SaveSettings(void)
{
	opt.WritePackFileName(m_Filename, m_Name);
}

/////////////////////////////////////////////////////////////////////////////////////////
// SmileyCategoryListType

void SmileyCategoryListType::ClearAndLoadAll(void)
{
	m_pSmileyPackStore->ClearAndFreeAll();

	for (auto &it : m_SmileyCategories)
		it->Load();
}

SmileyCategoryType* SmileyCategoryListType::GetSmileyCategory(const CMStringW &name)
{
	for (auto &it : m_SmileyCategories)
		if (name.CompareNoCase(it->GetName()) == 0)
			return it;

	return nullptr;
}

SmileyCategoryType* SmileyCategoryListType::GetSmileyCategory(unsigned index)
{
	return index < (unsigned)m_SmileyCategories.getCount() ? &m_SmileyCategories[index] : nullptr;
}

SmileyPackType* SmileyCategoryListType::GetSmileyPack(const CMStringW &categoryname)
{
	SmileyCategoryType *smc = GetSmileyCategory(categoryname);
	return smc != nullptr ? smc->GetSmileyPack() : nullptr;
}

void SmileyCategoryListType::SaveSettings(void)
{
	CMStringW catstr;
	for (auto &it : m_SmileyCategories) {
		it->SaveSettings();
		if (it->IsCustom()) {
			if (!catstr.IsEmpty())
				catstr += '#';
			catstr += it->GetName();
		}
	}
	opt.WriteCustomCategories(catstr);
}

void SmileyCategoryListType::AddAndLoad(const CMStringW &name, const CMStringW &displayName)
{
	if (GetSmileyCategory(name) != nullptr)
		return;

	AddCategory(name, displayName, smcExt);
	
	// Load only if other smileys have been loaded already
	if (m_SmileyCategories.getCount() > 1)
		m_SmileyCategories[m_SmileyCategories.getCount() - 1].Load();
}

void SmileyCategoryListType::AddCategory(const CMStringW &name, const CMStringW &displayName, SmcType typ, const CMStringW &defaultFilename)
{
	if (GetSmileyCategory(name) == nullptr)
		m_SmileyCategories.insert(new SmileyCategoryType(m_pSmileyPackStore, name, displayName, defaultFilename, typ));
}

bool SmileyCategoryListType::DeleteCustomCategory(int index)
{
	if (index < m_SmileyCategories.getCount()) {
		if (m_SmileyCategories[index].IsCustom()) {
			m_SmileyCategories.remove(index);
			return true;
		}
	}
	return false;
}

void SmileyCategoryListType::AddAccountAsCategory(PROTOACCOUNT *acc, const CMStringW &defaultFile)
{
	if (acc->IsEnabled() && acc->szProtoName && IsSmileyProto(acc->szModuleName)) {
		CMStringW displayName(acc->tszAccountName ? acc->tszAccountName : _A2T(acc->szModuleName));
		CMStringW PhysProtoName, paths;
		DBVARIANT dbv;

		if (db_get_ws(0, acc->szModuleName, "AM_BaseProto", &dbv) == 0) {
			PhysProtoName = L"AllProto";
			PhysProtoName += dbv.pwszVal;
			db_free(&dbv);
		}

		if (!PhysProtoName.IsEmpty())
			paths = g_SmileyCategories.GetSmileyCategory(PhysProtoName) ? g_SmileyCategories.GetSmileyCategory(PhysProtoName)->GetFilename() : L"";

		// assemble default path
		if (paths.IsEmpty()) {
			const char *packnam = acc->szProtoName;
			if (mir_strcmp(packnam, "JABBER") == 0)
				packnam = "JGMail";

			wchar_t path[MAX_PATH];
			mir_snwprintf(path, L"%s\\Smileys\\nova\\%S.msl", g_plugin.wszDefaultPath, packnam);
			if (_waccess(path, 0) != 0)
				paths = defaultFile;
		}

		CMStringW tname(_A2T(acc->szModuleName));
		AddCategory(tname, displayName, acc->bIsVirtual ? smcVirtualProto : smcProto, paths);
	}
}

void SmileyCategoryListType::AddProtoAsCategory(char *acc, const CMStringW &defaultFile)
{
	if (acc == nullptr)
		return;

	const char *packnam = acc;
	if (mir_strcmp(packnam, "JABBER") == 0)
		packnam = "JGMail";

	// assemble default path
	CMStringW paths(FORMAT, L"%s\\Smileys\\nova\\%S.msl", g_plugin.wszDefaultPath, packnam);
	paths = VARSW(paths);
	if (_waccess(paths.c_str(), 0) != 0)
		paths = defaultFile;

	CMStringW dName(acc), displayName;
	displayName.AppendFormat(TranslateT("%s global smiley pack"), dName.GetBuffer());
	CMStringW tname("AllProto");
	tname += _A2T(acc);
	AddCategory(tname, displayName, smcPhysProto, paths);
}

void SmileyCategoryListType::DeleteAccountAsCategory(PROTOACCOUNT *acc)
{
	CMStringW tname(_A2T(acc->szModuleName));

	for (auto &hContact : Contacts()) {
		char *proto = Proto_GetBaseAccountName(hContact);
		if (proto == nullptr)
			continue;

		DBVARIANT dbv;
		if (!db_get_ws(hContact, proto, "Transport", &dbv)) {
			bool found = (tname.CompareNoCase(dbv.pwszVal) == 0);
			db_free(&dbv);
			if (found)
				return;
		}
	}

	for (auto &it : m_SmileyCategories) {
		if (tname.CompareNoCase(it->GetName()) == 0) {
			m_SmileyCategories.removeItem(&it);
			break;
		}
	}
}

void SmileyCategoryListType::AddContactTransportAsCategory(MCONTACT hContact, const CMStringW &defaultFile)
{
	char *proto = Proto_GetBaseAccountName(hContact);
	if (proto == nullptr)
		return;

	DBVARIANT dbv;
	if (!db_get_ws(hContact, proto, "Transport", &dbv)) {
		if (dbv.pwszVal[0] == '\0') {
			db_free(&dbv);
			return;
		}
		char *trsp = mir_strdup(_T2A(dbv.pwszVal));
		_strlwr(trsp);

		const char *packname = nullptr;
		if (strstr(trsp, "icq") != nullptr)
			packname = "icq";

		mir_free(trsp);

		CMStringW paths, displayName(dbv.pwszVal);
		if (packname != nullptr) {
			paths.Format(L"%s\\Smileys\\nova\\%S.msl", g_plugin.wszDefaultPath, packname);
			paths = VARSW(paths);
			if (_waccess(paths.c_str(), 0) != 0)
				paths = defaultFile;
		}
		else paths = defaultFile;

		AddCategory(displayName, displayName, smcTransportProto, defaultFile);

		db_free(&dbv);
	}
}

void SmileyCategoryListType::AddAllProtocolsAsCategory(void)
{
	CMStringW displayName = TranslateT("Standard");
	CMStringW tname = L"Standard";
	AddCategory(tname, displayName, smcStd);

	const CMStringW &defaultFile = GetSmileyCategory(tname)->GetFilename();

	PROTOCOLDESCRIPTOR **proto;
	int protoCount = 0;
	Proto_EnumProtocols(&protoCount, &proto);

	for (int i = 0; i < protoCount; i++) {
		PROTOCOLDESCRIPTOR *pd = proto[i];
		if (pd->type == PROTOTYPE_PROTOWITHACCS)
			AddProtoAsCategory(pd->szName, defaultFile);
	}

	for (auto &pa : Accounts())
		AddAccountAsCategory(pa, defaultFile);

	for (auto &hContact : Contacts())
		AddContactTransportAsCategory(hContact, defaultFile);

	CMStringW cats;
	opt.ReadCustomCategories(cats);

	int cppv = 0;
	for (;;) {
		int cp = cats.Find('#', cppv);
		if (cp == -1)
			break;

		displayName = cats.Mid(cppv, cp - cppv);
		AddCategory(displayName, displayName, smcCustom, defaultFile);
		cppv = cp + 1;
	}

	if (cppv != cats.GetLength()) {
		displayName = cats.Mid(cppv);
		AddCategory(displayName, displayName, smcCustom, defaultFile);
	}
}

static const CMStringW testString(L"Test String");

SmileyLookup::SmileyLookup(const CMStringW &str, const bool regexs, const int ind, const CMStringW &smpt)
{
	m_ind = ind;
	if (regexs) {
		m_pattern.compile(str);
		m_valid = m_pattern.isValid();
		if (!m_valid) {
			wchar_t msgtxt[1024];
			mir_snwprintf(msgtxt, TranslateT("Regular expression \"%s\" in smiley pack \"%s\" malformed."), str.c_str(), smpt.c_str());
			Netlib_LogW(hNetlibUser, msgtxt);
		}
	}
	else {
		m_text = str;
		replaceCodes(m_text);
		m_valid = !str.IsEmpty();
	}
}

SmileyLookup::~SmileyLookup()
{
}

void SmileyLookup::Find(const CMStringW &str, SmileyLocVecType &smlcur, bool firstOnly)
{
	if (!m_valid) return;

	if (m_text.IsEmpty()) {
		while (m_pattern.nextMatch(str) >= 0) {
			CMStringW wszMatch(m_pattern.getMatch());
			smlcur.insert(new SmileyLocType(m_pattern.getPos(), wszMatch.GetLength()));
			if (firstOnly && m_ind != -1)
				return;
		}
	}
	else {
		const wchar_t *pos = str.c_str();
		while ((pos = wcsstr(pos, m_text.c_str())) != nullptr) {
			smlcur.insert(new SmileyLocType(pos - str.c_str(), m_text.GetLength()));
			pos += m_text.GetLength();
			if (firstOnly && m_ind != -1)
				return;
		}
	}
}
