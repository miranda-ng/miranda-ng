#pragma once

/*

lockfree hash-multi_map based on Ori Shalev and Nir Shavit

implementation
Copyright 2009-2010 Michael "Protogenes" Kunz

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

#include <utility>
#include "Hash.h"
#include "intrinsics.h"

#define NodePointer(listitem) ((PListItem)(((uintptr_t)(listitem)) & ~1))
#define NodeMark(listitem)    ((bool)     (((uintptr_t)(listitem)) & 1))

#define HashTablePtr(hashtable)        ((PHashTable)(((uintptr_t)(hashtable)) & ~31))
#define HashTableSize(hashtable)       ((uint32_t)(((uintptr_t)(hashtable)) & 31))
#define HashTable(tableptr, tablesize) ((void*)(((uintptr_t)(tableptr)) | ((tablesize) & 31)))

#define GCSelection(Sentinel) ((Sentinel) >> 63)
#define GCRefCount0(Sentinel) ((Sentinel) & 0x7fffffff)
#define GCRefCount1(Sentinel) (((Sentinel) >> 32) & 0x7fffffff)
#define GCMakeSentinel(Sel, RefCount0, RefCount1) ((((uint64_t)(Sel) & 1) << 63) | (((uint64_t)(RefCount1)) << 32) | ((uint64_t)(RefCount0)))

#define GCRef0 (1)
#define GCRef1 (0x0000000100000000)

namespace lockfree
{
	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t) = Hash>
	class hash_map
	{
	public:
		typedef std::pair<TKey, TData> value_type;

	private:
		typedef struct TListItem
		{
			TListItem * volatile Next;
			TListItem * volatile NextPurge;
			volatile uint32_t    Hash;
			value_type  Value;
		} TListItem, *PListItem;

		typedef struct THashTable 
		{
			volatile PListItem Table[256];
		} THashTable, *PHashTable;

		typedef struct {
			volatile uint64_t Sentinel;
			volatile PListItem Purge0;
			volatile PListItem Purge1;
		} THashTableReferences;

		THashTableReferences m_GarbageCollector;

		volatile uint32_t m_Count;
		void * volatile m_HashTableData;

		PListItem listInsert(PListItem BucketNode, PListItem Node);
		PListItem listDelete(PListItem BucketNode, uint32_t Hash, const TKey & Key);

		PListItem listDelete(PListItem BucketNode, PListItem Node);

		bool listFind(const PListItem BucketNode, const uint32_t Hash, const TKey & Key, const PListItem Node, volatile PListItem * & Prev, PListItem & Curr, PListItem & Next);

		uint32_t getMask(uint32_t Size)
		{
			const uint32_t mask[32] = {
				0x80000000, 0xc0000000, 0xe0000000, 0xf0000000, 
				0xf8000000, 0xfc000000, 0xfe000000, 0xff000000, 
				0xff800000, 0xffc00000, 0xffe00000, 0xfff00000, 
				0xfff80000, 0xfffc0000, 0xfffe0000, 0xffff0000, 
				0xffff8000, 0xffffc000, 0xffffe000, 0xfffff000, 
				0xfffff800, 0xfffffc00, 0xfffffe00, 0xffffff00, 
				0xffffff80, 0xffffffc0, 0xffffffe0, 0xfffffff0, 
				0xfffffff8, 0xfffffffc, 0xfffffffe, 0xffffffff
			};
			return mask[Size - 1];
		};

		PHashTable makeNewTable()
		{
			void * block = malloc(sizeof(THashTable) + 32 + sizeof(void*));
			PHashTable result = reinterpret_cast<PHashTable>((reinterpret_cast<uintptr_t>(block) + 31 + sizeof(void*)) & (~(uintptr_t)31));
			*(reinterpret_cast<void**>(result)-1) = block;
			memset(reinterpret_cast<void*>(result), 0, sizeof(THashTable));
			return reinterpret_cast<PHashTable>(result);
		};

		void destroyTable(PHashTable Table)
		{
			free(*(reinterpret_cast<void**>(Table) - 1));
		};

		PListItem makeDummyNode(uint32_t Hash)
		{
			PListItem result = new TListItem;
			result->Hash = Hash;
			result->Next = NULL;
			result->NextPurge = NULL;
			return result;
		};

		bool DisposeNode(volatile PListItem * Prev, PListItem Curr, PListItem Next);

		PListItem initializeBucket(uint32_t Bucket, uint32_t Mask);

		PListItem getBucket(uint32_t Bucket);

		void setBucket(uint32_t Bucket, PListItem Dummy);

		void DeleteTable(void * Table, uint32_t Size)
		{
			if (Size > 8)
			{
				for (uint32_t i = 0; i < 256; ++i)
				{
					if (HashTablePtr(Table)->Table[i])
						DeleteTable(HashTablePtr(Table)->Table[i], Size - 8);
				}
			}

			destroyTable(HashTablePtr(Table));
		};


		int addRef(int GC = -1);
		void delRef(int GC);

	public:


		class iterator
		{
		protected:
			friend class hash_map<TKey, TData, FHash>;				
			PListItem m_Item;
			typename hash_map<TKey, TData, FHash> * m_Owner;
			int m_GC;

			iterator(hash_map<TKey, TData, FHash> * Owner, PListItem Item, int GC)
				: m_Owner(Owner)
			{
				m_GC = GC;
				m_Item = Item;

				while (m_Item && (!(m_Item->Hash & 1) || NodeMark(m_Item->Next)))
					m_Item = NodePointer(m_Item->Next);

				if (!m_Item && (m_GC != -1))
				{
					m_Owner->delRef(m_GC);
					m_GC = -1;
				}

			};
		public:
			iterator(const iterator & Other)
				: m_Owner(Other.m_Owner),
				m_Item(Other.m_Item)
			{
				m_GC = -1;
				if (Other.m_GC != -1)
					m_GC = m_Owner->addRef(Other.m_GC);
			};
			~iterator()
			{
				if (m_GC != -1)
					m_Owner->delRef(m_GC);
			};

			operator bool() const
			{
				return m_Item != NULL;
			};
			bool operator !() const
			{
				return m_Item == NULL;
			};

			value_type * operator ->()
			{
				return &m_Item->Value;
			};
			value_type & operator *()
			{
				return m_Item->Value;
			};

			bool operator ==(iterator& Other)
			{
				return m_Item->Value.first == Other.m_Item->Value.first;
			};
			bool operator <  (iterator & Other)
			{
				return m_Item->Value.first < Other.m_Item->Value.first;
			};
			bool operator >  (iterator & Other)
			{
				return m_Item->Value.first > Other.m_Item->Value.first;
			};

			iterator& operator =(const iterator& Other)
			{
				m_Owner = Other.m_Owner;
				m_Item = Other.m_Item;

				if (Other.m_GC != m_GC)
				{
					if (m_GC != -1)
						m_Owner->delRef(m_GC);

					m_GC = Other.m_GC;

					if (Other.m_GC != -1)
						m_GC = m_Owner->addRef(Other.m_GC);

				}
				
				return *this;
			};

			iterator& operator ++() //pre  ++i
			{
				if (!m_Item)
					return *this;

				int gc = m_GC;
				m_GC = m_Owner->addRef();
				do 
				{
					m_Item = NodePointer(m_Item->Next);
				} while (m_Item && (!(m_Item->Hash & 1) || NodeMark(m_Item->Next)));

				m_Owner->delRef(gc);
				if (!m_Item)
				{
					m_Owner->delRef(m_GC);
					m_GC = -1;
				}
				return *this;
			};
			iterator  operator ++(int) //post i++
			{
				iterator bak(*this);
				++(*this);
				return bak;
			};
		};

		iterator begin()
		{
			return iterator(this, getBucket(0), addRef());
		};

		iterator end()
		{
			return iterator(this, NULL, -1);
		};

		hash_map();
		~hash_map();

		std::pair<iterator, bool> insert(const value_type & Val);

		iterator find(const TKey & Key);

		iterator erase(const iterator & Where);

		size_t erase(const TKey & Key);

	};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	int hash_map<TKey, TData, FHash>::addRef(int GC = -1)
	{
		uint64_t old;
		uint64_t newvalue;
		int res;
		do {
			old = m_GarbageCollector.Sentinel;
			if (GC == 0) // this is safe because refcount will never fall to zero because of the original reference
			{
				newvalue = old + GCRef0;
				res = 0;
			} else if (GC == 1)
			{				
				newvalue = old + GCRef1;
				res = 1;
			} else {
				if (GCSelection(old))
				{
					newvalue = old + GCRef1;
					res = 1;
				} else {
					newvalue = old + GCRef0;
					res = 0;
				}
			}
		}
		while (CMPXCHG_64(m_GarbageCollector.Sentinel, newvalue, old) != old);

		return res;
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	void hash_map<TKey, TData, FHash>::delRef(int GC)
	{
		uint64_t old;
		uint64_t newvalue;
		PListItem purge = NULL;
		do {
			old = m_GarbageCollector.Sentinel;
			if (GC)
			{
				newvalue = old - GCRef1;

				if (!GCSelection(old) && (GCRefCount1(old) == 1)) // the other gc is activated and we are the last one
				{
					if (!purge) // check if we had to loop...
						purge = m_GarbageCollector.Purge1;

					m_GarbageCollector.Purge1 = NULL;
				}

			} else {
				newvalue = old - GCRef0;

				if (GCSelection(old) && (GCRefCount0(old) == 1)) // the other gc is activated and we are the last one
				{
					if (!purge) // check if we had to loop...
						purge = m_GarbageCollector.Purge0;

					m_GarbageCollector.Purge0 = NULL;
				}

			}
		} while (CMPXCHG_64(m_GarbageCollector.Sentinel, newvalue, old) != old);

		purge = NodePointer(purge);
		while (purge)
		{
			PListItem tmp = purge;
			purge = purge->NextPurge;
			delete tmp;
		};
	};



	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	bool hash_map<TKey, TData, FHash>::DisposeNode(volatile PListItem * Prev, PListItem Curr, PListItem Next)
	{
		if (NodePointer(Curr) == CMPXCHG_Ptr(*Prev, NodePointer(Next), NodePointer(Curr)))
		{
			uint64_t old = m_GarbageCollector.Sentinel;
			PListItem del = NodePointer(Curr);

			if (GCSelection(old))
			{
				del->NextPurge = (PListItem)XCHG_Ptr(m_GarbageCollector.Purge1, del);

				if (!GCRefCount0(old))
					BTR_64(m_GarbageCollector.Sentinel, 63); // switch

			} else {

				del->NextPurge = (PListItem)XCHG_Ptr(m_GarbageCollector.Purge0, del);

				if (!GCRefCount1(old))
					BTS_64(m_GarbageCollector.Sentinel, 63); // switch

			}
			return true;
		}

		return false;
	};


	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::listInsert(PListItem BucketNode, PListItem Node)
	{
		PListItem volatile * prev;
		PListItem curr, next;
		do
		{
			if (listFind(BucketNode, Node->Hash, Node->Value.first, NULL, prev, curr, next))
				return NodePointer(curr);

			Node->Next = NodePointer(curr);

		} while (NodePointer(curr) != CMPXCHG_Ptr(*prev, Node, NodePointer(curr)));
		return Node;
	};


	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::listDelete(PListItem BucketNode, uint32_t Hash, const TKey & Key)
	{
		PListItem volatile * prev;
		PListItem curr, next;

		if (!listFind(BucketNode, Hash, Key, NULL, prev, curr, next))
			return NodePointer(curr);

		do 
		{
			if (!listFind(BucketNode, Hash, Key, NULL, prev, curr, next))
				return NodePointer(curr);

		} while (NodePointer(next) != CMPXCHG_Ptr(curr->Next, (PListItem)((uintptr_t)next | 1), NodePointer(next)));

		if (!DisposeNode(prev, curr, next))
			listFind(BucketNode, Hash, Key, NULL, prev, curr, next); // cleanup

		return NodePointer(curr);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::listDelete(PListItem BucketNode, PListItem Node)
	{
		PListItem volatile * prev;
		PListItem curr, next;
		if (!listFind(BucketNode, Node->Hash, Node->Value.first, Node, prev, curr, next))
			return NodePointer(curr);

		do 
		{
			if (!listFind(BucketNode, Node->Hash, Node->Value.first, Node, prev, curr, next))
				return NodePointer(curr);

		} while (NodePointer(next) != CMPXCHG_Ptr(curr->Next, (PListItem)((uintptr_t)next | 1), NodePointer(next)));

		if (!DisposeNode(prev, curr, next))
			listFind(BucketNode, Node->Hash, Node->Value.first, Node, prev, curr, next); // cleanup

		return NodePointer(curr);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	bool hash_map<TKey, TData, FHash>::listFind(const PListItem BucketNode, const uint32_t Hash, const TKey & Key, const PListItem Node, volatile PListItem * & Prev, PListItem & Curr, PListItem & Next)
	{
tryagain:
		Prev = &(BucketNode->Next);
		Curr = *Prev;
		do
		{
			if (NodePointer(Curr) == NULL)
				return false;

			Next = NodePointer(Curr)->Next;
			uint32_t h = NodePointer(Curr)->Hash;

			if (*Prev != NodePointer(Curr))
				goto tryagain; // don't judge me for that
			//return listFind(BucketNode, Hash, Key, Node, Prev, Curr, Next); // it's the same but stack overflow can happen

			if (!NodeMark(Next))
			{
				if (Node)
				{
					if ((h > Hash) || (Node == NodePointer(Curr)))
						return NodePointer(Curr) == Node;
				}
				else if ((h > Hash) || ((h == Hash) && !(NodePointer(Curr)->Value.first < Key)))
				{
					return (h == Hash) && (NodePointer(Curr)->Value.first == Key);
				}

				Prev = &(NodePointer(Curr)->Next);
			} else {
				if (!DisposeNode(Prev, Curr, Next))
					goto tryagain; // don't judge me for that
				//return listFind(BucketNode, Hash, Key, Node, Prev, Curr, Next); // it's the same but stack overflow can happen
			}
			Curr = Next;
		} while (true);

	};


	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::initializeBucket(uint32_t Bucket, uint32_t Mask)
	{
		uint32_t parent = Bucket & (Mask << 1);
		PListItem parentnode = getBucket(parent);
		if (parentnode == NULL)
			parentnode = initializeBucket(parent, Mask << 1);

		PListItem dummy = makeDummyNode(Bucket);
		PListItem bucketnode = listInsert(parentnode, dummy);
		if (bucketnode != dummy)
		{
			delete dummy;
			dummy = bucketnode;
		}
		setBucket(Bucket, dummy);

		return dummy;
	}

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::PListItem hash_map<TKey, TData, FHash>::getBucket(uint32_t Bucket)
	{
		void * table;
		uint32_t mask;

		table = (void*)m_HashTableData;
		mask = getMask(HashTableSize(table));

		uint32_t levelshift = (32 - HashTableSize(table)) & ~7;

		while (levelshift < 24)
		{
			table = HashTablePtr(table)->Table[((Bucket & mask) >> levelshift) & 0xff];
			levelshift = levelshift + 8;
			if (!HashTablePtr(table))
				return NULL;
		}
		return HashTablePtr(table)->Table[(Bucket & mask) >> 24];
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	void hash_map<TKey, TData, FHash>::setBucket(uint32_t Bucket, PListItem Dummy)
	{
		void * table;
		void *volatile * last;
		uint32_t mask;

		table = m_HashTableData;
		mask = getMask(HashTableSize(table));

		uint32_t levelshift = (32 - HashTableSize(table)) & ~7;

		while (levelshift < 24)
		{
			last = (void*volatile*)&HashTablePtr(table)->Table[((Bucket & mask) >> levelshift) & 0xff];
			table = *last;
			levelshift = levelshift + 8;
			if (!table)
			{
				PHashTable newtable = makeNewTable();
				table = CMPXCHG_Ptr<void>(*last, newtable, NULL);
				if (table)
				{
					destroyTable(newtable);
				} else {
					table = newtable;
				}
			}
		}
		HashTablePtr(table)->Table[(Bucket & mask) >> 24] = Dummy;
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	hash_map<TKey, TData, FHash>::hash_map()
	{
		m_Count = 0;

		m_GarbageCollector.Sentinel = GCMakeSentinel(0,0,0);
		m_GarbageCollector.Purge0 = NULL;
		m_GarbageCollector.Purge1 = NULL;

		m_HashTableData = HashTable(makeNewTable(), 1);
		setBucket(0x00000000, makeDummyNode(0x00000000));
		setBucket(0x80000000, makeDummyNode(0x80000000));
		HashTablePtr(m_HashTableData)->Table[0]->Next = getBucket(0x80000000);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	hash_map<TKey, TData, FHash>::~hash_map()
	{
		PListItem h = getBucket(0);
		DeleteTable(HashTablePtr(m_HashTableData), HashTableSize(m_HashTableData));

		while (h)
		{
			PListItem tmp = h;
			h = NodePointer(h->Next);
			delete tmp;
		};

		h = m_GarbageCollector.Purge0;
		while (h)
		{
			PListItem tmp = h;
			h = h->NextPurge;
			delete tmp;
		};

		h = m_GarbageCollector.Purge1;
		while (h)
		{
			PListItem tmp = h;
			h = h->NextPurge;
			delete tmp;
		};

	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename std::pair<typename hash_map<TKey, TData, FHash>::iterator, bool> hash_map<TKey, TData, FHash>::insert(const value_type & Val)
	{
		int gc = addRef();
		PListItem node = new TListItem;
		node->Value = Val;
		node->Hash = FHash(&node->Value.first, sizeof(TKey)) | 1;
		node->NextPurge = NULL;
		node->Next = NULL;

		void * tmp;
		void * newdata;
		tmp = (void*)m_HashTableData;

		uint32_t mask = getMask(HashTableSize(tmp));

		uint32_t bucket = node->Hash & mask;
		PListItem bucketnode = getBucket(bucket);

		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);
		PListItem retnode = listInsert(bucketnode, node);
		if (retnode != node)
		{
			delete node;
			return std::make_pair(iterator(this, retnode, gc), false);
		}

		if ((INC_32(m_Count) > ((uint32_t)1 << (HashTableSize(tmp) + 3))) && (HashTableSize(tmp) < 31) && (HashTableSize(tmp) == HashTableSize(m_HashTableData)))
		{
			newdata = HashTable(HashTablePtr(tmp), HashTableSize(tmp) + 1);

			if ((HashTableSize(tmp) & 0x7) == 0)
			{
				newdata = HashTable(makeNewTable(), HashTableSize(tmp) + 1);
				HashTablePtr(newdata)->Table[0] = (TListItem*)HashTablePtr(tmp);

				if (tmp != CMPXCHG_Ptr(m_HashTableData, newdata, tmp))
					destroyTable(HashTablePtr(newdata)); // someone else expanded the table. 
			} else {
				CMPXCHG_Ptr(m_HashTableData, newdata, tmp);
			}

		}

		return std::make_pair(iterator(this, node, gc), true);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::iterator hash_map<TKey, TData, FHash>::find(const TKey & Key)
	{
		int gc = addRef();
		uint32_t hash = FHash(&Key, sizeof(TKey)) | 1;
		uint32_t mask = getMask(HashTableSize(m_HashTableData));
		uint32_t bucket = hash & mask;
		PListItem bucketnode = getBucket(bucket);
		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);

		PListItem volatile * prev;
		PListItem curr, next;
		if (listFind(bucketnode, hash, Key, NULL, prev, curr, next))
			return iterator(this, NodePointer(curr), gc);

		return iterator(this, NULL, gc);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	typename hash_map<TKey, TData, FHash>::iterator hash_map<TKey, TData, FHash>::erase(const iterator & Where)
	{
		int gc = addRef();
		uint32_t hash = Where.m_Item->Hash;
		uint32_t mask = getMask(HashTableSize(m_HashTableData));
		uint32_t bucket = hash & mask;
		PListItem bucketnode = getBucket(bucket);

		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);

		PListItem res = listDelete(bucketnode, Where.m_Item);
		if (Where.m_Item == res)
		{
			DEC_32(m_Count);
			return iterator(this, NodePointer(res->Next), gc);
		}
		return iterator(this, res, gc);
	};

	template <typename TKey, typename TData, uint32_t (*FHash)(const void *, uint32_t)>
	size_t hash_map<TKey, TData, FHash>::erase(const TKey & Key)
	{
		int gc = addRef();
		uint32_t hash = FHash(&Key, sizeof(TKey)) | 1;
		uint32_t mask = getMask(HashTableSize(m_HashTableData));
		uint32_t bucket = hash & mask;
		PListItem bucketnode = getBucket(bucket);

		if (bucketnode == NULL)
			bucketnode = initializeBucket(bucket, mask);

		PListItem result = listDelete(bucketnode, hash, Key);
		if (result && (result->Value.first == Key))
		{
			DEC_32(m_Count);
			delRef(gc);
			return 1;
		}

		delRef(gc);
		return 0;
	};
}

#undef NodePointer
#undef NodeMark

#undef HashTablePtr
#undef HashTableSize
#undef HashTable

#undef GCSelection
#undef GCRefCount0
#undef GCRefCount1
#undef GCMakeSentinel

