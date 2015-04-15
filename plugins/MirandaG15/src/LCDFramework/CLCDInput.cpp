#include "stdafx.h"
#include "CLCDInput.h"
#include "CLCDOutputManager.h"

//************************************************************************
// CLCDInput::CLCDInput
//************************************************************************
CLCDInput::CLCDInput()
{
	m_lInputTime = 0;
	m_iLinePosition = 0;
	memset(&m_Marker, 0, sizeof(m_Marker));

	m_pScrollbar = NULL;
	m_bShowSymbols = true;
	m_iBreakKeys = KEYS_RETURN;
	m_bShowMarker = false;
	m_lBlinkTimer = 0;

	m_iLineCount = 0;
//	SetScrollbarAlignment(TOP);
	Reset();
}


//************************************************************************
// CLCDInput::~CLCDInput
//************************************************************************
CLCDInput::~CLCDInput()
{
	m_vLineOffsets.clear();
}

//************************************************************************
// CLCDInput::Initialize
//************************************************************************
bool CLCDInput::Initialize()
{
	if(!CLCDTextObject::Initialize())
		return false;
	
//	m_pParent = pParent;
	return true;
}

//************************************************************************
// CLCDInput::Shutdown
//************************************************************************
bool CLCDInput::Shutdown()
{
	if(!CLCDTextObject::Shutdown())
		return false;
	
	
	return true;
}
//************************************************************************
// CLCDInput::Update
//************************************************************************
void CLCDInput::SetScrollbar(CLCDBar *pScrollbar)
{
	m_pScrollbar = pScrollbar;
	if(m_pScrollbar)
	{
		m_pScrollbar->SetSliderSize(m_iLineCount);
		m_pScrollbar->SetRange(0,m_vLineOffsets.size());
		m_pScrollbar->ScrollTo(m_iLinePosition);
	}
}

//************************************************************************
// CLCDInput::GetLastInputTime
//************************************************************************
long CLCDInput::GetLastInputTime()
{
	return m_lInputTime;
}

//************************************************************************
// CLCDInput::Update
//************************************************************************
bool CLCDInput::Update()
{
	if(!CLCDTextObject::Update())
		return false;
	
	if(m_lBlinkTimer + 500 <= GetTickCount())
	{
		m_bShowMarker = !m_bShowMarker;
		m_lBlinkTimer = GetTickCount();
	}

	return true;
}

//************************************************************************
// CLCDInput::Draw
//************************************************************************
bool CLCDInput::Draw(CLCDGfx *pGfx)
{
	if(!CLCDTextObject::Draw(pGfx))
		return false;
	
	
	SelectObject(pGfx->GetHDC(),m_hFont);
				
	RECT rBoundary = { 0, 0,0 + GetWidth(), 0 + GetHeight() }; 
	int iLine = m_iLinePosition;
	int iEndLine = m_iLinePosition + m_iLineCount;
	int iLen = 0;
	TCHAR *pcOffset = NULL;
	while(iLine <  iEndLine && iLine < m_vLineOffsets.size())
	{
		// Calculate the text length
		if(iLine < m_vLineOffsets.size() -1)
		{
			iLen = m_vLineOffsets[iLine+1].iOffset - m_vLineOffsets[iLine].iOffset;
			// Draw the linebreak marker
			if(m_bShowSymbols && m_vLineOffsets[iLine+1].bLineBreak)
				pGfx->DrawFilledRect(m_vLineOffsets[iLine].iWidth+1,rBoundary.top+m_iFontHeight/3,m_iFontHeight/3,m_iFontHeight/3);
		}
		else
			iLen = m_strText.length() - m_vLineOffsets[iLine].iOffset;
		
		// Draw the text
		pcOffset = (TCHAR*)m_strText.c_str() + m_vLineOffsets[iLine].iOffset;
		DrawTextEx(pGfx->GetHDC(),
					(LPTSTR)pcOffset,
					iLen,
					&rBoundary,
					m_iTextFormat,
					&m_dtp);

		// Draw the input cursor
		if(m_pInput && m_bShowMarker && m_Marker[0].iLine == iLine)
		{
			// insert-mode cursor
			if(m_bInsert ||m_Marker[0].iXWidth == 1)
			{
				pGfx->DrawFilledRect(m_Marker[0].iXLine,
				m_iFontHeight*(iLine-m_iLinePosition),
				1,
				m_iFontHeight);
			}
			// replace-mode cursor
			else
			{
				RECT rMarker = {m_Marker[0].iXLine,
								m_iFontHeight*(iLine-m_iLinePosition),
								m_Marker[0].iXLine+m_Marker[0].iXWidth,
								m_iFontHeight*(iLine-m_iLinePosition+1)};
				InvertRect(pGfx->GetHDC(),&rMarker);
			}
		}

		rBoundary.top += m_iFontHeight;
		rBoundary.bottom += m_iFontHeight;

		iLine++;
	}
	return true;
}

