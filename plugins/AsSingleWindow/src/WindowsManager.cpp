#include "stdafx.h"
#include "WindowsManager.h"
#include "AsSingleWindow.h"

void sWindowInfo::saveState()
{
	WINDOWPLACEMENT wndPlace;
	wndPlace.length = sizeof(wndPlace);
	if (!GetWindowPlacement(this->hWnd, &wndPlace))
		return;

	switch (wndPlace.showCmd) {
	case SW_HIDE:
		this->eState = WINDOW_STATE_HIDDEN;
		break;

	case SW_MINIMIZE:
	case SW_SHOWMINIMIZED:
	case SW_SHOWMINNOACTIVE:
		this->eState = WINDOW_STATE_MINIMIZED;
		break;

	case SW_MAXIMIZE:
	case SW_RESTORE:
	case SW_SHOW:
	case SW_SHOWNA:
	case SW_SHOWNOACTIVATE:
	case SW_SHOWNORMAL:
		this->eState = WINDOW_STATE_NORMAL;
		break;
	}
}

void sWindowInfo::saveRect()
{
	switch (this->eState) {
	case WINDOW_STATE_HIDDEN:
	case WINDOW_STATE_MINIMIZED:
		WINDOWPLACEMENT wndPlace;
		wndPlace.length = sizeof(wndPlace);
		if (GetWindowPlacement(this->hWnd, &wndPlace))
			this->rLastSavedPosition = wndPlace.rcNormalPosition;
		break;

	default:
		GetWindowRect(this->hWnd, &this->rLastSavedPosition);
		break;
	}
}

void pluginSetProgress()
{
	EnterCriticalSection(&pluginVars.m_CS);
	pluginVars.IsUpdateInProgress = true;
	LeaveCriticalSection(&pluginVars.m_CS);
}

void pluginSetDone()
{
	EnterCriticalSection(&pluginVars.m_CS);
	pluginVars.IsUpdateInProgress = false;
	LeaveCriticalSection(&pluginVars.m_CS);
}

bool pluginIsAlreadyRunning()
{
	EnterCriticalSection(&pluginVars.m_CS);
	bool result = pluginVars.IsUpdateInProgress;
	LeaveCriticalSection(&pluginVars.m_CS);
	return result;
}

/**
 * Поиск окна в списке
 * возвращается указатель на структуру с информацией
 */
sWindowInfo* windowFind(HWND hWnd)
{
	for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr)
		if (itr->hWnd == hWnd)
			return &*itr;

	return nullptr;
}

/**
 * Поиск окна в списке
 * возвращается итератор
 */
windowsList::iterator windowFindItr(HWND hWnd)
{
	for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr)
		if (itr->hWnd == hWnd)
			return itr;

	return pluginVars.allWindows.end();
}

/**
 * Поиск окна в списке
 * возвращается реверсный итератор
 */
windowsList::reverse_iterator windowFindRevItr(HWND hWnd)
{
	for (auto ritr = pluginVars.allWindows.rbegin(); ritr != pluginVars.allWindows.rend(); ++ritr)
		if (ritr->hWnd == hWnd)
			return ritr;

	return pluginVars.allWindows.rend();
}

/**
 * Добавление окна в список окон и выставление всех начальных значений
 * здесь же выставляется хук на wndProc
 */
void windowAdd(HWND hWnd, bool IsMain)
{
	sWindowInfo thisWindowInfo;

	hWnd = windowGetRoot(hWnd);

	// Если окно уже есть в списке (могло быть спрятано)
	if (sWindowInfo* wndInfo = windowFind(hWnd)) {
		wndInfo->eState = WINDOW_STATE_NORMAL; // если окно пряталось ранее
		windowReposition(hWnd);
		return;
	}

	thisWindowInfo.hWnd = hWnd;
	thisWindowInfo.eState = WINDOW_STATE_NORMAL;
	mir_subclassWindow(hWnd, wndProcSync);

	pluginVars.allWindows.push_back(thisWindowInfo);

	if (IsMain)
		pluginVars.contactListHWND = hWnd;

	windowReposition(hWnd);
}

