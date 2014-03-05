#include "_globals.h"
#include "canvas.h"

void Canvas::updateTrans(BYTE* pData)
{
	// MEMO: this follwing transparency code only makes sense for m_nChannels == 4
	assert(m_nChannels == 4);

	// apply transparency, if any
	if (m_bTransColor) {
		for (int y = 0; y < m_nHeight; ++y) {
			COLORREF* pValue = reinterpret_cast<COLORREF*>(pData + y * m_nLineLength);

			for (int x = 0; x < m_nWidth; ++x) {
				*pValue = (*pValue & 0x00FFFFFF) | ((*pValue & 0x00FFFFFF) == m_TransColor ? 0x00000000 : 0xFF000000);
				++pValue;
			}
		}
	}
	else {
		for (int y = 0; y < m_nHeight; ++y) {
			COLORREF* pValue = reinterpret_cast<COLORREF*>(pData + y * m_nLineLength);

			for (int x = 0; x < m_nWidth; ++x) {
				*pValue |= 0xFF000000;
				++pValue;
			}
		}
	}
}

Canvas::Canvas(int nWidth, int nHeight) :
	m_nChannels(4),
	m_nWidth(nWidth),
	m_nHeight(nHeight),
	m_nLineLength((m_nChannels * m_nWidth + 3) & ~0x3),
	m_bTransColor(false),
	m_TransColor(0),
	m_pBMIH(NULL)
{}

Canvas::~Canvas()
{
	if (m_hOldBmp)
		SelectObject(m_hDC, m_hOldBmp);

	if (m_hBmp)
		DeleteObject(m_hBmp);

	if (m_hDC)
		DeleteDC(m_hDC);

	delete m_pBMIH;
}

void Canvas::setTrans(COLORREF transColor, bool bFill /* = false */)
{
	m_bTransColor = true;
	m_TransColor = transColor;

	if (bFill)
		fillBackground(transColor);
}

void Canvas::fillBackground(COLORREF bkColor)
{
	HDC hDC = beginDraw();

	RECT rAll = { 0, 0, m_nWidth, m_nHeight };

	SetBkColor(hDC, bkColor);
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rAll, NULL, 0, NULL);

	endDraw();
}

HDC Canvas::beginDraw()
{
	if (!m_pBMIH) {
		m_pBMIH = new BITMAPINFOHEADER;

		m_pBMIH->biSize = sizeof(BITMAPINFOHEADER);
		m_pBMIH->biWidth = m_nWidth;
		m_pBMIH->biHeight = m_nHeight;
		m_pBMIH->biPlanes = 1;
		m_pBMIH->biBitCount = 8 * m_nChannels;
		m_pBMIH->biCompression = BI_RGB;
		m_pBMIH->biSizeImage = m_nChannels * m_nWidth * m_nHeight;
		m_pBMIH->biXPelsPerMeter = 0;
		m_pBMIH->biYPelsPerMeter = 0;
		m_pBMIH->biClrUsed = 0;
		m_pBMIH->biClrImportant = 0;

		BYTE* pData = 0;

		m_hDC = CreateCompatibleDC(NULL);
		m_hBmp = CreateDIBSection(m_hDC, reinterpret_cast<BITMAPINFO*>(m_pBMIH), DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), NULL, 0);
	}

	m_hOldBmp = SelectObject(m_hDC, m_hBmp);

	return m_hDC;
}

void Canvas::endDraw()
{
	SelectObject(m_hDC, m_hOldBmp);
	m_hOldBmp = NULL;
}

bool Canvas::getDigest(Digest& digest)
{
	// we don't have a digest if the image is uninitialized
	if (!m_pBMIH)
		return false;

	// read data from DIB
	int nSize = m_nLineLength * m_nHeight;
	BYTE* pData = new BYTE[nSize];

	ZeroMemory(pData, nSize);

	if (GetDIBits(m_hDC, m_hBmp, 0, m_nHeight, pData, reinterpret_cast<BITMAPINFO*>(m_pBMIH), DIB_RGB_COLORS) != m_nHeight) {
		delete[] pData;
		return false;
	}

	// apply transparency, if any
	updateTrans(pData);

	// calculate hash
	mir_sha1_hash(pData, nSize, digest.m_Digest);

	delete[] pData;
	return true;
}

bool Canvas::writePNG(const TCHAR* szFileName)
{
	// read data from DIB
	BYTE* pData = new BYTE[m_nLineLength * m_nHeight];

	if (GetDIBits(m_hDC, m_hBmp, 0, m_nHeight, pData, reinterpret_cast<BITMAPINFO*>(m_pBMIH), DIB_RGB_COLORS) != m_nHeight) {
		delete[] pData;
		return false;
	}

	// apply transparency, if any
	updateTrans(pData);

	// calculate resulting image size
	long png_len = 0;

	if (!mu::png::dibToPng(m_pBMIH, pData, 0, &png_len) || png_len == 0) {
		delete[] pData;
		return false;
	}

	// get the resulting image data
	BYTE* pRawPNG = new BYTE[png_len];

	png_len = 0;

	if (!mu::png::dibToPng(m_pBMIH, pData, pRawPNG, &png_len)) {
		delete[] pData;
		delete[] pRawPNG;

		return false;
	}

	// write image data to file
	FILE* fp = _tfopen(szFileName, _T("wb"));

	if (!fp) {
		delete[] pData;
		delete[] pRawPNG;

		return false;
	}

	fwrite(pRawPNG, 1, png_len, fp);
	fclose(fp);

	// free buffers
	delete[] pRawPNG;
	delete[] pData;

	return true;
}