//************************************************************************
// CLCDInput::ShowSymbols
//************************************************************************
void CLCDInput::ShowSymbols(bool bShow)
{
	m_bShowSymbols = bShow;
}

//************************************************************************
// CLCDInput::SetBreakKeys
//************************************************************************
void CLCDInput::SetBreakKeys(int iKeys)
{
	m_iBreakKeys = iKeys;
}

//************************************************************************
// returns wether the input is currently active
//************************************************************************
bool CLCDInput::IsInputActive()
{
	return m_pInput != NULL;
}


//************************************************************************
// CLCDInput::OnSizeChanged
//************************************************************************
void CLCDInput::OnSizeChanged(SIZE OldSize)
{
	// if the width has changed, offsets need to be recalculated
	if(GetWidth() != OldSize.cx)
		OnFontChanged();
	// otherwise, just update scrollbar & linecount
	else if(m_pScrollbar)
	{
		m_iLineCount = GetHeight() / m_iFontHeight;
		m_pScrollbar->SetSliderSize(m_iLineCount);
	}
}

//************************************************************************
// CLCDInput::OnFontChanged
//************************************************************************
void CLCDInput::OnFontChanged()
{
	if(m_iFontHeight == 0)
		return;

	if(m_pScrollbar)
		m_pScrollbar->SetSliderSize(m_iLineCount);

	m_iLinePosition = 0;
	m_iLineCount = GetHeight() / m_iFontHeight;

	if(m_pScrollbar)
		m_pScrollbar->SetSliderSize(m_iLineCount);

	m_Marker[0].iLine = 0;
	m_Marker[0].iPosition = 0;
	m_Marker[0].iPosition = m_strText.length();
	
	// Delete all offsets and recalculate them
	m_vLineOffsets.clear();
	// Create a new offset
	SLineEntry offset;
	offset.bLineBreak = false;
	offset.iOffset = 0;
	m_vLineOffsets.push_back(offset);

	UpdateOffsets(0);
	
	UpdateMarker();
	if(m_iLineCount > 0)
		ScrollToMarker();
}

//************************************************************************
// CLCDInput::ActivateInput
//************************************************************************
void CLCDInput::ActivateInput()
{
	if(m_pInput)
		return;

	CLCDConnection *pLCDCon =  CLCDOutputManager::GetInstance()->GetLCDConnection();
	pLCDCon->SetAsForeground(1);

	m_hKBHook = SetWindowsHookEx(WH_KEYBOARD_LL, CLCDInput::KeyboardHook, GetModuleHandle(NULL), 0);
	if(!m_hKBHook)
		return;
	m_pInput = this;
	GetKeyboardState(m_acKeyboardState);
}

//************************************************************************
// CLCDInput::DeactivateInput
//************************************************************************
void CLCDInput::DeactivateInput()
{
	if(!m_pInput)
		return;
	UnhookWindowsHookEx(m_hKBHook);
	m_hKBHook = NULL;

	m_pInput = NULL;
	
	CLCDConnection *pLCDCon =  CLCDOutputManager::GetInstance()->GetLCDConnection();
	pLCDCon->SetAsForeground(0);
}

