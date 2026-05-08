#include "stdafx.h"
#include <vector>

// Forward declarations
uint32_t ParseLibreTimestamp(const CMStringW &timestamp);

static HGENMENU g_hContactMenuGraph = nullptr;
static HWND g_hGraphWindow = nullptr; // Track open graph window
#define WM_REFRESH_GRAPH (WM_USER + 100)

// Register graph font in Customize -> Fonts and Colors
void RegisterGraphFont()
{
	// Register font
	FontIDW fid = {};
	fid.flags = FIDF_DEFAULTVALID | FIDF_ALLOWEFFECTS;
	strcpy(fid.dbSettingsGroup, MODULENAME);
	strcpy(fid.setting, "GraphFont");
	mir_wstrcpy(fid.name, L"Graph Text");
	mir_wstrcpy(fid.group, L"LibreView");
	fid.deffontsettings.charset = DEFAULT_CHARSET;
	fid.deffontsettings.style = 0;
	fid.deffontsettings.size = 14;
	mir_wstrcpy(fid.deffontsettings.szFace, L"Arial");
	fid.deffontsettings.colour = RGB(0, 0, 0);
	g_plugin.addFont(&fid);
	
	// Register background color separately
	ColourIDW cid = {};
	strcpy(cid.dbSettingsGroup, MODULENAME);
	mir_wstrcpy(cid.group, L"LibreView");
	mir_wstrcpy(cid.name, L"Graph Background");
	strcpy(cid.setting, "GraphBg");
	cid.defcolour = RGB(255, 255, 255);
	g_plugin.addColor(&cid);
	
	// Register line color
	ColourIDW cidLine = {};
	strcpy(cidLine.dbSettingsGroup, MODULENAME);
	mir_wstrcpy(cidLine.group, L"LibreView");
	mir_wstrcpy(cidLine.name, L"Graph Line");
	strcpy(cidLine.setting, "GraphLine");
	cidLine.defcolour = RGB(0, 0, 0);
	g_plugin.addColor(&cidLine);
	
	// Register points color
	ColourIDW cidPoints = {};
	strcpy(cidPoints.dbSettingsGroup, MODULENAME);
	mir_wstrcpy(cidPoints.group, L"LibreView");
	mir_wstrcpy(cidPoints.name, L"Graph Points");
	strcpy(cidPoints.setting, "GraphPoints");
	cidPoints.defcolour = RGB(0, 0, 0);  // Default black
	g_plugin.addColor(&cidPoints);
}


struct GraphDataPoint {
	time_t timestamp;
	double value;
	int trendArrow;
};

static std::vector<GraphDataPoint> ParseGraphData(const JSONNode& graphData, bool useMgdl, bool bApiMgdl, int offset)
{
	std::vector<GraphDataPoint> result;
	
		for (auto &item : graphData) {
			GraphDataPoint point = {};
			
			CMStringW timestamp = item["Timestamp"].as_mstring();
			point.timestamp = ParseLibreTimestamp(timestamp);
			
			// Get raw value from appropriate field based on API units
			double rawValue;
			if (bApiMgdl) {
				rawValue = item["ValueInMgPerDl"].as_float();
			} else {
				rawValue = item["Value"].as_float();
			}
			
			// Apply offset BEFORE unit conversion (in API units)
			rawValue += (double)offset;
			
			// Convert to display units
			if (useMgdl) {
				// Display in mg/dL
				if (bApiMgdl) {
					point.value = rawValue;
				} else {
					point.value = rawValue * 18.0;
				}
			} else {
				// Display in mmol/L
				if (bApiMgdl) {
					point.value = rawValue / 18.0;
				} else {
					point.value = rawValue;
				}
			}
			
			if (point.timestamp > 0 && point.value > 0) {
				result.push_back(point);
			}
		}
	
	return result;
}

struct GraphDialogParams {
	CMStringW title;
	std::vector<GraphDataPoint> data;
	bool useMgdl;
};

