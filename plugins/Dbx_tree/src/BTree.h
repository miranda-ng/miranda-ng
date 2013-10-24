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

#include <stack>
#include "lockfree_hashmultimap.h"
#include "sigslot.h"
#include <stdint.h>

#include "Logger.h"

template <typename TKey, uint16_t SizeParam = 4>
class CBTree
{
public:
	typedef uint32_t TNodeRef; /// 32bit indices (not storing pointers)
	typedef sigslot::signal2< void *, TNodeRef > TOnRootChanged;

	#pragma pack(push, 1)  // push current alignment to stack, set alignment to 1 byte boundary

		typedef struct TNode {
			uint16_t Info;                 /// Node information (IsLeaf and stored KeyCount)
			uint16_t Signature;            /// signature
			TNodeRef Parent;               /// Handle to the parent node
			TKey Key[SizeParam * 2 - 1];   /// array with Keys
			TNodeRef Child[SizeParam * 2]; /// array with child node handles
		} TNode;

	#pragma pack(pop)

	class iterator
	{
	public:
		iterator();
		iterator(CBTree* Tree, TNodeRef Node, uint16_t Index);
		iterator(const iterator& Other);
		~iterator();

		CBTree * Tree();

		/**
			\brief Keeps track of changes in the tree and refresh the iterator
		**/
		void setManaged();
		bool wasDeleted();

		operator bool() const;
		bool operator !() const;

		const TKey & operator *();
		const TKey * operator->();


		bool operator == (iterator & Other);
		bool operator <  (iterator & Other);
		bool operator >  (iterator & Other);

		iterator& operator =(const iterator& Other);

		iterator& operator ++(); //pre  ++i
		iterator& operator --(); //pre  --i
		iterator  operator ++(int); //post i++
		iterator  operator --(int); //post i--


	protected:
		friend class CBTree;

		TNodeRef m_Node;
		uint16_t m_Index;
		CBTree* m_Tree;

		bool m_Managed;
		bool m_LoadedKey;
		TKey m_ManagedKey;
		bool m_ManagedDeleted;

		void Backup();
		void Dec();
		void Inc();
		void RemoveManaged(TNodeRef FromNode);
		void InsertManaged();
	};


	CBTree(TNodeRef RootNode = 0);
	virtual ~CBTree();

	iterator Insert(const TKey & Key);
	iterator Find(const TKey & Key);
	iterator LowerBound(const TKey & Key);
	iterator UpperBound(const TKey & Key);
	bool Delete(const TKey & Key);

	typedef sigslot::signal3<void *, const TKey &, uint32_t> TDeleteCallback;
	void DeleteTree(TDeleteCallback * CallBack, uint32_t Param);

	TNodeRef getRoot();
	void setRoot(TNodeRef NewRoot);

	TOnRootChanged & sigRootChanged() {return m_sigRootChanged;};


protected:
	static const uint16_t cIsLeafMask = 0x8000;
	static const uint16_t cKeyCountMask = 0x7FFF;
	static const uint16_t cFullNode = SizeParam * 2 - 1;
	static const uint16_t cEmptyNode = SizeParam - 1;

	typedef lockfree::hash_multimap<TNodeRef, iterator*> TManagedMap;

	TNodeRef m_Root;
	TOnRootChanged m_sigRootChanged;
	TManagedMap	m_ManagedIterators;

	bool m_DestroyTree;

	uint32_t m_AllocCount;
	uint32_t m_Count;
	uint32_t m_FreeIndex;
	TNode *  m_Alloc;

	virtual void PrepareInsertOperation();
	virtual TNode * CreateNewNode(TNodeRef & NodeRef);
	virtual void DeleteNode(TNodeRef Node);
	virtual TNode * Read(TNodeRef Node);
	virtual void Write(TNodeRef Node);

	void DestroyTree();


private:
	friend class iterator;

	bool InNodeFind(const TNode * Node, const TKey & Key, uint16_t & GreaterEqual);
	void SplitNode(TNodeRef Node, TNode * NodeData, TNodeRef & Left, TNodeRef & Right, TKey & UpKey, TNodeRef ParentNode, uint16_t ParentIndex);
	TNodeRef MergeNodes(TNodeRef Left, TNode * LeftData, TNodeRef Right, TNode * RightData, const TKey & DownKey, TNodeRef ParentNode, uint16_t ParentIndex);
	void KeyInsert(TNodeRef Node, TNode * NodeData, uint16_t Where);
	void KeyDelete(TNodeRef Node, TNode * NodeData, uint16_t Where);
	void KeyMove(TNodeRef Source, uint16_t SourceIndex, const TNode * SourceData, TNodeRef Dest, uint16_t DestIndex, TNode * DestData);
};






