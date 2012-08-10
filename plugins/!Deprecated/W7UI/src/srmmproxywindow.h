#ifndef srmmproxywindow_h__
#define srmmproxywindow_h__

class CSrmmProxyWindow: public CDwmWindow
{
public:
	CSrmmProxyWindow(HANDLE hContact, HWND hwndWindow, HWND hwndParent);
	~CSrmmProxyWindow();

	void OnTabActive();
	void OnTabInactive();

private:
	HANDLE m_hContact;
	HWND m_hwndWindow, m_hwndParent;
	HBITMAP m_hbmpPreview;
	bool m_refreshPreview;
	bool m_bActive, m_bUnread, m_bTyping;

	void Refresh();

	int __cdecl OnDbEventAdded(WPARAM wParam, LPARAM lParam);
	int __cdecl OnDbSettingChanged(WPARAM wParam, LPARAM lParam);
	int __cdecl OnAvatarChanged(WPARAM wParam, LPARAM lParam);
	int __cdecl OnContactTyping(WPARAM wParam, LPARAM lParam);

	HANDLE SetEventHook(char *evt, int (__cdecl CSrmmProxyWindow::*fn)(WPARAM, LPARAM));

	void UpdateIcon();

	bool IsActive();

protected:
	void OnActivate(HWND hwndFrom);
	void OnToolbar(int id, INT_PTR data);
	void OnRenderThumbnail(int width, int height);
	void OnRenderPreview();
	void OnTimer(int);
	void OnClose();
};

#endif // srmmproxywindow_h__