class CGraphDialog : public CDlgBase
{
	CMStringA m_title;
	std::vector<GraphDataPoint> m_data;
	bool m_useMgdl;
	HWND m_hToolTip;
	TOOLINFO m_ti;
	int m_hoverIndex;
	
public:
	CGraphDialog(const CMStringW& title, const std::vector<GraphDataPoint>& data, bool useMgdl) 
		: CDlgBase(g_plugin, IDD_GRAPH), m_data(data), m_useMgdl(useMgdl)
	{
		m_title = ptrA(mir_utf8encodeW(title.c_str()));
	}

	bool OnInitDialog() override
	{
		SetWindowTextA(m_hwnd, m_title);
		m_hoverIndex = -1;
		
		// Track this window globally
		g_hGraphWindow = m_hwnd;
		
		// Create tooltip window
		m_hToolTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, nullptr, 
			WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			m_hwnd, nullptr, g_plugin.getInst(), nullptr);
		
		if (m_hToolTip) {
			SetWindowPos(m_hToolTip, HWND_TOPMOST, 0, 0, 0, 0, 
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			
			// Setup tool info
			memset(&m_ti, 0, sizeof(m_ti));
			m_ti.cbSize = sizeof(m_ti);
			m_ti.uFlags = TTF_TRACK | TTF_ABSOLUTE;
			m_ti.hwnd = m_hwnd;
			m_ti.hinst = g_plugin.getInst();
			SendMessage(m_hToolTip, TTM_ADDTOOL, 0, (LPARAM)&m_ti);
			SendMessage(m_hToolTip, TTM_SETMAXTIPWIDTH, 0, 200);
		}
		
		DrawGraph();
		return true;
	}
	
	void OnDestroy() override
	{
		// Clear global window handle
		if (g_hGraphWindow == m_hwnd) {
			g_hGraphWindow = nullptr;
		}
		
		if (m_hToolTip) {
			DestroyWindow(m_hToolTip);
			m_hToolTip = nullptr;
		}
	}