template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::CBTree(TNodeRef RootNode = NULL)
: m_sigRootChanged(),
	m_ManagedIterators()
{
	m_Root = RootNode;
	m_DestroyTree = true;

	m_AllocCount = 0;
	m_Count = 0;
	m_FreeIndex = 0;
	m_Alloc = NULL;
}

template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::~CBTree()
{
	typename TManagedMap::iterator i = m_ManagedIterators.begin();
	while (i != m_ManagedIterators.end())
	{
		i->second->m_Tree = NULL;
		++i;
	}

	if (m_DestroyTree)
		DestroyTree();
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::InNodeFind(const TNode * Node, const TKey & Key, uint16_t & GreaterEqual)
{
	uint16_t l = 0;
	uint16_t r = (Node->Info & cKeyCountMask);
	bool res = false;
	GreaterEqual = 0;
	while ((l < r) && !res)
	{
		GreaterEqual = (l + r) >> 1;
		if (Node->Key[GreaterEqual] < Key)
		{
			GreaterEqual++;
			l = GreaterEqual;
		} else if (Node->Key[GreaterEqual] == Key)
		{
			//r = -1;
			res = true;
		} else {
			r = GreaterEqual;
		}
	}

	return res;
}


template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::SplitNode(TNodeRef Node, TNode * NodeData, TNodeRef & Left, TNodeRef & Right, TKey & UpKey, TNodeRef ParentNode, uint16_t ParentIndex)
{
	const uint16_t upindex = SizeParam - 1;
	TNode *ldata, *rdata;
	Left = Node;
	ldata = NodeData;
	rdata = CreateNewNode(Right);
	
	typename TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index == upindex)
		{
			it->second->m_Index = ParentIndex;
			it->second->m_Node = ParentNode;
			m_ManagedIterators.insert(std::make_pair(ParentNode, it->second));
			it = m_ManagedIterators.erase(it);
		} else if (it->second->m_Index > upindex)
		{
			it->second->m_Index = it->second->m_Index - upindex - 1;
			it->second->m_Node = Right;
			m_ManagedIterators.insert(std::make_pair(Right, it->second));
			it = m_ManagedIterators.erase(it);
		} else {
			++it;
		}
	}

	UpKey = NodeData->Key[upindex];

	memcpy(&(rdata->Key[0]), &(NodeData->Key[upindex+1]), sizeof(TKey) * (cFullNode - upindex));
	if ((NodeData->Info & cIsLeafMask) == 0)
	{
		memcpy(&(rdata->Child[0]), &(NodeData->Child[upindex+1]), sizeof(TNodeRef) * (cFullNode - upindex + 1));

		for (int i = 0; i <= upindex; i++)
		{
			TNode * tmp = Read(rdata->Child[i]);
			tmp->Parent = Right;
			Write(rdata->Child[i]);
		}
	}

	rdata->Info = (NodeData->Info & cIsLeafMask) | upindex;
	NodeData->Info = rdata->Info;
	rdata->Parent = NodeData->Parent;

	Write(Left);
	Write(Right);
}

template <typename TKey, uint16_t SizeParam>
inline typename CBTree<TKey, SizeParam>::TNodeRef CBTree<TKey, SizeParam>::MergeNodes(TNodeRef Left, TNode * LeftData, TNodeRef Right, TNode * RightData, const TKey & DownKey, TNodeRef ParentNode, uint16_t ParentIndex)
{
	uint16_t downindex = LeftData->Info & cKeyCountMask;
	LeftData->Key[downindex] = DownKey;

	typename TManagedMap::iterator it = m_ManagedIterators.find(Right);
	while ((it != m_ManagedIterators.end()) && (it->first == Right))
	{
		it->second->m_Index = it->second->m_Index + downindex + 1;
		it->second->m_Node = Left;
		m_ManagedIterators.insert(std::make_pair(Left, it->second));
		it = m_ManagedIterators.erase(it);
	}

	it = m_ManagedIterators.find(ParentNode);
	while ((it != m_ManagedIterators.end()) && (it->first == ParentNode))
	{
		if (it->second->m_Index == ParentIndex)
		{
			it->second->m_Index = downindex;
			it->second->m_Node = Left;
			m_ManagedIterators.insert(std::make_pair(Left, it->second));
			it = m_ManagedIterators.erase(it);
		} else {
			++it;
		}
	}

	memcpy(&(LeftData->Key[downindex+1]), &(RightData->Key[0]), sizeof(TKey) * (RightData->Info & cKeyCountMask));
	if ((LeftData->Info & cIsLeafMask) == 0)
	{
		memcpy(&(LeftData->Child[downindex+1]), &(RightData->Child[0]), sizeof(TNodeRef) * ((RightData->Info & cKeyCountMask) + 1));

		for (int i = 0; i <= (RightData->Info & cKeyCountMask); i++)
		{
			TNode * tmp = Read(RightData->Child[i]);
			tmp->Parent = Left;
			Write(RightData->Child[i]);
		}
	}

	LeftData->Info = ((LeftData->Info & cIsLeafMask) | (downindex + 1 + (RightData->Info & cKeyCountMask)));

	Write(Left);
	DeleteNode(Right);

	return Left;
}


