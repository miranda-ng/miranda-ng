/*
 *  Smart Auto Replier (SAR) - auto replier plugin for Miranda IM
 *
 *  Copyright (C) 2004 - 2012 by Volodymyr M. Shcherbyna <volodymyr@shcherbyna.com>
 *
 *      This file is part of SAR.
 *
 *  SAR is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  SAR is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SAR.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "RuleItem.h"
#include <map>

#define STORAGE_NAME	TEXT("sar.rsu")			/// a def. name of a file that is storing settings
typedef std::map<INT, RULE_ITEM> RulesHash;		/// storage representation in memory

/// storage manager..
class CRulesStorage : ISettings
{
public:	/// ctor'rs, dtors..
	CRulesStorage(void);
	~CRulesStorage(void);
public: /// ISettings interface...
	bool Serialize(ISettingsStream *pSettings, DWORD & ObjectSize);
	bool Deserialize(DWORD & ObjectSize);
public:
	void Init(void);	/// initing state of an object
	void DeInit(void);	/// deiniting state of an object
	DWORD AddRuleItem(RULE_ITEM item, bool & bExists); /// n.c.
	bool RemReplyAction(DWORD dwCrc32);				   /// n.c.
	RulesHash & getHashTable(void);					   /// n.c.		
	COMMON_RULE_ITEM & getCommonRule(void);			   /// n.c.
	void setCommonRule(COMMON_RULE_ITEM r);		   /// n.c.
	void Flush(void);											/// flushes all from memory to a file
	void FlushItem(RULE_ITEM & item);							/// flushes an item into file from memory
	bool IsRuleMatch(RULE_METAINFO & info, LPTSTR & strMess, LPTSTR & lpIncomingMsg);	/// n.c.
	void ClearCommonMessages(void);	/// releases memory that is commited for comm. rule
	bool RuleIsRegistered(LPTSTR lpContactName);
private:
	void  RawWriteDataBufByChunk(LPTSTR);			/// writes string to a file
	UINT  RawReadDataBufByChunk(LPTSTR & szData);	/// reads string from file
	UINT GetCommonMessages(void);	/// gets common rules from file to memory
	void SetCommonMessages(void);	/// sets common rules from memory to file	
private:
	COMMON_RULE_ITEM m_comItem;			/// common rule item..
	RulesHash m_hashTable;				/// main storage...
	HANDLE m_hFile;						/// handle to file  that is storing data
	TCHAR m_szSettFileName[MAX_PATH];	/// path to a file...
	CRITICAL_SECTION m_critSect;		/// needed for sync
};