	void RefreshData()
	{
		CLibreViewProto *ppro = nullptr;
		MCONTACT hContact = 0;
		for (auto &it : g_plugin.g_arInstances) {
			if (it->m_hContact != 0) {
				ppro = it;
				hContact = it->m_hContact;
				break;
			}
		}
		if (!ppro || !hContact) return;
		
		// Update m_useMgdl from current settings
		m_useMgdl = ppro->DisplayUnits == 1;
		
		// Get API units and offset from database
		const int apiUnits = ppro->getDword(hContact, "GlucoseUnits", 0);
		const bool bApiMgdl = apiUnits == 1;
		int offset = ppro->Offset;
		
		// Reload graphData from database
		std::vector<GraphDataPoint> newGraphData;
		CMStringA storedGraphData = ppro->getMStringA(hContact, "GraphData");
		if (!storedGraphData.IsEmpty()) {
			JSONNode jsonData = JSONNode::parse(storedGraphData.c_str());
			if (!jsonData.empty()) {
				newGraphData = ParseGraphData(jsonData, m_useMgdl, bApiMgdl, offset);
			}
		}
		
		// Add Value as last point if available
		CMStringW lastValue = ppro->getMStringW(hContact, "Value");
		CMStringW lastTimestamp = ppro->getMStringW(hContact, "Timestamp");
		
		if (!lastValue.IsEmpty() && !lastTimestamp.IsEmpty()) {
			GraphDataPoint lastPoint;
			
			const bool bUseMgdl = ppro->DisplayUnits == 1;
			
			// Apply offset to original API value, then convert to display units
			double rawValue = _wtof(lastValue.c_str()) + (double)offset;
			
			// Convert to display units for graph
			if (bApiMgdl && !bUseMgdl) {
				// API mg/dL -> Display mmol/L (convert)
				lastPoint.value = rawValue / 18.0;
			}
			else if (!bApiMgdl && bUseMgdl) {
				// API mmol/L -> Display mg/dL (convert)
				lastPoint.value = rawValue * 18.0;
			}
			else {
				// No conversion needed
				lastPoint.value = rawValue;
			}
			
			lastPoint.timestamp = ParseLibreTimestamp(lastTimestamp);
			
			// Add to graph data if not duplicate and value is positive (check last point)
			if (lastPoint.value > 0 && (newGraphData.empty() || 
				newGraphData.back().timestamp != lastPoint.timestamp ||
				newGraphData.back().value != lastPoint.value)) {
				newGraphData.push_back(lastPoint);
			}
		}
		
		// Update data and redraw
		m_data = newGraphData;
		InvalidateRect(m_hwnd, nullptr, TRUE);
	}

private:
	void DrawGraph()
	{
		// Update m_useMgdl from current settings
		CLibreViewProto *ppro = nullptr;
		for (auto &it : g_plugin.g_arInstances) {
			if (it->m_hContact != 0) {
				ppro = it;
				break;
			}
		}
		if (ppro) {
			m_useMgdl = ppro->DisplayUnits == 1;
		}
		
		HDC hdc = GetDC(m_hwnd);
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		
		// Clear background
		FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));
		
		if (m_data.empty()) {
			HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, 
				DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, 
				CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Arial");
			HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
			
			SetTextColor(hdc, RGB(128, 128, 128));
			SetBkMode(hdc, TRANSPARENT);
			DrawTextW(hdc, TranslateT("No data available"), -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			
			SelectObject(hdc, hOldFont);
			DeleteObject(hFont);
			ReleaseDC(m_hwnd, hdc);
			return;
		}
		
		// Calculate graph area (with margins)
		const int margin = 50;
		const int graphLeft = margin;
		const int graphTop = margin;
		const int graphRight = rc.right - margin;
		const int graphBottom = rc.bottom - margin; // No space for legend
		
		const int graphWidth = graphRight - graphLeft;
		const int graphHeight = graphBottom - graphTop;
		
		if (graphWidth <= 0 || graphHeight <= 0) {
			ReleaseDC(m_hwnd, hdc);
			return;
		}
		
		// Find min and max values
		double minValue = 0.0; // Always start from zero
		double maxValue = DBL_MIN;
		time_t minTime = m_data[0].timestamp, maxTime = m_data[0].timestamp;
		
		for (const auto& point : m_data) {
			if (point.value > maxValue) maxValue = point.value;
			if (point.timestamp < minTime) minTime = point.timestamp;
			if (point.timestamp > maxTime) maxTime = point.timestamp;
		}
		
		// Add 10% margin top only (bottom is zero)
		double valueRange = maxValue - minValue;
		if (valueRange < 0.1) valueRange = 1.0; // avoid division by zero
		maxValue += valueRange * 0.1;
		
		// Use maxValue directly for grid calculation (data is already in correct units)
		double displayMaxValue = maxValue;
		
		// Get font and colors from FontService using LOGFONTW
		LOGFONTW lf = {};
		memset(&lf, 0, sizeof(lf));
		
		// Initialize default font settings
		lf.lfHeight = -14;
		lf.lfWeight = FW_NORMAL;
		wcscpy(lf.lfFaceName, L"Arial");
		lf.lfCharSet = DEFAULT_CHARSET;
		lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
		lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
		lf.lfQuality = CLEARTYPE_QUALITY;
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
		
		// Get font and color from FontService
		COLORREF fontColor = Font_GetW(L"LibreView", L"Graph Text", &lf);
		
		// Get background, line and points colors from FontService
		COLORREF bgColor = Colour_GetW(L"LibreView", L"Graph Background");
		COLORREF lineColor = Colour_GetW(L"LibreView", L"Graph Line");
		COLORREF pointsColor = Colour_GetW(L"LibreView", L"Graph Points");
		
		// Fill the entire client area with background color FIRST
		HBRUSH hBgBrush = CreateSolidBrush(bgColor);
		FillRect(hdc, &rc, hBgBrush);
		DeleteObject(hBgBrush);
		
		// Create font from settings
		HFONT hLabelFont = CreateFontIndirectW(&lf);
		HFONT hOldFont = (HFONT)SelectObject(hdc, hLabelFont);
		
		// Set colors
		SetTextColor(hdc, fontColor);
		SetBkColor(hdc, bgColor);
		
		// Draw grid and axes - use fontColor for axes
		HPEN hGridPen = CreatePen(PS_SOLID, 1, RGB(220, 220, 220));
		HPEN hAxisPen = CreatePen(PS_SOLID, 2, fontColor);  // Use font color for axes
		HPEN hOldPen = (HPEN)SelectObject(hdc, hGridPen);
		
		// Horizontal grid lines and Y axis labels with fixed steps
		// Step in mmol/L: 3 mmol/L (50 mg/dL for mg/dL mode)
		double step = m_useMgdl ? 50.0 : 3.0;  // 50 mg/dL or 3 mmol/L
		double maxGridValue = ((int)(displayMaxValue / step) + 1) * step;  // Round up to next step
		int lineCount = (int)(maxGridValue / step) + 1;
		
		for (int i = 0; i < lineCount; i++) {
			double value = i * step;
			if (value > displayMaxValue * 1.1) break;  // Don't draw beyond graph area
			
			int y = graphBottom - (int)((value - minValue) / (displayMaxValue - minValue) * graphHeight);
			if (y < graphTop || y > graphBottom) continue;
			
			MoveToEx(hdc, graphLeft, y, nullptr);
			LineTo(hdc, graphRight, y);
			
			// Value labels (skip 0) - using common label font
			if (i > 0) {
				CMStringW label;
				if (m_useMgdl) {
					label.Format(L"%.0f", value);  // value is already in mg/dL
				} else {
					label.Format(L"%.1f", value);  // value is in mmol/L
				}
				
				RECT textRect = {0, y - 10, graphLeft - 5, y + 10};
				DrawTextW(hdc, label, -1, &textRect, DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
			}
		}
		
		SelectObject(hdc, hAxisPen);
		// Axes
		MoveToEx(hdc, graphLeft, graphTop, nullptr);
		LineTo(hdc, graphLeft, graphBottom);
		MoveToEx(hdc, graphLeft, graphBottom, nullptr);
		LineTo(hdc, graphRight, graphBottom);
		
		// Y axis unit label above the axis (centered over the axis tip)
		// Reuse the same hLabelFont already selected
		CMStringW yLabel = m_useMgdl ? TranslateT("mg/dL") : TranslateT("mmol/L");
		RECT yLabelRect = {graphLeft - 40, graphTop - 20, graphLeft + 40, graphTop - 5};
		DrawTextW(hdc, yLabel, -1, &yLabelRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
		
		// Draw time labels on X axis - switch back to grid pen for vertical lines
		SelectObject(hdc, hGridPen);
		
		// Time labels use the same font (hLabelFont already selected)
		
		// Calculate time range and find hour boundaries
		struct tm *tmStart = localtime(&minTime);
		struct tm *tmEnd = localtime(&maxTime);
		if (tmStart && tmEnd) {
			// Round start time to nearest hour
			time_t startHour = minTime - (tmStart->tm_min * 60) - tmStart->tm_sec;
			
			// Generate time labels for each hour only (no half hours to avoid duplication)
			time_t currentTime = startHour;
			while (currentTime <= maxTime) {
				// Calculate X position
				int x = graphLeft + (int)((double)(currentTime - minTime) / (maxTime - minTime) * graphWidth);
				
				if (x >= graphLeft && x <= graphRight) {
					// Format time - only show full hours
					struct tm *tmCurrent = localtime(&currentTime);
					if (tmCurrent) {
						// Draw vertical grid line (thin like horizontal grid lines, stop at axis)
						MoveToEx(hdc, x, graphTop, nullptr);
						LineTo(hdc, x, graphBottom);
						
						// Draw small tick mark outside axis only (no double line)
						MoveToEx(hdc, x, graphBottom, nullptr);
						LineTo(hdc, x, graphBottom + 4);
						
						// Format time using system settings
						SYSTEMTIME st;
						st.wYear = 1900 + tmCurrent->tm_year;
						st.wMonth = tmCurrent->tm_mon + 1;
						st.wDay = tmCurrent->tm_mday;
						st.wHour = tmCurrent->tm_hour;
						st.wMinute = 0;
						st.wSecond = 0;
						st.wMilliseconds = 0;
						st.wDayOfWeek = tmCurrent->tm_wday;
						
						wchar_t timeBuf[64];
						GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, nullptr, timeBuf, _countof(timeBuf));
						
						// Draw time label
						RECT textRect = {x - 30, graphBottom + 8, x + 30, graphBottom + 22};
						DrawTextW(hdc, timeBuf, -1, &textRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
					}
				}
				
				// Move to next hour
				currentTime += 3600; // Add 1 hour
			}
		}
		
		// Restore original font and cleanup
		SelectObject(hdc, hOldFont);
		DeleteObject(hLabelFont);
		
		SelectObject(hdc, hOldPen);
		DeleteObject(hGridPen);
		DeleteObject(hAxisPen);
		
		// Draw graph line
		if (m_data.size() > 1) {
			HPEN hLinePen = CreatePen(PS_SOLID, 2, lineColor);  // Configurable line color
			HPEN hPointPen = CreatePen(PS_SOLID, 1, pointsColor);   // Configurable points border
			HBRUSH hPointBrush = CreateSolidBrush(pointsColor);   // Configurable points fill
			
			SelectObject(hdc, hLinePen);
			
			// Draw main line - configurable color
			for (size_t i = 1; i < m_data.size(); i++) {
				int x1 = graphLeft + (int)((double)(m_data[i-1].timestamp - minTime) / (maxTime - minTime) * graphWidth);
				double value1 = m_data[i-1].value;
				double value2 = m_data[i].value;
				int y1 = graphBottom - (int)((value1 - minValue) / (displayMaxValue - minValue) * graphHeight);
				int x2 = graphLeft + (int)((double)(m_data[i].timestamp - minTime) / (maxTime - minTime) * graphWidth);
				int y2 = graphBottom - (int)((value2 - minValue) / (displayMaxValue - minValue) * graphHeight);
				
				MoveToEx(hdc, x1, y1, nullptr);
				LineTo(hdc, x2, y2);
			}
			
			// Draw points
			SelectObject(hdc, hPointPen);
			for (const auto& point : m_data) {
				int x = graphLeft + (int)((double)(point.timestamp - minTime) / (maxTime - minTime) * graphWidth);
				double value = point.value;
				int y = graphBottom - (int)((value - minValue) / (displayMaxValue - minValue) * graphHeight);
				
				HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hPointBrush);
				Ellipse(hdc, x - 3, y - 3, x + 3, y + 3);  // Smaller points (radius 3)
				SelectObject(hdc, hOldBrush);
			}
			
			DeleteObject(hLinePen);
			DeleteObject(hPointPen);
			DeleteObject(hPointBrush);
		}
		
		ReleaseDC(m_hwnd, hdc);
	}

	INT_PTR DlgProc(UINT msg, WPARAM wParam, LPARAM lParam) override
	{
		switch (msg) {
		case WM_REFRESH_GRAPH:
			RefreshData();
			return TRUE;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				BeginPaint(m_hwnd, &ps);
				DrawGraph();
				EndPaint(m_hwnd, &ps);
			}
			return TRUE;
			
		case WM_SIZE:
			InvalidateRect(m_hwnd, nullptr, TRUE);
			break;
			
		case WM_MOUSEMOVE:
			{
				int x = LOWORD(lParam);
				int y = HIWORD(lParam);
				int hoveredPoint = FindPointAt(x, y);
				
				if (hoveredPoint != m_hoverIndex) {
					m_hoverIndex = hoveredPoint;
					
					if (m_hoverIndex >= 0 && m_hToolTip) {
						// Build tooltip text
						const GraphDataPoint& point = m_data[m_hoverIndex];
						
						// Format value with localized units
						CMStringW valueStr;
						if (m_useMgdl) {
							valueStr.Format(L"%.0f %s", point.value, TranslateT("mg/dL"));
						} else {
							valueStr.Format(L"%.1f %s", point.value, TranslateT("mmol/L"));
						}
						
						// Format time using system format
						struct tm *tmLocal = localtime(&point.timestamp);
						wchar_t timeBuf[64] = {0};
						if (tmLocal) {
							SYSTEMTIME st;
							st.wYear = 1900 + tmLocal->tm_year;
							st.wMonth = tmLocal->tm_mon + 1;
							st.wDay = tmLocal->tm_mday;
							st.wHour = tmLocal->tm_hour;
							st.wMinute = tmLocal->tm_min;
							st.wSecond = 0;
							st.wMilliseconds = 0;
							st.wDayOfWeek = tmLocal->tm_wday;
							
							GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, nullptr, timeBuf, _countof(timeBuf));
						}
						
						// Build tooltip text (value and time only)
						CMStringW tooltipText;
						tooltipText.Format(L"%s\n%s", valueStr.c_str(), timeBuf);
						
						// Show tooltip
						m_ti.lpszText = (wchar_t*)tooltipText.c_str();
						SendMessage(m_hToolTip, TTM_UPDATETIPTEXT, 0, (LPARAM)&m_ti);
						
						POINT pt;
						GetCursorPos(&pt);
						SendMessage(m_hToolTip, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x + 10, pt.y - 20));
						SendMessage(m_hToolTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&m_ti);
					} else if (m_hToolTip) {
						// Hide tooltip
						SendMessage(m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
					}
				}
			}
			break;
			
		case WM_MOUSELEAVE:
			if (m_hToolTip) {
				SendMessage(m_hToolTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&m_ti);
			}
			m_hoverIndex = -1;
			break;
		}
		return CDlgBase::DlgProc(msg, wParam, lParam);
	}
	
