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
#include "Options.h"
#include "ExportManager.h"
#include "HistoryWindow.h"

bool DoTask(TaskOptions& to);
bool IsValidTask(TaskOptions& to, std::list<TaskOptions>* top = nullptr, std::wstring* err = nullptr, std::wstring* errDescr = nullptr);
std::wstring GetFileName(const std::wstring &baseName, std::wstring contactName, std::map<std::wstring, bool>& existingContacts, bool replaceContact);
std::wstring GetDirectoryName(const std::wstring &path);
std::wstring GetName(const std::wstring &path);
void ListDirectory(const std::wstring &basePath, const std::wstring &path, std::list<std::wstring>& files);
std::wstring ReplaceStr(const std::wstring& str, wchar_t oldCh, wchar_t newCh);
time_t GetNextExportTime(TaskOptions& to);
void SchedulerThreadFunc(void*);
volatile bool finishThread = false;
bool initTask = false;
HANDLE hThread = nullptr;
HANDLE hThreadEvent;
time_t nextExportTime;
void StartThread(bool init);
void StopThread();
bool GetNextExportTime(bool init, time_t now);
bool ExecuteCurrentTask(time_t now);
void GetZipFileTime(const wchar_t *file, uLong *dt);
std::wstring ReplaceExt(const std::wstring& file, const wchar_t* ext);
bool ZipFiles(const std::wstring& dir, std::wstring zipFilePath, const std::string& password);
bool UnzipFiles(const std::wstring& dir, std::wstring& zipFilePath, const std::string& password);
bool FtpFiles(const std::wstring& dir, const std::wstring& filePath, const std::wstring& ftpName);
bool FtpGetFiles(const std::wstring& dir, const std::list<std::wstring>& files, const std::wstring& ftpName);
void CreatePath(const wchar_t *szDir);
void DoError(const TaskOptions& to, const std::wstring error);

static HANDLE hPopupClass;

void OptionsSchedulerChanged()
{
	StartThread(false);
}

static int OnShutdown(WPARAM, LPARAM)
{
	Popup_UnregisterClass(hPopupClass);
	return 0;
}

void InitScheduler()
{
	POPUPCLASS test = {};
	test.flags = PCF_UNICODE;
	test.hIcon = Skin_LoadIcon(SKINICON_OTHER_HISTORY);
	test.iSeconds = 10;
	test.pszDescription.w = TranslateT("History task");
	test.pszName = MODULENAME;
	if (hPopupClass = Popup_RegisterClass(&test))
		HookEvent(ME_SYSTEM_SHUTDOWN, OnShutdown);

	StartThread(true);
}

void DeinitScheduler()
{
	StopThread();
}

int DoLastTask(WPARAM, LPARAM)
{
	for (std::vector<TaskOptions>::iterator it = Options::instance->taskOptions.begin(); it != Options::instance->taskOptions.end(); ++it)
		if (it->trigerType == TaskOptions::AtEnd && it->active)
			DoTask(*it);

	return 0;
}

bool IsValidTask(TaskOptions& to, std::list<TaskOptions>* top, std::wstring* err, std::wstring* errDescr)
{
	if (to.taskName.empty()) {
		if (err != nullptr)
			*err = TranslateT("Name");
		return false;
	}

	if (top != nullptr) {
		for (std::list<TaskOptions>::iterator it = top->begin(); it != top->end(); ++it) {
			if (it->taskName == to.taskName) {
				if (err != nullptr)
					*err = TranslateT("Name");
				return false;
			}
		}
	}

	if (!to.isSystem && to.contacts.size() == 0) {
		if (err != nullptr)
			*err = TranslateT("Contacts");
		if (errDescr != nullptr)
			*errDescr = TranslateT("At least one contact should be selected.");
		return false;
	}

	bool isImportTask = to.type == TaskOptions::Import || to.type == TaskOptions::ImportAndMarge;
	if (!isImportTask) {
		if (to.filterId > 1) {
			int filter = 0;

			for (int i = 0; i < (int)Options::instance->customFilters.size(); ++i) {
				if (to.filterName == Options::instance->customFilters[i].name) {
					filter = i + 2;
					break;
				}
			}

			if (filter < 2) {
				if (err != nullptr)
					*err = TranslateT("Filter");
				return false;
			}

			to.filterId = filter;
		}
		else if (to.filterId < 0) {
			if (err != nullptr)
				*err = TranslateT("Filter");
			return false;
		}
	}

	if (to.type == TaskOptions::Delete)
		return true;

	if (!Options::FTPAvail() && to.useFtp) {
		if (err != nullptr)
			*err = TranslateT("Upload to FTP");
		return false;
	}
	if (to.filePath.empty()) {
		if (err != nullptr)
			*err = TranslateT("Path to output file");
		return false;
	}
	if (to.useFtp && to.ftpName.empty()) {
		if (err != nullptr)
			*err = TranslateT("Session name");
		if (errDescr != nullptr)
			*errDescr = TranslateT("To create session open WinSCP, click New Session, enter data and save with specific name. Remember if FTP server using password you should save it in WinSCP.");
		return false;
	}
	if (to.useFtp && (to.filePath.find('\\') < to.filePath.length() || to.filePath.find(':') < to.filePath.length() || to.filePath[0] != L'/')) {
		if (err != nullptr)
			*err = TranslateT("Path to file");
		if (errDescr != nullptr)
			*errDescr = TranslateT("FTP path must contain '/' instead '\\' and start with '/'.");
		return false;
	}
	if (isImportTask && to.filePath.find(L"<date>") < to.filePath.length()) {
		if (err != nullptr)
			*err = TranslateT("Path to file");
		if (errDescr != nullptr)
			*errDescr = TranslateT("FTP path cannot contain <date> in import task.");
		return false;
	}
	if (!isImportTask && (to.exportType < IExport::RichHtml || to.exportType > IExport::Dat)) {
		if (err != nullptr)
			*err = TranslateT("Export to");
		return false;
	}
	if (isImportTask && (to.importType < IImport::Binary || to.importType > IImport::Dat)) {
		if (err != nullptr)
			*err = TranslateT("Import from");
		return false;
	}
	if ((to.trigerType == TaskOptions::Daily || to.trigerType == TaskOptions::Weekly || to.trigerType == TaskOptions::Monthly) && (to.dayTime < 0 || to.dayTime >= 24 * 60)) {
		if (err != nullptr)
			*err = TranslateT("Time");
		return false;
	}
	if (to.trigerType == TaskOptions::Weekly && (to.dayOfWeek < 0 || to.dayOfWeek >= 7)) {
		if (err != nullptr)
			*err = TranslateT("Day of week");
		return false;
	}
	if (to.trigerType == TaskOptions::Monthly && (to.dayOfMonth <= 0 || to.dayOfMonth >= 32)) {
		if (err != nullptr)
			*err = TranslateT("Day");
		return false;
	}
	if ((to.trigerType == TaskOptions::DeltaMin || to.trigerType == TaskOptions::DeltaHour) && (to.deltaTime < 0 || to.deltaTime >= 10000)) {
		if (err != nullptr)
			*err = TranslateT("Delta time");
		return false;
	}

	return true;
}

