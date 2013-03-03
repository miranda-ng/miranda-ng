/*
Copyright (C) 2012-13 Miranda NG team (http://miranda-ng.org)

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

#include "commonheaders.h"

struct MSubclassData
{
	HWND     m_hWnd;

	int      m_iHooks;
	WNDPROC *m_hooks;
	WNDPROC  m_origWndProc;

	~MSubclassData()
	{
		free(m_hooks);
	}
};

static LIST<MSubclassData> arSubclass(10, LIST<MSubclassData>::FTSortFunc(HandleKeySortT));

static LRESULT CALLBACK MSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSubclassData *p = arSubclass.find((MSubclassData*)&hwnd);
	if (p != NULL) {
		for (int i=0; i < p->m_iHooks; i++) {
			LRESULT res = p->m_hooks[i](hwnd, uMsg, wParam, lParam);
			if (res != 0)
				return res;
		}

		if (uMsg == WM_DESTROY) {
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)p->m_origWndProc);
			arSubclass.remove(p);
			delete p;			
		}
	}		

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

MIR_CORE_DLL(void) mir_subclassWindow(HWND hWnd, WNDPROC wndProc)
{
	MSubclassData *p = arSubclass.find((MSubclassData*)&hWnd);
	if (p == NULL) {
		p = new MSubclassData;
		p->m_hWnd = hWnd;
		p->m_origWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)MSubclassWndProc);
		p->m_iHooks = 0;
		p->m_hooks = (WNDPROC*)malloc( sizeof(WNDPROC));
		arSubclass.insert(p);
	}
	else p->m_hooks = (WNDPROC*)realloc(p->m_hooks, (p->m_iHooks+1)*sizeof(WNDPROC));

	p->m_hooks[p->m_iHooks++] = wndProc;		
}

MIR_CORE_DLL(void) KillModuleSubclassing(HMODULE hInst)
{
	for (int i=0; i < arSubclass.getCount(); i++) {
		MSubclassData *p = arSubclass[i];
		for (int j=0; j < p->m_iHooks; ) {
			if ( GetInstByAddress(p->m_hooks[j]) == hInst) {
				for (int k=j+1; k < p->m_iHooks; k++)
					p->m_hooks[k-1] = p->m_hooks[k];
				p->m_iHooks--;
			}
			else j++;
		}
	}
}