private:
	int FindPointAt(int mouseX, int mouseY)
	{
		if (m_data.empty()) return -1;
		
		const int margin = 50;
		RECT rc;
		GetClientRect(m_hwnd, &rc);
		
		const int graphLeft = margin;
		const int graphTop = margin;
		const int graphRight = rc.right - margin;
		const int graphBottom = rc.bottom - margin;
		const int graphWidth = graphRight - graphLeft;
		const int graphHeight = graphBottom - graphTop;
		
		// Find min and max (same as DrawGraph)
		double minValue = 0.0;
		double maxValue = DBL_MIN;
		time_t minTime = m_data[0].timestamp, maxTime = m_data[0].timestamp;
		
		for (const auto& point : m_data) {
			if (point.value > maxValue) maxValue = point.value;
			if (point.timestamp < minTime) minTime = point.timestamp;
			if (point.timestamp > maxTime) maxTime = point.timestamp;
		}
		
		double valueRange = maxValue - minValue;
		if (valueRange < 0.1) valueRange = 1.0;
		maxValue += valueRange * 0.1;
		
		// Check each point
		const int hitRadius = 8; // Slightly larger than visual radius for easier hitting
		for (size_t i = 0; i < m_data.size(); i++) {
			int px = graphLeft + (int)((double)(m_data[i].timestamp - minTime) / (maxTime - minTime) * graphWidth);
			int py = graphBottom - (int)((m_data[i].value - minValue) / (maxValue - minValue) * graphHeight);
			
			int dx = mouseX - px;
			int dy = mouseY - py;
			
			if (dx * dx + dy * dy <= hitRadius * hitRadius) {
				return (int)i;
			}
		}
		
		return -1;
	}
};