static INT_PTR CALLBACK DoRebuildEventsInMainAPCFunc(void *dwParam)
{
	MCONTACT *contacts = (MCONTACT*)dwParam;
	size_t size = (size_t)contacts[0];
	for (size_t i = 1; i <= size; ++i)
		HistoryWindow::RebuildEvents(contacts[i]);

	delete[] contacts;
	return 0;
}

bool DoTask(TaskOptions& to)
{
	std::wstring err;
	std::wstring errDescr;
	if (!IsValidTask(to, nullptr, &err, &errDescr)) {
		wchar_t msg[256];
		if (err.empty())
			wcscpy_s(msg, TranslateT("Some value is invalid"));
		else if (errDescr.empty())
			mir_snwprintf(msg, TranslateT("Invalid '%s' value."), err.c_str());
		else
			mir_snwprintf(msg, TranslateT("Invalid '%s' value.\n%s"), err.c_str(), errDescr.c_str());

		DoError(to, msg);
		return true;
	}

	uint32_t now = time(0);
	long long int t = to.eventDeltaTime * 60;
	if (to.eventUnit > TaskOptions::Minute)
		t *= 60LL;
	if (to.eventUnit > TaskOptions::Hour)
		t *= 24LL;
	if (t > 2147483647LL) {
		DoError(to, TranslateT("Unknown error"));
		return true;
	}

	bool error = false;
	std::wstring errorStr;
	std::list<ExportManager*> managers;
	if (to.type == TaskOptions::Delete) {
		if (to.isSystem) {
			ExportManager *exp = new ExportManager(nullptr, NULL, to.filterId);
			exp->SetDeleteWithoutExportEvents(t, now);
			managers.push_back(exp);
		}

		for (size_t i = 0; i < to.contacts.size(); ++i) {
			ExportManager *exp = new ExportManager(nullptr, to.contacts[i], to.filterId);
			exp->SetDeleteWithoutExportEvents(t, now);
			managers.push_back(exp);
		}
	}
	else if (to.type == TaskOptions::Import || to.type == TaskOptions::ImportAndMarge) {
		std::map<std::wstring, bool> existingContacts1;
		ExportManager mExp = ExportManager(nullptr, NULL, 1);
		std::wstring filePath = to.filePath;
		std::wstring dir;
		std::list<std::wstring> files;
		std::vector<MCONTACT> contacts;
		if (to.useFtp || to.compress) {
			std::map<std::wstring, bool> existingContacts;
			wchar_t temp[MAX_PATH];
			temp[0] = 0;
			GetTempPath(MAX_PATH, temp);
			dir = temp;
			dir += GetName(filePath);
			dir = GetFileName(dir, L"", existingContacts, true);
			dir = ReplaceExt(dir, L"");
			size_t pos = dir.find_last_of('.');
			if (pos < dir.length())
				dir = dir.substr(0, pos);

			DeleteDirectoryTreeW(dir.c_str());
			CreateDirectory(dir.c_str(), nullptr);
		}

		const wchar_t* ext = ExportManager::GetExt(to.importType);
		if (to.isSystem) {
			std::wstring n = GetFileName(filePath, mExp.GetContactName(), existingContacts1, true);
			n = ReplaceExt(n, ext);
			files.push_back(n);
			contacts.push_back(NULL);
		}

		for (size_t i = 0; i < to.contacts.size(); ++i) {
			mExp.m_hContact = to.contacts[i];
			std::wstring n = GetFileName(filePath, mExp.GetContactName(), existingContacts1, true);
			n = ReplaceExt(n, ext);
			files.push_back(n);
			contacts.push_back(to.contacts[i]);
		}

		if (to.useFtp) {
			if (to.compress) {
				std::map<std::wstring, bool> existingContacts;
				std::wstring n = GetFileName(filePath, L"", existingContacts, true);
				n = ReplaceExt(n, L"zip");
				files.clear();
				files.push_back(n);
				filePath = dir + L"\\" + GetName(filePath);
			}

			error = FtpGetFiles(dir, files, to.ftpName);
			if (error) {
				if (!errorStr.empty())
					errorStr += L"\n";

				errorStr += TranslateT("Cannot get FTP file(s).");
			}
		}

		if (!error && to.compress) {
			error = UnzipFiles(dir, filePath, to.zipPassword);
			if (error) {
				if (!errorStr.empty())
					errorStr += L"\n";

				errorStr += TranslateT("Cannot unzip file(s).");
			}

			if (to.useFtp)
				DeleteFile(filePath.c_str());
		}

		if (!error && (to.useFtp || to.compress)) {
			files.clear();
			std::list<std::wstring> files1;
			ListDirectory(dir, L"\\", files1);
			for (std::list<std::wstring>::iterator it = files1.begin(); it != files1.end(); ++it)
				files.push_back(dir + *it);
		}

		if (!error) {
			std::list<MCONTACT> contactList;
			for (std::list<std::wstring>::iterator it = files.begin(); it != files.end(); ++it) {
				mExp.SetAutoImport(*it);
				int ret = mExp.Import(to.importType, contacts);
				if (ret == -3) {
					if (contacts.size() == 1)
						ret = 0;
					else {
						std::map<std::wstring, bool> existingContacts;
						std::wstring name = GetName(*it);
						for (ret = 0; ret < (int)contacts.size(); ++ret) {
							mExp.m_hContact = contacts[ret];
							std::wstring n = GetFileName(to.filePath, mExp.GetContactName(), existingContacts, true);
							n = ReplaceExt(n, ext);
							n = GetName(n);
							if (n == name)
								break;
						}

						if (ret >= (int)contacts.size())
							ret = -1;
					}
				}

				if (ret >= 0) {
					mExp.m_hContact = contacts[ret];
					if (to.type == TaskOptions::Import) {
						HistoryEventList::AddImporter(mExp.m_hContact, to.importType, *it);
						contactList.push_back(mExp.m_hContact);
					}
					else {
						std::vector<IImport::ExternalMessage> messages;
						if (mExp.Import(to.importType, messages, nullptr)) {
							mExp.MargeMessages(messages);
							contactList.push_back(mExp.m_hContact);
						}
					}
				}
				else if (ret != -1) {
					if (!errorStr.empty())
						errorStr += L"\n";

					wchar_t msg[1024];
					mir_snwprintf(msg, TranslateT("Incorrect file format: %s."), GetName(*it).c_str());
					errorStr += msg;
				}
				else {
					if (!errorStr.empty())
						errorStr += L"\n";

					wchar_t msg[1024];

					mir_snwprintf(msg, TranslateT("Unknown contact in file: %s."), GetName(*it).c_str());
					errorStr += msg;
				}
			}

			if (contactList.size() > 0) {
				MCONTACT *pContacts = new MCONTACT[contactList.size() + 1];
				pContacts[0] = (MCONTACT)contactList.size();
				int i = 1;
				for (std::list<MCONTACT>::iterator it = contactList.begin(); it != contactList.end(); ++it)
					pContacts[i++] = *it;

				CallFunctionSync(DoRebuildEventsInMainAPCFunc, pContacts);
			}
		}

		if (to.useFtp || to.compress)
			DeleteDirectoryTreeW(dir.c_str());
	}
	else {
		std::map<std::wstring, bool> existingContacts;
		std::wstring filePath = to.filePath;
		std::wstring dir;
		if (!to.useFtp && !to.compress) {
			dir = GetDirectoryName(filePath);
			if (!dir.empty())
				CreateDirectory(dir.c_str(), nullptr);
		}
		else {
			filePath = GetName(filePath);
			wchar_t temp[MAX_PATH];
			temp[0] = 0;
			GetTempPath(MAX_PATH, temp);
			dir = temp;
			dir += filePath;
			dir = GetFileName(dir, L"", existingContacts, true);
			dir = ReplaceExt(dir, L"");
			size_t pos = dir.find_last_of('.');
			if (pos < dir.length())
				dir = dir.substr(0, pos);

			DeleteDirectoryTreeW(dir.c_str());
			CreateDirectory(dir.c_str(), nullptr);
			filePath = dir + L"\\" + filePath;
		}
		if (to.isSystem) {
			ExportManager *exp = new ExportManager(nullptr, NULL, to.filterId);
			exp->SetAutoExport(GetFileName(filePath, exp->GetContactName(), existingContacts, true), t, now);
			exp->m_useImportedMessages = to.exportImported;
			if (!exp->Export(to.exportType)) {
				error = true;
				if (!errorStr.empty())
					errorStr += L"\n";

				wchar_t msg[1024];

				mir_snwprintf(msg, TranslateT("Cannot export history for contact: %s."), exp->GetContactName().c_str());
				errorStr += msg;
			}

			if (to.type == TaskOptions::Export)
				delete exp;
			else
				managers.push_back(exp);
		}

		if (!error) {
			for (size_t i = 0; i < to.contacts.size(); ++i) {
				ExportManager *exp = new ExportManager(nullptr, to.contacts[i], to.filterId);
				exp->SetAutoExport(GetFileName(filePath, exp->GetContactName(), existingContacts, true), t, now);
				exp->m_useImportedMessages = to.exportImported;
				if (!exp->Export(to.exportType)) {
					error = true;
					if (!errorStr.empty())
						errorStr += L"\n";

					wchar_t msg[1024];
					mir_snwprintf(msg, TranslateT("Cannot export history for contact: %s."), exp->GetContactName().c_str());
					errorStr += msg;
					break;
				}

				if (to.type == TaskOptions::Export)
					delete exp;
				else
					managers.push_back(exp);
			}
		}

		if (error) {
			if (to.compress && !to.useFtp)
				DeleteDirectoryTreeW(dir.c_str());
		}
		else if (to.compress) {
			std::wstring zipFilePath = to.filePath;
			std::wstring zipDir = dir;
			if (!to.useFtp) {
				zipDir = GetDirectoryName(zipFilePath);
				if (!zipDir.empty())
					CreateDirectory(zipDir.c_str(), nullptr);
			}
			else {
				zipFilePath = GetName(zipFilePath);
				wchar_t temp[MAX_PATH];
				temp[0] = 0;
				GetTempPath(MAX_PATH, temp);
				zipDir = temp;
				zipDir += L"zip<date>";
				zipDir = GetFileName(zipDir, L"", existingContacts, true);
				DeleteDirectoryTreeW(zipDir.c_str());
				CreateDirectory(zipDir.c_str(), nullptr);
				zipFilePath = zipDir + L"\\" + zipFilePath;
			}
			error = ZipFiles(dir + L"\\", zipFilePath, to.zipPassword);
			dir = zipDir;
			if (error) {
				if (!errorStr.empty())
					errorStr += L"\n";

				errorStr += TranslateT("Cannot compress file(s).");
			}
		}

		if (to.useFtp) {
			if (!error) {
				error = FtpFiles(dir, to.filePath, to.ftpName);
				if (error) {
					if (!errorStr.empty())
						errorStr += L"\n";

					errorStr += TranslateT("Cannot send FTP file(s).");
				}
			}

			DeleteDirectoryTreeW(dir.c_str());
		}
	}

	if (to.type == TaskOptions::Delete || to.type == TaskOptions::ExportAndDelete) {
		for (std::list<ExportManager*>::iterator it = managers.begin(); it != managers.end(); ++it) {
			if (!error)
				(*it)->DeleteExportedEvents();

			delete *it;
		}
	}

	if (error)
		DoError(to, errorStr.empty() ? TranslateT("Unknown error") : errorStr);

	return error;
}

