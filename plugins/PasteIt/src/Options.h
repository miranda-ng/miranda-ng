/*
Paste It plugin
Copyright (C) 2011 Krzysztof Kral

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
#include "PasteToWeb.h"

class WebOptions
{
public:
	WebOptions(int serv);
	virtual ~WebOptions();
	virtual WebOptions* Copy();
	int server;
	bool isSendFileName;
	bool isPublicPaste;
	bool isCombo1;
	std::list<PasteFormat> combo1Values;
	std::wstring combo1Desc;
	bool isPastebin;

	std::list<PasteFormat> formats;
	std::wstring defFormatId;
	bool sendFileName;
	bool publicPaste;
	std::wstring combo1;
	std::wstring pastebinUserKey;
};

class Options
{
public:
	Options();
	~Options();
	static int InitOptions(WPARAM wParam, LPARAM lParam);

	static INT_PTR CALLBACK DlgProcOptsMain(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsPages(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsConfigure(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK DlgProcOptsLogin(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

	static void InitCodepageCB(HWND hwndCB, unsigned int codepage);
	static void SetCodepageCB(HWND hwndCB, unsigned int codepage);
	static unsigned int GetCodepageCB(HWND hwndCB, bool errorReport, unsigned int defCp);

	static Options *instance;
	void Save();
	void Load();

	int defWeb;
	unsigned int codepage;
	bool autoUTF;
	bool confDlg;
	bool autoSend;
	void SetDefWeb(int web);
	WebOptions* webOptions[PasteToWeb::pages];
};

