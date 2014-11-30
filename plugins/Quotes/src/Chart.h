#ifndef __FAE7F26E_61ED_4951_BE87_5E022CDF21DF_Chart_h__
#define __FAE7F26E_61ED_4951_BE87_5E022CDF21DF_Chart_h__

#pragma once

namespace detail
{
	template<class T> struct CConverter
	{
		static double Convert(const T& v)
		{
			return boost::numeric_cast<double>(v);
		}

		static tstring ToString(const T& v)
		{
			return boost::lexical_cast<tstring>(v);
		}
	};

	template<> struct CConverter<double>
	{
		static double Convert(double v)
		{
			return v;
		}

		static tstring ToString(double v)
		{
			tostringstream s;
			s.imbue(std::locale(""));
			s << std::fixed << v;
			return s.str();
		}
	};
}

template<class TXValue,class TYValue,class TXConverter = detail::CConverter<TXValue>,class TYConverter = detail::CConverter<TYValue> > 
class CChart
{
private:
	typedef std::pair<TXValue,TYValue> TValue;
	typedef std::vector<TValue> TValues;

public:
	CChart() : m_MaxY(),m_MinY()
	{
		ZeroMemory(&m_rect,sizeof(m_rect));
	}

	~CChart()
	{
	}

	void AddValue(const TXValue& x,const TYValue& y)
	{
		if(m_aValues.empty())
		{
			m_MaxY = m_MinY = y;
		}
		else			
		{
			m_MaxY = __max(y,m_MaxY);
			m_MinY = __min(y,m_MinY);
		}
		m_aValues.push_back(std::make_pair(x,y));
	}

	void SetRect(int x,int y,int cx,int cy)
	{
		m_rect.left = x;
		m_rect.right = x + cx;
		m_rect.top = y;
		m_rect.bottom = y + cy;
	}

