#ifndef _CLCDGfx_H_
#define _CLCDGfx_H_

enum ETransitionType {TRANSITION_FADE,TRANSITION_VLINES,TRANSITION_HLINES,TRANSITION_MORPH,TRANSITION_RANDOM};

struct SLCDPixel
{
	COLORREF cValue;
	double dSpeed;
	POINT Start;
	POINT Destination;
	POINT Position;
};

class CLCDGfx
{
public:
	CLCDGfx(void);
	virtual ~CLCDGfx(void);

	bool Initialize(int nWidth, int nHeight, int nBPP, PBYTE pBitmapBits);
	bool IsInitialized();
	bool Shutdown(void);

	void BeginDraw(void);
	void ClearScreen(void);
	COLORREF GetPixel(int nX, int nY);
	void SetPixel(int nX, int nY, COLORREF color);
	void SetPixel(int nX, int nY, BYTE r, BYTE g, BYTE b);
	void DrawLine(int nX1, int nY1, int nX2, int nY2);
	void DrawFilledRect(int nX, int nY, int nWidth, int nHeight);
	void DrawRect(int iX, int iY, int iWidth, int iHeight);
	void DrawText(int nX, int nY, LPCTSTR sText);
	void DrawText(int nX,int nY,int iWidth,tstring strText);
	void DrawBitmap(int nX,int nY,int nWidth, int nHeight,HBITMAP hBitmap);
	void EndDraw(void);

	void SetClipRegion(int iX,int iY,int iWidth,int iHeight);
	RECT GetClipRegion();

	inline int GetClipWidth() { return m_rClipRegion.right-m_rClipRegion.left; };
	inline int GetClipHeight() { return m_rClipRegion.bottom-m_rClipRegion.top; };

	HDC GetHDC(void);
	BITMAPINFO *GetBitmapInfo(void);
	HBITMAP GetHBITMAP(void);

	void StartTransition(ETransitionType eType = TRANSITION_RANDOM,LPRECT rect = NULL);

protected:
	void Cache();
	int findNearestMatch(PBYTE targetArray,int iSourceIndex);

	void EndTransition();

	RECT m_rClipRegion;
	RECT m_rTransitionRegion;

	int m_nWidth;
	int m_nHeight;
	int m_nBPP;
	BITMAPINFO *m_pBitmapInfo;
	HDC m_hDC;
	HBITMAP m_hBitmap;
	HBITMAP m_hPrevBitmap;
	PBYTE m_pBitmapBits,m_pLcdBitmapBits,m_pSavedBitmapBits;

	bool m_bInitialized;

	DWORD m_dwTransitionStart;
	bool m_bTransition;

	ETransitionType m_eTransition;
	vector<SLCDPixel*> m_LMovingPixels;
	vector<SLCDPixel*> m_LStaticPixels;

	double m_dWave;
	DWORD m_dwLastDraw;
};

#endif