std::wstring GetFileName(const std::wstring &baseName, std::wstring contactName, std::map<std::wstring, bool>& existingContacts, bool replaceContact)
{
	std::wstring str = baseName;
	size_t pos = baseName.find(L"<contact>");
	if (replaceContact && pos < baseName.length()) {
		str = baseName.substr(0, pos);
		std::wstring baseName1 = contactName;
		if (!baseName1.empty()) {
			std::wstring name = baseName1;
			int i = 0;
			wchar_t buf[32];
			std::map<std::wstring, bool>::iterator it = existingContacts.find(name);
			while (it != existingContacts.end()) {
				_itow_s(++i, buf, 10);
				name = baseName1 + buf;
				it = existingContacts.find(name);
			}

			str += name;
			existingContacts[name] = true;
		}
		str += baseName.substr(pos + 9);
	}

	pos = str.find(L"<date>");
	if (pos < str.length()) {
		wchar_t time[256];
		SYSTEMTIME st;
		GetLocalTime(&st);
		mir_snwprintf(time, L"%d-%02d-%02d %02d%02d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
		std::wstring str1 = str.substr(0, pos);
		str1 += time;
		str1 += str.substr(pos + 6);
		str = str1;
	}

	return str;
}

std::wstring GetDirectoryName(const std::wstring &path)
{
	size_t find = path.find_last_of(L"\\/");
	if (find < path.length())
		return path.substr(0, find);

	return L"";
}

void ListDirectory(const std::wstring &basePath, const std::wstring &path, std::list<std::wstring>& files)
{
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = FindFirstFile((basePath + path + L"*").c_str(), &findFileData);
	if (hFind == INVALID_HANDLE_VALUE)
		return;

	do {
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			std::wstring name = findFileData.cFileName;
			if (name != L"." && name != L"..")
				ListDirectory(basePath, path + findFileData.cFileName + L"\\", files);
		}
		else files.push_back(path + findFileData.cFileName);
	} while (FindNextFile(hFind, &findFileData));
	FindClose(hFind);
}

