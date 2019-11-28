#include "stdafx.h"

#define MSG_WND_CLASS L"MIM_SNMsgWindow"

HWND HKHwnd;

enum KB_ACTIONS {KB_NEW_NOTE = 1, KB_TOGGLE_NOTES, KB_VIEW_NOTES, KB_NEW_REMINDER, KB_VIEW_REMINDERS };

void RegisterKeyBindings()
{
	HOTKEYDESC desc = {};
	desc.szSection.w = _A2W(SECTIONNAME);
	desc.dwFlags = HKD_UNICODE;

	desc.pszName = MODULENAME"/NewNote";
	desc.szDescription.w = LPGENW("New Note");
	desc.lParam = KB_NEW_NOTE;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_INSERT);
	desc.pszService = MS_NOTES_NEW;
	g_plugin.addHotkey(&desc);

	desc.pszName = MODULENAME"/ToggleNotesVis";
	desc.szDescription.w = LPGENW("Toggle Notes Visibility");
	desc.lParam = KB_TOGGLE_NOTES;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_ADD);
	desc.pszService = MS_NOTES_SHOWHIDE;
	g_plugin.addHotkey(&desc);

	desc.pszName = MODULENAME"/BringNotesFront";
	desc.szDescription.w = LPGENW("Bring All Notes to Front");
	desc.lParam = KB_VIEW_NOTES;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_HOME);
	desc.pszService = MS_NOTES_DISPLAY;
	g_plugin.addHotkey(&desc);

	desc.pszName = MODULENAME"/NewReminder";
	desc.szDescription.w = LPGENW("New Reminder");
	desc.lParam = KB_NEW_REMINDER;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL|HOTKEYF_SHIFT, VK_SUBTRACT);
	desc.pszService = MS_REMINDER_NEW;
	g_plugin.addHotkey(&desc);

	desc.pszName = MODULENAME"/ViewReminders";
	desc.szDescription.w = LPGENW("View Reminders");
	desc.lParam = KB_VIEW_REMINDERS;
	desc.DefHotKey = HOTKEYCODE(HOTKEYF_CONTROL | HOTKEYF_SHIFT, VK_DIVIDE);
	desc.pszService = MS_REMINDER_VIEW;
	g_plugin.addHotkey(&desc);
}

LRESULT CALLBACK NotifyHotKeyWndProc(HWND AHwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message) {
	case WM_TIMER:
		KillTimer(HKHwnd, 1026);
		BOOL b = CheckRemindersAndStart();
		SetTimer(HKHwnd, 1026, b ? REMINDER_UPDATE_INTERVAL_SHORT : REMINDER_UPDATE_INTERVAL, nullptr);

		return FALSE;
	}

	return DefWindowProc(AHwnd, Message, wParam, lParam);
}

void CreateMsgWindow(void)
{
	WNDCLASSEX TWC = {};
	TWC.cbSize = sizeof(TWC);
	TWC.hInstance = hmiranda;
	TWC.lpszClassName = MSG_WND_CLASS;
	TWC.lpfnWndProc = NotifyHotKeyWndProc;
	RegisterClassEx(&TWC);

	HKHwnd = CreateWindowExW(WS_EX_TOOLWINDOW, MSG_WND_CLASS, L"StickyNotes", 0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, hmiranda, nullptr);
	SetTimer(HKHwnd, 1026, REMINDER_UPDATE_INTERVAL, nullptr);
}

void DestroyMsgWindow(void)
{
	KillTimer(HKHwnd, 1026);
	DestroyWindow(HKHwnd);
}
