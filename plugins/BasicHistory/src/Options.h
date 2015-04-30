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

#pragma once
#include "IImport.h"
#include "IExport.h"

struct FilterOptions
{
	FilterOptions()
		:onlyIncomming(false),
		onlyOutgoing(false)
	{
	}

	FilterOptions(const std::wstring& _name)
		:name(_name),
		onlyIncomming(false),
		onlyOutgoing(false)
	{
	}

	std::wstring name;
	std::vector<int> events;
	bool onlyIncomming;
	bool onlyOutgoing;
};

struct TaskOptions
{
	bool compress;
	bool useFtp;
	bool isSystem;
	bool active;
	bool forceExecute;
	bool showMBAfterExecute;
	bool exportImported;
	enum TaskType
	{
		Export,
		Delete,
		ExportAndDelete,
		Import,
		ImportAndMarge
	} type;

	enum EventUnit
	{
		Minute,
		Hour,
		Day
	} eventUnit;

	enum TrigerType
	{
		AtStart,
		AtEnd,
		Daily, 
		Weekly,
		Monthly,
		DeltaMin,
		DeltaHour
	} trigerType;

	IExport::ExportType exportType;
	IImport::ImportType importType;
	int eventDeltaTime;
	int filterId;
	int dayTime;
	int dayOfWeek;
	int dayOfMonth;
	int deltaTime;
	int orderNr;
	time_t lastExport;
	std::wstring ftpName;
	std::wstring filterName;
	std::wstring filePath;
	std::wstring taskName;
	std::string zipPassword; // char* because zip file using ANSI password
	std::vector<MCONTACT> contacts;
	TaskOptions()
	{
		forceExecute = false;
		showMBAfterExecute = false;
		exportImported = true;
		type = Export;
		eventUnit = Hour;
		trigerType = AtStart;
		exportType = IExport::RichHtml;
		importType = IImport::Binary;
		eventDeltaTime = 0;
		filterId = 0;
		compress = true;
		useFtp = false;
		isSystem = false;
		active = true;
		dayTime = 20 * 60;
		dayOfWeek = 0;
		dayOfMonth = 1;
		deltaTime = 24;
		orderNr = 0;
		TCHAR buf[MAX_PATH];
		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, buf)))
		{
			filePath = buf;
			filePath += _T("\\");
		}

		filePath += TranslateT("History");
		filePath += _T("_<contact>_<date>.<ext>");
		lastExport = time(NULL);
	}
};

class Options
{
private:
	std::vector<FilterOptions> customFiltersTemp;
public:
	Options();
	~Options();
	static int InitOptions(WPARAM wParam, LPARAM lParam);

	static INT_PTR CALLBACK DlgProcOptsMain(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsGroupList(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsMessages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsSearching(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsExport(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsScheduler(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsTask(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static bool FTPAvail();

	static Options *instance;
	void Save();
	void SaveTasks(std::list<TaskOptions>* tasks);
	void SaveTaskTime(TaskOptions& to);
	void Load(void);
	void LoadTasks();

	bool showContacts, showContactGroups, noFindBorder, groupNewOnTop, groupShowEvents, groupShowTime, groupShowName, groupShowMessage;
	bool messagesNewOnTop, messagesShowDate, messagesShowSec, messagesShowName, messagesShowEvents, messagesUseSmileys;
	bool searchForInList, searchForInMess, searchMatchCase, searchMatchWhole, searchOnlyIn, searchOnlyOut, searchOnlyGroup, searchAllContacts;
	bool schedulerAlerts, schedulerHistoryAlerts;
	int groupMessageLen, groupTime, groupMessagesNumber;

	std::vector<FilterOptions> customFilters;
	int defFilter;
	unsigned int codepageTxt, codepageHtml1, codepageHtml2;
	std::wstring encodingTxt, encodingHtml1, encodingHtml2;
	bool exportHtml1ShowDate, exportHtml2ShowDate, exportHtml2UseSmileys;
	std::wstring extCssHtml2;
	std::vector<TaskOptions> taskOptions;
	std::wstring ftpLogPath;
	std::wstring ftpExePathDef;
	std::wstring ftpExePath;
	mir_cs criticalSection;

	enum Fonts
	{
		OutTimestamp = 0,
		InTimestamp,
		OutName,
		InName,
		OutMessages,
		InMessages,
		GroupList,
		Find,
	};

	enum Colors
	{
		OutBackground = 0,
		InBackground,
		GroupListBackground,
		WindowBackground,
		ContactListBackground,
		FindBackground,
	};

	COLORREF GetFont(Fonts fontId, PLOGFONT font);
	COLORREF GetColor(Colors colorId);
};