/**
 * Поиск окна верхнего уровня
 */
HWND windowGetRoot(HWND hWnd)
{
	HWND hWndParent = GetParent(hWnd);
	while (IsWindow(hWndParent) && (hWndParent != GetDesktopWindow())) // IsWindowVisible() ?
	{
		hWnd = hWndParent;
		hWndParent = GetParent(hWnd);
	}
	return hWnd;
}

void windowListUpdate()
{
	bool isRemoved = false;
	windowsList::iterator itr = pluginVars.allWindows.begin();
	while (itr != pluginVars.allWindows.end())
		// Не удаляем КЛ в принципе, нет необходимости
		if (!IsWindow(itr->hWnd) && itr->hWnd != pluginVars.contactListHWND) {
			itr = pluginVars.allWindows.erase(itr);
			isRemoved = true;
		}
		else
			++itr;

	if (isRemoved)
		if (!pluginVars.allWindows.empty())
			// TODO: разобраться, почему после этого КЛ пропадает в трей
			allWindowsMoveAndSize(pluginVars.contactListHWND);
}

/**
 * Установка стартовых координат и размера окна
 * базируется на основе координат другого окна в списке
 */
void windowReposition(HWND hWnd)
{
	// TODO: Подумать, нужен ли тут hWnd вообще
	RECT prevWindowPos;

	hWnd = windowGetRoot(hWnd);

	// TODO: Проверить, возможно нужен выход из цикла
	if (sWindowInfo* wndInfo = windowFind(hWnd)) {
		for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr)
			if (itr->hWnd != hWnd) // TODO: очень странная логика
				if (GetWindowRect(itr->hWnd, &prevWindowPos)) {
					SendMessage(itr->hWnd, WM_MOVE, 0, MAKELPARAM(prevWindowPos.left, prevWindowPos.top));
					break;
				}
	}
	else if (!pluginVars.allWindows.empty()) {
		auto itr = pluginVars.allWindows.begin();
		if (GetWindowRect(itr->hWnd, &prevWindowPos))
			SendMessage(itr->hWnd, WM_MOVE, 0, MAKELPARAM(prevWindowPos.left, prevWindowPos.top));
	}
}

/**
 * Перемещение и ресайз всех окон списка
 * hWnd - окно-источник события перемещения/ресайза
 */