std::wstring ReplaceStr(const std::wstring& str, wchar_t oldCh, wchar_t newCh)
{
	std::wstring ret;
	size_t start = 0;
	size_t find;
	while ((find = str.find_first_of(oldCh, start)) < str.length()) {
		ret += str.substr(start, find - start);
		ret += newCh;
		start = find + 1;
	}

	ret += str.substr(start, str.length() - start);
	return ret;
}

time_t GetNextExportTime(TaskOptions& to)
{
	tm t;
	time_t newTime;

	switch (to.trigerType) {
	case TaskOptions::Daily:
		localtime_s(&t, &to.lastExport);
		t.tm_hour = to.dayTime / 60;
		t.tm_min = to.dayTime % 60;
		t.tm_sec = 0;
		newTime = mktime(&t);
		if (newTime <= to.lastExport)
			newTime += 60 * 60 * 24;
		return newTime;

	case TaskOptions::Weekly:
		localtime_s(&t, &to.lastExport);
		t.tm_hour = to.dayTime / 60;
		t.tm_min = to.dayTime % 60;
		t.tm_sec = 0;
		{
			int dow = (to.dayOfWeek + 1) % 7;
			newTime = mktime(&t);
			while (dow != t.tm_wday) {
				newTime += 60 * 60 * 24;
				localtime_s(&t, &newTime);
				newTime = mktime(&t);
			}
		}

		if (newTime <= to.lastExport)
			newTime += 7 * 60 * 60 * 24;
		return newTime;

	case TaskOptions::Monthly:
		localtime_s(&t, &to.lastExport);
		t.tm_hour = to.dayTime / 60;
		t.tm_min = to.dayTime % 60;
		t.tm_sec = 0;
		newTime = mktime(&t);

		while (to.dayOfMonth != t.tm_mday || newTime <= to.lastExport) {
			int lastM = t.tm_mon;
			int lastD = t.tm_mday;
			newTime += 60 * 60 * 24;
			localtime_s(&t, &newTime);
			newTime = mktime(&t);
			if (to.dayOfMonth > 28 && t.tm_mon != lastM && (newTime - 60 * 60 * 24) > to.lastExport) {
				lastM = t.tm_mon;
				if (to.dayOfMonth > lastD) {
					newTime -= 60 * 60 * 24;
					break;
				}
			}
		}

		return newTime;

	case TaskOptions::DeltaMin:
		return to.lastExport + to.deltaTime * 60;
	case TaskOptions::DeltaHour:
		return to.lastExport + to.deltaTime * 60 * 60;
	default:
		return to.lastExport;
	}
}