//************************************************************************
// CLCDInput::KeyboardHook
//************************************************************************
CLCDInput* CLCDInput::m_pInput = NULL;

LRESULT CALLBACK CLCDInput::KeyboardHook(int Code, WPARAM wParam, LPARAM lParam)
{
	return m_pInput->ProcessKeyEvent(Code,wParam,lParam);
}

//************************************************************************
// CLCDInput::ProcessKeyEvent
//************************************************************************
LRESULT CLCDInput::ProcessKeyEvent(int Code, WPARAM wParam, LPARAM lParam)
{
	// Event verarbeiten
	if(Code == HC_ACTION)
	{	
		KBDLLHOOKSTRUCT *key = (KBDLLHOOKSTRUCT *)(lParam);
	
		bool bKeyDown = !(key->flags & LLKHF_UP);
		bool bToggled = (m_acKeyboardState[key->vkCode] & 0x0F);
		if(bKeyDown)
			bToggled = !bToggled;
		m_acKeyboardState[key->vkCode] = (bKeyDown?0x80:0x00) | (bToggled?0x01:0x00);
		if(key->vkCode == VK_LSHIFT || key->vkCode == VK_RSHIFT)
			m_acKeyboardState[VK_SHIFT] = m_acKeyboardState[key->vkCode];
		else if(key->vkCode == VK_LCONTROL || key->vkCode == VK_RCONTROL)
			m_acKeyboardState[VK_CONTROL] = m_acKeyboardState[key->vkCode];
		else if(key->vkCode == VK_LMENU || key->vkCode == VK_RMENU)
			m_acKeyboardState[VK_MENU] = m_acKeyboardState[key->vkCode];
		
		/*
		if(bKeyDown)
			TRACE(_T("Key pressed: %i\n"),key->vkCode);
		else
			TRACE(_T("Key released: %i\n"),key->vkCode);
		*/
		// Only handle Keyup
		if(bKeyDown)
		{
			// Actions with Control/Menu keys
				if((m_acKeyboardState[VK_LMENU] & 0x80 || m_acKeyboardState[VK_CONTROL] & 0x80)
					&& m_acKeyboardState[VK_SHIFT] & 0x80)
				{
					ActivateKeyboardLayout((HKL)HKL_NEXT,0);//KLF_SETFORPROCESS);
					TRACE(_T("Keyboardlayout switched!\n"));
					return 1;
				}

			int res = 0,size = 0,dir = MARKER_HORIZONTAL,scroll = 0;
/*
			if(key->vkCode == VK_DELETE) {
				dir = MARKER_HOLD;
				res = -1;
				if(m_strText[m_Marker[0].iPosition] == '\r')
					res = -2;
				if(m_strText.length() >= m_Marker[0].iPosition + -res) {
					m_strText.erase(m_Marker[0].iPosition,-res);
					scroll = 1;
					size = 1;
				} else {
					res = 0;
				}
			}
			else */if(key->vkCode == VK_BACK )
			{
				if(m_Marker[0].iPosition != 0)
				{
					res = -1;
					if(m_strText[m_Marker[0].iPosition+res] == '\n')
						res = -2;

					m_strText.erase(m_Marker[0].iPosition+res,-res);	
					scroll = 1;
					size = res;
				}
			}
			// Marker navigation
			else if (key->vkCode == VK_INSERT)
			{
				m_bInsert = !m_bInsert;
			}
			else if(key->vkCode == VK_HOME)
			{
				res = m_vLineOffsets[m_Marker[0].iLine].iOffset - m_Marker[0].iPosition;
				scroll = 1;
			}
			else if(key->vkCode == VK_END)
			{
				if(m_vLineOffsets.size()-1 == m_Marker[0].iLine)
					res = m_strText.length() - m_Marker[0].iPosition;
				else
				{
					res = (m_vLineOffsets[m_Marker[0].iLine+1].iOffset - 1 - m_vLineOffsets[m_Marker[0].iLine+1].bLineBreak) -m_Marker[0].iPosition;
				}
				scroll = 1;
			}
			else if(key->vkCode == VK_UP)
			{
				res = -1;
				dir = MARKER_VERTICAL;
			}
			else if(key->vkCode == VK_DOWN)
			{
				res = 1;
				dir = MARKER_VERTICAL;
			}
			else if(key->vkCode == VK_LEFT)
				res = -1;
			else if(key->vkCode == VK_RIGHT)
				res = 1;

			else
			{

#ifdef _UNICODE
				TCHAR output[4];
#else
				unsigned char output[2];
#endif

				if(key->vkCode == VK_RETURN)
				{
					bool bCtrlDown = (bool)(m_acKeyboardState[VK_CONTROL] & 0x80);
					if( bCtrlDown != (m_iBreakKeys == KEYS_RETURN))
					{
						DeactivateInput();
						//m_pParent->OnInputFinished();
						return 1;
					}
					else
					{
						res = 2;
						output[0] = '\r';
						output[1] = '\n';
						output[2] = 0;
					}
				}
				else
				{
#ifdef _UNICODE
					res = ToUnicode(key->vkCode,key->scanCode,m_acKeyboardState,output,4,0);
#else
					res = ToAscii(  key->vkCode,key->scanCode,m_acKeyboardState,(WORD*)output,0);
#endif
				}

				if(res <= 0)
					res = 0;
				else
				{
					if(output[0] != '\r' && output[0] <= 0x001F)
						return 1;

					if(m_bInsert || m_strText[m_Marker[0].iPosition] == '\r')
						m_strText.insert(m_Marker[0].iPosition,(TCHAR*)output,res);
					else
						m_strText.replace(m_Marker[0].iPosition,res,(TCHAR*)output);
					
					scroll = 1;
					size = res;
				}
			}
			if(res != 0)
			{
				if(dir != MARKER_HOLD) {
					MoveMarker(dir,res);
				}
				UpdateOffsets(size);
				UpdateMarker();
				ScrollToMarker();
				m_lInputTime = GetTickCount();
			}
			//WrapLine();
			// ----
			
			

			// Block this KeyEvent
		}
		return 1; 
	}
	return CallNextHookEx(m_hKBHook, Code, wParam, lParam);
}