static void GraphThreadFunc(void *param)
{
	MCONTACT hContact = (MCONTACT)(uintptr_t)param;
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	if (!ppro) return;
	
	bool useMgdl = ppro->DisplayUnits == 1;
	
	// Get API units and offset from database
	const int apiUnits = ppro->getDword(hContact, "GlucoseUnits", 0);
	const bool bApiMgdl = apiUnits == 1;
	double offset = ppro->Offset;
	
	// Get graphData from database only
	std::vector<GraphDataPoint> graphData;
	CMStringA storedGraphData = ppro->getMStringA(hContact, "GraphData");
	if (!storedGraphData.IsEmpty()) {
		JSONNode jsonData = JSONNode::parse(storedGraphData.c_str());
		if (!jsonData.empty()) {
			graphData = ParseGraphData(jsonData, useMgdl, bApiMgdl, offset);
		}
	}
	
	// Add Value as last point if available
	CMStringW lastValue = ppro->getMStringW(hContact, "Value");
	CMStringW lastTimestamp = ppro->getMStringW(hContact, "Timestamp");
	
	if (!lastValue.IsEmpty() && !lastTimestamp.IsEmpty()) {
		GraphDataPoint lastPoint;
		const bool bUseMgdl = ppro && ppro->DisplayUnits == 1;
		
		// Apply offset to original API value, then convert to display units
		double rawValue = _wtof(lastValue.c_str()) + (double)offset;
		
		// Convert to display units for graph
		if (bApiMgdl && !bUseMgdl) {
			// API mg/dL -> Display mmol/L (convert)
			lastPoint.value = rawValue / 18.0;
		}
		else if (!bApiMgdl && bUseMgdl) {
			// API mmol/L -> Display mg/dL (convert)
			lastPoint.value = rawValue * 18.0;
		}
		else {
			// No conversion needed
			lastPoint.value = rawValue;
		}
		
		lastPoint.timestamp = ParseLibreTimestamp(lastTimestamp);
		
		// Add to graph data if not duplicate and value is positive (check last point)
		if (lastPoint.value > 0 && (graphData.empty() || 
			graphData.back().timestamp != lastPoint.timestamp ||
			graphData.back().value != lastPoint.value)) {
			graphData.push_back(lastPoint);
		}
	}
	
	// Build title as "Nick: Glucose history"
	CMStringW nick = ppro->getMStringW(hContact, "Nick");
	CMStringW title;
	if (!nick.IsEmpty()) {
		title.Format(L"%s: %s", nick.c_str(), TranslateT("Glucose history"));
	} else {
		title = TranslateT("Glucose history");
	}
	
	// Check if graph window is already open
	if (g_hGraphWindow && IsWindow(g_hGraphWindow)) {
		// Focus existing window instead of creating new one
		SetForegroundWindow(g_hGraphWindow);
		ShowWindow(g_hGraphWindow, SW_RESTORE);
		return;
	}
	
	// Show dialog in main thread
	GraphDialogParams *params = new GraphDialogParams;
	params->title = title;
	params->data = graphData;
	params->useMgdl = useMgdl;
	
	CallFunctionAsync([](void *param) {
		GraphDialogParams *params = (GraphDialogParams*)param;
		CGraphDialog* dlg = new CGraphDialog(params->title, params->data, params->useMgdl);
		dlg->Show();
		delete params;
	}, params);
}