void SchedulerThreadFunc(void*)
{
	if (initTask) {
		WaitForSingleObject(hThreadEvent, 5 * 1000);
		initTask = false;
	}

	while (!finishThread) {
		uint32_t timeWait;
		time_t now = time(0);
		while (nextExportTime <= now)
			if (!ExecuteCurrentTask(now))
				return;

		time_t dif = nextExportTime - now;
		timeWait = (dif > 60 * 60 * 24) ? (60 * 60 * 1000) : (60 * 1000);

		WaitForSingleObject(hThreadEvent, timeWait);
	}
}

void StartThread(bool init)
{
	StopThread();

	initTask = false;
	bool isExport = GetNextExportTime(init, time(0));
	if (isExport) {
		finishThread = false;
		hThreadEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		hThread = mir_forkthread(SchedulerThreadFunc);
	}
}

void StopThread()
{
	if (hThread == nullptr)
		return;

	finishThread = true;
	SetEvent(hThreadEvent);
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThreadEvent);
	hThread = nullptr;
	hThreadEvent = nullptr;
}

bool GetNextExportTime(bool init, time_t now)
{
	mir_cslock lck(Options::instance->criticalSection);
	bool isExport = false;
	for (std::vector<TaskOptions>::iterator it = Options::instance->taskOptions.begin(); it != Options::instance->taskOptions.end(); ++it) {
		if (it->forceExecute) {
			nextExportTime = now;
			isExport = true;
			initTask = init;
			break;
		}
		else if (it->active && it->trigerType != TaskOptions::AtStart && it->trigerType != TaskOptions::AtEnd) {
			time_t t = GetNextExportTime(*it);
			if (isExport) {
				if (t < nextExportTime)
					nextExportTime = t;
			}
			else {
				nextExportTime = t;
				isExport = true;
				initTask = init;
			}
		}
		else if (it->active && it->trigerType == TaskOptions::AtStart && init) {
			it->forceExecute = true;
			it->showMBAfterExecute = false;
			nextExportTime = now;
			isExport = true;
			initTask = true;
		}
	}

	return isExport;
}

static INT_PTR CALLBACK DoTaskFinishInMainAPCFunc(void *param)
{
	wchar_t *item = (wchar_t*)param;
	MessageBox(nullptr, item, TranslateT("Task finished"), MB_OK | MB_ICONINFORMATION);
	delete[] item;
	return 0;
}

bool ExecuteCurrentTask(time_t now)
{
	TaskOptions to;
	bool isExport = false;
	{
		mir_cslock lck(Options::instance->criticalSection);
		for (auto it = Options::instance->taskOptions.begin(); it != Options::instance->taskOptions.end(); ++it) {
			if (it->forceExecute) {
				it->lastExport = time(0);
				Options::instance->SaveTaskTime(*it);
				to = *it;
				isExport = true;
				break;
			}
			else if (it->active && it->trigerType != TaskOptions::AtStart && it->trigerType != TaskOptions::AtEnd) {
				time_t t = GetNextExportTime(*it);
				if (t <= now) {
					it->lastExport = time(0);
					Options::instance->SaveTaskTime(*it);
					to = *it;
					isExport = true;
					break;
				}
			}
		}
	}

	if (isExport) {
		bool error = DoTask(to);
		if (to.forceExecute) {
			{
				mir_cslock lck(Options::instance->criticalSection);
				for (auto it = Options::instance->taskOptions.begin(); it != Options::instance->taskOptions.end(); ++it) {
					if (it->taskName == to.taskName) {
						it->forceExecute = false;
						it->showMBAfterExecute = false;
						break;
					}
				}
			}

			if (to.showMBAfterExecute) {
				size_t size = to.taskName.size() + 1024;
				wchar_t *name = new wchar_t[size];
				if (error)
					mir_snwprintf(name, size, TranslateT("Task '%s' execution failed"), to.taskName.c_str());
				else
					mir_snwprintf(name, size, TranslateT("Task '%s' finished successfully"), to.taskName.c_str());
				CallFunctionSync(DoTaskFinishInMainAPCFunc, name);
			}
		}
	}

	return GetNextExportTime(false, now);
}

