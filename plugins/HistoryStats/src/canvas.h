#if !defined(HISTORYSTATS_GUARD_CANVAS_H)
#define HISTORYSTATS_GUARD_CANVAS_H

#include "stdafx.h"

class Canvas
	: private pattern::NotCopyable<Canvas>
{
public:
	class Digest
	{
	public:
		unsigned char m_Digest[20];

		Digest() { memset(m_Digest, 0, 20); }
		Digest(const Digest& other) { memcpy(m_Digest, other.m_Digest, 20); }
		const Digest& operator =(const Digest& other) { memcpy(m_Digest, other.m_Digest, 20); return *this; }

		bool operator ==(const Digest& other) const { return (memcmp(m_Digest, other.m_Digest, 20) == 0); }
		bool operator <(const Digest& other) const { return (memcmp(m_Digest, other.m_Digest, 20) < 0); }
	};

public:
	static bool hasPNG() { return mu::png::_available(); }

private:
	int m_nChannels;
	int m_nWidth;
	int m_nHeight;
	int m_nLineLength;

	bool m_bTransColor;
	COLORREF m_TransColor;

	BITMAPINFOHEADER* m_pBMIH;
	HDC m_hDC;
	HBITMAP m_hBmp;
	HGDIOBJ m_hOldBmp;

private:
	void updateTrans(BYTE* pData);

public:
	explicit Canvas(int nWidth, int nHeight);
	~Canvas();

	void setTrans(COLORREF transColor, bool bFill = false);
	void fillBackground(COLORREF bkColor);

	HDC beginDraw();
	void endDraw();

	bool getDigest(Digest& digest);
	bool writePNG(const TCHAR* szFileName);
};

#endif // HISTORYSTATS_GUARD_CANVAS_H
