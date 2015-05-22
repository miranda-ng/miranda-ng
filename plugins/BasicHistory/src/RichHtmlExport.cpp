/*
Basic History plugin
Copyright (C) 2011-2012 Krzysztof Kral

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "StdAfx.h"
#include "RichHtmlExport.h"
#include "Options.h"
#include "resource.h"
#define EXP_FILE (*stream)

RichHtmlExport::~RichHtmlExport()
{
}

extern HINSTANCE hInst;
extern bool g_SmileyAddAvail;

std::wstring MakeTextHtmled(const std::wstring& message, std::queue<std::pair<size_t, size_t> >* positionMap = NULL)
{
	std::wstring ret;
	std::wstring search = _T("&<>\t\r\n");
	size_t start = 0;
	size_t find;
	size_t currentAdd = 0;
	while((find = message.find_first_of(search, start)) < message.length()) {
		ret += message.substr(start, find - start);
		switch(message[find]) {
		case _T('&'):
			ret += _T("&amp;");
			break;
		case _T('<'):
			ret += _T("&lt;");
			break;
		case _T('>'):
			ret += _T("&gt;");
			break;
		case _T('\t'):
			ret += _T(" ");
			break;
		case _T('\n'):
			ret += _T("<br>");
			break;
		}

		start = find + 1;
		if (positionMap != NULL) {
			size_t len = ret.length() - start - currentAdd;
			if (len != 0) {
				positionMap->push(std::pair<size_t, size_t>(start + currentAdd, len));
				currentAdd += len;
			}
		}
	}
	
	ret += message.substr(start, message.length() - start);
	return ret;
}

std::wstring UrlHighlightHtml(const std::wstring& message, bool& isUrl)
{
	std::wstring ret;
	std::wstring htmlStop = _T("\'\" []<>\r\n");
	std::wstring search = _T("://");
	size_t start = 0;
	size_t find;
	while((find = message.find(search, start)) < message.length()) {
		size_t urlStart = message.find_last_of(htmlStop, find);
		size_t urlEnd = message.find_first_of(htmlStop, find + 3);
		if (urlStart >= message.length())
			urlStart = -1;
		if (urlEnd >= message.length())
			urlEnd = message.length();
		if (((int)urlEnd -3 - (int)find > 0) && ((int)find - (int)urlStart -1 > 0)) {
			ret += message.substr(start, (urlStart + 1) - start);
			std::wstring url = message.substr(urlStart + 1, urlEnd - urlStart - 1);
			start = urlEnd;
			ret += _T("<a class=url target=_blank href=\"");
			ret += url + _T("\">") + url + _T("</a>");
			isUrl = true;
		}
		else {
			ret += message.substr(start, (find + 3) - start);
			start = find + 3;
		}
	}

	ret += message.substr(start, message.length() - start);
	return ret;
}

std::wstring RemoveExt(const std::wstring &fileName)
{
	size_t find = fileName.find_last_of(L'.');
	if (find < fileName.length())
		return fileName.substr(0, find);

	return fileName;
}

std::wstring GetName(const std::wstring &path)
{
	size_t find = path.find_last_of(L"\\/");
	if (find < path.length())
		return path.substr(find + 1);

	return path;
}

void ExtractFile(short int res, const std::wstring &fileName)
{
	HRSRC rSrc = FindResource(hInst, MAKEINTRESOURCE(res), MAKEINTRESOURCE(CUSTOMRES));
	if (rSrc != NULL) {
		HGLOBAL res = LoadResource(hInst, rSrc);
		int size = SizeofResource(hInst, rSrc);
		if (res != NULL) {
			char* resData = (char*)LockResource(res);
			if (resData != NULL) {
				std::ofstream stream (fileName.c_str(), std::ios_base::binary);
				if (stream.is_open()) {
					stream.write(resData, size);
					stream.close();
				}
			}

			FreeResource(res);
		}
	}
}

#pragma pack(push, 2)
typedef struct
{
    BYTE        bWidth;          // Width, in pixels, of the image
    BYTE        bHeight;         // Height, in pixels, of the image
    BYTE        bColorCount;     // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;       // Reserved ( must be 0)
    WORD        wPlanes;         // Color Planes
    WORD        wBitCount;       // Bits per pixel
    DWORD       dwBytesInRes;    // How many bytes in this resource?
    DWORD       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
    WORD           idReserved;   // Reserved (must be 0)
    WORD           idType;       // Resource Type (1 for icons)
    WORD           idCount;      // How many images?
    //ICONDIRENTRY   idEntries; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

#pragma pack(pop)

typedef struct tagMyBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[256];
} MYBITMAPINFO;

void IcoSave(const std::wstring &fileName, HICON hicon)
{
	std::ofstream store (fileName.c_str(), std::ios_base::binary);
	if (!store.is_open())
		return;

	ICONINFO	ii;
	if ( !GetIconInfo(hicon, &ii)) {
		store.close();
		return;
	}

	HBITMAP hbmMask = ii.hbmMask;
	HBITMAP hbmColor = ii.hbmColor;
	BITMAP  bmiMask;
	BITMAP  bmiColor;
	if (GetObject(hbmColor,sizeof(bmiColor),&bmiColor) &&
		GetObject(hbmMask,sizeof(bmiMask),&bmiMask) &&
		(bmiColor.bmWidth==bmiMask.bmWidth) &&
		(bmiColor.bmHeight==bmiMask.bmHeight) &&
		(bmiMask.bmHeight) > 0 &&
		(bmiMask.bmWidth) > 0)
	{
		BITMAPINFOHEADER  icobmi = {0};
		MYBITMAPINFO info1 = {0};
		MYBITMAPINFO info2 = {0};
		
		HDC hDC = CreateCompatibleDC(NULL);
		info1.bmiHeader.biSize = sizeof(info1.bmiHeader);
		info1.bmiHeader.biWidth     = bmiColor.bmWidth;
		info1.bmiHeader.biHeight    = bmiColor.bmHeight;
		info1.bmiHeader.biPlanes    = 1;
		info1.bmiHeader.biBitCount  = bmiColor.bmBitsPixel;
		unsigned int size = GetDIBits(hDC,hbmColor,0,info1.bmiHeader.biHeight,NULL,(BITMAPINFO*)&info1,DIB_RGB_COLORS);
		char* bits1 = new char[info1.bmiHeader.biSizeImage];
		size = GetDIBits(hDC,hbmColor,0,info1.bmiHeader.biHeight,bits1,(BITMAPINFO*)&info1,DIB_RGB_COLORS);
		info2.bmiHeader.biSize = sizeof(info2.bmiHeader);
		info2.bmiHeader.biWidth     = bmiMask.bmWidth;
		info2.bmiHeader.biHeight    = bmiMask.bmHeight;
		info2.bmiHeader.biPlanes    = 1;
		info2.bmiHeader.biBitCount  = bmiMask.bmBitsPixel;
		size = GetDIBits(hDC,hbmColor,0,info1.bmiHeader.biHeight,NULL,(BITMAPINFO*)&info2,DIB_RGB_COLORS);
		char* bits2 = new char[info2.bmiHeader.biSizeImage];
		size = GetDIBits(hDC,hbmMask,0,info2.bmiHeader.biHeight,bits2,(BITMAPINFO*)&info2,DIB_RGB_COLORS);

		ICONDIR            icodir;
		ICONDIRENTRY      icoent;
		icodir.idReserved = 0;
		icodir.idType     = 1;
		icodir.idCount    = 1;
 
		icoent.bWidth        = (unsigned char)bmiColor.bmWidth;
		icoent.bHeight       = (unsigned char)bmiColor.bmHeight;
		icoent.bColorCount   = 8<=bmiColor.bmBitsPixel?0:1<<bmiColor.bmBitsPixel;
		icoent.bReserved     = 0;
		icoent.wPlanes       = bmiColor.bmPlanes;
		icoent.wBitCount     = bmiColor.bmBitsPixel;
		icoent.dwBytesInRes  = sizeof(BITMAPINFOHEADER) + info1.bmiHeader.biSizeImage + info2.bmiHeader.biSizeImage;

		icoent.dwImageOffset = sizeof(icodir) + sizeof(icoent);
 
		store.write((char*)&icodir,sizeof(icodir));
		store.write((char*)&icoent,sizeof(icoent));
 
		icobmi.biSize      = sizeof(icobmi);
		icobmi.biWidth     = bmiColor.bmWidth;
		icobmi.biHeight    = bmiColor.bmHeight + bmiMask.bmHeight;
		icobmi.biPlanes    = info1.bmiHeader.biPlanes;
		icobmi.biBitCount  = bmiColor.bmBitsPixel;
		icobmi.biSizeImage = 0;
 
		store.write((char*)&icobmi,sizeof(icobmi));

		store.write(bits1, info1.bmiHeader.biSizeImage);
		store.write(bits2, info2.bmiHeader.biSizeImage);
		DeleteDC(hDC);
		delete [] bits1;
		delete [] bits2;
	}
	
	store.close();
	if (ii.hbmColor) DeleteObject(ii.hbmColor);
	if (ii.hbmMask ) DeleteObject(ii.hbmMask );
}

bool DeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin = true)
{
  size_t len = mir_tstrlen(lpszDir);
  TCHAR *pszFrom = new TCHAR[len+2];
  _tcscpy_s(pszFrom, len+2, lpszDir);
  pszFrom[len] = 0;
  pszFrom[len+1] = 0;
  
  SHFILEOPSTRUCT fileop;
  fileop.hwnd   = NULL;    // no status display
  fileop.wFunc  = FO_DELETE;  // delete operation
  fileop.pFrom  = pszFrom;  // source file name as double null terminated string
  fileop.pTo    = NULL;    // no destination needed
  fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT;  // do not prompt the user
  
  if (!noRecycleBin)
    fileop.fFlags |= FOF_ALLOWUNDO;

  fileop.fAnyOperationsAborted = FALSE;
  fileop.lpszProgressTitle     = NULL;
  fileop.hNameMappings         = NULL;

  int ret = SHFileOperation(&fileop);
  delete [] pszFrom;  
  return (ret == 0);
}

void RichHtmlExport::WriteHeader(const std::wstring &fileName, const std::wstring &filterName, const std::wstring &myName, const std::wstring &myId, const std::wstring &name1, const std::wstring &proto1, const std::wstring &id1, const std::string& baseProto1, const std::wstring& encoding)
{
	baseProto = baseProto1;
	folder = RemoveExt(fileName) + TranslateT("_files");
	folderName = GetName(folder);
	DeleteDirectory(folder.c_str());
	CreateDirectory(folder.c_str(), NULL);
	std::wstring css =  folder + _T("\\history.css");
	BOOL cssCopied = FALSE;
	if (!Options::instance->extCssHtml2.empty())
		cssCopied = CopyFile(Options::instance->extCssHtml2.c_str(), css.c_str(), FALSE);

	if (!cssCopied)
		ExtractFile(IDR_CSS, css);
	ExtractFile(IDR_JS, folder + _T("\\history.js"));

	HICON ico = LoadIconEx(IDI_PLUSEX);
	IcoSave(folder + _T("\\pnode.ico"), ico);
	Skin_ReleaseIcon(ico);

	ico = LoadIconEx(IDI_MINUSEX);
	IcoSave(folder + _T("\\mnode.ico"), ico);
	Skin_ReleaseIcon(ico);

	ico = LoadIconEx(IDI_INM);
	IcoSave(folder + _T("\\event0.ico"), ico);
	Skin_ReleaseIcon(ico);

	ico = LoadIconEx(IDI_OUTM);
	IcoSave(folder + _T("\\event1.ico"), ico);
	Skin_ReleaseIcon(ico);

	EXP_FILE << _T("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n");
	EXP_FILE << _T("<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=") << encoding << _T("\">\n");
	EXP_FILE << _T("<title>") << TranslateT("History Log") << _T(" [") << MakeTextHtmled(myName) << _T("] - [") << MakeTextHtmled(name1) << _T("]</title>\n");
	EXP_FILE << _T("<link rel=\"Stylesheet\" href=\"") << folderName << _T("\\history.css\" type=\"text/css\">\n");
	EXP_FILE << _T("<script type=\"text/javascript\" src=\"") << folderName << _T("\\history.js\"></script>\n");
	EXP_FILE << _T("</head><body>\n");

	EXP_FILE << _T("<span id=\"menubar\">\n");
	EXP_FILE << _T("<a class=mainmenu onmouseover='this.className=\"mainmenusel\";' href=\"javascript:void(0)\" onClick=\"ShowMenu(1)\" onMouseOut='HideMenu();this.className=\"mainmenu\";'>") << TranslateT("Menu") << _T("</a></span>\n");
	EXP_FILE << _T("<span class=floatmenu id=L1 onmouseover=clearTimeout(timer) onmouseout=HideMenu()>\n");
	EXP_FILE << _T("<table><tr>\n");
	EXP_FILE << _T("<td class=menuitemunsel onmouseover='this.className=\"menuitemsel\"' onmouseout='this.className=\"menuitemunsel\"'>\n");
	EXP_FILE << _T("<a class=menuitem onmouseover=ShowMenu(1) href=\"javascript:void(0)\" onclick=OpenAll(1)>") << TranslateT("Open all") << _T("</a>\n");
	EXP_FILE << _T("</td></tr><tr>\n");
	EXP_FILE << _T("<td class=menuitemunsel onmouseover='this.className=\"menuitemsel\"' onmouseout='this.className=\"menuitemunsel\"'>\n");
	EXP_FILE << _T("<a class=menuitem onmouseover=ShowMenu(1) href=\"javascript:void(0)\" onclick=OpenAll(0)>") << TranslateT("Close all") << _T("</a>\n");
	EXP_FILE << _T("</td></tr></table></span>\n");
	EXP_FILE << _T("<script language=\"JavaScript\">\n");
	EXP_FILE << _T("<!--\n");
	EXP_FILE << _T("var menu = document.getElementById(\"menubar\");\n");
	EXP_FILE << _T("if (menu != null)\n");
	EXP_FILE << _T("  menu.style.visibility = \"visible\";\n");
	EXP_FILE << _T("// -->\n");
	EXP_FILE << _T("</script>\n");

	EXP_FILE << _T("<h4>") << TranslateT("History Log") << _T("</h4>\n<h3>");
	EXP_FILE << MakeTextHtmled(myName);
	if (proto1.length() || myId.length())
		EXP_FILE << _T(" (") << MakeTextHtmled(proto1) << _T(": ") << MakeTextHtmled(myId) << _T(") - ");
	else
		EXP_FILE << _T(" - ");

	EXP_FILE << MakeTextHtmled(name1);
	if (proto1.length() || id1.length())
		EXP_FILE << _T(" (") << MakeTextHtmled(proto1) << _T(": ") << MakeTextHtmled(id1) << _T(")</h3>\n");
	else
		EXP_FILE << _T("</h3>\n");

	EXP_FILE << _T("<h6>") << TranslateT("Filter:") << _T(" ") << MakeTextHtmled(filterName) << _T("</h6>\n");
	groupId = 0;
}

void RichHtmlExport::WriteFooter()
{
	if (groupId > 0)
		EXP_FILE << _T("</div>\n");

	EXP_FILE << _T("<div class=mes id=bottom></div>\n</body></html>\n");
}

void RichHtmlExport::WriteGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText)
{
	TCHAR *id = isMe ? _T("out") : _T("inc");
	TCHAR* ev = (isMe ? _T("1") : _T("0"));
	if (groupId > 0)
		EXP_FILE << _T("</div>\n");
	
	bool isUrl = false;
	std::wstring& mes = ReplaceSmileys(isMe, eventText, isUrl);
	EXP_FILE << _T("<div class=mes id=session>\n");
	EXP_FILE << _T("<span class=eventimg id=") << id << _T("><img src=\"") << folderName << _T("\\pnode.ico\" class=sessionimage width=\"16\" height=\"16\" onclick=\"toggleFolder('group") << groupId << _T("', this)\"/>");
	EXP_FILE << _T("<img src=\"") << folderName << _T("\\event") << ev << _T(".ico\" class=sessionimage width=\"16\" height=\"16\" onclick=\"toggleFolder('group") << groupId << _T("', this)\"/></span>\n");
	EXP_FILE << _T("<span class=date id=") << id << _T(">") << time << _T("</span>\n<span class=text>\n") << mes;
	EXP_FILE << _T("</span>\n</div>\n");
	EXP_FILE << _T("<div class=group id=group") << groupId << _T(">\n");
	++groupId;
}

void RichHtmlExport::WriteMessage(bool isMe, const std::wstring &longDate, const std::wstring &shortDate, const std::wstring &user, const std::wstring &message, const DBEVENTINFO& dbei)
{
	TCHAR *id = isMe ? _T("out") : _T("inc");
	TCHAR* ev = (isMe ? _T("1") : _T("0"));
	TCHAR* ev1 = ev;
	bool isUrl = false;
	std::wstring& mes = ReplaceSmileys(isMe, message, isUrl);
	if (isUrl)
		ev = _T("2");
	EXP_FILE << _T("<div class=mes id=event") << ev << _T(">\n");
	EXP_FILE << _T("<div class=eventimg id=") << id << _T(">")  << _T("<img src=\"") << folderName << _T("\\event") << ev1 << _T(".ico\" class=sessionimage width=\"16\" height=\"16\"/></div>\n");
	EXP_FILE << _T("<div class=date id=") << id << _T(">") << (Options::instance->exportHtml2ShowDate ? longDate : shortDate) << _T("</div>\n");
	EXP_FILE << _T("<div class=nick id=") << id << _T(">") << MakeTextHtmled(user) << _T("</div>\n");
	EXP_FILE << _T("<div class=text>\n");
	EXP_FILE << mes;
	EXP_FILE << _T("\n</div>\n");
	EXP_FILE << _T("</div>\n");
}

std::wstring RichHtmlExport::ReplaceSmileys(bool isMe, const std::wstring &msg, bool &isUrl)
{
	if (!Options::instance->exportHtml2UseSmileys || !g_SmileyAddAvail)
		return UrlHighlightHtml(MakeTextHtmled(msg), isUrl);

	TCHAR* msgbuf = new TCHAR[msg.length() + 1];
	memcpy_s(msgbuf, (msg.length() + 1) * sizeof(TCHAR), msg.c_str(), (msg.length() + 1) * sizeof(TCHAR));
	SMADD_BATCHPARSE2 sp = {0};
	SMADD_BATCHPARSERES *spr;
	sp.cbSize = sizeof(sp);
	sp.Protocolname = baseProto.length() == 0 ? NULL : baseProto.c_str();
	sp.str = msgbuf;
	sp.flag = SAFL_TCHAR | SAFL_PATH | (isMe ? SAFL_OUTGOING : 0);
	spr = (SMADD_BATCHPARSERES*)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);
	delete[] msgbuf;

	// Did not find a simley
	if (spr == NULL || (INT_PTR)spr == CALLSERVICE_NOTFOUND)
		return UrlHighlightHtml(MakeTextHtmled(msg), isUrl);

	std::queue<std::pair<size_t, size_t> > positionMap;
	std::wstring newMsg = MakeTextHtmled(msg, &positionMap);
	std::wstring smileyMsg;
		
	size_t last_pos=0;
	std::pair<size_t, size_t> pos(0, 0);
	size_t currentAdd = 0;
	if (!positionMap.empty()) {
		pos = positionMap.front();
		positionMap.pop();
	}

	for (unsigned i = 0; i < sp.numSmileys; ++i) {
		size_t startChar = spr[i].startChar + currentAdd;
		while(startChar >= pos.first && pos.second) {
			startChar += pos.second;
			currentAdd += pos.second;
			if (!positionMap.empty()) {
				pos = positionMap.front();
				positionMap.pop();
			}
			else pos = std::pair<size_t, size_t>(0, 0);
		}

		size_t endChar = spr[i].startChar + spr[i].size + currentAdd;
		while(endChar >= pos.first && pos.second) {
			endChar += pos.second;
			currentAdd += pos.second;
			if (!positionMap.empty()) {
				pos = positionMap.front();
				positionMap.pop();
			}
			else pos = std::pair<size_t, size_t>(0, 0);
		}

		size_t size = endChar - startChar;

		if (spr[i].filepath != NULL) { // For deffective smileypacks
			// Add text
			if (startChar - last_pos > 0) {
				smileyMsg += newMsg.substr(last_pos, startChar - last_pos);
			}

			std::wstring smileyName = GetName(spr[i].filepath);
			if (smileys.find(smileyName) == smileys.end()) {
				smileys.insert(smileyName);
				CopyFile(spr[i].filepath, (folder + _T("\\") + smileyName).c_str(), FALSE);
			}

			std::wstring smileyText = newMsg.substr(startChar, size);
			smileyMsg += _T("<img class=smiley src=\"");
			smileyMsg += folderName;
			smileyMsg += _T("\\");
			smileyMsg += smileyName;
			smileyMsg += _T("\" alt=\"");
			smileyMsg += smileyText;
			smileyMsg += _T("\"/>");
		}

		// Get next
		last_pos = endChar;
	}

	// Add rest of text
	if (last_pos < newMsg.length())
		smileyMsg += newMsg.substr(last_pos);

	CallService(MS_SMILEYADD_BATCHFREE, 0, (LPARAM)spr);
	return UrlHighlightHtml(smileyMsg, isUrl);
}