void GetZipFileTime(const wchar_t *file, uLong *dt)
{
	FILETIME ftLocal;
	WIN32_FIND_DATA ff32;
	HANDLE hFind = FindFirstFile(file, &ff32);
	if (hFind != INVALID_HANDLE_VALUE) {
		FileTimeToLocalFileTime(&(ff32.ftLastWriteTime), &ftLocal);
		FileTimeToDosDateTime(&ftLocal, ((LPWORD)dt) + 1, ((LPWORD)dt) + 0);
		FindClose(hFind);
	}
}

/* calculate the CRC32 of a file,
	because to encrypt a file, we need known the CRC32 of the file before */
bool GetFileCrc(const wchar_t *filenameinzip, unsigned char *buf, unsigned long, unsigned long *result_crc)
{
	unsigned long calculate_crc = 0;
	bool error = true;
	HANDLE hFile = CreateFile(filenameinzip, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile != INVALID_HANDLE_VALUE) {
		DWORD readed;
		do {
			if (!ReadFile(hFile, buf, 1024, &readed, nullptr)) {
				error = false;
				break;
			}

			if (readed > 0)
				calculate_crc = crc32(calculate_crc, buf, readed);
		} while (readed > 0);
		CloseHandle(hFile);
	}
	else error = false;

	*result_crc = calculate_crc;
	return error;
}