	void Draw(HDC hdc)const
	{
		RECT rc = m_rect;
		DrawBackground(hdc,rc);
		if(false == m_aValues.empty())
		{
			::InflateRect(&rc,-10,-10);
			DrawGrid(hdc,rc);
			DrawAxis(hdc,rc);
			DrawPoints(hdc,rc);
		}
		else
		{
			HFONT hFont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
			HFONT hOldFont = static_cast<HFONT>(::SelectObject(hdc,hFont));

			LPCTSTR pszText = TranslateT("There is no to show");
			int nDrawTextResult = ::DrawText(hdc,pszText,::mir_tstrlen(pszText),&rc,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
			assert(0 != nDrawTextResult);

			::SelectObject(hdc,hOldFont);
			BOOL bResult = ::DeleteObject(hFont);
			assert(TRUE == bResult);
		}
	}

private:
	void DrawBackground(HDC hdc,RECT& rc)const
	{
// 		HBRUSH hBrush = ::CreateSolidBrush(RGB(255,0,0));//user preferable background color here!
// 		::FillRect(hdc,&m_rect,hBrush);
// 		::DeleteBrush(hBrush);
	}

	void DrawGrid(HDC hdc,RECT& rc)const
	{
		enum{number_of_lines = 5};
		HPEN hPen = ::CreatePen(PS_SOLID,1,RGB(125,125,125));
		HPEN hPenOld = static_cast<HPEN>(::SelectObject(hdc,hPen));
		HFONT hFont = static_cast<HFONT>(::GetStockObject(DEFAULT_GUI_FONT));
		HFONT hOldFont = static_cast<HFONT>(::SelectObject(hdc,hFont));

		//vertical grid
		int step = (rc.bottom-rc.top)/number_of_lines;
		TYValue y_val = m_MinY + ((m_MaxY-m_MinY)/number_of_lines);
		int nXIndent = 0;
		for(int y = rc.bottom-step;y > rc.top;y-=step,y_val+=((m_MaxY-m_MinY)/number_of_lines))
		{
			tstring sY = TYConverter::ToString(y_val);
			SIZE sizeText = {0,0};
			BOOL bResult = ::GetTextExtentPoint32(hdc,sY.c_str(), (int)sY.size(), &sizeText);
			assert(TRUE == bResult);
			nXIndent = __max(nXIndent,sizeText.cx);
		}

		y_val = m_MinY + ((m_MaxY-m_MinY)/number_of_lines);
		nXIndent += 2;
		rc.left += nXIndent;
		for(int y = rc.bottom-step;y > rc.top;y-=step,y_val+=((m_MaxY-m_MinY)/number_of_lines))
		{
			tstring sY = TYConverter::ToString(y_val);
			SIZE sizeText = {0,0};
			BOOL bResult = ::GetTextExtentPoint32(hdc, sY.c_str(), (int)sY.size(), &sizeText);
			assert(TRUE == bResult);

			RECT rcText = {rc.left-nXIndent,y-(sizeText.cy/2),rc.left-1,y+(sizeText.cy/2)};
			int nDrawTextResult = ::DrawText(hdc, sY.c_str(), (int)sY.size(), &rcText, DT_SINGLELINE|DT_VCENTER|DT_RIGHT);
			assert(0 != nDrawTextResult);

			bResult = ::MoveToEx(hdc,rc.left,y,NULL);
			assert(TRUE == bResult);

			bResult = ::LineTo(hdc,rc.right,y);
			assert(TRUE == bResult);
		}

		// horizontal grid
		HRGN rgnAllLables = ::CreateRectRgn(0,0,0,0);
		HRGN rgnTemporary = ::CreateRectRgn(0,0,0,0);
		bool bFixedRect = false;
		step = (rc.right-rc.left)/number_of_lines;
		TXValue x_val = m_aValues[0].first + ((m_aValues[m_aValues.size()-1].first-m_aValues[0].first)/number_of_lines);
		for(int x = rc.left+step;x < rc.right;x+=step,x_val+=((m_aValues[m_aValues.size()-1].first-m_aValues[0].first)/number_of_lines))
		{
			tstring sX = TXConverter::ToString(x_val);
			SIZE sizeText = {0,0};
			BOOL bResult = ::GetTextExtentPoint32(hdc, sX.c_str(), (int)sX.size(), &sizeText);
			assert(TRUE == bResult);

			if(false == bFixedRect)
			{
				rc.bottom -= sizeText.cy+2;
				bFixedRect = true;
			}

			RECT rcText = {x-(sizeText.cx/2),rc.bottom,x+(sizeText.cx/2),rc.bottom+sizeText.cy-1};
			// Draw a label if it doesn't overlap with previous ones
			HRGN rgnCurrentLable = ::CreateRectRgnIndirect(&rcText);
			if(NULLREGION == ::CombineRgn(rgnTemporary,rgnCurrentLable,rgnAllLables,RGN_AND))
			{
				int nDrawTextResult = ::DrawText(hdc, sX.c_str(), (int)sX.size(), &rcText, DT_SINGLELINE|DT_VCENTER|DT_CENTER);
				assert(0 != nDrawTextResult);
				int nCombineRgnResult = ::CombineRgn(rgnTemporary,rgnCurrentLable,rgnAllLables,RGN_OR);
				assert(ERROR != nCombineRgnResult);
				nCombineRgnResult = ::CombineRgn(rgnAllLables,rgnTemporary,NULL,RGN_COPY);
				assert(ERROR != nCombineRgnResult);
			}
			bResult = ::DeleteObject(rgnCurrentLable);
			assert(TRUE == bResult);
			
			bResult = ::MoveToEx(hdc,x,rc.bottom,NULL);
			assert(TRUE == bResult);

			bResult = ::LineTo(hdc,x,rc.top);
			assert(TRUE == bResult);
		}

		BOOL bResult = ::DeleteObject(rgnAllLables);
		assert(TRUE == bResult);
		bResult = ::DeleteObject(rgnTemporary);
		assert(TRUE == bResult);

		::SelectObject(hdc,hOldFont);
		::SelectObject(hdc,hPenOld);
		bResult = ::DeleteObject(hFont);
		assert(TRUE == bResult);
		bResult = ::DeleteObject(hPen);
		assert(TRUE == bResult);
	}

	void DrawAxis(HDC hdc,RECT& rc)const
	{
		HPEN hPen = ::CreatePen(PS_SOLID,2,RGB(0,0,0));
		HPEN hPenOld = static_cast<HPEN>(::SelectObject(hdc,hPen));

		// draw Y-axes
		BOOL bResult = ::MoveToEx(hdc,rc.left+1,rc.bottom-1,NULL);
		assert(TRUE == bResult);
		bResult = ::LineTo(hdc,rc.left+1,rc.top+1);
		assert(TRUE == bResult);

		// draw X-axes
		bResult = ::MoveToEx(hdc,rc.left+1,rc.bottom-1,NULL);
		assert(TRUE == bResult);
		bResult = ::LineTo(hdc,rc.right-1,rc.bottom-1);
		assert(TRUE == bResult);

		::SelectObject(hdc,hPenOld);
		bResult = ::DeleteObject(hPen);
		assert(TRUE == bResult);
	}

	void DrawPoints(HDC hdc,RECT& rc)const
	{		
		TXValue xMin(m_aValues[0].first);
		double dx = TXConverter::Convert(m_aValues[m_aValues.size()-1].first-xMin);
		double dY = TYConverter::Convert(m_MaxY-m_MinY);

		HPEN hPen = ::CreatePen(PS_SOLID,1,RGB(255,0,0));
		HGDIOBJ hPenOld = ::SelectObject(hdc,hPen);

		HBRUSH hBrush = ::CreateSolidBrush(RGB(255,0,0));
		HGDIOBJ hBrushOld = ::SelectObject(hdc,hBrush);

		bool bPrevValid = false;
		int xPrex,yPrev;

		BOOST_FOREACH(const TValue& v,m_aValues)
		{
			double k = TXConverter::Convert(v.first-xMin);

			int x = rc.left+boost::numeric_cast<int>((rc.right-rc.left)*(k/dx));
			k = TYConverter::Convert(v.second-m_MinY);
			int y = rc.bottom-boost::numeric_cast<int>((rc.bottom-rc.top)*(k/dY));
			::Ellipse(hdc,x-5,y-5,x+5,y+5);
			if(bPrevValid)
			{
				BOOL bResult = ::MoveToEx(hdc,xPrex,yPrev,NULL);
				assert(TRUE == bResult);
				bResult = ::LineTo(hdc,x,y);
				assert(TRUE == bResult);
			}

			xPrex = x,yPrev = y;
			bPrevValid = true;
		}

		::SelectObject(hdc,hPenOld);
		BOOL bResult = ::DeleteObject(hPen);
		assert(TRUE == bResult);

		::SelectObject(hdc,hBrushOld);
		bResult = ::DeleteObject(hBrush);
		assert(TRUE == bResult);
	}

private:
	TValues m_aValues;
	RECT m_rect;
	TYValue m_MaxY;
	TYValue m_MinY;
};

#endif // __FAE7F26E_61ED_4951_BE87_5E022CDF21DF_Chart_h__
