#pragma once

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
	MCONTACT     hContact;
	HWND         hwnd;
	wchar_t        ptszName[USERNAME_LEN];
	int          iIcon;
	CDropTarget *dropTarget;
	DockOpt      dockOpt;
	BOOL         fTipActive;
	BOOL         fTipTimerActive;
	POINT        ptTipSt;

	uint8_t         btAlpha;
	MyBitmap     bmpContent;

	POINT        ptPos;
	SIZE         szSize;

public:
	ThumbInfo();
	~ThumbInfo();

	void GetThumbRect(RECT *rc);
	void PositionThumb(int nX, int nY); 
	void PositionThumbWorker(int nX, int nY, POINT *rcNewPos); 
	void ResizeThumb();
	void RefreshContactIcon(int iIcon);
	void RefreshContactStatus(int idStatus);
	void DeleteContactPos();
	void OnLButtonDown();
	void OnLButtonUp();
	void OnMouseMove(int nX, int nY);
	void ThumbSelect(BOOL bMouse);
	void ThumbDeselect(BOOL bMouse);
	void SetThumbOpacity(uint8_t btAlpha);
	void KillTip();
	void UpdateContent();
	void PopupMessageDialog();
	void OnTimer(uint8_t idTimer);
};

void UndockThumbs(ThumbInfo *pThumb1, ThumbInfo *pThumb2);
void DockThumbs(ThumbInfo *pThumbLeft, ThumbInfo *pThumbRight);

class ThumbList: public OBJLIST<ThumbInfo>
{
public:
	ThumbList();
	~ThumbList();

	ThumbInfo* AddThumb(HWND hwnd, wchar_t *ptszName, MCONTACT hContact);
	void       RemoveThumb(ThumbInfo *pThumb);

	ThumbInfo* FindThumb(HWND hwnd);
	ThumbInfo* FindThumbByContact(MCONTACT hContact);
};

extern ThumbList thumbList;