bool ZipFiles(const std::wstring &dir, std::wstring zipFilePath, const std::string &password)
{
	std::list<std::wstring> files;
	std::map<std::wstring, bool> existingContacts;
	ListDirectory(dir, L"", files);
	bool error = false;
	if (files.size() > 0) {
		zlib_filefunc_def pzlib_filefunc_def;
		fill_win32_filefunc(&pzlib_filefunc_def);
		zipFilePath = GetFileName(zipFilePath, L"", existingContacts, true);
		zipFilePath = ReplaceExt(zipFilePath, L"zip");
		zipFile zf = zipOpen2((LPCSTR)(LPTSTR)zipFilePath.c_str(), APPEND_STATUS_CREATE, nullptr, &pzlib_filefunc_def);
		if (zf != nullptr) {
			unsigned char buf[1024];
			char bufF[MAX_PATH + 20];
			while (files.size() > 0) {
				std::wstring zipDir = *files.begin();
				std::wstring localDir = dir + L"\\" + zipDir;
				zip_fileinfo zi = { 0 };
				GetZipFileTime(localDir.c_str(), &zi.dosDate);
				if (zipDir.size() > MAX_PATH + 19) {
					error = true;
					break;
				}

				BOOL badChar;
				WideCharToMultiByte(CP_OEMCP, WC_NO_BEST_FIT_CHARS, zipDir.c_str(), -1, bufF, MAX_PATH + 20, nullptr, &badChar);
				int flag = 0;
				if (badChar) {
					flag = 0x800; // UTF
					WideCharToMultiByte(CP_UTF8, 0, zipDir.c_str(), -1, bufF, MAX_PATH + 20, nullptr, nullptr);
				}

				unsigned long calculate_crc = 0;
				const char* passwordCh = nullptr;
				if (password.size() > 0) {
					if (!GetFileCrc(localDir.c_str(), buf, 1024, &calculate_crc)) {
						error = true;
						break;
					}

					passwordCh = password.c_str();
				}

				int err = zipOpenNewFileInZip4_64(zf, bufF, &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION, 0,
					-MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, passwordCh, calculate_crc, 0, flag, 0);
				if (err == ZIP_OK) {
					HANDLE hFile = CreateFile(localDir.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
					if (hFile != INVALID_HANDLE_VALUE) {
						DWORD readed;
						do {
							err = ZIP_OK;
							if (!ReadFile(hFile, buf, 1024, &readed, nullptr)) {
								error = true;
								break;
							}

							if (readed > 0)
								err = zipWriteInFileInZip(zf, buf, readed);
						} while ((err == ZIP_OK) && (readed > 0));
						CloseHandle(hFile);
					}

					if (zipCloseFileInZip(zf) != ZIP_OK) {
						error = true;
						break;
					}
				}
				else {
					error = true;
					break;
				}

				files.pop_front();
			}

			zipClose(zf, nullptr);
		}
		else error = true;
	}

	DeleteDirectoryTreeW(dir.c_str());
	return error;
}

bool UnzipFiles(const std::wstring &dir, std::wstring &zipFilePath, const std::string &password)
{
	bool error = false;
	zlib_filefunc_def pzlib_filefunc_def;
	fill_win32_filefunc(&pzlib_filefunc_def);
	std::wstring fileNameInZip;
	std::map<std::wstring, bool> existingContacts;
	zipFilePath = GetFileName(zipFilePath, L"", existingContacts, true);
	zipFilePath = ReplaceExt(zipFilePath, L"zip");
	unzFile zf = unzOpen2((LPCSTR)(LPTSTR)zipFilePath.c_str(), &pzlib_filefunc_def);
	if (zf == nullptr)
		return true;

	char buf[8192];
	char bufF[MAX_PATH + 20];
	unz_file_info file_info;
	do {
		int err = unzGetCurrentFileInfo(zf, &file_info, bufF, MAX_PATH + 20, buf, 8192, nullptr, 0);
		if (err == UNZ_OK) {
			UINT cp = CP_OEMCP;
			if (file_info.flag & 0x800)// UTF
				cp = CP_UTF8;

			// Get Unicode file name for InfoZip style archives, otherwise assume PKZip/WinZip style
			if (file_info.size_file_extra) {
				char *p = buf;
				unsigned long size = min(file_info.size_file_extra, 8192);
				while (size > 0) {
					unsigned short id = *(unsigned short*)p;
					unsigned len = *(unsigned short*)(p + 2);

					if (size < (len + 4)) break;

					if (id == 0x7075 && len > 5 && (len - 5) < MAX_PATH + 20 && *(p + 4) == 1) {
						memcpy(bufF, p + 9, len - 5);
						bufF[len - 5] = 0;
						cp = CP_UTF8;
						break;
					}
					size -= len + 4;
					p += len + 4;
				}
			}

			int sizeC = (int)mir_strlen(bufF);
			int sizeW = MultiByteToWideChar(cp, 0, bufF, sizeC, nullptr, 0);
			fileNameInZip.resize(sizeW);
			MultiByteToWideChar(cp, 0, bufF, sizeC, (wchar_t*)fileNameInZip.c_str(), sizeW);
			fileNameInZip = dir + L"\\" + fileNameInZip;
			for (size_t i = 0; i < fileNameInZip.length(); ++i)
				if (fileNameInZip[i] == L'/')
					fileNameInZip[i] = L'\\';

			if (file_info.external_fa & FILE_ATTRIBUTE_DIRECTORY)
				CreatePath(fileNameInZip.c_str());
			else {
				const char* passwordCh = nullptr;
				if (password.size() > 0)
					passwordCh = password.c_str();

				err = unzOpenCurrentFilePassword(zf, passwordCh);
				if (err == UNZ_OK) {
					CreatePath(GetDirectoryName(fileNameInZip).c_str());
					HANDLE hFile = CreateFile(fileNameInZip.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, 0, nullptr);
					if (hFile != INVALID_HANDLE_VALUE) {
						DWORD writed;
						for (;;) {
							err = unzReadCurrentFile(zf, buf, 8192);
							if (err <= 0) break;

							if (!WriteFile(hFile, buf, err, &writed, FALSE)) {
								err = -1;
								break;
							}
						}

						CloseHandle(hFile);
						if (err < 0) {
							error = true;
							break;
						}
					}
					else {
						unzCloseCurrentFile(zf);
						error = true;
						break;
					}

					if (unzCloseCurrentFile(zf) != ZIP_OK) {
						error = true;
						break;
					}
				}
				else {
					error = true;
					break;
				}
			}
		}
		else {
			error = true;
			break;
		}
	} while (unzGoToNextFile(zf) == UNZ_OK);

	unzClose(zf);
	return error;
}

bool FtpFiles(const std::wstring& dir, const std::wstring& filePath, const std::wstring& ftpName)
{
	std::list<std::wstring> files;
	std::map<std::wstring, bool> existingContacts;
	ListDirectory(dir, L"\\", files);
	if (files.size() > 0) {
		std::wofstream stream((dir + L"\\script.sc").c_str());
		if (stream.is_open()) {
			std::wstring ftpDir = GetDirectoryName(filePath);
			ftpDir = GetFileName(ftpDir, L"", existingContacts, false);
			stream << "option batch continue\noption confirm off\nopen \""
				<< ftpName << "\"\noption transfer binary\n";
			std::wstring lastCD;
			size_t filSize = files.size();
			while (files.size() > 0) {
				std::wstring localDir = *files.begin();
				std::wstring currentCD = ftpDir + GetDirectoryName(ReplaceStr(localDir, L'\\', L'/'));
				if (currentCD != lastCD) {
					if (!currentCD.empty() && currentCD != L"/")
						stream << "mkdir \"" << currentCD << "\"\n";
					stream << "cd \"" << currentCD << "\"\n";
					lastCD = currentCD;
				}

				std::wstring name = GetName(localDir);
				stream << "call MDTM " << name << "\n";
				stream << "put \"." << localDir << "\"\n";
				stream << "call MDTM " << name << "\n";
				files.pop_front();
			}

			stream.close();
			std::wstring &log = Options::instance->ftpLogPath;
			CreateDirectory(GetDirectoryName(log).c_str(), nullptr);
			DeleteFile(log.c_str());

			wchar_t cmdLine[MAX_PATH];
			mir_snwprintf(cmdLine, L"\"%s\" /nointeractiveinput /log=\"%s\" /script=script.sc", Options::instance->ftpExePath.c_str(), log.c_str());

			STARTUPINFO startupInfo = { 0 };
			startupInfo.cb = sizeof(STARTUPINFO);

			PROCESS_INFORMATION processInfo;
			if (CreateProcess(nullptr, cmdLine, nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &startupInfo, &processInfo)) {
				WaitForSingleObject(processInfo.hProcess, INFINITE);
				CloseHandle(processInfo.hThread);
				CloseHandle(processInfo.hProcess);
				if (log.empty())
					return false;

				std::wifstream logStream(log.c_str());
				if (logStream.is_open()) {
					bool isInMDTM = false;
					std::list<std::wstring> dates;
					while (!logStream.eof()) {
						std::wstring lineStr;
						std::getline(logStream, lineStr);
						if (lineStr.length() > 1) {
							if (lineStr[0] == L'>') {
								if (isInMDTM) {
									if (lineStr.find(L"Script:") < lineStr.length()) {
										dates.push_back(L"");
										isInMDTM = false;
									}
								}

								if (lineStr.find(L"Script: call MDTM") < lineStr.length())
									isInMDTM = true;
							}
							else if (isInMDTM && lineStr[0] == L'<') {
								size_t ss = lineStr.find(L"Script: 213 ");
								if (ss < lineStr.length()) {
									ss += 12;
									if (ss < lineStr.length()) {
										lineStr = lineStr.substr(ss);
										if (lineStr.size() == 14) {
											dates.push_back(lineStr);
											isInMDTM = false;
										}
									}
								}
							}
						}
					}

					if (dates.size() > 0 && dates.size() == filSize * 2) {
						for (std::list<std::wstring>::const_iterator it = dates.begin(); it != dates.end(); ++it) {
							std::wstring date1 = *it++;
							if (it->empty() || date1 == *it)
								return true;
						}

						return false;
					}
				}
			}
		}
	}

	return true;
}

bool FtpGetFiles(const std::wstring& dir, const std::list<std::wstring>& files, const std::wstring& ftpName)
{
	std::wstring script = dir + L"\\script.sc";
	std::wofstream stream(script.c_str());
	if (stream.is_open()) {
		stream << "option batch continue\noption confirm off\nopen \""
			<< ftpName << "\"\noption transfer binary\n";
		std::wstring lastCD;
		std::list<std::wstring> localFiles;
		for (std::list<std::wstring>::const_iterator it = files.begin(); it != files.end(); ++it) {
			std::wstring fileName = GetName(*it);
			localFiles.push_back(dir + L"\\" + fileName);
			std::wstring currentCD = GetDirectoryName(*it);
			if (currentCD != lastCD) {
				stream << "cd \"" << currentCD << "\"\n";
				lastCD = currentCD;
			}

			stream << "get \"" << fileName << "\"\n";
		}

		stream.close();
		std::wstring &log = Options::instance->ftpLogPath;
		CreateDirectory(GetDirectoryName(log).c_str(), nullptr);
		DeleteFile(log.c_str());
		wchar_t cmdLine[MAX_PATH];
		mir_snwprintf(cmdLine, L"\"%s\" /nointeractiveinput /log=\"%s\" /script=script.sc", Options::instance->ftpExePath.c_str(), log.c_str());
		STARTUPINFO				startupInfo = { 0 };
		PROCESS_INFORMATION		processInfo;
		startupInfo.cb = sizeof(STARTUPINFO);
		if (CreateProcess(nullptr, cmdLine, nullptr, nullptr, FALSE, 0, nullptr, dir.c_str(), &startupInfo, &processInfo)) {
			WaitForSingleObject(processInfo.hProcess, INFINITE);
			CloseHandle(processInfo.hThread);
			CloseHandle(processInfo.hProcess);
		}

		DeleteFile(script.c_str());
		for (std::list<std::wstring>::const_iterator it = localFiles.begin(); it != localFiles.end(); ++it) {
			uint32_t atr = GetFileAttributes(it->c_str());
			if (atr == INVALID_FILE_ATTRIBUTES || atr & FILE_ATTRIBUTE_DIRECTORY)
				return true;
		}

		return false;
	}

	return true;
}

void CreatePath(const wchar_t *szDir)
{
	if (!szDir) return;

	uint32_t dwAttributes;
	wchar_t *pszLastBackslash, szTestDir[MAX_PATH];

	mir_wstrncpy(szTestDir, szDir, _countof(szTestDir));
	if ((dwAttributes = GetFileAttributes(szTestDir)) != INVALID_FILE_ATTRIBUTES && (dwAttributes & FILE_ATTRIBUTE_DIRECTORY))
		return;

	pszLastBackslash = wcsrchr(szTestDir, '\\');
	if (pszLastBackslash == nullptr)
		return;

	*pszLastBackslash = '\0';
	CreatePath(szTestDir);
	*pszLastBackslash = '\\';

	CreateDirectory(szTestDir, nullptr);
}

INT_PTR ExecuteTaskService(WPARAM wParam, LPARAM)
{
	{
		mir_cslock lck(Options::instance->criticalSection);
		int taskNr = (int)wParam;
		if (taskNr < 0 || taskNr >= (int)Options::instance->taskOptions.size())
			return FALSE;

		Options::instance->taskOptions[taskNr].forceExecute = true;
		Options::instance->taskOptions[taskNr].showMBAfterExecute = true;
	}
	StartThread(false);
	return TRUE;
}

void DoError(const TaskOptions& to, const std::wstring _error)
{
	wchar_t msg[256];
	mir_snwprintf(msg, TranslateT("Task '%s' execution failed:"), to.taskName.c_str());
	if (Options::instance->schedulerHistoryAlerts) {
		std::wstring error = msg;
		error += L"\n";
		error += _error;

		DBEVENTINFO dbei = {};
		dbei.szModule = MODULENAME;
		dbei.flags = DBEF_UTF | DBEF_READ;
		dbei.timestamp = time(0);
		// For now I do not convert event data from string to blob, and event type must be message to handle it properly
		dbei.eventType = EVENTTYPE_MESSAGE;
		int len = (int)error.length() + 1;
		dbei.cbBlob = WideCharToMultiByte(CP_UTF8, 0, error.c_str(), len, nullptr, 0, nullptr, nullptr);
		char* buf = new char[dbei.cbBlob];
		dbei.cbBlob = WideCharToMultiByte(CP_UTF8, 0, error.c_str(), len, buf, dbei.cbBlob, nullptr, nullptr);
		dbei.pBlob = (uint8_t*)buf;
		db_event_add(NULL, &dbei);
	}

	if (Options::instance->schedulerAlerts) {
		if (Miranda_IsTerminated())
			return;

		ShowClassPopupW(MODULENAME, msg, _error.c_str());
	}
}
