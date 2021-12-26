#include "stdafx.h"
#include "canvas.h"

void Canvas::updateTrans(uint8_t* pData)
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

Canvas::Canvas(int nWidth, int nHeight) : m_nChannels(4), m_nWidth(nWidth),
	m_nHeight(nHeight), m_nLineLength((m_nChannels * m_nWidth + 3) & ~0x3),
	m_bTransColor(false), m_TransColor(0),	m_pBMIH(nullptr), m_hDC(nullptr),
	m_hBmp(nullptr), m_hOldBmp(nullptr)
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
	ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rAll, nullptr, 0, nullptr);

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

		uint8_t* pData = nullptr;

		m_hDC = CreateCompatibleDC(nullptr);
		m_hBmp = CreateDIBSection(m_hDC, reinterpret_cast<BITMAPINFO*>(m_pBMIH), DIB_RGB_COLORS, reinterpret_cast<void**>(&pData), nullptr, 0);
	}

	m_hOldBmp = SelectObject(m_hDC, m_hBmp);

	return m_hDC;
}

void Canvas::endDraw()
{
	SelectObject(m_hDC, m_hOldBmp);
	m_hOldBmp = nullptr;
}

bool Canvas::getDigest(Digest& digest)
{
	// we don't have a digest if the image is uninitialized
	if (!m_pBMIH)
		return false;

	// read data from DIB
	int nSize = m_nLineLength * m_nHeight;
	uint8_t* pData = new uint8_t[nSize];

	memset(pData, 0, nSize);

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

bool Canvas::writePNG(const wchar_t* szFileName)
{
	IMGSRVC_INFO img = { 0 };
	img.cbSize = sizeof(img);
	img.dwMask = IMGI_HBITMAP;
	img.hbm = m_hBmp;
	img.fif = FIF_PNG;
	img.pwszName = (wchar_t*)szFileName;
	return Image_Save(&img) == 0;
}
