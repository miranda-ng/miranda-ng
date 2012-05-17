#ifndef subclassmgr_h__
#define subclassmgr_h__

struct TSubclassData
{
	WNDPROC oldWndProc;
	LPARAM lParam;
};

typedef LRESULT (*TSubclassProc)(MSG *msg, TSubclassData *data);

class CSubclassMgr
{
public:
	static void Subclass(HWND hwnd, TSubclassProc newWndProc, LPARAM lParam)
	{
		TWindowInfo *wi = new TWindowInfo;
		wi->hwnd = hwnd;
		wi->newWndProc = newWndProc;
		wi->lParam = lParam;
		Instance().m_windows.insert(wi);
		wi->oldWndProc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)GlobalSubclassProc);
	}

private:
	CSubclassMgr(): m_windows(5, TWindowInfo::Compare) {}
	CSubclassMgr(const CSubclassMgr &);
	CSubclassMgr &operator=(const CSubclassMgr &);

	static CSubclassMgr &Instance()
	{
		static CSubclassMgr theInstance;
		return theInstance;
	}

	struct TWindowInfo
	{
		HWND hwnd;
		WNDPROC oldWndProc;
		TSubclassProc newWndProc;
		LPARAM lParam;

		static int Compare(const TWindowInfo *p1, const TWindowInfo *p2)
		{
			return (int)p1->hwnd - (int)p2->hwnd;
		}
	};

	OBJLIST<TWindowInfo> m_windows;

	static LRESULT CALLBACK GlobalSubclassProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		TWindowInfo search = { hwnd };
		TWindowInfo *wnd = Instance().m_windows.find(&search);
		if (!wnd) return DefWindowProc(hwnd, message, wParam, lParam);

		MSG msg = { hwnd, message, wParam, lParam };
		TSubclassData data = { wnd->oldWndProc, wnd->lParam };
		LRESULT result = wnd->newWndProc(&msg, &data);

		if (message == WM_DESTROY)
		{
			SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)wnd->oldWndProc);
			Instance().m_windows.remove(wnd);
		}

		return result;
	}
};

#endif // subclassmgr_h__
