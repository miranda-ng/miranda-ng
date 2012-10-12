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
#include "BTree.h"
#include "BlockManager.h"
#include "Logger.h"

template <typename TKey, uint16_t SizeParam = 4>
class CFileBTree :	public CBTree<TKey, SizeParam>
{
private:

protected:
	CBlockManager & m_BlockManager;
	uint16_t cSignature;

	virtual void PrepareInsertOperation();
	virtual TNode * CreateNewNode(TNodeRef & NodeRef);
	virtual void DeleteNode(TNodeRef Node);
	virtual TNode * Read(TNodeRef Node);
	virtual void Write(TNodeRef Node);
public:
	CFileBTree(CBlockManager & BlockManager, typename CBTree<TKey, SizeParam>::TNodeRef RootNode, uint16_t Signature);
	virtual ~CFileBTree();
};




template <typename TKey, uint16_t SizeParam>
CFileBTree<TKey, SizeParam>::CFileBTree(CBlockManager & BlockManager, typename CBTree<TKey, SizeParam>::TNodeRef RootNode, uint16_t Signature)
:	CBTree<TKey, SizeParam>::CBTree(RootNode),
	m_BlockManager(BlockManager)
{
	cSignature = Signature;
	CBTree<TKey, SizeParam>::m_DestroyTree = false;
}

template <typename TKey, uint16_t SizeParam>
CFileBTree<TKey, SizeParam>::~CFileBTree()
{
	CBTree<TKey, SizeParam>::m_DestroyTree = false;
}


template <typename TKey, uint16_t SizeParam>
void CFileBTree<TKey, SizeParam>::PrepareInsertOperation()
{

}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNode * CFileBTree<TKey, SizeParam>::CreateNewNode(typename CBTree<TKey, SizeParam>::TNodeRef & NodeRef)
{
	return reinterpret_cast<TNode*>( m_BlockManager.CreateBlock<uint32_t>(NodeRef, cSignature << 16, sizeof(typename CBTree<TKey, SizeParam>::TNode) - 4) - 1);
}

template <typename TKey, uint16_t SizeParam>
void CFileBTree<TKey, SizeParam>::DeleteNode(typename CBTree<TKey, SizeParam>::TNodeRef Node)
{
	m_BlockManager.DeleteBlock(Node);
}

template <typename TKey, uint16_t SizeParam>
typename CBTree<TKey, SizeParam>::TNode * CFileBTree<TKey, SizeParam>::Read(typename CBTree<TKey, SizeParam>::TNodeRef Node)
{
	uint32_t sig = 0;
	uint32_t size = 0;
	TNode * res = reinterpret_cast<TNode*>( m_BlockManager.ReadBlock<uint32_t>(Node, size, sig) - 1); /// HACK using knowledge about the blockmanager here
	
	CHECK(res->Signature == cSignature,
		logCRITICAL, _T("Signature check failed"));

	return res;
}

template <typename TKey, uint16_t SizeParam>
void CFileBTree<TKey, SizeParam>::Write(typename CBTree<TKey, SizeParam>::TNodeRef Node)
{
	m_BlockManager.UpdateBlock(Node, 0);
}