//************************************************************************
// CLCDInput::MoveMarker
//************************************************************************
void CLCDInput::MoveMarker(int iDir,int iMove,bool bShift)
{
	// Just cursor
	if(!bShift)
	{
		m_lBlinkTimer = GetTickCount();
		m_bShowMarker = true;

		if(iDir == MARKER_HORIZONTAL)
		{
			m_Marker[0].iPosition += iMove;
		
		}
		if(iDir == MARKER_VERTICAL)
		{
			if(iMove < 0 && m_Marker[0].iLine == 0)
				return;
			if(iMove > 0 && m_Marker[0].iLine == m_vLineOffsets.size()-1)
				return;

			m_Marker[0].iLine += iMove;

			int iX = 0,iX2 = 0;

			SIZE sizeChar = {0,0};
			int iBegin = m_vLineOffsets[m_Marker[0].iLine].iOffset;
			int iLen = 0;
			if(m_Marker[0].iLine < m_vLineOffsets.size() -1)
				iLen = m_vLineOffsets[m_Marker[0].iLine+1].iOffset - m_vLineOffsets[m_Marker[0].iLine].iOffset;
			else
				iLen = m_strText.length() - m_vLineOffsets[m_Marker[0].iLine].iOffset;
			
			HDC hDC = CreateCompatibleDC(NULL);
			if(NULL == hDC)
				return;
			SelectObject(hDC, m_hFont);   
			m_Marker[0].iXWidth = 1;
			m_Marker[0].iPosition = -1;

			int *piWidths = new int[iLen];
			int iMaxChars;
			int iChar=iBegin;

			GetTextExtentExPoint(hDC,m_strText.c_str() + iBegin,iLen,GetWidth(),&iMaxChars,piWidths,&sizeChar);
			for(;iChar<iBegin+iMaxChars;iChar++)
			{
				iX2 = iX;
				iX = piWidths[iChar-iBegin];

				if(m_Marker[0].iPosition >= 0 &&
					iChar >= m_Marker[0].iPosition)
				{
					m_Marker[0].iXWidth = sizeChar.cx;
					break;
				}

				if(iX >= m_Marker[0].iXLine || (iChar < iBegin+iLen -1 && m_strText[iChar+1] == 10))
				{
					if( m_Marker[0].iXLine - iX2 <= iX - m_Marker[0].iXLine)
					{
						m_Marker[0].iPosition = iChar;
						m_Marker[0].iXLine = iX2;
						m_Marker[0].iXWidth = sizeChar.cx;
						break;
					}
					else
					{
						m_Marker[0].iPosition = iChar+1;
						m_Marker[0].iXLine = iX;
					}
				}
			}
			
			delete[] piWidths;

			if(m_Marker[0].iPosition == -1)
			{
				m_Marker[0].iPosition = iChar;
				m_Marker[0].iXLine = iX;
			}
			DeleteObject(hDC);
		}

		for(int i=0;i<2;i++)
		{
			if(m_Marker[i].iPosition < 0)
				m_Marker[i].iPosition = 0;
			else if(m_Marker[i].iPosition > m_strText.length()  )
				m_Marker[i].iPosition = m_strText.length();
		}
		if(m_Marker[0].iPosition > 0 && m_strText[m_Marker[0].iPosition-1] == '\r')
			m_Marker[0].iPosition+= (iDir == MARKER_HORIZONTAL && iMove>0)?1:-1;

	}
}

