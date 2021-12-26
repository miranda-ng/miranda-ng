#include "stdafx.h"

HistoryMap history_map;

#define ID_REPAINT_TIMER		10101

struct WindowData {
	uint32_t item_id;
	HistoryList list;
	HWND hwnd_chk_grid;
	HWND hwnd_chk_stat;
	bool show_grid;
	bool show_stat;
};

#define WM_REBUILDLIST			(WM_USER + 5)

LRESULT CALLBACK GraphWindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_REBUILDLIST:
	{
		WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

		bool found = false;
		mir_cslock lck(data_list_cs);
		for (pinglist_it i = data_list.begin(); i != data_list.end(); ++i) {
			if (i->item_id == wd->item_id) {
				wd->list = history_map[wd->item_id];
				found = true;
				break;
			}
		}

		if (!found) {
			PostMessage(hwnd, WM_CLOSE, 0, 0);
			return TRUE;
		}

		InvalidateRect(hwnd, nullptr, FALSE);
	}
	return TRUE;
	case WM_SHOWWINDOW:
		if (wParam == TRUE && lParam == 0) {
			WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			if (wd->hwnd_chk_grid == nullptr) {
				wd->hwnd_chk_grid = CreateWindow(L"BUTTON", TranslateT("Show grid lines"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, nullptr, g_plugin.getInst(), nullptr);
				SendMessage(wd->hwnd_chk_grid, BM_SETCHECK, wd->show_grid ? BST_CHECKED : BST_UNCHECKED, 0);
			}
			if (wd->hwnd_chk_stat == nullptr) {
				wd->hwnd_chk_stat = CreateWindow(L"BUTTON", TranslateT("Show stats"), WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 0, 0, 0, 0, hwnd, nullptr, g_plugin.getInst(), nullptr);
				SendMessage(wd->hwnd_chk_stat, BM_SETCHECK, wd->show_stat ? BST_CHECKED : BST_UNCHECKED, 0);
			}
			KillTimer(hwnd, ID_REPAINT_TIMER);
#ifdef min
			SetTimer(hwnd, ID_REPAINT_TIMER, min(options.ping_period * 1000, 5000), nullptr);
#else
			SetTimer(hwnd, ID_REPAINT_TIMER, std::min(options.ping_period * 1000, 5000), 0);
#endif

			SendMessage(hwnd, WM_REBUILDLIST, 0, 0);
		}
		break;
	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED) {
			WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
			if ((HWND)lParam == wd->hwnd_chk_grid) {
				wd->show_grid = (SendMessage(wd->hwnd_chk_grid, BM_GETCHECK, 0, 0) == BST_CHECKED);
			}
			else if ((HWND)lParam == wd->hwnd_chk_stat) {
				wd->show_stat = (SendMessage(wd->hwnd_chk_stat, BM_GETCHECK, 0, 0) == BST_CHECKED);
			}
			InvalidateRect(hwnd, nullptr, TRUE);
		}
		return TRUE;
	case WM_TIMER:
	{
		SendMessage(hwnd, WM_REBUILDLIST, 0, 0);
	}
	return TRUE;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		RECT r;
		WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		if (wd && (hdc = BeginPaint(hwnd, &ps)) != nullptr)
		{
			GetClientRect(hwnd, &r);
			FillRect(hdc, &r, GetSysColorBrush(COLOR_WINDOW));

			short max_value = -1, min_value = (short)0x7FFF,
				graph_height = 0; // this is minimum graph height, in ms

			double avg = 0;
			for (HistoryList::Iterator hli = wd->list.begin(); hli.has_val(); hli.next())
			{
				if (hli.val().first > max_value)
					max_value = hli.val().first;
				if (hli.val().first >= 0 && hli.val().first < min_value)
					min_value = hli.val().first;
				avg += hli.val().first;
			}

			if (wd->list.size())
				avg /= wd->list.size();

			graph_height = (int)(max_value * 1.2f);

			if (graph_height < MIN_GRAPH_HEIGHT)
				graph_height = MIN_GRAPH_HEIGHT;

#ifdef max
			float unit_width = (r.right - r.left) / (float)max((int)wd->list.size(), MIN_BARS), // space for at least MIN_BARS bars
#else
			float unit_width = (r.right - r.left) / (float)std::max((int)wd->list.size(), MIN_BARS), // space for at least MIN_BARS bars
#endif
				unit_height = (r.bottom - r.top) / (float)graph_height;

			time_t last_time = 0, time, start_time = 0;
			if (wd->list.size())
				start_time = wd->list.begin().val().second;

			RECT bar;
			bar.bottom = r.bottom;
			float x = r.right - (unit_width * wd->list.size() + 0.5f);
			bar.left = (int)(x + 0.5f);
			bar.right = (int)(x + unit_width + 0.5f);

			// set up pen for horiz (ping time) and vert (time covered by graph) lines
			HPEN hPenOld, hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
			hPenOld = (HPEN)SelectObject(hdc, hPen);

			for (HistoryList::Iterator hi = wd->list.begin(); hi.has_val(); hi.next())
			{
				if (hi.val().first != -1)
				{
					bar.top = bar.bottom - (int)(hi.val().first * unit_height + 0.5f);
					FillRect(hdc, &bar, GetSysColorBrush(COLOR_HOTLIGHT));
				}

				time = hi.val().second - start_time;

				if (time / MARK_PERIOD != last_time / MARK_PERIOD)
				{ // new minute
					MoveToEx(hdc, bar.left, r.bottom, nullptr);
					LineTo(hdc, bar.left, r.top);
				}

				last_time = time;

				x += unit_width;
				bar.left = bar.right;
				bar.right = (int)(x + unit_width + 0.5f);
			}

			if (wd->show_grid)
			{
				// draw horizontal lines to mark every 100ms
				for (int li = 0; li < graph_height; li += MARK_TIME)
				{
					MoveToEx(hdc, r.left, r.bottom - (int)(li * unit_height + 0.5f), nullptr);
					LineTo(hdc, r.right, r.bottom - (int)(li * unit_height + 0.5f));
				}
			}

			HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
			if (wd->show_stat)
			{
				SelectObject(hdc, hPen2);
				MoveToEx(hdc, r.left, r.bottom - (int)(avg * unit_height + 0.5f), nullptr);
				LineTo(hdc, r.right, r.bottom - (int)(avg * unit_height + 0.5f));
				if (max_value != avg)
				{
					MoveToEx(hdc, r.left, r.bottom - (int)(max_value * unit_height + 0.5f), nullptr);
					LineTo(hdc, r.right, r.bottom - (int)(max_value * unit_height + 0.5f));
					MoveToEx(hdc, r.left, r.bottom - (int)(min_value * unit_height + 0.5f), nullptr);
					LineTo(hdc, r.right, r.bottom - (int)(min_value * unit_height + 0.5f));
				}
			}

			// restore pen
			SelectObject(hdc, hPenOld);
			DeleteObject(hPen);
			DeleteObject(hPen2);

			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, GetSysColor(COLOR_3DDKSHADOW));
			wchar_t buff[64];
			if (wd->show_grid)
			{
				mir_snwprintf(buff, TranslateT("%d ms"), MARK_TIME);
				TextOut(hdc, r.right - 100, r.bottom - (int)(unit_height * MARK_TIME + 0.5f), buff, (int)mir_wstrlen(buff));
			}

			if (wd->show_stat)
			{
				SetTextColor(hdc, RGB(255, 0, 0));
				mir_snwprintf(buff, TranslateT("AVG %.1lf ms"), avg);
				TextOut(hdc, r.left + 10, r.bottom - (int)(avg * unit_height + 0.5f), buff, (int)mir_wstrlen(buff));
				if (max_value != avg) {
					mir_snwprintf(buff, TranslateT("MAX %hd ms"), max_value);
					TextOut(hdc, r.left + 10, r.bottom - (int)(max_value * unit_height + 0.5f), buff, (int)mir_wstrlen(buff));
					mir_snwprintf(buff, TranslateT("MIN %hd ms"), min_value);
					TextOut(hdc, r.left + 10, r.bottom - (int)(min_value * unit_height + 0.5f), buff, (int)mir_wstrlen(buff));
				}
			}

			EndPaint(hwnd, &ps);
		}
	}
	return TRUE;

	case WM_ERASEBKGND:
		return TRUE;

	case WM_SIZE:
	{
		WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		RECT r;
		GetClientRect(hwnd, &r);
		if (wd->hwnd_chk_grid != nullptr) SetWindowPos(wd->hwnd_chk_grid, nullptr, r.right - 150, r.top + 10, 120, 20, SWP_NOZORDER | SWP_NOACTIVATE);
		if (wd->hwnd_chk_stat != nullptr) SetWindowPos(wd->hwnd_chk_stat, nullptr, r.right - 150, r.top + 30, 120, 20, SWP_NOZORDER | SWP_NOACTIVATE);
	}
	InvalidateRect(hwnd, nullptr, FALSE);
	break;
	case WM_CLOSE:
	{
		KillTimer(hwnd, ID_REPAINT_TIMER);
		WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		{
			char buff[30];
			mir_snprintf(buff, "pinggraphwnd%d", wd->item_id);
			Utils_SaveWindowPosition(hwnd, 0, MODULENAME, buff);
		}
	}
	break;
	case WM_DESTROY:
	{
		WindowData *wd = (WindowData *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		{
			char buff[30];
			mir_snprintf(buff, "WindowHandle%d", wd->item_id);
			g_plugin.setDword(buff, 0);
		}
		delete wd;
	}
	// drop through
	};

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

