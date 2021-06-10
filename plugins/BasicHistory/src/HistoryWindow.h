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
#include "Searcher.h"

class HistoryWindow : public SearchContext
{
private:
	HistoryWindow(MCONTACT _hContact);
	void Initialise();
	void Destroy();
	void SplitterMoved(HWND splitter, LONG pos, bool screenPos);
	static INT_PTR CALLBACK DlgProcHistory(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static int HistoryDlgResizer(HWND, LPARAM, UTILRESIZECONTROL *urc);
	static void FillHistoryThread(HistoryWindow *hInfo);
	static void Close(HistoryWindow* historyWindow);
	static void ChangeToFreeWindow(HistoryWindow* historyWindow);
	void ReplaceIcons(HWND hwndDlg, int selStart, BOOL isSent);
	void EnableWindows(BOOL enable);
	void ReloadContacts();
	bool DoHotkey(UINT msg, LPARAM lParam, WPARAM wParam, int window);
	void RestorePos();
	void SavePos(bool all);
	void FindToolbarClicked(LPNMTOOLBAR lpnmTB);
	void ConfigToolbarClicked(LPNMTOOLBAR lpnmTB);
	void DeleteToolbarClicked(LPNMTOOLBAR lpnmTB);
	void Delete(int what);
	bool ContactChanged(bool sync = false);
	void GroupImagesChanged();
	void FormatQuote(std::wstring& quote, const MessageData& md, const std::wstring& msg);
	void FontsChanged();
	void ReloadMainOptions();
	void DoImport(IImport::ImportType type);

	static std::map<MCONTACT, HistoryWindow*> windows;
	static std::vector<HistoryWindow*> freeWindows;
	bool isDestroyed;
	LONG splitterY;
	LONG splitterOrgY;
	LONG splitterX;
	LONG splitterOrgX;
	HICON *eventIcons;
	int allIconNumber;
	HICON plusIco, minusIco, findNextIco, findPrevIco, configIco, deleteIco;
	bool isContactList;
	LONG listOryginalPos;
	bool isLoading;
	Searcher searcher;
	bool isGroupImages;
	HIMAGELIST himlSmall, himlNone;
	HBRUSH bkBrush;
	HBRUSH bkFindBrush;
	MCONTACT hSystem;
	HWND splitterXhWnd, splitterYhWnd;
	bool isStartSelect;
protected:
	virtual void AddGroup(bool isMe, const std::wstring &time, const std::wstring &user, const std::wstring &eventText, int ico);
public:
	~HistoryWindow();
	static void Deinit();
	static void Open(MCONTACT hContact);
	static void Close(MCONTACT hContact);
	static void RebuildEvents(MCONTACT hContact);
	static bool IsInList(HWND hWnd);
	static int FontsChanged(WPARAM wParam, LPARAM lParam);
	static void OptionsMainChanged();
	static void OptionsGroupChanged();
	static void OptionsSearchingChanged();
	void Show();
	void Focus();

	// SearchContext interface
	virtual void SelectEventGroup(int sel);
	virtual MCONTACT GetNextContact(MCONTACT hContact, int adder);
	virtual void SelectContact(MCONTACT _hContact);
};