//************************************************************************
// CLCDInput::GetText
//************************************************************************
tstring CLCDInput::GetText()
{
	return m_strText;
}

//************************************************************************
// CLCDInput::Reset
//************************************************************************
void CLCDInput::Reset()
{
	m_lInputTime = 0;
	m_bInsert = true;

	memset(&m_Marker[0], 0, sizeof(SMarker));

	m_strText = _T("");
	m_vLineOffsets.clear();
	m_iLinePosition = 0;
	SLineEntry offset;
	offset.bLineBreak = false;
	offset.iOffset = 0;
	m_vLineOffsets.push_back(offset);
	
	if(m_pScrollbar)
	{
		m_pScrollbar->ScrollTo(0);
		m_pScrollbar->SetRange(0,0);
		m_pScrollbar->SetSliderSize(m_iLineCount);
	}
}

//************************************************************************
// CLCDInput::UpdateOffsets
//************************************************************************
void CLCDInput::UpdateOffsets(int iModified)
{
	if(m_vLineOffsets.size() == 0 && m_strText.empty())
		return;

	HDC hDC = CreateCompatibleDC(NULL);
	if(NULL == hDC)
		return;
	
	// Reset the marker
	m_Marker[0].iXLine = 0;
	m_Marker[0].iXWidth = 1;
	
	// Initialize variables
	int iLen = m_strText.length();
	int *piWidths = new int[iLen];
	TCHAR *pszText = (TCHAR*)m_strText.c_str();
	tstring::size_type pos = 0;
	int iMaxChars = 0;

	SIZE sizeWord = {0, 0};
	SIZE sizeChar = {0, 0};
	SIZE sizeLine =  {0, 0};
	SelectObject(hDC, m_hFont);   

	int iLine = -1;
	// Find the first line to update
	for(int i=m_vLineOffsets.size()-1;i>=0;i--)
		if(m_vLineOffsets[i].iOffset <= m_Marker[0].iPosition)
		{
			iLine = i;
			break;
		}

	if(iModified < 0 && iLine-1 >= 0)
		iLine--;

	bool bLineClosed = false;
		
	// TRACE(_T("InputText: Begin Update at #%i\n"),iLine);
	for(;iLine<m_vLineOffsets.size();iLine++)
	{
		bLineClosed = false;

		int iChar = m_vLineOffsets[iLine].iOffset;
		int iWordOffset = iChar;
		
		if(!(iLen == 0) && iChar > iLen)
		{
			// TRACE(_T("InputText: Deleted offset #%i\n"),iLine);	
			m_vLineOffsets.erase(m_vLineOffsets.begin()+iLine);
			continue;
		}
		
		sizeLine.cx = 0;
		sizeWord.cx = 0;
		iWordOffset = iChar+1;

		while(iChar<iLen)
		{
			iWordOffset= iChar;

			GetTextExtentExPoint(hDC,pszText+iChar,iLen-iChar,GetWidth(),&iMaxChars,piWidths,&sizeLine);
			pos = m_strText.find(_T("\n"),iChar);
			// check for linebreaks
			if(pos != tstring::npos && pos >= iChar && pos <= iChar + iMaxChars)
			{
				iWordOffset = pos + 1;
				iMaxChars = pos - iChar;
			}
			// if the string doesnt fit, try to wrap the last word to the next line
			else
			{
				// find the last space in the line
				pos = m_strText.rfind(_T(" "),iChar + iMaxChars);
				if(pos != tstring::npos && pos >= iChar)
					iWordOffset = pos +1;
				else
					iWordOffset = iChar+iMaxChars;
			}

			if(m_Marker[0].iPosition >= iChar && m_Marker[0].iPosition <= iChar + iMaxChars)
			{
				if(m_Marker[0].iPosition > iChar)
				{
					m_Marker[0].iXLine = piWidths[m_Marker[0].iPosition -1 - iChar];
					if(m_strText[m_Marker[0].iPosition -1] == '\n' )
 						m_Marker[0].iXLine = 0;
				}
				if(m_Marker[0].iPosition < iChar + iMaxChars)
				{
					if(m_strText[m_Marker[0].iPosition] > 0x001F)
						m_Marker[0].iXWidth = piWidths[m_Marker[0].iPosition - iChar]-m_Marker[0].iXLine;
				}
			}
		
			//iChar += iMaxChars;

			if(m_strText[iChar] == '\n' || sizeLine.cx > GetWidth())
			{
				
				bLineClosed = true;

				int iDistance = INFINITE;

				
				// Check if a matching offset already exists
				for(int iLine2 = iLine+1;iLine2<m_vLineOffsets.size();iLine2++)
				{
					if(m_vLineOffsets[iLine2].bLineBreak == (m_strText[iChar] == '\n'))
					{
						iDistance =  iChar - (m_vLineOffsets[iLine2].iOffset-1);
						if(m_vLineOffsets[iLine2].iOffset == iWordOffset || iDistance == iModified)	
						{
							// if there are other offsets in front of this one, delete them
							if(iLine2 != iLine + 1 )
							{						
								// TRACE(_T("InputText: Deleted offsets #%i to #%i\n"),iLine+1,iLine2-1);	
								m_vLineOffsets.erase(m_vLineOffsets.begin()+iLine+1,m_vLineOffsets.begin()+iLine2);
							}
							break;
						}
					}
				}
				// A matching offset was found
				if(iDistance == iModified)
				{
					if(iModified != 0)
					{
						// Update line's width
						if(iMaxChars > 0)
						{
							if(m_strText[iChar] == '\n' && iMaxChars >= 2)
								m_vLineOffsets[iLine].iWidth = piWidths[iMaxChars-2];
							else
								m_vLineOffsets[iLine].iWidth = piWidths[iMaxChars-1];
						}
						else
							m_vLineOffsets[iLine].iWidth = 0;

						// TRACE(_T("InputText: shifted offsets #%i to end %i position(s)\n"),iLine+1,iDistance);
						for(iLine++;iLine<m_vLineOffsets.size();iLine++)
							m_vLineOffsets[iLine].iOffset += iDistance;
						
						goto finished;
					}
				}
				// if no matching offset was found, a new one has to be created
				else if(iDistance != 0)
				{
					SLineEntry offset;
					offset.bLineBreak = (m_strText[iChar] == '\n');
					offset.iOffset = iWordOffset;
					if(iLine == m_vLineOffsets.size()-1)
						m_vLineOffsets.push_back(offset);
					else
						m_vLineOffsets.insert(m_vLineOffsets.begin()+iLine+1,offset);
					
					// TRACE(_T("InputText: Inserted new  %soffset at #%i\n"),m_strText[iChar] == '\n'?_T("linebreak "):_T(""),iLine+1);	
				}
				break;
			}

			iChar += iMaxChars;
		}
		// Update line's width
		if(iMaxChars > 0)
		{
			if(m_strText[iChar-1] == '\n' && iMaxChars >= 2)
				m_vLineOffsets[iLine].iWidth = piWidths[iMaxChars-2];
			else
				m_vLineOffsets[iLine].iWidth = piWidths[iMaxChars-1];
		}
		else
			m_vLineOffsets[iLine].iWidth = 0;

		if(iLine != m_vLineOffsets.size() - 1 && !bLineClosed)
		{
			// TRACE(_T("InputText: Deleted offsets #%i to #%i\n"),iLine+1,m_vLineOffsets.size()-1);	
			m_vLineOffsets.erase(m_vLineOffsets.begin()+iLine+1,m_vLineOffsets.end());
		}
		
	}

finished:
	delete[] piWidths;
	DeleteObject(hDC);
	
	if(m_pScrollbar)
		m_pScrollbar->SetRange(0,m_vLineOffsets.size()-1);
}