INT_PTR ShowGraph(WPARAM wParam, LPARAM lParam) {
	char buff[30];
	mir_snprintf(buff, "WindowHandle%d", (uint32_t)wParam);
	HWND hGraphWnd = (HWND)g_plugin.getDword(buff, 0);
	if (hGraphWnd) {
		ShowWindow(hGraphWnd, SW_SHOW);
		SetWindowPos(hGraphWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return 0;
	}

	WNDCLASS wndclass;
	wndclass.style = 0;
	wndclass.lpfnWndProc = GraphWindowProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = g_plugin.getInst();
	wndclass.hIcon = hIconResponding;
	wndclass.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
	wndclass.lpszMenuName = nullptr;
	wndclass.lpszClassName = _A2W(MODULENAME) L"GraphWindow";
	RegisterClass(&wndclass);

	wchar_t title[256];
	mir_wstrncpy(title, TranslateT("Ping Graph"), _countof(title));
	if (lParam) {
		mir_wstrncat(title, L" - ", _countof(title) - mir_wstrlen(title));
		mir_wstrncat(title, (wchar_t *)lParam, _countof(title) - mir_wstrlen(title));
	}

	HWND parent = nullptr;
	hGraphWnd = CreateWindowEx(0, _A2W(MODULENAME) L"GraphWindow", title,
		(WS_THICKFRAME | WS_CAPTION | WS_SYSMENU | WS_CLIPCHILDREN) & ~CS_VREDRAW & ~CS_HREDRAW,
		0, 0, 800, 600, parent, nullptr, g_plugin.getInst(), nullptr);

	WindowData *wd = new WindowData;
	wd->item_id = (uint32_t)wParam; // wParam is destination id
	wd->hwnd_chk_grid = nullptr;
	wd->hwnd_chk_stat = nullptr;
	wd->show_grid = g_plugin.getByte("ShowGridLines", 0) ? true : false;
	wd->show_stat = g_plugin.getByte("ShowStats", 1) ? true : false;

	g_plugin.setDword(buff, (UINT_PTR)hGraphWnd);

	SetWindowLongPtr(hGraphWnd, GWLP_USERDATA, (LONG_PTR)wd);

	mir_snprintf(buff, "pinggraphwnd%d", wd->item_id);
	Utils_RestoreWindowPosition(hGraphWnd, 0, MODULENAME, buff);

	if (!IsWindowVisible(hGraphWnd))
		ShowWindow(hGraphWnd, SW_SHOW);

	return 0;
}

// save window positions, close windows
void graphs_cleanup() {
	int list_size = GetListSize(0, 0);
	char buff[64];
	HWND hwnd;

	for (int i = 0; i < list_size; i++) {
		mir_snprintf(buff, "WindowHandle%d", i);
		if (hwnd = (HWND)g_plugin.getDword(buff, 0)) {
			DestroyWindow(hwnd);
			g_plugin.setDword(buff, 0);
			mir_snprintf(buff, "WindowWasOpen%d", i);
			g_plugin.setByte(buff, 1);
		}
	}
}

// remove old data, possibly left from a crash
void graphs_init() {
	PINGLIST pl;
	char buff[64];
	CallService(MODULENAME "/GetPingList", 0, (LPARAM)&pl);
	for (pinglist_it i = pl.begin(); i != pl.end(); ++i) {
		mir_snprintf(buff, "WindowHandle%d", i->item_id); // clean up from possible crash
		g_plugin.setDword(buff, 0);
		mir_snprintf(buff, "WindowWasOpen%d", i->item_id); // restore windows that were open on shutdown
		if (g_plugin.getByte(buff, 0)) {
			g_plugin.setByte(buff, 0);
			ShowGraph((WPARAM)i->item_id, (LPARAM)i->pszLabel);
		}
	}
}
