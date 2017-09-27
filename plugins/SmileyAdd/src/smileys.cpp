/*
Miranda NG SmileyAdd Plugin
Copyright (C) 2012 - 2017 Miranda NG project (https://miranda-ng.org)
Copyright (C) 2005 - 2011 Boris Krasnovskiy All Rights Reserved
Copyright (C) 2003 - 2004 Rein-Peter de Boer

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

static HWND hwndHidden = NULL;

static void CALLBACK timerProc(HWND, UINT, UINT_PTR param, DWORD)
{
	SmileyType *pType = (SmileyType*)param;
	pType->MoveToNextFrame();
}

// these two functions must be called from the main thread
static void CALLBACK sttStartTimer(PVOID obj)
{
	if (hwndHidden == NULL)
		hwndHidden = CreateWindowEx(0, L"STATIC", NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL);

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
	m_SmileyIcon = NULL;
	m_xepimg = NULL;
	m_flags = 0;
	m_index = 0;
	m_size.cx = 0;
	m_size.cy = 0;
}

SmileyType::~SmileyType()
{
	if (m_xepimg) {
		m_xepimg->Release();
		m_xepimg = NULL;
	}

	if (m_SmileyIcon != NULL) {
		DestroyIcon(m_SmileyIcon);
		m_SmileyIcon = NULL;
	}
}

void SmileyType::AddObject(ISmileyBase *pObject)
{
	if (m_arSmileys.getCount() == 0) {
		if (m_xepimg == NULL)
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
	int iFrameDelay = (m_xepimg == NULL) ? 0 : m_xepimg->GetFrameDelay();
	if (iFrameDelay <= 0)
		KillTimer(hwndHidden, (DWORD_PTR)this);
	else
		SetTimer(hwndHidden, (DWORD_PTR)this, iFrameDelay*10, timerProc);
}

void SmileyType::MoveToNextFrame()
{
	m_index = m_xepimg->SelectNextFrame(m_index);

	for (int i = 0; i < m_arSmileys.getCount(); i++)
		m_arSmileys[i]->Draw();

	SetFrameDelay(); // reset timer
}

HICON SmileyType::GetIcon(void)
{
	if (m_SmileyIcon == NULL) {
		ImageBase *img = CreateCachedImage();
		if (!img)
			return NULL;
		
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
	if (!img) return NULL;
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
	m_hSmList = NULL;
	errorFound = false;
}

SmileyType* SmileyPackType::GetSmiley(unsigned index)
{
	return (index < (unsigned)m_SmileyList.getCount()) ? &m_SmileyList[index] : NULL;
}

static DWORD_PTR ConvertServiceParam(MCONTACT hContact, const wchar_t *param)
{
	if (param == NULL)
		return 0;
	if (mir_wstrcmpi(L"hContact", param) == 0)
		return hContact;
	if (iswdigit(*param))
		return _wtoi(param);

	return (DWORD_PTR)param;
}

void SmileyType::CallSmileyService(MCONTACT hContact)
{
	_TPattern *srvsplit = _TPattern::compile(L"(.*)\\|(.*)\\|(.*)");
	_TMatcher *m0 = srvsplit->createWCMatcher(GetTriggerText());
	m0->findFirstMatch();

	CMStringW name = m0->getGroup(1);
	CMStringW par1 = m0->getGroup(2);
	CMStringW par2 = m0->getGroup(3);

	delete m0;
	delete srvsplit;

	char str[MAXMODULELABELLENGTH];
	const char *proto = "";

	if (name[0] == '/') {
		proto = (const char*)GetContactProto(hContact);
		if (proto == NULL) return;
	}
	mir_snprintf(str, "%s%s", proto, _T2A(name.c_str()));
	CallService(str,
		ConvertServiceParam(hContact, par1.c_str()),
		ConvertServiceParam(hContact, par2.c_str()));
}



SmileyPackType::~SmileyPackType()
{
	if (m_hSmList != NULL) ImageList_Destroy(m_hSmList);
}

static const wchar_t urlRegEx[] =
L"(?:ftp|https|http|file|aim|webcal|irc|msnim|xmpp|gopher|mailto|news|nntp|telnet|wais|prospero)://?[\\w.?%:/$+;]*";
static const wchar_t pathRegEx[] = L"[\\s\"][a-zA-Z]:[\\\\/][\\w.\\-\\\\/]*";
static const wchar_t timeRegEx[] = L"\\d{1,2}:\\d{2}:\\d{2}|\\d{1,2}:\\d{2}";

void SmileyPackType::AddTriggersToSmileyLookup(void)
{
	_TPattern *p = _TPattern::compile(L"\\s+");
	{
		CMStringW emptystr;
		m_SmileyLookup.insert(new SmileyLookup(urlRegEx, true, -1, emptystr));
		m_SmileyLookup.insert(new SmileyLookup(pathRegEx, true, -1, emptystr));
		m_SmileyLookup.insert(new SmileyLookup(timeRegEx, true, -1, emptystr));
	}

	for (int dist = 0; dist < m_SmileyList.getCount(); dist++) {
		if (m_SmileyList[dist].IsRegEx()) {
			SmileyLookup *dats = new SmileyLookup(m_SmileyList[dist].GetTriggerText(), true, dist, GetFilename());
			if (dats->IsValid())
				m_SmileyLookup.insert(dats);
			else
				errorFound = true;
			if (m_SmileyList[dist].m_InsertText.IsEmpty())
				m_SmileyList[dist].m_InsertText = m_SmileyList[dist].m_ToolText;
		}
		else if (!m_SmileyList[dist].IsService()) {
			bool first = true;
			int li = 0;
			_TMatcher *m0 = p->createWCMatcher(m_SmileyList[dist].GetTriggerText());
			while (m0->findNextMatch()) {
				int stind = m0->getStartingIndex();
				if (li != stind) {
					CMStringW out;
					ReplaceAllSpecials(m0->getString().Mid(li, stind - li), out);
					SmileyLookup *dats = new SmileyLookup(out, false, dist, GetFilename());
					if (dats->IsValid()) {
						m_SmileyLookup.insert(dats);
						if (first) {
							m_SmileyList[dist].m_InsertText = out;
							first = false;
						}
					} // fallthrough  
				}
				li = m0->getEndingIndex();
			}

			int stind = (int)m0->getString().GetLength();
			if (li < stind) {
				CMStringW out;
				ReplaceAllSpecials(m0->getString().Mid(li, stind - li), out);
				SmileyLookup *dats = new SmileyLookup(out, false, dist, GetFilename());
				if (dats->IsValid()) {
					m_SmileyLookup.insert(dats);
					if (first) {
						m_SmileyList[dist].m_InsertText = out;
						first = false;
					}
				}
				else
					delete dats;
			}
			delete m0;
		}
	}
	delete p;
}

void SmileyPackType::ReplaceAllSpecials(const CMStringW &Input, CMStringW &Output)
{
	Output = _TPattern::replace(L"%%_{1,2}%%", Input, L" ");
	Output = _TPattern::replace(L"%%''%%", Output, L"\"");
}

void SmileyPackType::Clear(void)
{
	m_SmileyList.destroy();
	m_SmileyLookup.destroy();
	if (m_hSmList != NULL) { ImageList_Destroy(m_hSmList); m_hSmList = NULL; }
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

	CMStringW modpath;
	pathToAbsolute(filename, modpath);

	// Load xep file
	int fh = _wopen(modpath.c_str(), _O_BINARY | _O_RDONLY);
	if (fh == -1) {
		if (!noerr) {
			static const wchar_t errmsg[] = LPGENW("Smiley pack %s for category \"%s\" not found.\nSelect correct smiley pack in the Options -> Customize -> Smileys.");
			wchar_t msgtxt[1024];
			mir_snwprintf(msgtxt, TranslateW(errmsg), modpath.c_str(), packname.c_str());
			ReportError(msgtxt);
		}

		m_Name = L"Nothing loaded";
		return false;
	}

	m_Filename = filename;

	// Find file size
	const long flen = _filelength(fh);

	// Allocate file buffer
	char *buf = new char[flen + sizeof(wchar_t)];

	// Read xep file in
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

	bool res;
	if (filename.Find(L".xep") == -1)
		res = LoadSmileyFileMSL(tbuf, onlyInfo, modpath);
	else
		res = LoadSmileyFileXEP(tbuf, onlyInfo, modpath);

	if (errorFound)
		ReportError(TranslateT("There were problems loading smiley pack (it should be corrected).\nSee network log for details."));

	return res;
}

bool SmileyPackType::LoadSmileyFileMSL(CMStringW &tbuf, bool onlyInfo, CMStringW &modpath)
{
	CMStringW pathstr, packstr;
	{
		_TPattern *pathsplit = _TPattern::compile(L"(.*\\\\)(.*)\\.|$");
		_TMatcher *m0 = pathsplit->createWCMatcher(modpath);
		m0->findFirstMatch();
		pathstr = m0->getGroup(1);
		packstr = m0->getGroup(2);
		delete m0;
		delete pathsplit;
	}
	{
		_TPattern *otherf = _TPattern::compile(
			L"^\\s*(Name|Author|Date|Version|ButtonSmiley)\\s*=\\s*\"(.*)\"",
			_TPattern::MULTILINE_MATCHING);

		_TMatcher *m0 = otherf->createWCMatcher(tbuf);
		while (m0->findNextMatch()) {
			if (m0->getGroup(1) == L"Name") m_Name = m0->getGroup(2);
			if (m0->getGroup(1) == L"Author") m_Author = m0->getGroup(2);
			if (m0->getGroup(1) == L"Date") m_Date = m0->getGroup(2);
			if (m0->getGroup(1) == L"Version") m_Version = m0->getGroup(2);
			if (m0->getGroup(1) == L"ButtonSmiley") m_ButtonSmiley = m0->getGroup(2);
		}
		delete m0;
		delete otherf;
	}

	if (!onlyInfo) {
		selec.x = selec.y = win.x = win.y = 0;
		{
			_TPattern *pat = _TPattern::compile(
				L"^\\s*(Selection|Window)Size\\s*=\\s*(\\d+)\\s*,\\s*(\\d+)",
				_TPattern::MULTILINE_MATCHING);
			_TMatcher *m0 = pat->createWCMatcher(tbuf);
			while (m0->findNextMatch()) {
				POINT tpt;
				tpt.x = _wtol(m0->getGroup(2).c_str());
				tpt.y = _wtol(m0->getGroup(3).c_str());

				if (m0->getGroup(1) == L"Selection")
					selec = tpt;
				else if (m0->getGroup(1) == L"Window")
					win = tpt;
			}
			delete m0;
			delete pat;
		}

		_TPattern *smiley = _TPattern::compile(
			L"^\\s*Smiley(\\*)?\\s*="	// Is Hidden
			L"(?:\\s*\"(.*)\")"			// Smiley file name
			L"(?:[\\s,]+(\\-?\\d+))"		// Icon resource id
			L"(?:[\\s,]+(R|S)?\"(.*?)\")"	// Trigger text
			L"(?:[\\s,]+\"(.*?)\")?"		// Tooltip or insert text
			L"(?:[\\s,]+\"(.*?)\")?",		// Tooltip text
			_TPattern::MULTILINE_MATCHING);

		SmileyVectorType hiddenSmileys;
		unsigned smnum = 0;
		{
			_TMatcher *m0 = smiley->createWCMatcher(tbuf);
			while (m0->findNextMatch()) {
				CMStringW resname = m0->getGroup(2);
				if (resname.Find(L"http://") != -1) {
					if (GetSmileyFile(resname, packstr))
						continue;
				}
				else if (!resname.IsEmpty())
					resname.Insert(0, pathstr);

				SmileyType *dat = new SmileyType;

				const int iconIndex = _wtol(m0->getGroup(3).c_str());

				dat->SetHidden(m0->getStartingIndex(1) >= 0);
				if (m0->getStartingIndex(4) >= 0) {
					dat->SetRegEx(m0->getGroup(4) == L"R");
					dat->SetService(m0->getGroup(4) == L"S");
				}
				dat->m_TriggerText = m0->getGroup(5);
				if (dat->IsRegEx()) {
					if (m0->getStartingIndex(6) >= 0)
						ReplaceAllSpecials(m0->getGroup(6), dat->m_InsertText);

					if (m0->getStartingIndex(7) >= 0)
						ReplaceAllSpecials(m0->getGroup(7), dat->m_ToolText);
					else
						dat->m_ToolText = dat->m_InsertText;
				}
				else {
					if (m0->getStartingIndex(6) >= 0)
						ReplaceAllSpecials(m0->getGroup(6), dat->m_ToolText);
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
			delete smiley;
			delete m0;
		}

		m_VisibleCount = m_SmileyList.getCount();

		m_SmileyList.splice(hiddenSmileys);

		AddTriggersToSmileyLookup();
	}

	return true;
}


static void DecodeHTML(CMStringW &str)
{
	if (str.Find('&') != -1) {
		str = _TPattern::replace(CMStringW(L"&lt;"), str, CMStringW(L"<"));
		str = _TPattern::replace(CMStringW(L"&gt;"), str, CMStringW(L">"));
	}
}


static IStream* DecodeBase64Data(const char *pData)
{
	unsigned dataLen;
	ptrA data((char*)mir_base64_decode(pData, &dataLen));
	if (data == NULL)
		return NULL;

	// Read image list
	HGLOBAL hBuffer = GlobalAlloc(GMEM_MOVEABLE, dataLen);
	if (!hBuffer)
		return NULL;

	void *dst = GlobalLock(hBuffer);
	memcpy(dst, data, dataLen);
	GlobalUnlock(hBuffer);

	IStream *pStream = NULL;
	CreateStreamOnHGlobal(hBuffer, TRUE, &pStream);
	return pStream;
}


bool SmileyPackType::LoadSmileyFileXEP(CMStringW &tbuf, bool onlyInfo, CMStringW&)
{
	_TMatcher *m0, *m1, *m2;

	_TPattern *dbname_re = _TPattern::compile(L"<DataBaseName>\\s*\"(.*?)\"\\s*</DataBaseName>",
		_TPattern::MULTILINE_MATCHING);
	_TPattern *author_re = _TPattern::compile(L"<PackageAuthor>\\s*\"(.*?)\"\\s*</PackageAuthor>",
		_TPattern::MULTILINE_MATCHING);
	_TPattern *settings_re = _TPattern::compile(L"<settings>(.*?)</settings>",
		_TPattern::MULTILINE_MATCHING | _TPattern::DOT_MATCHES_ALL);

	m0 = settings_re->createWCMatcher(tbuf);
	if (m0->findFirstMatch()) {
		CMStringW settings = m0->getGroup(1);

		m1 = author_re->createWCMatcher(settings);
		if (m1->findFirstMatch()) {
			m_Author = m1->getGroup(1);
			DecodeHTML(m_Author);
		}
		delete m1;

		m1 = dbname_re->createWCMatcher(settings);
		if (m1->findFirstMatch()) {
			m_Name = m1->getGroup(1);
			DecodeHTML(m_Name);
		}
		delete m1;
	}
	delete m0;

	delete dbname_re;
	delete author_re;
	delete settings_re;

	if (!onlyInfo) {
		_TPattern *record_re = _TPattern::compile(L"<record.*?ImageIndex=\"(.*?)\".*?>(?:\\s*\"(.*?)\"?(.*?)</record>)",
			_TPattern::MULTILINE_MATCHING | _TPattern::DOT_MATCHES_ALL);
		_TPattern *expression_re = _TPattern::compile(L"<Expression>\\s*\"(.*?)\"\\s*</Expression>",
			_TPattern::MULTILINE_MATCHING);
		_TPattern *pastetext_re = _TPattern::compile(L"<PasteText>\\s*\"(.*?)\"\\s*</PasteText>",
			_TPattern::MULTILINE_MATCHING);
		_TPattern *images_re = _TPattern::compile(L"<images>(.*?)</images>",
			_TPattern::MULTILINE_MATCHING | _TPattern::DOT_MATCHES_ALL);
		_TPattern *image_re = _TPattern::compile(L"<Image>(.*?)</Image>",
			_TPattern::MULTILINE_MATCHING | _TPattern::DOT_MATCHES_ALL);
		_TPattern *imagedt_re = _TPattern::compile(L"<!\\[CDATA\\[(.*?)\\]\\]>",
			_TPattern::MULTILINE_MATCHING);

		m0 = images_re->createWCMatcher(tbuf);
		if (m0->findFirstMatch()) {
			CMStringW images = m0->getGroup(1);

			m1 = imagedt_re->createWCMatcher(images);
			if (m1->findFirstMatch()) {
				IStream *pStream = DecodeBase64Data(_T2A(m1->getGroup(1).c_str()));
				if (pStream != NULL) {
					if (m_hSmList != NULL) ImageList_Destroy(m_hSmList);
					m_hSmList = ImageList_Read(pStream);
					pStream->Release();
				}
			}
			delete m1;
		}
		delete m0;

		m0 = record_re->createWCMatcher(tbuf);
		while (m0->findNextMatch()) {
			SmileyType *dat = new SmileyType;

			dat->SetRegEx(true);
			dat->SetImList(m_hSmList, _wtol(m0->getGroup(1).c_str()));
			dat->m_ToolText = m0->getGroup(2);
			DecodeHTML(dat->m_ToolText);

			CMStringW rec = m0->getGroup(3);

			m1 = expression_re->createWCMatcher(rec);
			if (m1->findFirstMatch()) {
				dat->m_TriggerText = m1->getGroup(1);
				DecodeHTML(dat->m_TriggerText);
			}
			delete m1;

			m1 = pastetext_re->createWCMatcher(rec);
			if (m1->findFirstMatch()) {
				dat->m_InsertText = m1->getGroup(1);
				DecodeHTML(dat->m_InsertText);
			}
			delete m1;
			dat->SetHidden(dat->m_InsertText.IsEmpty());

			m1 = image_re->createWCMatcher(rec);
			if (m1->findFirstMatch()) {
				CMStringW images = m1->getGroup(1);

				m2 = imagedt_re->createWCMatcher(images);
				if (m2->findFirstMatch()) {
					IStream *pStream = DecodeBase64Data(_T2A(m2->getGroup(1).c_str()));
					if (pStream != NULL) {
						dat->LoadFromImage(pStream);
						pStream->Release();
					}
				}
				delete m2;
			}
			delete m1;

			m_SmileyList.insert(dat);
		}
		delete m0;

		delete record_re;
		delete expression_re;
		delete pastetext_re;
		delete images_re;
		delete image_re;
		delete imagedt_re;
	}

	m_VisibleCount = m_SmileyList.getCount();

	AddTriggersToSmileyLookup();

	selec.x = 0;
	selec.y = 0;
	win.x = 0;
	win.y = 0;

	return true;
}


//
// SmileyPackListType
//


bool SmileyPackListType::AddSmileyPack(CMStringW &filename, CMStringW &packname)
{
	bool res = true;
	if (GetSmileyPack(filename) == NULL) {  //not exist yet, so add
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
	CMStringW modpath;
	pathToAbsolute(filename, modpath);

	for (int i = 0; i < m_SmileyPacks.getCount(); i++) {
		CMStringW modpath1;
		pathToAbsolute(m_SmileyPacks[i].GetFilename(), modpath1);
		if (mir_wstrcmpi(modpath.c_str(), modpath1.c_str()) == 0) return &m_SmileyPacks[i];
	}
	return NULL;
}

void SmileyPackListType::ClearAndFreeAll()
{
	m_SmileyPacks.destroy();
}


//
// SmileyCategoryType
//


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

//
// SmileyCategoryListType
//

void SmileyCategoryListType::ClearAndLoadAll(void)
{
	m_pSmileyPackStore->ClearAndFreeAll();

	for (int i = 0; i < m_SmileyCategories.getCount(); i++)
		m_SmileyCategories[i].Load();
}


SmileyCategoryType* SmileyCategoryListType::GetSmileyCategory(const CMStringW &name)
{
	for (int i = 0; i < m_SmileyCategories.getCount(); i++)
		if (name.CompareNoCase(m_SmileyCategories[i].GetName()) == 0)
			return &m_SmileyCategories[i];

	return NULL;
}


SmileyCategoryType* SmileyCategoryListType::GetSmileyCategory(unsigned index)
{
	return index < (unsigned)m_SmileyCategories.getCount() ? &m_SmileyCategories[index] : NULL;
}


SmileyPackType* SmileyCategoryListType::GetSmileyPack(CMStringW &categoryname)
{
	SmileyCategoryType *smc = GetSmileyCategory(categoryname);
	return smc != NULL ? smc->GetSmileyPack() : NULL;
}


void SmileyCategoryListType::SaveSettings(void)
{
	CMStringW catstr;
	for (int i = 0; i < m_SmileyCategories.getCount(); i++) {
		m_SmileyCategories[i].SaveSettings();
		if (m_SmileyCategories[i].IsCustom()) {
			if (!catstr.IsEmpty()) catstr += '#';
			catstr += m_SmileyCategories[i].GetName();
		}
	}
	opt.WriteCustomCategories(catstr);
}


void SmileyCategoryListType::AddAndLoad(const CMStringW &name, const CMStringW &displayName)
{
	if (GetSmileyCategory(name) != NULL)
		return;

	AddCategory(name, displayName, smcExt);
	// Load only if other smileys have been loaded already
	if (m_SmileyCategories.getCount() > 1)
		m_SmileyCategories[m_SmileyCategories.getCount() - 1].Load();
}


void SmileyCategoryListType::AddCategory(const CMStringW &name, const CMStringW &displayName, SmcType typ, const CMStringW &defaultFilename)
{
	if (GetSmileyCategory(name) == NULL)
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
	if (Proto_IsAccountEnabled(acc) && acc->szProtoName && IsSmileyProto(acc->szModuleName)) {
		CMStringW displayName(acc->tszAccountName ? acc->tszAccountName : _A2T(acc->szModuleName));
		CMStringW PhysProtoName, paths;
		DBVARIANT dbv;

		if (db_get_ws(NULL, acc->szModuleName, "AM_BaseProto", &dbv) == 0) {
			PhysProtoName = L"AllProto";
			PhysProtoName += dbv.ptszVal;
			db_free(&dbv);
		}

		if (!PhysProtoName.IsEmpty())
			paths = g_SmileyCategories.GetSmileyCategory(PhysProtoName) ? g_SmileyCategories.GetSmileyCategory(PhysProtoName)->GetFilename() : L"";

		if (paths.IsEmpty()) {
			const char *packnam = acc->szProtoName;
			if (mir_strcmp(packnam, "JABBER") == 0)
				packnam = "JGMail";
			else if (strstr(packnam, "SIP") != NULL)
				packnam = "MSN";

			char path[MAX_PATH];
			mir_snprintf(path, "Smileys\\nova\\%s.msl", packnam);

			paths = _A2T(path);
			CMStringW patha;
			pathToAbsolute(paths, patha);

			if (_waccess(patha.c_str(), 0) != 0)
				paths = defaultFile;
		}

		CMStringW tname(_A2T(acc->szModuleName));
		AddCategory(tname, displayName, acc->bIsVirtual ? smcVirtualProto : smcProto, paths);
	}
}

void SmileyCategoryListType::AddProtoAsCategory(char *acc, const CMStringW &defaultFile)
{
	if (acc == NULL)
		return;

	const char *packnam = acc;
	if (mir_strcmp(packnam, "JABBER") == 0)
		packnam = "JGMail";
	else if (strstr(packnam, "SIP") != NULL)
		packnam = "MSN";

	char path[MAX_PATH];
	mir_snprintf(path, "Smileys\\nova\\%s.msl", packnam);

	CMStringW paths = _A2T(path), patha;
	pathToAbsolute(paths, patha);

	if (_waccess(patha.c_str(), 0) != 0)
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

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *proto = GetContactProto(hContact);
		if (proto == NULL)
			continue;

		DBVARIANT dbv;
		if (!db_get_ws(hContact, proto, "Transport", &dbv)) {
			bool found = (tname.CompareNoCase(dbv.ptszVal) == 0);
			db_free(&dbv);
			if (found)
				return;
		}
	}

	for (int i = 0; i < m_SmileyCategories.getCount(); i++) {
		if (tname.CompareNoCase(m_SmileyCategories[i].GetName()) == 0) {
			m_SmileyCategories.remove(i);
			break;
		}
	}
}

void SmileyCategoryListType::AddContactTransportAsCategory(MCONTACT hContact, const CMStringW &defaultFile)
{
	char *proto = GetContactProto(hContact);
	if (proto == NULL)
		return;

	DBVARIANT dbv;
	if (!db_get_ws(hContact, proto, "Transport", &dbv)) {
		if (dbv.ptszVal[0] == '\0') {
			db_free(&dbv);
			return;
		}
		char *trsp = mir_strdup(_T2A(dbv.ptszVal));
		_strlwr(trsp);

		const char *packname = NULL;
		if (strstr(trsp, "msn") != NULL)
			packname = "msn";
		else if (strstr(trsp, "icq") != NULL)
			packname = "icq";
		else if (strstr(trsp, "yahoo") != NULL)
			packname = "yahoo";
		else if (strstr(trsp, "aim") != NULL)
			packname = "aim";
		else if (strstr(trsp, "lcs") != NULL)
			packname = "msn";

		mir_free(trsp);

		CMStringW displayName = dbv.ptszVal;
		if (packname != NULL) {
			char path[MAX_PATH];
			mir_snprintf(path, "Smileys\\nova\\%s.msl", packname);

			CMStringW paths = _A2T(path), patha;
			pathToAbsolute(paths, patha);

			if (_waccess(patha.c_str(), 0) != 0)
				paths = defaultFile;

			AddCategory(displayName, displayName, smcTransportProto, paths);
		}
		else AddCategory(displayName, displayName, smcTransportProto, defaultFile);

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
		if (pd->type == PROTOTYPE_PROTOCOL && pd->cbSize == sizeof(*pd))
			AddProtoAsCategory(pd->szName, defaultFile);
	}

	PROTOACCOUNT **accList;
	Proto_EnumAccounts(&protoCount, &accList);
	for (int i = 0; i < protoCount; i++)
		AddAccountAsCategory(accList[i], defaultFile);

	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
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
	wchar_t msgtxt[1024];

	m_ind = ind;
	if (regexs) {
		m_pattern = _TPattern::compile(str);
		m_valid = m_pattern != NULL;
		if (m_valid) {
			_TMatcher *matcher = m_pattern->createWCMatcher(testString);
			m_valid &= (!matcher->findFirstMatch() ||
				matcher->getStartingIndex() != matcher->getEndingIndex());
			if (!m_valid) {
				static const wchar_t errmsg[] = LPGENW("Regular expression \"%s\" in smiley pack \"%s\" could produce \"empty matches\".");
				mir_snwprintf(msgtxt, TranslateW(errmsg), str.c_str(), smpt.c_str());
			}
			delete matcher;
		}
		else {
			static const wchar_t errmsg[] = LPGENW("Regular expression \"%s\" in smiley pack \"%s\" malformed.");
			mir_snwprintf(msgtxt, TranslateW(errmsg), str.c_str(), smpt.c_str());
		}

		if (!m_valid)
			Netlib_LogW(hNetlibUser, msgtxt);
	}
	else {
		m_text = str;
		m_pattern = NULL;
		m_valid = !str.IsEmpty();
	}
}


SmileyLookup::~SmileyLookup()
{
	delete m_pattern;
}


void SmileyLookup::Find(const CMStringW &str, SmileyLocVecType &smlcur, bool firstOnly) const
{
	if (!m_valid) return;

	if (m_text.IsEmpty()) {
		_TMatcher *matcher = m_pattern->createWCMatcher(str);
		while (matcher->findNextMatch()) {
			int st = matcher->getStartingIndex();
			int sz = matcher->getEndingIndex() - st;
			if (sz != 0) {
				smlcur.insert(new SmileyLocType(st, sz));
				if (firstOnly && m_ind != -1)
					return;
			}
		}
		delete matcher;
	}
	else {
		const wchar_t *pos = str.c_str();
		while ((pos = wcsstr(pos, m_text.c_str())) != NULL) {
			smlcur.insert(new SmileyLocType(pos - str.c_str(), m_text.GetLength()));
			pos += m_text.GetLength();
			if (firstOnly && m_ind != -1)
				return;
		}
	}
}