//************************************************************************
// CLCDInput::UpdateMarker
//************************************************************************
void CLCDInput::UpdateMarker()
{
	// Adjust the markers propertys
	for(int i=m_vLineOffsets.size()-1;i>= 0;i--)
		if(m_Marker[0].iPosition >= m_vLineOffsets[i].iOffset)
		{
			if(m_Marker[0].iPosition == m_vLineOffsets[i].iOffset)
				m_Marker[0].iXLine = 0;
			m_Marker[0].iLine = i;
			break;
		}
}

//************************************************************************
// CLCDInput::ScrollLine
//************************************************************************
bool CLCDInput::ScrollLine(bool bDown)
{
	if(bDown && m_iLinePosition + (m_iLineCount-1) < m_vLineOffsets.size() -1)
			m_iLinePosition++;
	else if(!bDown && m_iLinePosition  > 0)
			m_iLinePosition--;
	else
		return false;
	
	if(m_pScrollbar)
		m_pScrollbar->ScrollTo(m_iLinePosition);
	return true;
}


//************************************************************************
// CLCDInput::ScrollToMarker
//************************************************************************
void CLCDInput::ScrollToMarker()
{
	if(m_Marker[0].iLine < m_iLinePosition)
		m_iLinePosition = m_Marker[0].iLine;
	if(m_Marker[0].iLine > m_iLinePosition + (m_iLineCount-1))
	{
		ScrollLine();
		if(m_Marker[0].iLine > m_iLinePosition + (m_iLineCount-1))
			m_iLinePosition = (m_Marker[0].iLine / m_iLineCount )*m_iLineCount;
	}

	if(m_iLinePosition > m_vLineOffsets.size()-1)
		m_iLinePosition = m_vLineOffsets.size() -1;
	if(m_iLinePosition < 0)
		m_iLinePosition = 0;

	if(m_pScrollbar)
		m_pScrollbar->ScrollTo(m_iLinePosition);
}

//************************************************************************
// CLCDInput::GetLineCount
//************************************************************************
int CLCDInput::GetLineCount()
{
	return m_vLineOffsets.size();
}

//************************************************************************
// CLCDInput::GetLine
//************************************************************************
int CLCDInput::GetLine()
{
	return m_iLinePosition;
}