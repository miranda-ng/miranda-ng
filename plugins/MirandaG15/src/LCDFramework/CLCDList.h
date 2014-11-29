#ifndef _CLCDLIST_H_
#define _CLCDLIST_H_

#include "CLCDTextObject.h"
#include "CLCDBar.h"
#include <math.h>


enum EListEntryType { ROOT = 0,CONTAINER = 1,ITEM = 2};

template <class T, class G = tstring> class CListEntry
{
public:
	CListEntry(CListEntry<T,G> *pParent)
	{
		m_iIndex = -1;
		m_iEntryCount = 0;
		m_Position = NULL;
		m_pParent = pParent;
		if(pParent == NULL)
		{
			m_iLevel = 0;
			m_eType = ROOT;
			m_pRoot = this;
		}
		else
			m_iLevel = m_pParent->GetLevel()+1;
	}
	
	virtual ~CListEntry()
	{
	
	}

	int GetLevel()
	{
		return m_iLevel;
	}

	virtual CListEntry<T,G> *GetNextEntry()
	{
		if(m_pParent == NULL)
			return NULL;

		return m_pParent->GetNextEntry(this);
	}
	
	virtual CListEntry<T,G> *GetPreviousEntry()
	{
		if(m_pParent == NULL)
			return NULL;

		return m_pParent->GetPreviousEntry(this);
	}

	virtual CListEntry<T,G> *GetNextEntry(CListEntry<T,G> *pEntry)
	{
		return NULL;
	}

	virtual CListEntry<T,G> *GetPreviousEntry(CListEntry<T,G> *pEntry)
	{
		return NULL;
	}
	
	EListEntryType GetType()
	{
		return m_eType;
	}
	
	int	GetEntryCount()
	{
		return m_iEntryCount;
	}

	virtual void UpdateEntryCount()
	{
		m_iEntryCount = 0;
	}

	void SetRoot(CListEntry<T,G>* pRoot)
	{
		m_pRoot = pRoot;
	}

	virtual void DeleteItem(T Entry)
	{
	}

	virtual void DeleteGroup(G Group)
	{
	}

	CListEntry<T,G> *GetPosition()
	{
		return m_Position;
	}
	
	virtual void SetPosition(CListEntry<T,G> *pPosition)
	{
		m_Position = pPosition;
	}

	CListEntry<T,G> *GetParent()
	{
		return m_pParent;
	}
	
	int	GetIndex()
	{
		return m_iIndex;
	}

	void SetIndex(int iIndex)
	{
		m_iIndex = iIndex;
	}

protected:
	int			m_iIndex;
	int			m_iEntryCount;
	int			m_iLevel;
	EListEntryType m_eType;
	CListEntry<T,G> *m_pParent;
	CListEntry<T,G> *m_pRoot;	
	CListEntry<T,G> *m_Position;
};

template <class T, class G = tstring> class CListItem : public CListEntry<T,G>
{
public:
	CListItem(CListEntry<T,G> *pParent,T Entry) : CListEntry<T,G>(pParent)
	{
		m_Item = Entry;
		m_eType = ITEM;
	}
	
	~CListItem()
	{
		m_pRoot->DeleteItem(GetItemData());
	}

	T GetItemData()
	{
		return m_Item;
	}
private:
	T	m_Item;
};

