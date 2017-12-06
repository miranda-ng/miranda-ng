#include "stdafx.h"
#include "CEventLog.h"
#include "CConfig.h"
#include "CAppletManager.h"

//************************************************************************
// constructor
//************************************************************************
CEventLog::CEventLog()
{
	m_dwLastScroll = 0;
}

//************************************************************************
// destructor
//************************************************************************
CEventLog::~CEventLog()
{
	
}

//************************************************************************
// deinitializes the list
//************************************************************************
bool CEventLog::Shutdown()
{
	if(!CLCDList<CEventLogEntry*>::Shutdown())
		return false;

	return true;
}

//************************************************************************
// initializes the list
//************************************************************************
bool CEventLog::Initialize()
{
	if(!CLCDList<CEventLogEntry*>::Initialize())
		return false;

	return true;
}

//************************************************************************
// Called to set the objects font
//************************************************************************
bool CEventLog::SetFont(LOGFONT &lf)
{
	if(!CLCDList<CEventLogEntry*>::SetFont(lf))
		return false;

	SetEntryHeight(m_iFontHeight<6?6:m_iFontHeight);
	
	return true;
}

//************************************************************************
// adds an entry to the list
//************************************************************************
CListItem<CEventLogEntry*> *CEventLog::AddItem(CEventLogEntry *pEntry)
{
	CListItem<CEventLogEntry*> *pItem = CLCDList<CEventLogEntry*>::AddItem(pEntry);
	if(GetEntryCount() > CConfig::GetIntSetting(NOTIFY_LOGSIZE))
		RemoveItem(((CListItem<CEventLogEntry*>*)GetFirstEntry())->GetItemData());
	
	if(GetTickCount() - m_dwLastScroll > 10000)
		SetPosition(pItem);
	return pItem;
}

//************************************************************************
// Called to delete the specified entry
//************************************************************************
void CEventLog::DeleteEntry(CEventLogEntry *pEntry)
{
	delete pEntry;
}

//************************************************************************
// Called to draw the specified entry
//************************************************************************
void CEventLog::DrawEntry(CLCDGfx *pGfx,CEventLogEntry *pEntry,bool bSelected)
{
	SelectObject(pGfx->GetHDC(),m_hFont);

	bool bLargeIcons = GetEntryHeight() > 8;
	int iOffset = (m_iFontHeight-(bLargeIcons?8:6))/2;
	HBITMAP hBitmap = CAppletManager::GetInstance()->GetEventBitmap(pEntry->eType,bLargeIcons);
	pGfx->DrawBitmap(0,iOffset<0?0:iOffset,bLargeIcons?8:6,bLargeIcons?8:6,hBitmap);

	iOffset = bLargeIcons?10:7;
	if(CConfig::GetBoolSetting(NOTIFY_TIMESTAMPS))
		pGfx->DrawText(iOffset,0,pGfx->GetClipWidth()-iOffset,pEntry->strTimestamp + pEntry->strValue);
	else
		pGfx->DrawText(iOffset,0,pGfx->GetClipWidth()-iOffset,pEntry->strValue);

	if(bSelected && GetTickCount() - m_dwLastScroll < 1000)
	{
		RECT invert = { 0,0,GetWidth(),m_iFontHeight};
		InvertRect(pGfx->GetHDC(),&invert);
	}
}


void CEventLog::SetPosition(CListEntry<CEventLogEntry*> *pEntry)
{
	CLCDList<CEventLogEntry*>::SetPosition(pEntry);
}

bool CEventLog::ScrollUp()
{
	m_dwLastScroll = GetTickCount();
	return CLCDList<CEventLogEntry*>::ScrollUp();
}

bool CEventLog::ScrollDown()
{
	m_dwLastScroll = GetTickCount();
	return CLCDList<CEventLogEntry*>::ScrollDown();
}
