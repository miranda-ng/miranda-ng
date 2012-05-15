/*

dbx_tree: tree database driver for Miranda IM

Copyright 2007-2010 Michael "Protogenes" Kunz,

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#pragma once 

#include <vector>

template <class TType>
class CIterationHeap
{
public:
	enum TIterationType {ITForward, ITBackward};

	CIterationHeap(TType & InitialItem, TIterationType ForBack, bool DeleteItems = true);
	~CIterationHeap();

	bool Insert(TType & Item);
	TType & Top();
	void Pop();
	
protected:
	typedef struct THeapElement {
		TType * Elem;
		size_t Index;
	} THeapElement, * PHeapElement;

	std::vector <PHeapElement> m_Heap;
	TIterationType m_Type;
	bool m_DeleteItems;	

	bool A_b4_B(PHeapElement a, PHeapElement b);
private:

};


template <class TType>
inline bool CIterationHeap<TType>::A_b4_B(PHeapElement a, PHeapElement b)
{
	if (m_Type == ITForward)
	{
		if ((**a->Elem) == (**b->Elem)) return a->Index < b->Index;
		return (**a->Elem) < (**b->Elem);
	} else {
		if ((**a->Elem) == (**b->Elem)) return a->Index > b->Index;
		return (**a->Elem) > (**b->Elem);
	}
}

template <class TType>
CIterationHeap<TType>::CIterationHeap(TType & InitialItem, TIterationType MinMax, bool DeleteItems)
:	m_Heap()
{
	m_Heap.resize(1);
	m_Heap.reserve(1 << 1);
	m_Type = MinMax;
	
	m_Heap[0] = new THeapElement;
	m_Heap[0]->Elem = &InitialItem;
	m_Heap[0]->Index = m_Heap.size();
	m_DeleteItems = DeleteItems;
}

template <class TType>
CIterationHeap<TType>::~CIterationHeap()
{
	unsigned int i = 0; 
	while ((i < m_Heap.size()) && (m_Heap[i]))
	{
		if (m_DeleteItems && m_Heap[i]->Elem)
			delete m_Heap[i]->Elem;

		delete m_Heap[i];
		++i;
	}
}

template <class TType>
bool CIterationHeap<TType>::Insert(TType & Item)
{
	if (!Item) 
		return false;

	if (m_Heap.capacity() == m_Heap.size() + 1)		
		m_Heap.reserve(m_Heap.capacity() << 1);
	
	m_Heap.push_back(NULL);

	size_t way = m_Heap.capacity() >> 2;	
	size_t index = 0;	
	PHeapElement ins = new THeapElement;
	ins->Elem = &Item;
	ins->Index = m_Heap.size();

	PHeapElement next;

	while ((way > 0) && (index + 1 < m_Heap.size()))
	{
		next = m_Heap[index];
		if ((!(*next->Elem)) || A_b4_B(ins, next))
		{
			m_Heap[index] = ins;
			ins = next;
		}

		if (way & m_Heap.size())	 //right
		{
			index = (index << 1) + 2;
		} else {	// left
			index = (index << 1) + 1;
		}
		way = way >> 1;						
	}

	m_Heap[index] = ins;

	return true;
}

template <class TType>
TType & CIterationHeap<TType>::Top()
{
	return *m_Heap[0]->Elem;
}

template <class TType>
void CIterationHeap<TType>::Pop()
{
	if (m_Type == ITForward)
		++(*m_Heap[0]->Elem);
	else
		--(*m_Heap[0]->Elem);

	size_t index = 0;
	PHeapElement ins = m_Heap[0];
	size_t big = 1;

	while ((big > 0) && (index < (m_Heap.size() >> 1)))
	{
		big = 0;

		if ((((index << 1) + 2) < m_Heap.size()) && (*m_Heap[(index << 1) + 2]->Elem)) 
		{
			if (*ins->Elem)
			{
				if (A_b4_B(m_Heap[(index << 1) + 2], m_Heap[(index << 1) + 1]))
					big = (index << 1) + 2;					
				else 
					big = (index << 1) + 1;

			} else {				
				m_Heap[index] = m_Heap[(index << 1) + 2];
				index = (index << 1) + 2;
				m_Heap[index] = ins;
			}
		} else if ((((index << 1) + 1) < m_Heap.size()) && (*m_Heap[(index << 1) + 1]->Elem))
		{
			if (*ins->Elem) 
			{
				big = (index << 1) + 1;
			} else {
				m_Heap[index] = m_Heap[(index << 1) + 1];
				index = (index << 1) + 1;
				m_Heap[index] = ins;
			}
		}

		if ((big > 0) && A_b4_B(m_Heap[big], ins))
		{
			m_Heap[index] = m_Heap[big];
			index = big;
			m_Heap[big] = ins;
		} else {
			big = 0;
		}
	}
}