template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::KeyInsert(TNodeRef Node, TNode * NodeData, uint16_t Where)
{
	memcpy(&(NodeData->Key[Where+1]), &(NodeData->Key[Where]), sizeof(TKey) * ((NodeData->Info & cKeyCountMask) - Where));

	if ((NodeData->Info & cIsLeafMask) == 0)
		memcpy(&(NodeData->Child[Where+1]), &(NodeData->Child[Where]), sizeof(TNodeRef) * ((NodeData->Info & cKeyCountMask) - Where + 1));

	NodeData->Info++;

	typename TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index >= Where)
			it->second->m_Index++;

		++it;
	}
}

template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::KeyDelete(TNodeRef Node, TNode * NodeData, uint16_t Where)
{
	NodeData->Info--;

	typename TManagedMap::iterator it = m_ManagedIterators.find(Node);
	while ((it != m_ManagedIterators.end()) && (it->first == Node))
	{
		if (it->second->m_Index == Where)
		{
			it->second->Backup();
		} else if (it->second->m_Index > Where)
		{
			it->second->m_Index--;
		}

		++it;
	}

	memcpy(&(NodeData->Key[Where]), &(NodeData->Key[Where+1]), sizeof(TKey) * ((NodeData->Info & cKeyCountMask) - Where));

	if ((NodeData->Info & cIsLeafMask) == 0)
		memcpy(&(NodeData->Child[Where]), &(NodeData->Child[Where+1]), sizeof(TNodeRef) * ((NodeData->Info & cKeyCountMask) - Where + 1));
}


