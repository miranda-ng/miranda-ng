#ifndef clistproxywindow_h__
#define clistproxywindow_h__

class CClistProxyWindow: public CDwmWindow
{
public:
	CClistProxyWindow();
	~CClistProxyWindow();

	void Flash();
	void SetOverlayIcon(HICON hIcon);
	void SetOverlayIconHandle(HANDLE hIcolibIcon);
	void AddOverlayEvent(int idx);
	void RemoveOverlayEvent(int idx);
	int AllocateOverlayEvent(HANDLE hIcolibIcon);

private:
	class COverlayEventSlot
	{
	private:
		HANDLE m_hIcolibItem;
		TCHAR *m_overlayText;
		int m_level;

	public:
		COverlayEventSlot(HANDLE hIcolibItem, TCHAR *overlayText)
		{
			m_hIcolibItem = hIcolibItem;
			m_overlayText = mir_tstrdup(overlayText);
			m_level = 0;
		}
		~COverlayEventSlot() { mir_free(m_overlayText); }
		void Push() { m_level++; }
		void Pop() { if (m_level > 0) m_level--; }
		operator bool() { return m_level > 0; }
		HANDLE GetIcon() { return m_hIcolibItem; }
	};

	int m_activeOverlay;
	HICON m_overlayIcon;
	HANDLE m_overlayIconHandle;
	TCHAR *m_overlayText;
	OBJLIST<COverlayEventSlot> m_overlayEvents;

	int __cdecl OnStatusModeChanged(WPARAM wParam, LPARAM lParam);
	int __cdecl OnAvatarChanged(WPARAM wParam, LPARAM lParam);
	int __cdecl OnModulesLoaded(WPARAM wParam, LPARAM lParam);
	int __cdecl OnAccListChanged(WPARAM wParam, LPARAM lParam);

	HANDLE SetEventHook(char *evt, int (__cdecl CClistProxyWindow::*fn)(WPARAM, LPARAM));

	void Update();

protected:
	void OnActivate(HWND hwndFrom);
	void OnToolbar(int id, INT_PTR data);
	void OnRenderThumbnail(int width, int height);
	void OnClose();
	void OnTimer(int id);
};

extern CClistProxyWindow *g_clistProxyWnd;

#endif // clistproxywindow_h__
