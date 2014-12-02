/*

IEView Plugin for Miranda IM
Copyright (C) 2005-2010  Piotr Piastucki

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "ieview_common.h"

INT_PTR HandleIEWindow(WPARAM, LPARAM lParam)
{
	IEVIEWWINDOW *window = (IEVIEWWINDOW *) lParam;
	IEView::init();
	Options::init();
	if (window->iType == IEW_CREATE) {
		HTMLBuilder *builder = NULL;
		switch (window->dwMode) {
		case IEWM_MUCC:
			builder = new MUCCHTMLBuilder();
			break;
		case IEWM_CHAT:
			builder = new ChatHTMLBuilder();
			break;
		case IEWM_TABSRMM:
			builder = new TabSRMMHTMLBuilder();
			break;
		case IEWM_SCRIVER:
			builder = new ScriverHTMLBuilder();
			break;
		case IEWM_HISTORY:
			builder = new HistoryHTMLBuilder();
			break;
		case IEWM_BROWSER:
			builder = NULL;
			break;
		default:
			builder = new ScriverHTMLBuilder();
			break;
		}
		IEView *view = new IEView(window->parent, builder, window->x, window->y, window->cx, window->cy);
		window->hwnd = view->getHWND();
	}
	else if (window->iType == IEW_SETPOS) {
		IEView *view = IEView::get(window->hwnd);
		if (view != NULL)
			view->setWindowPos(window->x, window->y, window->cx,window->cy);
	}
	else if (window->iType == IEW_SCROLLBOTTOM) {
		IEView *view = IEView::get(window->hwnd);
		if (view != NULL)
			view->scrollToBottom();
	}
	else if (window->iType == IEW_DESTROY) {
		IEView *view = IEView::get(window->hwnd);
		if (view != NULL)
			delete view;
	}
	return 0;
}

INT_PTR HandleIEEvent(WPARAM, LPARAM lParam)
{
	IEVIEWEVENT *event = (IEVIEWEVENT *) lParam;
	IEView::init();
	Options::init();
	IEView *view = IEView::get(event->hwnd);
	if (view != NULL) {
		if (event->iType == IEE_LOG_DB_EVENTS)
			view->appendEventOld(event);
		else if (event->iType == IEE_CLEAR_LOG)
			view->clear(event);
		else if (event->iType == IEE_GET_SELECTION)
			return (INT_PTR)view->getSelection(event);
		else if (event->iType == IEE_SAVE_DOCUMENT)
			view->saveDocument();
		else if (event->iType == IEE_LOG_MEM_EVENTS)
			view->appendEvent(event);
	}
	return 0;
}

INT_PTR HandleIENavigate(WPARAM, LPARAM lParam)
{
	IEVIEWNAVIGATE *navigate = (IEVIEWNAVIGATE *) lParam;
	IEView::init();
	Options::init();
	IEView *view = IEView::get(navigate->hwnd);
	if (view != NULL) {
		if (navigate->iType == IEN_NAVIGATE) {
			view->navigate(navigate);
		}
	}
	return 0;
}
