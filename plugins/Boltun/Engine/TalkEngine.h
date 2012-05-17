//***********************************************************
//	Copyright © 2008 Valentin Pavlyuchenko
//
//	This file is part of Boltun.
//
//    Boltun is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 2 of the License, or
//    (at your option) any later version.
//
//    Boltun is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//	  along with Boltun. If not, see <http://www.gnu.org/licenses/>.
//
//***********************************************************

#ifndef TalkEngineH
#define TalkEngineH

#include "Mind.h"
#include "ValueChooser.h"
#include "PerContactData.h"
#include "UnrecentChooser.h"

class TalkBot
{
public:
	struct MessageInfo
	{
		private:
			tstring Question;
			MessageInfo(tstring q, tstring a)
				:Question(q), Answer(a)
			{
			}
		public:
			tstring Answer;
			MessageInfo(tstring q)
				:Question(q)
			{
			}
		friend class TalkBot;
	};
private:
	typedef enum
	{
		BEST, LOOKSLIKE/*, LOOKSLIKE2*/, BAD, FAIL, NOTHING
	} Level;
	friend tstring LevelToStr(TalkBot::Level target); 

	struct ContactData
	{
		ValueChooser<> initial;
		//ValueChooser<std::set<tstring> > question;
		//ValueChooser<std::set<tstring> > special;
		ValueChooser<> escape;
		ValueChooser<> failure;
		UnRecentChooser chooser;
		tstring lastMessage;
		long long lastMessageTime;
		int repeatCount;
		inline ContactData(const Mind& mind)
			:initial(mind.GetData()->initial),
			//question(mind.GetData()->question),
			//special(mind.GetData()->special),
			escape(mind.GetData()->escape),
			failure(mind.GetData()->failure),
			repeatCount(0)
		{
		}
	};

	PerContactData<Mind, ContactData, void*>* contactDatas;
	const Mind mind;
	bool beSilent;
	bool makeLowercase;
	bool understandAlways;
	void UpdateStartChar(tstring& str);
	typedef std::multimap<tstring, tstring>::const_iterator mm_cit; 
	bool FindExact(ContactData* contactData, const tstring &incomingMessage,
		const std::multimap<tstring, tstring>& map, tstring& res);
	bool FindAny(ValueChooser<> &ch, tstring& res);
	void FindByKeywords(ContactData* contactData, const std::vector<tstring> &keywords, tstring& res/*, tstring& ures*/, bool isQuestion);
	bool FindByOthers(ContactData* contactData, const std::vector<tstring> &otherwords, tstring& res, bool isQuestion);
	tstring AllReplies(const tstring &incomingMessage, ContactData* contactData, Level &maxValue, std::multimap<Level, tstring> &mm);
	tstring ReplaceAliases(const tstring &message);
	tstring ChooseResult(ContactData* contactData, Level maxValue, const std::multimap<Level, tstring> &mm);
	void RecordAnswer(ContactData *contactData, const TalkBot::MessageInfo& info);
#ifdef _DEBUG
public:
#endif
	void SplitSectences(const tstring &incomingMessage, std::vector<tstring>& vec);
	void SplitAndSortWords(tstring sentence, std::vector<tstring>& keywords,
		std::vector<tstring>& otherwords, bool& isQuestion);
public:
	TalkBot(const Mind& goodMind);
	~TalkBot();
	const Mind& GetMind() const;
	void SetSilent(const bool isSilent);
	void SetLowercase(const bool isLowercase);
	void SetUnderstandAlways(const bool understandAlways);
	//const MindData *GetData();
	tstring GetInitMessage(void* contact);
	MessageInfo* Reply(void* contact, const tstring incomingMessage, bool saveChoice);
	void AnswerGiven(void* contact, const MessageInfo& info);
};

#endif