void allWindowsMoveAndSize(HWND hWnd)
{
	// Если склеивание отключено
	if (pluginVars.Options.DrivenWindowPos == ASW_CLWINDOWPOS_DISABLED)
		return;

	// Окно должно быть в списке и иметь нормальное состояние
	if (sWindowInfo* wndInfo = windowFind(hWnd)) {
		if (wndInfo->eState != WINDOW_STATE_NORMAL)
			return;
	}
	else
		return;

	// Отключаем связь, если окон больше двух и выбрана соотв. опция
	if (pluginVars.Options.WindowsMerging == ASW_WINDOWS_MERGEDISABLE)
		if (pluginVars.allWindows.size() > 2)
			return;

	// Просмотр окон от текущего до конца
	windowsList::iterator itrC = windowFindItr(hWnd);
	if (itrC != pluginVars.allWindows.end()) {
		windowsList::iterator itrN = ++windowFindItr(hWnd);
		for (; itrC != pluginVars.allWindows.end(), itrN != pluginVars.allWindows.end(); ++itrC, ++itrN) {
			// Режим только двух окон
			if (pluginVars.Options.WindowsMerging == ASW_WINDOWS_MERGEONE)
				if ((itrC->hWnd != pluginVars.contactListHWND) && (itrN->hWnd != pluginVars.contactListHWND))
					continue;

			// itrC проверяется в начале функции
			UINT incCount = 0;
			bool isItrInList = true;
			while (itrN->eState != WINDOW_STATE_NORMAL) {
				++itrN;
				++incCount;
				if (itrN == pluginVars.allWindows.end()) {
					isItrInList = false;
					break;
				}
			}
			if (!isItrInList)
				break;

			sWndCoords wndCoord;
			if (calcNewWindowPosition(itrC->hWnd, itrN->hWnd, &wndCoord, (pluginVars.Options.DrivenWindowPos == ASW_CLWINDOWPOS_RIGHT) ? WINDOW_POSITION_RIGHT : WINDOW_POSITION_LEFT))
				SetWindowPos(itrN->hWnd, itrC->hWnd, wndCoord.x, wndCoord.y, wndCoord.width, wndCoord.height, SWP_NOACTIVATE);

			itrN->saveRect();

			for (; incCount != 0; incCount--)
				++itrC;
		}
	}

	// Просмотр окон от текущего до начала
	windowsList::reverse_iterator ritrC = windowFindRevItr(hWnd);
	if (ritrC != pluginVars.allWindows.rend()) {
		windowsList::reverse_iterator ritrN = ++windowFindRevItr(hWnd);
		for (; ritrC != pluginVars.allWindows.rend(), ritrN != pluginVars.allWindows.rend(); ++ritrC, ++ritrN) {
			// Режим только двух окон
			if (pluginVars.Options.WindowsMerging == ASW_WINDOWS_MERGEONE)
				if ((ritrC->hWnd != pluginVars.contactListHWND) && (ritrN->hWnd != pluginVars.contactListHWND))
					continue;

			UINT incCount = 0;
			bool isItrInList = true;
			while (ritrN->eState != WINDOW_STATE_NORMAL) {
				++ritrN;
				++incCount;
				if (ritrN == pluginVars.allWindows.rend()) {
					isItrInList = false;
					break;
				}
			}
			if (!isItrInList)
				break;

			sWndCoords wndCoord;
			if (calcNewWindowPosition(ritrC->hWnd, ritrN->hWnd, &wndCoord, (pluginVars.Options.DrivenWindowPos == ASW_CLWINDOWPOS_RIGHT) ? WINDOW_POSITION_LEFT : WINDOW_POSITION_RIGHT))
				SetWindowPos(ritrN->hWnd, ritrC->hWnd, wndCoord.x, wndCoord.y, wndCoord.width, wndCoord.height, SWP_NOACTIVATE);

			ritrN->saveRect();

			for (; incCount != 0; incCount--)
				++ritrC;
		}
	}

	if (sWindowInfo* wndInfo = windowFind(hWnd))
		wndInfo->saveRect();
}

void allWindowsActivation(HWND hWnd)
{
	if (sWindowInfo* wndInfo = windowFind(hWnd)) {
		WindowState wndState = wndInfo->eState;
		for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr) {
			if (itr->hWnd == hWnd)
				continue;

			switch (wndState) {
				// Восстанавливаем все окна и выстраиваем на переднем плане
			case WINDOW_STATE_NORMAL:
				ShowWindow(itr->hWnd, SW_SHOWNA);
				ShowWindow(itr->hWnd, SW_RESTORE);
				SetWindowPos(itr->hWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOACTIVATE);
				itr->eState = WINDOW_STATE_NORMAL;
				break;

				// Прячем окна диалогов, окно КЛ - скрываем
			case WINDOW_STATE_MINIMIZED:
				if (itr->hWnd != pluginVars.contactListHWND)
					ShowWindow(itr->hWnd, SW_MINIMIZE);
				else
					ShowWindow(itr->hWnd, SW_HIDE);
				itr->eState = WINDOW_STATE_MINIMIZED;
				break;

				// Прячем все окна
			case WINDOW_STATE_HIDDEN:
			case WINDOW_STATE_CLOSED:
				ShowWindow(itr->hWnd, SW_HIDE);
				itr->eState = WINDOW_STATE_HIDDEN;
				break;
			}
		}
	}
}

