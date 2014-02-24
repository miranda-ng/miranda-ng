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
#include "ExportManager.h"
#include "TxtExport.h"
#include "PlainHtmlExport.h"
#include "RichHtmlExport.h"
#include "BinaryExport.h"
#include "DatExport.h"
#include "Options.h"
#include "codecvt_CodePage.h"

ExportManager::ExportManager(HWND _hwnd, MCONTACT _hContact, int filter) :
	EventList(_hContact, filter),
	hwnd(_hwnd),
	oldOnTop(false)
{
}

std::wstring GetFile(const TCHAR* ext, HWND hwnd, bool open)
{
	TCHAR filter[512];
	std::locale loc;
	TCHAR extUpper[32];
	_tcscpy_s(extUpper, ext);
	extUpper[0] = std::toupper(ext[0], loc);
	mir_sntprintf(filter, SIZEOF(filter), TranslateT("%s Files (*.%s)"), extUpper, ext);
	size_t len = _tcslen(filter) + 1;
	mir_sntprintf(filter + len, 512 - len, _T("*.%s"), ext);
	len += _tcslen(filter + len);
	filter[++len] = 0;
	TCHAR stzFilePath[1024];
	_tcscpy_s(stzFilePath, TranslateT("History"));
	_tcscat_s(stzFilePath, _T("."));
	_tcscat_s(stzFilePath, ext);
	len = _tcslen(stzFilePath) + 1;
	stzFilePath[len] = 0;
	OPENFILENAME ofn = {0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = stzFilePath;
	ofn.lpstrTitle = open ? TranslateT("Import") : TranslateT("Export");
	ofn.nMaxFile = 1024;
	ofn.lpstrDefExt = ext;
	if (open) {
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		if (GetOpenFileName(&ofn))
			return stzFilePath;
	}
	else {
		ofn.Flags = OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST | OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
		if (GetSaveFileName(&ofn))
			return stzFilePath;
	}

	return L"";
}

std::wstring ReplaceExt(const std::wstring& file, const TCHAR* ext)
{
	size_t pos = file.find(_T("<ext>"));
	if (pos < file.length()) {
		std::wstring fileName = file.substr(0, pos);
		fileName += ext;
		fileName += file.substr(pos + 5);
		return fileName;
	}

	return file;
}

bool ExportManager::Export(IExport::ExportType type)
{
	exp = NULL;
	UINT cp;
	std::wstring encoding;
	bool isBin = false;
	switch(type) {
	case IExport::Txt:
		exp = new TxtExport();
		cp = Options::instance->codepageTxt;
		encoding = Options::instance->encodingTxt;
		isFlat = true;
		break;
	case IExport::PlainHtml:
		exp = new PlainHtmlExport();
		cp = Options::instance->codepageHtml1;
		encoding = Options::instance->encodingHtml1;
		break;
	case IExport::RichHtml:
		exp = new RichHtmlExport();
		cp = Options::instance->codepageHtml2;
		encoding = Options::instance->encodingHtml2;
		break;
	case IExport::Binary:
		exp = new BinaryExport();
		cp = CP_UTF8;
		encoding = L"UTF8";
		isFlat = true;
		oldOnTop = true;
		isBin = true;
		break;
	case IExport::Dat:
		exp = new DatExport();
		cp = CP_UTF8;
		encoding = L"UTF8";
		isFlat = true;
		oldOnTop = true;
		isBin = true;
		break;
	default:
		return false;
	}

	std::wstring fileName;
	if (file.empty())
		fileName = GetFile(exp->GetExt(), hwnd, false);
	else
		fileName = ReplaceExt(file, exp->GetExt());

	if (fileName.empty())
		return false;

	std::wofstream* stream;
	if (!isBin) {
		stream = new std::wofstream (fileName.c_str());
		if (!stream->is_open())
			return false;
	
		std::locale filelocale(std::locale(), new codecvt_CodePage<wchar_t>(cp));
		stream->imbue(filelocale);
		exp->SetStream(stream);
	}
	else {
		std::ofstream* cstream = new std::ofstream (fileName.c_str(), std::ios_base::binary);
		if (!cstream->is_open())
			return false;
	
		stream = (std::wofstream*)cstream;
		exp->SetStream(stream);
	}

	exp->WriteHeader(fileName, GetFilterName(), GetMyName(), GetMyId(), GetContactName(), GetProtocolName(), GetContactId(), GetBaseProtocol(), encoding);

	RefreshEventList();

	exp->WriteFooter();
	if (!isBin) {
		stream->close();
		delete stream;
	}
	else {
		std::ofstream* cstream = (std::ofstream*)stream;
		cstream->close();
		delete cstream;
	}

	delete exp;
	return true;
}

const TCHAR* ExportManager::GetExt(IImport::ImportType type)
{
	IImport* imp = NULL;
	switch(type) {
	case IImport::Binary:
		imp = new BinaryExport();
		break;
	case IImport::Dat:
		imp = new DatExport();
		break;
	default:
		return L"";
	}
	
	const TCHAR* ext = imp->GetExt();
	delete imp;
	return ext;
}

int ExportManager::Import(IImport::ImportType type, const std::vector<MCONTACT>& contacts)
{
	IImport* imp = NULL;
	switch(type) {
	case IImport::Binary:
		imp = new BinaryExport();
		break;
	case IImport::Dat:
		imp = new DatExport();
		break;
	default:
		return -2;
	}

	std::wstring fileName;
	if (file.empty())
		return -2;
	else
	{
		fileName = ReplaceExt(file, imp->GetExt());
	}

	if (fileName.empty())
		return -2;

	std::ifstream* stream = new std::ifstream (fileName.c_str(), std::ios_base::binary);
	if (!stream->is_open())
		return -2;
	
	imp->SetStream(stream);
	int t = imp->IsContactInFile(contacts);
	stream->close();
	delete stream;
	delete imp;
	return t;
}

bool ExportManager::Import(IImport::ImportType type, std::vector<IImport::ExternalMessage>& eventList, std::wstring* err, bool* differentContact, std::vector<MCONTACT>* contacts)
{
	IImport* imp = NULL;
	switch(type) {
	case IImport::Binary:
		imp = new BinaryExport();
		break;
	case IImport::Dat:
		imp = new DatExport();
		break;
	default:
		return false;
	}
	
	std::wstring fileName;
	if (file.empty())
		file = fileName = GetFile(imp->GetExt(), hwnd, true);
	else
		fileName = ReplaceExt(file, imp->GetExt());

	std::ifstream* stream = new std::ifstream (fileName.c_str(), std::ios_base::binary);
	if (!stream->is_open())
		return false;
	
	imp->SetStream(stream);
	std::vector<MCONTACT> v;
	v.push_back(hContact);
	bool ret = true;
	int contInFile = imp->IsContactInFile(v);
	if (contInFile == -1) {
		ret = false;
		if (err != NULL)
			*err = TranslateT("File does not contain selected contact");
		
		if (contacts != NULL && differentContact != NULL) {
			contInFile = imp->IsContactInFile(*contacts);
			if (contInFile >= 0) {
				*differentContact = true;
				hContact = (*contacts)[contInFile];
			}
		}
	}
	else if (contInFile == 0 || contInFile == -3) {
		ret = imp->GetEventList(eventList);
		if (!ret && err != NULL)
			*err = TranslateT("File is corrupted");
	}
	else {
		ret = false;
		if (err != NULL)
			*err = TranslateT("File is corrupted");
	}
	stream->close();
	delete stream;
	delete imp;
	return ret;
}

void ExportManager::AddGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText, int ico)
{
	if (exp == NULL)
		return;
	
	exp->WriteGroup(isMe, time, user, eventText);
	TCHAR str[MAXSELECTSTR + 8]; // for safety reason
	str[0] = 0;
	tm lastTime;
	bool isFirst = true;
	bool lastMe = false;
	EventData data;
	std::deque<EventIndex> revDeq;
	std::deque<EventIndex>& deq = eventList.back();
	if (!oldOnTop && Options::instance->messagesNewOnTop) {
		revDeq.insert(revDeq.begin(), deq.rbegin(), deq.rend());
		deq = revDeq;
	}
	for (std::deque<EventIndex>::iterator it = deq.begin(); it != deq.end(); ++it) {
		EventIndex hDbEvent = *it;
		if (GetEventData(hDbEvent, data)) {
			lastMe = data.isMe;

			TCHAR* formatDate = Options::instance->messagesShowSec ? _T("d s") : _T("d t");
			TCHAR* longFormatDate = Options::instance->messagesShowSec ? _T("d s") : _T("d t");
			if (!Options::instance->messagesShowDate) {
				if (isFirst) {
					isFirst = false;
					formatDate = Options::instance->messagesShowSec ? _T("s") : _T("t");
					time_t tt = data.timestamp;
					localtime_s(&lastTime,  &tt);
				}
				else {
					time_t tt = data.timestamp;
					tm t;
					localtime_s(&t,  &tt);
					if (lastTime.tm_yday == t.tm_yday && lastTime.tm_year == t.tm_year)
						formatDate = Options::instance->messagesShowSec ? _T("s") : _T("t");
				}
			}
				
			tmi.printTimeStamp(NULL, data.timestamp, longFormatDate, str , MAXSELECTSTR, 0);
			std::wstring longDate = str;
			tmi.printTimeStamp(NULL, data.timestamp, formatDate, str , MAXSELECTSTR, 0);
			std::wstring shortDate = str;

			std::wstring user;
			if (lastMe)
				user = myName;
			else
				user = contactName;
				
			GetEventMessage(hDbEvent, str);
			std::wstring strMessage = str;
			if (strMessage.length() + 1 >= MAXSELECTSTR)
				continue;

			if (hDbEvent.isExternal)
				GetExtEventDBei(hDbEvent);

			exp->WriteMessage(lastMe, longDate, shortDate, user, strMessage, gdbei);
		}
	}
}

void ExportManager::DeleteExportedEvents()
{
	for (size_t j = 0; j < eventList.size(); ++j)
		for (size_t i = 0; i < eventList[j].size(); ++i)
			DeleteEvent(eventList[j][i]);
}

void ExportManager::SetDeleteWithoutExportEvents(int _deltaTime, DWORD _now)
{
	exp = NULL;
	deltaTime = _deltaTime;
	now = _now;
	RefreshEventList();
}
