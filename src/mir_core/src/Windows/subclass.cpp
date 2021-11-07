/*
Copyright (C) 2012-21 Miranda NG team (https://miranda-ng.org)

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

#include "../stdafx.h"

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

static LIST<MSubclassData> arSubclass(10, HandleKeySortT);

/////////////////////////////////////////////////////////////////////////////////////////

static LRESULT CALLBACK MSubclassWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSubclassData *p = arSubclass.find((MSubclassData*)&hwnd);
	if (p != nullptr) {
		if (p->m_iHooks)
			return p->m_hooks[p->m_iHooks-1](hwnd, uMsg, wParam, lParam);

		return p->m_origWndProc(hwnd, uMsg, wParam, lParam);
	}

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////
// This is for wine: it return wrong WNDPROC for edit control in some cases.
#ifdef WIN64
#define STD_WND_PROC_ADDR_MASK	0x7FFF00000
#else
#define STD_WND_PROC_ADDR_MASK	0xFFFF0000
#endif

MIR_CORE_DLL(void) mir_subclassWindow(HWND hWnd, WNDPROC wndProc)
{
	if (hWnd == nullptr)
		return;

	MSubclassData *p = arSubclass.find((MSubclassData*)&hWnd);
	if (p == nullptr) {
		p = new MSubclassData;
		p->m_hWnd = hWnd;
		p->m_origWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)MSubclassWndProc);
		if (((SIZE_T)p->m_origWndProc & STD_WND_PROC_ADDR_MASK) == STD_WND_PROC_ADDR_MASK) { /* XXX: fix me. Wine fix. */
			p->m_origWndProc = (WNDPROC)GetClassLongPtr(hWnd, GCLP_WNDPROC);
			if (((SIZE_T)p->m_origWndProc & 0x7FFF0000) == 0x7FFF0000) /* Delay crash. */
				p->m_origWndProc = DefWindowProc;
		}
		p->m_iHooks = 0;
		p->m_hooks = (WNDPROC*)malloc(sizeof(WNDPROC));
		arSubclass.insert(p);
	}
	else {
		for (int i=0; i < p->m_iHooks; i++)
			if (p->m_hooks[i] == wndProc)
				return;

		void *tmp = realloc(p->m_hooks, (p->m_iHooks+1)*sizeof(WNDPROC));
		if (tmp == nullptr)
			return;

		p->m_hooks = (WNDPROC *)tmp;
	}

	p->m_hooks[p->m_iHooks++] = wndProc;
}

MIR_CORE_DLL(void) mir_subclassWindowFull(HWND hWnd, WNDPROC wndProc, WNDPROC oldWndProc)
{
	MSubclassData *p = arSubclass.find((MSubclassData*)&hWnd);
	if (p == nullptr) {
		p = new MSubclassData;
		p->m_hWnd = hWnd;
		p->m_origWndProc = oldWndProc;
		p->m_iHooks = 0;
		p->m_hooks = (WNDPROC*)malloc(sizeof(WNDPROC));
		arSubclass.insert(p);

		SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)MSubclassWndProc);
	}
	else {
		for (int i=0; i < p->m_iHooks; i++)
			if (p->m_hooks[i] == wndProc)
				return;

		void *tmp = realloc(p->m_hooks, (p->m_iHooks+1)*sizeof(WNDPROC));
		if (tmp == nullptr)
			return;

		p->m_hooks = (WNDPROC *)tmp;
	}

	p->m_hooks[p->m_iHooks++] = wndProc;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void removeHook(MSubclassData *p, int idx)
{
	// untie hook from a window to prevent calling mir_callNextSubclass from saveProc
	for (int i = idx + 1; i < p->m_iHooks; i++)
		p->m_hooks[i-1] = p->m_hooks[i];
	p->m_iHooks--;
}

static WNDPROC finalizeSubclassing(HWND hWnd, MSubclassData *p)
{
	WNDPROC saveProc = p->m_origWndProc;
	arSubclass.remove(p);
	delete p;

	SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)saveProc);
	return saveProc;
}

MIR_CORE_DLL(void) mir_unsubclassWindow(HWND hWnd, WNDPROC wndProc)
{
	MSubclassData *p = arSubclass.find((MSubclassData*)&hWnd);
	if (p == nullptr)
		return;

	for (int i = 0; i < p->m_iHooks; i++) {
		if (p->m_hooks[i] == wndProc) {
			removeHook(p, i);
			i--;
		}
	}

	if (p->m_iHooks == 0)
		finalizeSubclassing(hWnd, p);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(LRESULT) mir_callNextSubclass(HWND hWnd, WNDPROC wndProc, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	MSubclassData *p = arSubclass.find((MSubclassData*)&hWnd);
	if (p == nullptr)
		return DefWindowProc(hWnd, uMsg, wParam, lParam);

	for (int i = p->m_iHooks - 1; i >= 0; i--) {
		if (p->m_hooks[i] != wndProc)
			continue;

		// next hook exists, call it
		if (i != 0)
			return p->m_hooks[i-1](hWnd, uMsg, wParam, lParam);

		// last hook called, ping the default window procedure
		if (uMsg != WM_NCDESTROY)
			return p->m_origWndProc(hWnd, uMsg, wParam, lParam);

		WNDPROC saveProc = finalizeSubclassing(hWnd, p);
		return saveProc(hWnd, uMsg, wParam, lParam);
	}

	// invalid / closed hook
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) KillModuleSubclassing(HMODULE hInst)
{
	for (auto &it : arSubclass.rev_iter()) {
		for (int j = 0; j < it->m_iHooks; j++) {
			if (GetInstByAddress(it->m_hooks[j]) == hInst) {
				removeHook(it, j);
				j--;
			}
		}

		if (it->m_iHooks == 0)
			finalizeSubclassing(it->m_hWnd, it);
	}
}