template <class T, class G = tstring> class CListContainer : public CListEntry<T,G>
{
public:
	typedef typename list<CListEntry<T,G>* >::iterator iterator;

	typename list<CListEntry<T,G>* >::iterator end()
	{
		return m_Entrys.end();
	}
	
	typename list<CListEntry<T,G>* >::iterator begin()
	{
		return m_Entrys.begin();
	}

	typename list<CListEntry<T,G>* >::size_type size()
	{
		return m_Entrys.size();
	}

	bool empty()
	{
		bool b = m_Entrys.empty();
		return m_Entrys.empty();
	}

	CListContainer(CListEntry<T,G> *pParent) : CListEntry<T,G>(pParent)
	{
		if(m_pParent != NULL)
		{
			m_eType = CONTAINER;
			m_bOpen = false;
		}
		else
			m_bOpen = true;
	}

	~CListContainer()
	{
		if(m_pRoot != NULL) {
			m_pRoot->DeleteGroup(GetGroupData());
		}
		Clear();
	}

	void Clear()
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		CListItem<T,G> *pItem = NULL;
		CListContainer<T,G> *pContainer = NULL;

		while(iter != m_Entrys.end())
		{
			delete *iter;
			if(m_pRoot && m_pRoot->GetPosition() == *iter)
			{
				if(GetType() == ROOT)
					m_pRoot->SetPosition(NULL);
				else
					m_pRoot->SetPosition(this);
			}
			
			iter++;
		}
		m_Entrys.clear();
	}

	void SetGroupData(G GroupData)
	{
		m_GroupData = GroupData;
	}

	bool IsEmpty()
	{
		return m_Entrys.empty();
	}

	CListEntry<T,G> *GetLastOwnEntry()
	{
		if(m_Entrys.empty())
			return NULL;

		return *(--m_Entrys.end());
	}

	CListEntry<T,G> *GetLastEntry()
	{
		if(!m_Entrys.empty())
		{

			CListEntry<T,G> *pEntry = *(--m_Entrys.end());
			if(pEntry->GetType() == ITEM || !((CListContainer<T,G>*)pEntry)->IsOpen() || ((CListContainer<T,G>*)pEntry)->IsEmpty())
				return pEntry;
			return ((CListContainer<T,G>*)pEntry)->GetLastEntry();
		}
		return NULL;
	}

	CListEntry<T,G> *GetFirstEntry()
	{
		if(!m_Entrys.empty())
			return *(m_Entrys.begin());
		return NULL;
	}

	CListEntry<T,G> *GetNextEntry()
	{
		if(!IsOpen() || m_Entrys.empty())
		{
			if(!m_pParent)
				return NULL;
			return m_pParent->GetNextEntry(this);
		}

		return *m_Entrys.begin();
	}

	CListEntry<T,G> *GetNextEntry(CListEntry<T,G> *pEntry)
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		while(iter != m_Entrys.end())
		{
			if((CListEntry<T,G>*)(*iter) == pEntry)
			{
				if(++iter == m_Entrys.end())
				{
					if(m_pParent == NULL)
						return NULL;
					return m_pParent->GetNextEntry(this);
				}
				else
					return *iter;
			}
			iter++;
		}
		return NULL;
	}

	CListEntry<T,G> *GetPreviousEntry(CListEntry<T,G> *pEntry)
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();

		while(iter != m_Entrys.end())
		{
			if((CListEntry<T,G>*)(*iter) == pEntry)
			{
				if(iter == m_Entrys.begin())
				{
					if(m_pParent == NULL)
						return NULL;
					return this;
				}
				else
				{
					iter--;
					if((*iter)->GetType() == CONTAINER)
					{
						CListContainer<T,G>* pContainer = (CListContainer<T,G>*)*iter;
						if(pContainer->IsOpen() && !pContainer->IsEmpty())
							return pContainer->GetLastEntry();
					}
					return *iter;
				}
			}
			iter++;
		}
		return NULL;
	}

	virtual CListItem<T,G> *InsertItem(iterator _Where,T Entry)
	{
		CListItem<T,G> *pItem = new CListItem<T,G>(this,Entry);
		pItem->SetRoot(m_pRoot);
		m_Entrys.insert(_Where,pItem);

		m_pRoot->UpdateEntryCount();

		return pItem;
	}

	virtual CListContainer<T,G> *InsertGroup(iterator _Where,G Group)
	{
		CListContainer<T,G> *pGroup = new CListContainer<T,G>(this);
		pGroup->SetGroupData(Group);
		pGroup->SetRoot(m_pRoot);
		m_Entrys.insert(_Where,(CListEntry<T,G>*)pGroup);
		
		m_pRoot->UpdateEntryCount();

		return pGroup;
	}

	virtual CListItem<T,G> * AddItem(T Entry)
	{
		return InsertItem(end(),Entry);
	}

	virtual CListContainer<T,G> * AddGroup(G Group)
	{
		return InsertGroup(end(),Group);	
	}
	
	

	virtual void RemoveGroup(G Group)
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		CListContainer<T,G> *pContainer = NULL;
		while(iter != m_Entrys.end())
		{
			if((*iter)->GetType() == CONTAINER)
			{
				pContainer = (CListContainer<T,G>*)(*iter);
				if(pContainer->GetGroupData() == Group)
				{
					pContainer->Clear();
					if(m_pRoot && m_pRoot->GetPosition() == *iter)
					{
						CListEntry<T,G> *pPosition = (*iter)->GetPreviousEntry();
						if(!pPosition)
							pPosition = (*iter)->GetNextEntry();
						m_pRoot->SetPosition(pPosition);
					}
					delete *iter;
					m_Entrys.erase(iter);
					m_pRoot->UpdateEntryCount();
					return;
				}
			}
			iter++;
		}
	}

	virtual void RemoveItem(T Entry)
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		CListItem<T,G> *pItem = NULL;
		while(iter != m_Entrys.end())
		{
			if((*iter)->GetType() == ITEM)
			{
				pItem = (CListItem<T,G>*)(*iter);
				if(pItem->GetItemData() == Entry)
				{
					if(m_pRoot && m_pRoot->GetPosition() == *iter)
					{
						CListEntry<T,G> *pPosition = (*iter)->GetPreviousEntry();
						if(!pPosition)
							pPosition = (*iter)->GetNextEntry();
						m_pRoot->SetPosition(pPosition);
					}
					delete *iter;
					
					m_Entrys.erase(iter);
					m_pRoot->UpdateEntryCount();
					return;
				}
			}
			iter++;
		}
	}

	CListContainer<T,G> *GetGroup(G Group)
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		CListContainer<T,G> *pContainer = NULL;
		while(iter != m_Entrys.end())
		{
			if((*iter)->GetType() == CONTAINER)
			{
				pContainer = (CListContainer<T,G>*)(*iter);
				if(pContainer->GetGroupData() == Group)
					return pContainer;
			}
			iter++;
		}
		return NULL;
	}
	
	G GetGroupData()
	{
		return m_GroupData;
	}

	bool IsOpen()
	{
		return m_bOpen;
	}
	
	void ToggleOpen()
	{
		m_bOpen = !m_bOpen;
		
		if (m_pRoot) {
			m_pRoot->UpdateEntryCount();
			m_pRoot->SetPosition(this);
		}
	}
	
	void SetOpen(bool bOpen = true)
	{
		if(bOpen == m_bOpen)
			return;

		m_bOpen = bOpen;

		if (m_pRoot) {
			m_pRoot->UpdateEntryCount();
			m_pRoot->SetPosition(this);
		}
	}
	
	void CollapseAll()
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		CListContainer<T,G>* pContainer = NULL;
		while(iter != m_Entrys.end())
		{
			if((*iter)->GetType() == CONTAINER)
			{
				pContainer = (CListContainer<T,G>*)(*iter);
				pContainer->CollapseAll();
				pContainer->SetOpen(false);
			}
			iter++;
		}
	}
	
	void ExpandAll()
	{
		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		CListContainer<T,G>* pContainer = NULL;
		while(iter != m_Entrys.end())
		{
			if((*iter)->GetType() == CONTAINER)
			{
				pContainer = (CListContainer<T,G>*)(*iter);
				pContainer->ExpandAll();
				pContainer->SetOpen(true);
			}
			iter++;
		}
	}

	void UpdateEntryCount()
	{
		m_iEntryCount = 0;

		int iIndex = GetIndex()+1;
		
		if(!IsOpen())
			return;

		list< CListEntry<T,G>* >::iterator iter = m_Entrys.begin();
		while(iter != m_Entrys.end())
		{
			(*iter)->SetIndex(iIndex+m_iEntryCount);
			(*iter)->UpdateEntryCount();
			m_iEntryCount += 1+(*iter)->GetEntryCount();

			iter++;
		}

		if(GetType() == ROOT)
		{
			if(GetPosition() == NULL && !m_Entrys.empty())
				SetPosition(*m_Entrys.begin());
			else
				SetPosition(GetPosition());
		}
	}
	
	template<class _Pr3>
	void sort(_Pr3 _Pred) {
		m_Entrys.sort(_Pred);
		UpdateEntryCount();
		m_pRoot->SetPosition(m_pRoot->GetPosition());
	}

