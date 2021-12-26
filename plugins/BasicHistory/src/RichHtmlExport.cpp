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

extern bool g_SmileyAddAvail;

std::wstring MakeTextHtmled(const std::wstring& message, std::queue<std::pair<size_t, size_t> >* positionMap = nullptr)
{
	std::wstring ret;
	std::wstring search = L"&<>\t\r\n";
	size_t start = 0;
	size_t find;
	size_t currentAdd = 0;
	while ((find = message.find_first_of(search, start)) < message.length()) {
		ret += message.substr(start, find - start);
		switch (message[find]) {
		case '&':
			ret += L"&amp;";
			break;
		case '<':
			ret += L"&lt;";
			break;
		case '>':
			ret += L"&gt;";
			break;
		case '\t':
			ret += L" ";
			break;
		case '\n':
			ret += L"<br>";
			break;
		}

		start = find + 1;
		if (positionMap != nullptr) {
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
	std::wstring htmlStop = L"\'\" []<>\r\n";
	std::wstring search = L"://";
	size_t start = 0;
	size_t find;
	while ((find = message.find(search, start)) < message.length()) {
		size_t urlStart = message.find_last_of(htmlStop, find);
		size_t urlEnd = message.find_first_of(htmlStop, find + 3);
		if (urlStart >= message.length())
			urlStart = -1;
		if (urlEnd >= message.length())
			urlEnd = message.length();
		if (((int)urlEnd - 3 - (int)find > 0) && ((int)find - (int)urlStart - 1 > 0)) {
			ret += message.substr(start, (urlStart + 1) - start);
			std::wstring url = message.substr(urlStart + 1, urlEnd - urlStart - 1);
			start = urlEnd;
			ret += L"<a class=url target=_blank href=\"";
			ret += url + L"\">" + url + L"</a>";
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

void ExtractFile(short int iRes, const std::wstring &fileName)
{
	HRSRC rSrc = FindResource(g_plugin.getInst(), MAKEINTRESOURCE(iRes), MAKEINTRESOURCE(CUSTOMRES));
	if (rSrc != nullptr) {
		HGLOBAL res = LoadResource(g_plugin.getInst(), rSrc);
		int size = SizeofResource(g_plugin.getInst(), rSrc);
		if (res != nullptr) {
			char* resData = (char*)LockResource(res);
			if (resData != nullptr) {
				std::ofstream stream(fileName.c_str(), std::ios_base::binary);
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
	uint8_t        bWidth;          // Width, in pixels, of the image
	uint8_t        bHeight;         // Height, in pixels, of the image
	uint8_t        bColorCount;     // Number of colors in image (0 if >=8bpp)
	uint8_t        bReserved;       // Reserved ( must be 0)
	uint16_t        wPlanes;         // Color Planes
	uint16_t        wBitCount;       // Bits per pixel
	uint32_t       dwBytesInRes;    // How many bytes in this resource?
	uint32_t       dwImageOffset;   // Where in the file is this image?
} ICONDIRENTRY, *LPICONDIRENTRY;

typedef struct
{
	uint16_t           idReserved;   // Reserved (must be 0)
	uint16_t           idType;       // Resource Type (1 for icons)
	uint16_t           idCount;      // How many images?
	//ICONDIRENTRY   idEntries; // An entry for each image (idCount of 'em)
} ICONDIR, *LPICONDIR;

#pragma pack(pop)

typedef struct tagMyBITMAPINFO
{
	BITMAPINFOHEADER    bmiHeader;
	RGBQUAD             bmiColors[256];
} MYBITMAPINFO;

void IcoSave(const std::wstring &fileName, HICON hicon)
{
	std::ofstream store(fileName.c_str(), std::ios_base::binary);
	if (!store.is_open())
		return;

	ICONINFO	ii;
	if (!GetIconInfo(hicon, &ii)) {
		store.close();
		return;
	}

	HBITMAP hbmMask = ii.hbmMask;
	HBITMAP hbmColor = ii.hbmColor;
	BITMAP  bmiMask;
	BITMAP  bmiColor;
	if (GetObject(hbmColor, sizeof(bmiColor), &bmiColor) &&
		GetObject(hbmMask, sizeof(bmiMask), &bmiMask) &&
		(bmiColor.bmWidth == bmiMask.bmWidth) &&
		(bmiColor.bmHeight == bmiMask.bmHeight) &&
		(bmiMask.bmHeight) > 0 &&
		(bmiMask.bmWidth) > 0) {
		BITMAPINFOHEADER  icobmi = { 0 };
		MYBITMAPINFO info1 = { 0 };
		MYBITMAPINFO info2 = { 0 };

		HDC hDC = CreateCompatibleDC(nullptr);
		info1.bmiHeader.biSize = sizeof(info1.bmiHeader);
		info1.bmiHeader.biWidth = bmiColor.bmWidth;
		info1.bmiHeader.biHeight = bmiColor.bmHeight;
		info1.bmiHeader.biPlanes = 1;
		info1.bmiHeader.biBitCount = bmiColor.bmBitsPixel;
		unsigned int size = GetDIBits(hDC, hbmColor, 0, info1.bmiHeader.biHeight, nullptr, (BITMAPINFO*)&info1, DIB_RGB_COLORS);
		char* bits1 = new char[info1.bmiHeader.biSizeImage];
		size = GetDIBits(hDC, hbmColor, 0, info1.bmiHeader.biHeight, bits1, (BITMAPINFO*)&info1, DIB_RGB_COLORS);
		info2.bmiHeader.biSize = sizeof(info2.bmiHeader);
		info2.bmiHeader.biWidth = bmiMask.bmWidth;
		info2.bmiHeader.biHeight = bmiMask.bmHeight;
		info2.bmiHeader.biPlanes = 1;
		info2.bmiHeader.biBitCount = bmiMask.bmBitsPixel;
		size = GetDIBits(hDC, hbmColor, 0, info1.bmiHeader.biHeight, nullptr, (BITMAPINFO*)&info2, DIB_RGB_COLORS);
		char* bits2 = new char[info2.bmiHeader.biSizeImage];
		size = GetDIBits(hDC, hbmMask, 0, info2.bmiHeader.biHeight, bits2, (BITMAPINFO*)&info2, DIB_RGB_COLORS);

		ICONDIR            icodir;
		ICONDIRENTRY      icoent;
		icodir.idReserved = 0;
		icodir.idType = 1;
		icodir.idCount = 1;

		icoent.bWidth = (unsigned char)bmiColor.bmWidth;
		icoent.bHeight = (unsigned char)bmiColor.bmHeight;
		icoent.bColorCount = 8 <= bmiColor.bmBitsPixel ? 0 : 1 << bmiColor.bmBitsPixel;
		icoent.bReserved = 0;
		icoent.wPlanes = bmiColor.bmPlanes;
		icoent.wBitCount = bmiColor.bmBitsPixel;
		icoent.dwBytesInRes = sizeof(BITMAPINFOHEADER) + info1.bmiHeader.biSizeImage + info2.bmiHeader.biSizeImage;

		icoent.dwImageOffset = sizeof(icodir) + sizeof(icoent);

		store.write((char*)&icodir, sizeof(icodir));
		store.write((char*)&icoent, sizeof(icoent));

		icobmi.biSize = sizeof(icobmi);
		icobmi.biWidth = bmiColor.bmWidth;
		icobmi.biHeight = bmiColor.bmHeight + bmiMask.bmHeight;
		icobmi.biPlanes = info1.bmiHeader.biPlanes;
		icobmi.biBitCount = bmiColor.bmBitsPixel;
		icobmi.biSizeImage = 0;

		store.write((char*)&icobmi, sizeof(icobmi));

		store.write(bits1, info1.bmiHeader.biSizeImage);
		store.write(bits2, info2.bmiHeader.biSizeImage);
		DeleteDC(hDC);
		delete[] bits1;
		delete[] bits2;
	}

	store.close();
	if (ii.hbmColor) DeleteObject(ii.hbmColor);
	if (ii.hbmMask) DeleteObject(ii.hbmMask);
}

void RichHtmlExport::WriteHeader(const std::wstring &fileName, const std::wstring &filterName, const std::wstring &myName, const std::wstring &myId, const std::wstring &name1, const std::wstring &proto1, const std::wstring &id1, const std::string& baseProto1, const std::wstring& encoding)
{
	baseProto = baseProto1;
	folder = RemoveExt(fileName) + L"_files";
	folderName = GetName(folder);
	DeleteDirectoryTreeW(folder.c_str());
	CreateDirectory(folder.c_str(), nullptr);
	std::wstring css = folder + L"\\history.css";
	BOOL cssCopied = FALSE;
	if (!Options::instance->extCssHtml2.empty())
		cssCopied = CopyFile(Options::instance->extCssHtml2.c_str(), css.c_str(), FALSE);

	if (!cssCopied)
		ExtractFile(IDR_CSS, css);
	ExtractFile(IDR_JS, folder + L"\\history.js");

	HICON ico = LoadIconEx(IDI_PLUSEX);
	IcoSave(folder + L"\\pnode.ico", ico);
	IcoLib_ReleaseIcon(ico);

	ico = LoadIconEx(IDI_MINUSEX);
	IcoSave(folder + L"\\mnode.ico", ico);
	IcoLib_ReleaseIcon(ico);

	ico = LoadIconEx(IDI_INM);
	IcoSave(folder + L"\\event0.ico", ico);
	IcoLib_ReleaseIcon(ico);

	ico = LoadIconEx(IDI_OUTM);
	IcoSave(folder + L"\\event1.ico", ico);
	IcoLib_ReleaseIcon(ico);

	EXP_FILE << L"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n";
	EXP_FILE << L"<html><head>\n<meta http-equiv=\"Content-Type\" content=\"text/html; charset=" << encoding << L"\">\n";
	EXP_FILE << L"<title>" << TranslateT("History log") << L" [" << MakeTextHtmled(myName) << L"] - [" << MakeTextHtmled(name1) << L"]</title>\n";
	EXP_FILE << L"<link rel=\"Stylesheet\" href=\"" << folderName << L"\\history.css\" type=\"text/css\">\n";
	EXP_FILE << L"<script type=\"text/javascript\" src=\"" << folderName << L"\\history.js\"></script>\n";
	EXP_FILE << L"</head><body>\n";

	EXP_FILE << L"<span id=\"menubar\">\n";
	EXP_FILE << L"<a class=mainmenu onmouseover='this.className=\"mainmenusel\";' href=\"javascript:void(0)\" onClick=\"ShowMenu(1)\" onMouseOut='HideMenu();this.className=\"mainmenu\";'>" << TranslateT("Menu") << L"</a></span>\n";
	EXP_FILE << L"<span class=floatmenu id=L1 onmouseover=clearTimeout(timer) onmouseout=HideMenu()>\n";
	EXP_FILE << L"<table><tr>\n";
	EXP_FILE << L"<td class=menuitemunsel onmouseover='this.className=\"menuitemsel\"' onmouseout='this.className=\"menuitemunsel\"'>\n";
	EXP_FILE << L"<a class=menuitem onmouseover=ShowMenu(1) href=\"javascript:void(0)\" onclick=OpenAll(1)>" << TranslateT("Open all") << L"</a>\n";
	EXP_FILE << L"</td></tr><tr>\n";
	EXP_FILE << L"<td class=menuitemunsel onmouseover='this.className=\"menuitemsel\"' onmouseout='this.className=\"menuitemunsel\"'>\n";
	EXP_FILE << L"<a class=menuitem onmouseover=ShowMenu(1) href=\"javascript:void(0)\" onclick=OpenAll(0)>" << TranslateT("Close all") << L"</a>\n";
	EXP_FILE << L"</td></tr></table></span>\n";
	EXP_FILE << L"<script language=\"JavaScript\">\n";
	EXP_FILE << L"<!--\n";
	EXP_FILE << L"var menu = document.getElementById(\"menubar\");\n";
	EXP_FILE << L"if (menu != null)\n";
	EXP_FILE << L"  menu.style.visibility = \"visible\";\n";
	EXP_FILE << L"// -->\n";
	EXP_FILE << L"</script>\n";

	EXP_FILE << L"<h4>" << TranslateT("History log") << L"</h4>\n<h3>";
	EXP_FILE << MakeTextHtmled(myName);
	if (proto1.length() || myId.length())
		EXP_FILE << L" (" << MakeTextHtmled(proto1) << L": " << MakeTextHtmled(myId) << L") - ";
	else
		EXP_FILE << L" - ";

	EXP_FILE << MakeTextHtmled(name1);
	if (proto1.length() || id1.length())
		EXP_FILE << L" (" << MakeTextHtmled(proto1) << L": " << MakeTextHtmled(id1) << L")</h3>\n";
	else
		EXP_FILE << L"</h3>\n";

	EXP_FILE << L"<h6>" << TranslateT("Filter:") << L" " << MakeTextHtmled(filterName) << L"</h6>\n";
	groupId = 0;
}

void RichHtmlExport::WriteFooter()
{
	if (groupId > 0)
		EXP_FILE << L"</div>\n";

	EXP_FILE << L"<div class=mes id=bottom></div>\n</body></html>\n";
}

void RichHtmlExport::WriteGroup(bool isMe, const std::wstring &time, const std::wstring&, const std::wstring &eventText)
{
	wchar_t *id = isMe ? L"out" : L"inc";
	wchar_t* ev = (isMe ? L"1" : L"0");
	if (groupId > 0)
		EXP_FILE << L"</div>\n";

	bool isUrl = false;
	std::wstring mes = ReplaceSmileys(isMe, eventText, isUrl);
	EXP_FILE << L"<div class=mes id=session>\n";
	EXP_FILE << L"<span class=eventimg id=" << id << L"><img src=\"" << folderName << L"\\pnode.ico\" class=sessionimage width=\"16\" height=\"16\" onclick=\"toggleFolder('group" << groupId << L"', this)\"/>";
	EXP_FILE << L"<img src=\"" << folderName << L"\\event" << ev << L".ico\" class=sessionimage width=\"16\" height=\"16\" onclick=\"toggleFolder('group" << groupId << L"', this)\"/></span>\n";
	EXP_FILE << L"<span class=date id=" << id << L">" << time << L"</span>\n<span class=text>\n" << mes;
	EXP_FILE << L"</span>\n</div>\n";
	EXP_FILE << L"<div class=group id=group" << groupId << L">\n";
	++groupId;
}

void RichHtmlExport::WriteMessage(bool isMe, const std::wstring &longDate, const std::wstring &shortDate, const std::wstring &user, const std::wstring &message, const DBEVENTINFO&)
{
	wchar_t *id = isMe ? L"out" : L"inc";
	wchar_t* ev = (isMe ? L"1" : L"0");
	wchar_t* ev1 = ev;
	bool isUrl = false;
	std::wstring mes = ReplaceSmileys(isMe, message, isUrl);
	if (isUrl)
		ev = L"2";
	EXP_FILE << L"<div class=mes id=event" << ev << L">\n";
	EXP_FILE << L"<div class=eventimg id=" << id << L">" << L"<img src=\"" << folderName << L"\\event" << ev1 << L".ico\" class=sessionimage width=\"16\" height=\"16\"/></div>\n";
	EXP_FILE << L"<div class=date id=" << id << L">" << (Options::instance->exportHtml2ShowDate ? longDate : shortDate) << L"</div>\n";
	EXP_FILE << L"<div class=nick id=" << id << L">" << MakeTextHtmled(user) << L"</div>\n";
	EXP_FILE << L"<div class=text>\n";
	EXP_FILE << mes;
	EXP_FILE << L"\n</div>\n";
	EXP_FILE << L"</div>\n";
}

std::wstring RichHtmlExport::ReplaceSmileys(bool isMe, const std::wstring &msg, bool &isUrl)
{
	if (!Options::instance->exportHtml2UseSmileys || !g_SmileyAddAvail)
		return UrlHighlightHtml(MakeTextHtmled(msg), isUrl);

	wchar_t* msgbuf = new wchar_t[msg.length() + 1];
	memcpy_s(msgbuf, (msg.length() + 1) * sizeof(wchar_t), msg.c_str(), (msg.length() + 1) * sizeof(wchar_t));
	SMADD_BATCHPARSE2 sp = { 0 };
	SMADD_BATCHPARSERES *spr;
	sp.cbSize = sizeof(sp);
	sp.Protocolname = baseProto.length() == 0 ? nullptr : baseProto.c_str();
	sp.str = msgbuf;
	sp.flag = SAFL_TCHAR | SAFL_PATH | (isMe ? SAFL_OUTGOING : 0);
	spr = (SMADD_BATCHPARSERES*)CallService(MS_SMILEYADD_BATCHPARSE, 0, (LPARAM)&sp);
	delete[] msgbuf;

	// Did not find a simley
	if (spr == nullptr || (INT_PTR)spr == CALLSERVICE_NOTFOUND)
		return UrlHighlightHtml(MakeTextHtmled(msg), isUrl);

	std::queue<std::pair<size_t, size_t> > positionMap;
	std::wstring newMsg = MakeTextHtmled(msg, &positionMap);
	std::wstring smileyMsg;

	size_t last_pos = 0;
	std::pair<size_t, size_t> pos(0, 0);
	size_t currentAdd = 0;
	if (!positionMap.empty()) {
		pos = positionMap.front();
		positionMap.pop();
	}

	for (unsigned i = 0; i < sp.numSmileys; ++i) {
		size_t startChar = spr[i].startChar + currentAdd;
		while (startChar >= pos.first && pos.second) {
			startChar += pos.second;
			currentAdd += pos.second;
			if (!positionMap.empty()) {
				pos = positionMap.front();
				positionMap.pop();
			}
			else pos = std::pair<size_t, size_t>(0, 0);
		}

		size_t endChar = spr[i].startChar + spr[i].size + currentAdd;
		while (endChar >= pos.first && pos.second) {
			endChar += pos.second;
			currentAdd += pos.second;
			if (!positionMap.empty()) {
				pos = positionMap.front();
				positionMap.pop();
			}
			else pos = std::pair<size_t, size_t>(0, 0);
		}

		size_t size = endChar - startChar;

		if (spr[i].filepath != nullptr) { // For deffective smileypacks
			// Add text
			if (startChar - last_pos > 0) {
				smileyMsg += newMsg.substr(last_pos, startChar - last_pos);
			}

			std::wstring smileyName = GetName(spr[i].filepath);
			if (smileys.find(smileyName) == smileys.end()) {
				smileys.insert(smileyName);
				CopyFile(spr[i].filepath, (folder + L"\\" + smileyName).c_str(), FALSE);
			}

			std::wstring smileyText = newMsg.substr(startChar, size);
			smileyMsg += L"<img class=smiley src=\"";
			smileyMsg += folderName;
			smileyMsg += L"\\";
			smileyMsg += smileyName;
			smileyMsg += L"\" alt=\"";
			smileyMsg += smileyText;
			smileyMsg += L"\"/>";
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
