/*
Copyright © 2012-15 Miranda NG team
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <windows.h>

INT_PTR CALLBACK first_run_dialog(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK tweet_proc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK pin_proc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK options_proc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);
INT_PTR CALLBACK popup_options_proc(HWND hwndDlg,UINT msg,WPARAM wParam,LPARAM lParam);