void windowChangeState(HWND hWnd, WPARAM cmd, LPARAM)
{
	if (sWindowInfo* wndInfo = windowFind(hWnd)) {
		switch (cmd) {
		case SC_CLOSE:
			wndInfo->eState = WINDOW_STATE_CLOSED;
			windowReposition(hWnd);
			break;

		case SC_MAXIMIZE:
			wndInfo->eState = WINDOW_STATE_MAXIMIZED;
			windowReposition(hWnd);
			break;

		case SC_MINIMIZE:
			wndInfo->eState = WINDOW_STATE_MINIMIZED;
			allWindowsActivation(hWnd);
			break;

		case SC_RESTORE:
		case SC_MOVE:
		case SC_SIZE:
			wndInfo->eState = WINDOW_STATE_NORMAL;
			allWindowsActivation(hWnd);
			windowReposition(hWnd);
			break;
		}
	}
}

void windowChangeState(HWND hWnd, WindowState newState)
{
	if (sWindowInfo* wndInfo = windowFind(hWnd)) {
		wndInfo->eState = newState;
		switch (newState) {
		case WINDOW_STATE_NORMAL:
		case WINDOW_STATE_HIDDEN:
			allWindowsActivation(hWnd);
			break;
		}
	}
}

void windowActivation(HWND hWnd, HWND prevhWnd)
{
	// Не активируем окно, если предыдущим активным было уже привязанное окно
	if (sWindowInfo* wndInfo = windowFind(prevhWnd))
		return;

	// Почему-то этот код приводит к скукоживанию КЛа / двойному восстановлению
	/*
	hWnd = windowGetRoot(hWnd);
	if (sWindowInfo* wndInfo = windowFind(hWnd))
	{
		 if (wndInfo->eState == WINDOW_STATE_MINIMIZED)
			  ShowWindow(wndInfo->hWnd, SW_RESTORE);
		 if (wndInfo->eState == WINDOW_STATE_HIDDEN)
			  ShowWindow(wndInfo->hWnd, SW_SHOW);
		 wndInfo->eState = WINDOW_STATE_NORMAL;

		 allWindowsActivation(hWnd);
	}
	*/

	for (auto itr = pluginVars.allWindows.begin(); itr != pluginVars.allWindows.end(); ++itr) {
		if (itr->hWnd != hWnd) {
			if (itr->eState == WINDOW_STATE_MINIMIZED) {
				ShowWindow(itr->hWnd, SW_RESTORE);
				// itr->eState = WINDOW_STATE_NORMAL; - ведет к глюкам
			}
			SetWindowPos(itr->hWnd, hWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
}

LRESULT CALLBACK wndProcSync(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!pluginIsAlreadyRunning()) {
		pluginSetProgress();

		windowListUpdate();

		switch (msg) {
		case WM_SYSCOMMAND:
			windowChangeState(hWnd, wParam, lParam);
			break;

		case WM_SIZE:
			allWindowsMoveAndSize(hWnd);
			break;

		case WM_MOVE:
			allWindowsMoveAndSize(hWnd);
			break;

		case WM_SHOWWINDOW:
			windowChangeState(hWnd, wParam ? WINDOW_STATE_NORMAL : WINDOW_STATE_HIDDEN);
			allWindowsMoveAndSize(hWnd);
			break;

		case WM_ACTIVATE:
			if (wParam)
				windowActivation(hWnd, (HWND)lParam);
			break;
		}

		pluginSetDone();
	}

	return mir_callNextSubclass(hWnd, wndProcSync, msg, wParam, lParam);
}

bool calcNewWindowPosition(HWND hWndLeading, HWND hWndDriven, sWndCoords* wndCoord, eWindowPosition wndPos)
{
	RECT rWndLeading, rWndDriven;

	if (!GetWindowRect(hWndLeading, &rWndLeading) || !GetWindowRect(hWndDriven, &rWndDriven))
		return false;

	wndCoord->width = rWndDriven.right - rWndDriven.left;
	wndCoord->height = rWndLeading.bottom - rWndLeading.top;
	wndCoord->y = rWndLeading.top;
	if (wndPos == WINDOW_POSITION_RIGHT)
		wndCoord->x = rWndLeading.left - wndCoord->width;
	else if (wndPos == WINDOW_POSITION_LEFT)
		wndCoord->x = rWndLeading.right;

	return true;
}