private:
	typename list< CListEntry<T,G>* > m_Entrys;
	G m_GroupData;
	bool m_bOpen;
};


template <class T, class G = tstring> class CLCDList :  public CLCDTextObject, public CListContainer<T,G>
{
friend CListContainer<T,G>;
friend CListItem<T,G>;
public:
	//************************************************************************
	// Constructor
	//************************************************************************
	CLCDList() : CListContainer<T,G>(NULL)
	{
		m_pScrollbar = NULL;
		m_iIndention = 10;
		m_iColumns = 1;
		m_bDrawTreeLines = true;
		m_iEntryHeight = 10;
	}

	//************************************************************************
	// Destructor
	//************************************************************************
	~CLCDList()
	{
	}

	//************************************************************************
	// Initializes the list
	//************************************************************************
	bool Initialize()
	{
		if(!CLCDTextObject::Initialize())
			return false;

		return true;
	}

	//************************************************************************
	// Deinitializes the list
	//************************************************************************
	bool Shutdown()
	{
		if(!CLCDTextObject::Shutdown())
			return false;
	
		Clear();

		return true;
	}

	//************************************************************************
	// updates the list
	//************************************************************************
	bool Update()
	{
		if(!CLCDTextObject::Update())
			return false;

		return true;
	}

	//************************************************************************
	// draws the list
	//************************************************************************
	bool Draw(CLCDGfx *pGfx)
	{
		if(!CLCDTextObject::Draw(pGfx))
			return false;
		
		SelectObject(pGfx->GetHDC(),m_hFont);

		POINT ptPrevViewportOrg = { 0, 0 };
		HRGN hRgn = NULL;
		int iHeight = 0,iLimit=0;
		int iYOffset = 0, iXOffset=0;
		int iColWidth = (GetWidth()- (m_iColumns-1)*3)/m_iColumns;
		int iSpace = GetHeight() - (GetHeight()/m_iEntryHeight)*m_iEntryHeight;
		int iPerPage = (GetHeight()/m_iEntryHeight)*m_iColumns;
		
		int iEntriesDrawn = 0;
		CListEntry<T,G> *pPosition = m_Position;
		
		// if nothing is selected, skip drawing
		if(pPosition == NULL)
			return true;

		bool bDrawGroup = false;
		bool bSelected = false;

		// calculate the start offset
		
		if(m_iStartIndex < pPosition->GetIndex())
		{
			while(pPosition && pPosition->GetIndex() != m_iStartIndex)
				pPosition = pPosition->GetPreviousEntry();
		}
		
		if(m_iStartIndex > 0 && pPosition->GetIndex() > 0)
			pPosition = pPosition->GetPreviousEntry();

		for(int iCol = 0;iCol<m_iColumns;iCol++)
		{
			iHeight = 0;
			if(iCol == 0)
			{
				if(pPosition->GetIndex() < m_iStartIndex)
					iHeight -= m_iEntryHeight-iSpace;
				else if(GetEntryCount() >= (iPerPage/m_iColumns) +1)
					iHeight = iSpace;
			}

			// bottom selection
			while(pPosition != NULL )
			{
				iYOffset = iHeight;

				bSelected = m_Position == pPosition;
				bDrawGroup = pPosition->GetType() == CONTAINER;
				
				// ~~~~~~~~~~~~~~~~~~~~~~
				// Draw tree lines
				// ~~~~~~~~~~~~~~~~~~~~~~

				// set the clip region for the entry
				int iClipHeight = m_iEntryHeight;
				if(GetOrigin().y+iYOffset+iClipHeight > GetOrigin().y + GetHeight())
					iClipHeight = GetHeight() - iYOffset;

				pGfx->SetClipRegion(GetOrigin().x+iXOffset,GetOrigin().y+iYOffset,
								iColWidth, iClipHeight);
				
				// offset the control at its origin so entry use (0,0)
				SetViewportOrgEx(pGfx->GetHDC(),
							 GetOrigin().x+iXOffset,
							 GetOrigin().y+iYOffset,
							&ptPrevViewportOrg);

				if(m_bDrawTreeLines)
				{
					for(int i=1;i<pPosition->GetLevel();i++)
					{
						if(i == pPosition->GetLevel()-1)
						{
							// -
							pGfx->DrawLine((i-1)*m_iIndention+m_iIndention/2,m_iEntryHeight/2,i*m_iIndention,m_iEntryHeight/2);
							// |
							if(pPosition == ((CListContainer<T,G>*)pPosition->GetParent())->GetLastOwnEntry())
								pGfx->DrawLine((i-1)*m_iIndention+m_iIndention/2,0,(i-1)*m_iIndention+m_iIndention/2,m_iEntryHeight/2);
							// |
							// |
							else
								pGfx->DrawLine((i-1)*m_iIndention+m_iIndention/2,0,(i-1)*m_iIndention+m_iIndention/2,m_iEntryHeight);
						}
						else
						{
							CListEntry<T,G> *pPosition2 = pPosition;
							for(int j = pPosition->GetLevel();j>i+1;j--)
								pPosition2 = pPosition2->GetParent();
							// |
							// |
							if(pPosition2 != ((CListContainer<T,G>*)pPosition2->GetParent())->GetLastOwnEntry())
								pGfx->DrawLine((i-1)*m_iIndention+m_iIndention/2,0,(i-1)*m_iIndention+m_iIndention/2,m_iEntryHeight);
						}	
					}
				}

				// ~~~~~~~~~~~~~~~~~~~~~~
				// Draw the entry
				// ~~~~~~~~~~~~~~~~~~~~~~
				pGfx->SetClipRegion(GetOrigin().x+(pPosition->GetLevel()-1)*m_iIndention+iXOffset,
									GetOrigin().y+iYOffset,
									iColWidth-(pPosition->GetLevel()-1)*m_iIndention,
									iClipHeight);
				// set the offset
				SetViewportOrgEx(pGfx->GetHDC(),
							 GetOrigin().x+(pPosition->GetLevel()-1)*m_iIndention+iXOffset,
							 GetOrigin().y+iYOffset,
							&ptPrevViewportOrg);

				// draw the entry
				if(!bDrawGroup)
					DrawEntry(pGfx,((CListItem<T,G>*)pPosition)->GetItemData(),bSelected);
				else
				// draw the group
					DrawGroup(pGfx,((CListContainer<T,G>*)pPosition)->GetGroupData(),((CListContainer<T,G>*)pPosition)->IsOpen(),bSelected);
				
				// ~~~~~~~~~~~~~~~~~~~~~~

				if(pPosition->GetIndex() >= m_iStartIndex && iHeight + m_iEntryHeight <= GetHeight())
					iEntriesDrawn++;
				
				iHeight += m_iEntryHeight;
				pPosition = pPosition->GetNextEntry();
				
				if(iHeight >= GetHeight())
					break;
			}
			if(iCol != m_iColumns-1)
			{
				pGfx->SetClipRegion(GetOrigin().x,
									GetOrigin().y,
									GetWidth(),
									GetHeight());
				// set the offset
				SetViewportOrgEx(pGfx->GetHDC(),
							 GetOrigin().x,
							 GetOrigin().y,
							&ptPrevViewportOrg);

				pGfx->DrawLine(iCol*3 + iColWidth + 1,0,iCol*3 + iColWidth + 1,GetHeight());
			}
			iXOffset += 3 + iColWidth;
		}

		if(m_pScrollbar)
		{
			m_pScrollbar->ScrollTo(m_iStartIndex);		
			m_pScrollbar->SetSliderSize(iEntriesDrawn);
		}		
		return true;
	}


	void SetPosition(CListEntry<T,G> *pEntry)
	{
		CListContainer<T,G>::SetPosition(pEntry);

		if(pEntry == NULL)
			return;

		int iPerPage = (GetHeight()/m_iEntryHeight)*m_iColumns;
		m_iStartIndex = pEntry->GetIndex();
		if(m_iStartIndex + (iPerPage-1) > GetEntryCount()-1)
			m_iStartIndex = (GetEntryCount()-1)-(iPerPage-1);
		if(m_iStartIndex < 0)
			m_iStartIndex = 0;
	}

	//************************************************************************
	// scrolls up
	//************************************************************************
	bool ScrollUp()
	{
		if(m_Position != NULL)
		{
			CListEntry<T,G> *pEntry = m_Position->GetPreviousEntry();
			if(pEntry != NULL)
			{
				m_Position = pEntry;
				
				int iPerPage = (GetHeight()/m_iEntryHeight)*m_iColumns;
				if(m_Position->GetIndex() < m_iStartIndex)
					m_iStartIndex--;
				return true;
			}
		}
		return false;
	}

	//************************************************************************
	// scrolls down
	//************************************************************************
	bool ScrollDown()
	{		
		if(m_Position != NULL)
		{
			CListEntry<T,G> *pEntry = m_Position->GetNextEntry();
			if(pEntry != NULL)
			{
				m_Position = pEntry;

				int iPerPage = (GetHeight()/m_iEntryHeight)*m_iColumns;
				if(m_Position->GetIndex() >= m_iStartIndex + iPerPage)
					m_iStartIndex++;
				return true;
			}			
		}
		return false;
	}

	//************************************************************************
	// returns the selected list entry
	//************************************************************************
	CListEntry<T,G> *GetSelectedEntry()
	{
		return m_Position;
	}

	//************************************************************************
	// associates a scrollbar with the list
	//************************************************************************
	void SetScrollbar(CLCDBar *pScrollbar)
	{
		m_pScrollbar = pScrollbar;
		if(m_pScrollbar)
		{
			m_pScrollbar->SetRange(0,m_iEntryCount-1);
			m_pScrollbar->ScrollTo(m_Position != NULL?m_Position->GetIndex():0);
		
			m_pScrollbar->SetAlignment(TOP);
		}
	}

	//************************************************************************
	// sets the group indention in pixels
	//************************************************************************
	void SetIndention(int iIndention)
	{
		m_iIndention = iIndention;
	}
	
	//************************************************************************
	// sets the lists entry height
	//************************************************************************
	void SetEntryHeight(int iEntryHeight)
	{
		m_iEntryHeight = iEntryHeight;
	}

	//************************************************************************
	// returns the lists entry height
	//************************************************************************
	int GetEntryHeight()
	{
		return m_iEntryHeight;
	}

	//************************************************************************
	// enables/disables drawing of treelines
	//************************************************************************
	void SetDrawTreeLines(bool bDraw)
	{
		m_bDrawTreeLines = bDraw;
	}

	//************************************************************************
	// sets the amount of columns the list uses
	//************************************************************************
	void SetColumns(int iColumns)
	{
		if(m_iColumns == iColumns)
			return;
		m_iColumns = iColumns;
		SetPosition(GetPosition());
	}
protected:
	//************************************************************************
	// called when the lists size has changed
	//************************************************************************
	void OnSizeChanged()
	{
		SetPosition(GetPosition());
	}

	//************************************************************************
	// updates the list's entry count
	//************************************************************************
	void UpdateEntryCount()
	{
		CListContainer<T,G>::UpdateEntryCount();
		if(m_pScrollbar)
		{
			m_pScrollbar->SetRange(0,m_iEntryCount-1);
			if(GetPosition() != NULL)
				m_pScrollbar->ScrollTo(GetPosition()->GetIndex());
		}
	}

	//************************************************************************
	// Called to delete the specified entry
	//************************************************************************
	virtual void DeleteEntry(T Entry)
	{

	}

	//************************************************************************
	// Called to delete the specified group
	//************************************************************************
	virtual void DeleteGroup(G Group)
	{

	}

	//************************************************************************
	// Called to draw the specified entry
	//************************************************************************
	virtual void DrawEntry(CLCDGfx *pGfx,T Entry, bool bSelected)
	{
	}
	
	//************************************************************************
	// Called to draw the specified entry
	//************************************************************************
	virtual void DrawGroup(CLCDGfx *pGfx,G Group, bool bOpen, bool bSelected)
	{
	}



protected:
	int		m_iStartIndex;
	int		m_iColumns;

	bool	m_bDrawTreeLines;
	int		m_iIndention;
	int		m_iEntryHeight;
	CLCDBar	*m_pScrollbar;
};

#endif