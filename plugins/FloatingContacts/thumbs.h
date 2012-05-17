/////////////////////////////////////////////////////////////////////////////
//
#define	USERNAME_LEN				50
class CDropTarget;

typedef struct _DockOpt
{
	HWND	hwndLeft;
	HWND	hwndRight;
}
DockOpt;

struct ThumbInfo
{
public: // TODO: make private
	HWND			hwnd;
	TCHAR			ptszName[ USERNAME_LEN ];
	HANDLE			hContact;
	int				iIcon;
	CDropTarget *	dropTarget;
	DockOpt			dockOpt;
	BOOL			fTipActive;
	BOOL			fTipTimerActive;
	POINT			ptTipSt;

	BYTE			btAlpha;
	MyBitmap		bmpContent;

	POINT			ptPos;
	SIZE			szSize;

public:
	ThumbInfo();
	~ThumbInfo();

	void GetThumbRect			(RECT *rc);
	void PositionThumb			(short nX, short nY); 
	void PositionThumbWorker	(short nX, short nY, POINT *rcNewPos); 
	void ResizeThumb			();
	void RefreshContactIcon		(int iIcon);
	void RefreshContactStatus	(int idStatus);
	void DeleteContactPos		();
	void OnLButtonDown			(short nX, short nY);
	void OnLButtonUp			();
	void OnMouseMove			(short nX, short nY, WPARAM wParam);
	void ThumbSelect			(BOOL bMouse);
	void ThumbDeselect			(BOOL bMouse);
	void SetThumbOpacity		(BYTE btAlpha);
	void KillTip				();
	void UpdateContent			();
	void PopUpMessageDialog		();
	void OnTimer				(BYTE idTimer);
};

void UndockThumbs	( ThumbInfo *pThumb1, ThumbInfo *pThumb2 );
void DockThumbs		( ThumbInfo *pThumbLeft, ThumbInfo *pThumbRight, BOOL bMoveLeft );

class ThumbList: public LIST<ThumbInfo>
{
public:
	ThumbList();
	~ThumbList();

	ThumbInfo*	AddThumb			(HWND hwnd, TCHAR *ptszName, HANDLE hContact);
	void		RemoveThumb			(ThumbInfo *pThumb);

	ThumbInfo*	FindThumb			(HWND hwnd);
	ThumbInfo*	FindThumbByContact	(HANDLE hContact);

private:
	static int cmp(const ThumbInfo *p1, const ThumbInfo *p2);
};

extern ThumbList thumbList;
