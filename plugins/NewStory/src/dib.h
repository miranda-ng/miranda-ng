#pragma once

namespace simpledib
{
	class dib
	{
		HBITMAP   m_bmp;
		HBITMAP   m_oldBmp;
		HDC       m_hdc;
		LPRGBQUAD m_bits;
		int       m_width, m_height;

		HDC       m_hTargetDC;
		POINT     m_oldViewPort;
		bool      m_restore_view_port = false, m_ownData = false;
		RECT      m_rcTarget;
	
	public:
		dib();
		~dib();

		__forceinline int width() const { return m_width; }
		__forceinline int height() const { return m_height; }
		__forceinline HDC hdc()	const { return m_hdc; }
		__forceinline HBITMAP bmp() const { return m_bmp; }
		__forceinline LPRGBQUAD	bits() const { return m_bits; }
		__forceinline operator HDC() { return m_hdc; }

		bool create(int width, int height, bool topdowndib = false);
		bool create(HDC hdc, HBITMAP bmp, LPRGBQUAD bits, int width, int height);

		void clear();
		void destroy(bool del_bmp = true);
		void draw(HDC hdc, int x, int y);
		void draw(HDC hdc, LPRECT rcDraw);
		HDC  beginPaint(HDC hdc, LPRECT rcDraw);
		void endPaint(bool copy = false);
		HBITMAP detach_bitmap();
	};
}