template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::KeyMove(TNodeRef Source, uint16_t SourceIndex, const TNode * SourceData, TNodeRef Dest, uint16_t DestIndex, TNode * DestData)
{
	DestData->Key[DestIndex] = SourceData->Key[SourceIndex];

	typename TManagedMap::iterator it = m_ManagedIterators.find(Source);
	while ((it != m_ManagedIterators.end()) && (it->first == Source))
	{
		if (it->second->m_Index == SourceIndex)
		{
			it->second->m_Index = DestIndex;
			it->second->m_Node = Dest;
			m_ManagedIterators.insert(std::make_pair(Dest, it->second));
			it = m_ManagedIterators.erase(it);
		} else {
			++it;
		}
	}
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::Insert(const TKey & Key)
{
	TNode *node, *node2;
	TNodeRef actnode;
	TNodeRef nextnode;
	bool exists;
	uint16_t ge;

	PrepareInsertOperation();

	if (!m_Root)
	{
		node = CreateNewNode(m_Root);
		node->Info = cIsLeafMask;
		Write(m_Root);
		m_sigRootChanged.emit(this, m_Root);
	}

	actnode = m_Root;
	node = Read(actnode);
	if ((node->Info & cKeyCountMask) == cFullNode)  // root split
	{
		// be a little tricky and let the main code handle the actual splitting.
		// just assign a new root with keycount to zero and one child = old root
		// the InNode test will fail with GreaterEqual = 0
		node2 = CreateNewNode(nextnode);
		node2->Info = 0;
		node2->Child[0] = actnode;
		Write(nextnode);

		node->Parent = nextnode;
		Write(actnode);

		node = node2;
		actnode = nextnode;

		m_Root = nextnode;
		m_sigRootChanged.emit(this, m_Root);
	}

	while (actnode)
	{
		exists = InNodeFind(node, Key, ge);
		if (exists)  // already exists
		{
			return iterator(this, actnode, ge);
		} else {
			if (node->Info & cIsLeafMask) // direct insert to leaf node
			{
				KeyInsert(actnode, node, ge);

				node->Key[ge] = Key;

				Write(actnode);

				return iterator(this, actnode, ge);

			} else {  // middle node
				nextnode = node->Child[ge];
				node2 = Read(nextnode);

				if ((node2->Info & cKeyCountMask) == cFullNode) // split the childnode
				{
					KeyInsert(actnode, node, ge);
					SplitNode(nextnode, node2, node->Child[ge], node->Child[ge+1], node->Key[ge], actnode, ge);

					Write(actnode);
					if (node->Key[ge] == Key)
					{
						return iterator(this, actnode, ge);
					} else {
						if (node->Key[ge] < Key)
						{
							nextnode = node->Child[ge+1];
						} else {
							nextnode = node->Child[ge];
						}
					}

				}
				actnode = nextnode;
				node = Read(actnode);
				
			} // if (node.Info & cIsLeafMask)
		} // if (exists)
	}	// while (actnode)

	// something went wrong
	return iterator(this, 0, 0xFFFF);
}


template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::Find(const TKey & Key)
{
	TNode * node;
	TNodeRef actnode = m_Root;
	uint16_t ge;

	if (!m_Root) return iterator(this, 0, 0xFFFF);

	node = Read(actnode);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			return iterator(this, actnode, ge);
		}

		if (!(node->Info & cIsLeafMask))
		{
			actnode = node->Child[ge];
			node = Read(actnode);
		} else {
			actnode = 0;
		}
	}

	return iterator(this, 0, 0xFFFF);
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::LowerBound(const TKey & Key)
{
	TNode * node;
	TNodeRef actnode = m_Root;
	uint16_t ge;

	if (!m_Root) return iterator(this, 0, 0xFFFF);

	node = Read(actnode);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			return iterator(this, actnode, ge);
		}

		if (node->Info & cIsLeafMask)
		{
			if (ge >= (node->Info & cKeyCountMask))
			{
				iterator i(this, actnode, ge - 1);
				++i;
				return i;
			} else {
				return iterator(this, actnode, ge);
			}
		} else {
			actnode = node->Child[ge];
			node = Read(actnode);
		}
	}

	return iterator(this, 0, 0xFFFF);
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::UpperBound(const TKey & Key)
{
	TNode * node;
	TNodeRef actnode = m_Root;
	uint16_t ge;
	if (!m_Root) return iterator(this, 0, 0xFFFF);

	node = Read(actnode);

	while (actnode)
	{
		if (InNodeFind(node, Key, ge))
		{
			return iterator(this, actnode, ge);
		}

		if (node->Info & cIsLeafMask)
		{
			if (ge == 0)
			{
				iterator i(this, actnode, 0);
				--i;
				return i;
			} else {
				return iterator(this, actnode, ge - 1);
			}
		} else {
			actnode = node->Child[ge];
			node = Read(actnode);
		}
	}

	return iterator(this, 0, 0xFFFF);
}