static INT_PTR GraphMenuCommand(WPARAM hContact, LPARAM)
{
	CLibreViewProto *ppro = g_plugin.getInstance(hContact);
	if (!ppro || !ppro->m_hContact)
		return 0;
	
	mir_forkthread(GraphThreadFunc, (void*)(uintptr_t)hContact);
	
	return 0;
}

static int OnPrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(g_hContactMenuGraph, g_plugin.getInstance(hContact) != nullptr);
	return 0;
}

void RefreshGraphWindow()
{
	if (g_hGraphWindow && IsWindow(g_hGraphWindow)) {
		SendMessage(g_hGraphWindow, WM_REFRESH_GRAPH, 0, 0);
	}
}

void InitGraphMenu()
{
	// Register customizable font for graph
	RegisterGraphFont();
	
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x82c22dbc, 0x9768, 0x4c1c, 0x9d, 0x72, 0x8d, 0x63, 0x3f, 0xf0, 0xe1, 0xae);
	mi.position = -0x7FFFFFFF + 1;
	mi.hIcolibItem = Skin_GetProtoIcon(MODULENAME, ID_STATUS_ONLINE);
	mi.name.a = LPGEN("&Glucose history");
	mi.pszService = MODULENAME "/Graph";
	g_hContactMenuGraph = Menu_AddContactMenuItem(&mi);
	
	CreateServiceFunction(MODULENAME "/Graph", GraphMenuCommand);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, OnPrebuildContactMenu);
}