template <typename TKey, uint16_t SizeParam>
bool CBTree<TKey, SizeParam>::Delete(const TKey& Key)
{
	if (!m_Root) return false;

	TNode *node, *node2, *lnode, *rnode;
	
	TNodeRef actnode = m_Root;
	TNodeRef nextnode, l, r;
	bool exists, skipread;
	uint16_t ge;

	bool foundininnernode = false;
	bool wantleftmost = false;
	TNodeRef innernode = 0;
	TNode * innernodedata = NULL;
	uint16_t innerindex = 0xFFFF;

	node = Read(actnode);

	while (actnode)
	{
		skipread = false;

		if (foundininnernode)
		{
			exists = false;
			if (wantleftmost)
				ge = 0;
			else
				ge = node->Info & cKeyCountMask;

		} else {
			exists = InNodeFind(node, Key, ge);
		}

		if (exists)
		{
			if (node->Info & cIsLeafMask)  // delete in leaf
			{
				KeyDelete(actnode, node, ge);
				Write(actnode);

				return true;

			} else { // delete in inner node
				l = node->Child[ge];
				r = node->Child[ge+1];
				lnode = Read(l);
				rnode = Read(r);


				if (((rnode->Info & cKeyCountMask) == cEmptyNode) && ((lnode->Info & cKeyCountMask) == cEmptyNode))
				{ // merge childnodes and keep going
					nextnode = MergeNodes(l, lnode, r, rnode, node->Key[ge], actnode, ge);

					KeyDelete(actnode, node, ge);
					node->Child[ge] = nextnode;

					if ((actnode == m_Root) && ((node->Info & cKeyCountMask) == 0))
					{ // root node is empty. delete it
						DeleteNode(actnode);
						m_Root = nextnode;
						m_sigRootChanged.emit(this, m_Root);
					} else {
						Write(actnode);
					}

				} else { // need a key-data-pair from a leaf to replace deleted pair -> save position
					foundininnernode = true;
					innernode = actnode;
					innerindex = ge;
					innernodedata = node;

					if ((lnode->Info & cKeyCountMask) == cEmptyNode)
					{
						wantleftmost = true;
						nextnode = r;
					} else {
						wantleftmost = false;
						nextnode = l;
					}
				}
			}

		} else if (node->Info & cIsLeafMask) { // we are at the bottom. finish it
			if (foundininnernode)
			{
				if (wantleftmost)
				{
					KeyMove(actnode, 0, node, innernode, innerindex, innernodedata);
					Write(innernode);

					KeyDelete(actnode, node, 0);
					Write(actnode);

				} else {
					KeyMove(actnode, (node->Info & cKeyCountMask) - 1, node, innernode, innerindex, innernodedata);
					Write(innernode);

					//KeyDelete(actnode, node, node.Info & cKeyCountMask);
					node->Info--;
					Write(actnode);
				}
			}
			return foundininnernode;

		} else { // inner node. go on and check if moving or merging is neccessary
			nextnode = node->Child[ge];
			node2 = Read(nextnode);

			if ((node2->Info & cKeyCountMask) == cEmptyNode) // move or merge
			{
				// set l and r for easier access
				if (ge > 0)
				{
					l = node->Child[ge - 1];
					lnode = Read(l);
				} else
					l = 0;

				if (ge < (node->Info & cKeyCountMask))
				{
					r = node->Child[ge + 1];
					rnode = Read(r);
				} else
					r = 0;

				if ((r != 0) && ((rnode->Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the right
				{
					// move key-data-pair down from current to the next node
					KeyMove(actnode, ge, node, nextnode, node2->Info & cKeyCountMask, node2);

					// move the child from right to next node
					node2->Child[(node2->Info & cKeyCountMask) + 1] = rnode->Child[0];

					// move key-data-pair up from right to current node
					KeyMove(r, 0, rnode, actnode, ge, node);
					Write(actnode);

					// decrement right node key count and remove the first key-data-pair
					KeyDelete(r, rnode, 0);

					// increment KeyCount of the next node
					node2->Info++;

					if ((node2->Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						TNode * tmp = Read(node2->Child[node2->Info & cKeyCountMask]);
						tmp->Parent = nextnode;
						Write(node2->Child[node2->Info & cKeyCountMask]);
					}


					Write(r);
					Write(nextnode);
					node = node2;
					skipread = true;

				} else if ((l != 0) && ((lnode->Info & cKeyCountMask) > cEmptyNode)) // move a Key-Data-pair from the left
				{					
					// increment next node key count and make new first key-data-pair
					KeyInsert(nextnode, node2, 0);

					// move key-data-pair down from current to the next node
					KeyMove(actnode, ge - 1, node, nextnode, 0, node2);

					// move the child from left to next node
					node2->Child[0] = lnode->Child[lnode->Info & cKeyCountMask];

					// move key-data-pair up from left to current node
					KeyMove(l, (lnode->Info & cKeyCountMask) - 1, lnode, actnode, ge - 1, node);
					Write(actnode);

					// decrement left node key count
					lnode->Info--;
					Write(l);

					if ((node2->Info & cIsLeafMask) == 0) // update the parent property of moved child
					{
						TNode * tmp = Read(node2->Child[0]);
						tmp->Parent = nextnode;
						Write(node2->Child[0]);
					}

					Write(nextnode);
					node = node2;
					skipread = true;

				} else {
					if (l != 0) // merge with the left node
					{
						nextnode = MergeNodes(l, lnode, nextnode, node2, node->Key[ge - 1], actnode, ge - 1);
						KeyDelete(actnode, node, ge - 1);
						node->Child[ge - 1] = nextnode;

					} else { // merge with the right node
						nextnode = MergeNodes(nextnode, node2, r, rnode, node->Key[ge], actnode, ge);
						KeyDelete(actnode, node, ge);
						node->Child[ge] = nextnode;
					}

					if ((actnode == m_Root) && ((node->Info & cKeyCountMask) == 0))
					{
						DeleteNode(actnode);
						m_Root = nextnode;
						m_sigRootChanged(this, nextnode);
					} else {
						Write(actnode);
					}
				}
			}
		} // if (exists) else if (node.Info & cIsLeafMask)

		actnode = nextnode;
		if (!skipread)
			node = Read(actnode);

	} // while(actnode)

	return false;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNodeRef CBTree<TKey, SizeParam>::getRoot()
{
	return m_Root;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::setRoot(TNodeRef NewRoot)
{
	m_Root = NewRoot;
	return;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::PrepareInsertOperation()
{
	if (m_Count + 64 > m_AllocCount)
	{
		m_AllocCount += 64;
		m_Alloc = (TNode *)realloc(m_Alloc, sizeof(TNode) * m_AllocCount);

		for (TNodeRef i = m_AllocCount - 64; i < m_AllocCount; ++i)
			m_Alloc[i].Parent = i + 1;
		
		m_Alloc[m_AllocCount - 1].Parent = 0;
		
		if (m_FreeIndex)
		{
			TNodeRef i = m_FreeIndex; 
			while (m_Alloc[i].Parent)
				i = m_Alloc[i].Parent;
			
			m_Alloc[i].Parent = m_AllocCount - 64;
			
		} else {
			m_FreeIndex = m_AllocCount - 63;
		}
		
	}
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNode * CBTree<TKey, SizeParam>::CreateNewNode(TNodeRef & NodeRef)
{
	NodeRef = m_FreeIndex;
	m_FreeIndex = m_Alloc[m_FreeIndex].Parent;
	m_Count++;
	memset(m_Alloc + NodeRef, 0, sizeof(TNode));
	return m_Alloc + NodeRef;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::DeleteNode(TNodeRef Node)
{
	CHECK((Node > 0) && (Node < m_AllocCount), logERROR, _T("Invalid Node"));
	m_Alloc[Node].Parent = m_FreeIndex;
	m_FreeIndex = Node;
	m_Count--;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNode * CBTree<TKey, SizeParam>::Read(TNodeRef Node)
{
	CHECK((Node > 0) && (Node < m_AllocCount), logERROR, _T("Invalid Node"));
	return m_Alloc + Node;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::Write(TNodeRef Node)
{
	return;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::DestroyTree()
{
	std::stack<TNodeRef> s;
	TNodeRef node;
	TNode* nodedata;
	uint16_t i;

	if (m_Root)
		s.push(m_Root);
	while (!s.empty())
	{
		node = s.top();
		nodedata = Read(node);
		s.pop();

		if ((nodedata->Info & cIsLeafMask) == 0)
		{
			for (i = 0; i <= (nodedata->Info & cKeyCountMask); i++)
				s.push(nodedata->Child[i]);
		}

		DeleteNode(node);
	}

	free(m_Alloc);
	m_Alloc = NULL;
	m_AllocCount = 0;
	m_Count = 0;
	m_FreeIndex = 0;
}


template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::DeleteTree(TDeleteCallback * CallBack, uint32_t Param)
{
	std::stack<TNodeRef> s;
	TNodeRef actnode;
	TNode * node;
	uint16_t i;

	typename TManagedMap::iterator it = m_ManagedIterators.begin();
	while (it != m_ManagedIterators.end())
	{
		it->second->m_Node = 0;
		it->second->m_Index = 0xffff;
		++it;
	}

	if (m_Root)
		s.push(m_Root);

	m_Root = 0;
	m_sigRootChanged.emit(this, m_Root);

	while (!s.empty())
	{
		actnode = s.top();
		s.pop();

		node = Read(actnode);

		if ((node->Info & cIsLeafMask) == 0)
		{
			for (i = 0; i <= (node->Info & cKeyCountMask); i++)
				s.push(node->Child[i]);
		
		}
		if (CallBack)
		{
			for (i = 0; i < (node->Info & cKeyCountMask); i++)
				CallBack->emit(this, node->Key[i], Param);
		}

		DeleteNode(actnode);
	}
}







template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::iterator()
{
	m_Tree = NULL;
	m_Node = 0;
	m_Index = 0xFFFF;
	m_Managed = false;
	m_ManagedDeleted = false;
	m_LoadedKey = false;
}
template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::iterator(CBTree* Tree, TNodeRef Node, uint16_t Index)
{
	m_Tree = Tree;
	m_Node = Node;
	m_Index = Index;
	m_Managed = false;
	m_ManagedDeleted = false;
	m_LoadedKey = false;
}
template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::iterator(const iterator& Other)
{
	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	m_ManagedDeleted = Other.m_ManagedDeleted;
	m_Managed = Other.m_Managed;
	m_LoadedKey = Other.m_LoadedKey;
	m_ManagedKey = Other.m_ManagedKey;

	if (m_Managed)
		InsertManaged();
}

template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam>::iterator::~iterator()
{
	RemoveManaged(m_Node);
}


template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::setManaged()
{
	if (!m_Managed)
		InsertManaged();

	m_Managed = true;
}

template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::iterator::RemoveManaged(TNodeRef FromNode)
{
	if (m_Managed && m_Tree)
	{
		typename TManagedMap::iterator i = m_Tree->m_ManagedIterators.find(FromNode);

		while ((i != m_Tree->m_ManagedIterators.end()) && (i->second != this) && (i->first == FromNode))
			++i;

		if ((i != m_Tree->m_ManagedIterators.end()) && (i->second == this))
			m_Tree->m_ManagedIterators.erase(i);
	}
}
template <typename TKey, uint16_t SizeParam>
inline void CBTree<TKey, SizeParam>::iterator::InsertManaged()
{
	if (m_Tree)
		m_Tree->m_ManagedIterators.insert(std::make_pair(m_Node, this));
}

template <typename TKey, uint16_t SizeParam>
bool CBTree<TKey, SizeParam>::iterator::wasDeleted()
{
	return m_ManagedDeleted;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::Backup()
{
	if ((!m_ManagedDeleted) && (*this))
	{
		TNode * tmp;
		if (!m_LoadedKey)
		{
			tmp = m_Tree->Read(m_Node);
			m_ManagedKey = tmp->Key[m_Index];
		}
		m_LoadedKey = true;
	}

	m_ManagedDeleted = true;
}

template <typename TKey, uint16_t SizeParam>
CBTree<TKey, SizeParam> * CBTree<TKey, SizeParam>::iterator::Tree()
{
	return m_Tree;
}

template <typename TKey, uint16_t SizeParam>
const TKey& CBTree<TKey, SizeParam>::iterator::operator *()
{
	if (!m_LoadedKey)
	{
		TNode * node;
		node = m_Tree->Read(m_Node);
		m_ManagedKey = node->Key[m_Index];
		m_LoadedKey = true;
	}
	return m_ManagedKey;
}

template <typename TKey, uint16_t SizeParam>
const TKey* CBTree<TKey, SizeParam>::iterator::operator ->()
{
	if (!m_LoadedKey)
	{
		TNode * node;
		node = m_Tree->Read(m_Node);
		m_ManagedKey = node->Key[m_Index];
		m_LoadedKey = true;
	}
	return &m_ManagedKey;
}

template <typename TKey, uint16_t SizeParam>
inline CBTree<TKey, SizeParam>::iterator::operator bool() const
{
	if (m_Tree && m_Node)
	{
		TNode * node;
		node = m_Tree->Read(m_Node);
		return (m_Index < (node->Info & cKeyCountMask));
	} else
		return false;
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator !() const
{
	if (m_Tree && m_Node)
	{
		TNode * node;
		node = m_Tree->Read(m_Node);
		return (m_Index > (node->Info & cKeyCountMask));
	} else
		return true;
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator ==(iterator & Other)
{
	//return (m_Tree == Other.m_Tree) && (m_Node == Other.m_Node) && (m_Index == Other.m_Index) && (!m_ManagedDeleted) && (!Other.m_ManagedDeleted);
	return Key() == Other.Key();
}

template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator <  (iterator & Other)
{
	return Key() < Other.Key();
}
template <typename TKey, uint16_t SizeParam>
inline bool CBTree<TKey, SizeParam>::iterator::operator >  (iterator & Other)
{
	return Key() > Other.Key();
}


template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator&
CBTree<TKey, SizeParam>::iterator::operator =(const iterator& Other)
{
	RemoveManaged(m_Node);

	m_Tree = Other.m_Tree;
	m_Node = Other.m_Node;
	m_Index = Other.m_Index;
	m_ManagedDeleted = Other.m_ManagedDeleted;
	m_Managed = Other.m_Managed;
	m_LoadedKey = Other.m_LoadedKey;
	m_ManagedKey = Other.m_ManagedKey;

	if (m_Managed)
		InsertManaged();

	return *this;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator&
CBTree<TKey, SizeParam>::iterator::operator ++() //pre  ++i
{
	TNodeRef oldnode = m_Node;
	if (m_Managed && m_ManagedDeleted)
	{
		TKey oldkey = m_ManagedKey;
		m_LoadedKey = false;
		m_ManagedDeleted = false;
		iterator other = m_Tree->LowerBound(m_ManagedKey);
		m_Node = other.m_Node;
		m_Index = other.m_Index;
		while (((**this) == oldkey) && (*this))
			Inc();

	} else
		Inc();

	if (m_Managed && (oldnode != m_Node))
	{
		RemoveManaged(oldnode);
		InsertManaged();
	}
	return *this;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator&
CBTree<TKey, SizeParam>::iterator::operator --() //pre  --i
{
	TNodeRef oldnode = m_Node;
	if (m_Managed && m_ManagedDeleted)
	{
		TKey oldkey = m_ManagedKey;
		m_LoadedKey = false;

		m_ManagedDeleted = false;
		iterator other = m_Tree->UpperBound(m_ManagedKey);
		m_Node = other.m_Node;
		m_Index = other.m_Index;
		while (((**this) == oldkey) && (*this))
			Dec();
	} else
		Dec();

	if (m_Managed && (oldnode != m_Node))
	{
		RemoveManaged(oldnode);
		InsertManaged();
	}
	return *this;
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::iterator::operator ++(int) //post i++
{
	iterator tmp(*this);
	++(*this);
	return tmp;
}
template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::iterator
CBTree<TKey, SizeParam>::iterator::operator --(int) //post i--
{
	iterator tmp(*this);
	--(*this);
	return tmp;
}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::Inc()
{
	TNode * node;
	TNodeRef nextnode;
	node = m_Tree->Read(m_Node);

	m_LoadedKey = false;

	if ((node->Info & cIsLeafMask) && ((node->Info & cKeyCountMask) > m_Index + 1)) // leaf
	{
		m_Index++;
		return;
	}

	if ((node->Info & cIsLeafMask) == 0) // inner node. go down
	{
		m_Node = node->Child[m_Index + 1];
		node = m_Tree->Read(m_Node);

		m_Index = 0;

		while ((node->Info & cIsLeafMask) == 0)  // go down to a leaf
		{
			m_Node = node->Child[0];
			node = m_Tree->Read(m_Node);
		}

		return;
	}

	while (m_Index >= (node->Info & cKeyCountMask) - 1) // go up
	{
		if (m_Node == m_Tree->m_Root) // the root is the top, we cannot go further
		{
			m_Index = 0xFFFF;
			m_Node = 0;
			return;
		}

		nextnode = node->Parent;
		node = m_Tree->Read(nextnode);
		m_Index = 0;

		while ((m_Index <= (node->Info & cKeyCountMask)) && (node->Child[m_Index] != m_Node))
			m_Index++;

		m_Node = nextnode;

		if (m_Index < (node->Info & cKeyCountMask))
			return;
	}

}

template <typename TKey, uint16_t SizeParam>
void CBTree<TKey, SizeParam>::iterator::Dec()
{
	TNode * node;
	TNodeRef nextnode;
	node = m_Tree->Read(m_Node);

	m_LoadedKey = false;

	if ((node->Info & cIsLeafMask) && (m_Index > 0)) // leaf
	{
		m_Index--;
		return;
	}

	if ((node->Info & cIsLeafMask) == 0) // inner node. go down
	{
		m_Node = node->Child[m_Index];
		node = m_Tree->Read(m_Node);
		m_Index = (node->Info & cKeyCountMask) - 1;

		while ((node->Info & cIsLeafMask) == 0)  // go down to a leaf
		{
			m_Node = node->Child[node->Info & cKeyCountMask];
			node = m_Tree->Read(m_Node);
			m_Index = (node->Info & cKeyCountMask) - 1;
		}

		return;
	}

	while (m_Index == 0) // go up
	{
		if (m_Node == m_Tree->m_Root) // the root is the top, we cannot go further
		{
			m_Index = 0xFFFF;
			m_Node = 0;
			return;
		}

		nextnode = node->Parent;
		node = m_Tree->Read(nextnode);
		m_Index = 0;

		while ((m_Index <= (node->Info & cKeyCountMask)) && (node->Child[m_Index] != m_Node))
			m_Index++;

		m_Node = nextnode;

		if (m_Index > 0)
		{
			m_Index--;
			return;
		}
	}
}
