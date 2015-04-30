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
	TalkBot& operator=(const TalkBot&);

public:
	struct MessageInfo
	{
	private:
		std::tstring Question;
		MessageInfo(std::tstring q, std::tstring a)
			:Question(q), Answer(a)
		{
		}
	public:
		std::tstring Answer;
		MessageInfo(std::tstring q)
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
	friend std::tstring LevelToStr(TalkBot::Level target);

	struct ContactData
	{
		ValueChooser<> initial;
		//ValueChooser<std::set<std::tstring> > question;
		//ValueChooser<std::set<std::tstring> > special;
		ValueChooser<> escape;
		ValueChooser<> failure;
		UnRecentChooser chooser;
		std::tstring lastMessage;
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

	PerContactData<Mind, ContactData, MCONTACT>* contactDatas;
	const Mind mind;
	bool beSilent;
	bool makeLowercase;
	bool understandAlways;
	void UpdateStartChar(std::tstring &str);
	typedef std::multimap<std::tstring, std::tstring>::const_iterator mm_cit;
	bool FindExact(ContactData *contactData, const std::tstring &incomingMessage, const std::multimap<std::tstring, std::tstring> &map, std::tstring &res);
	bool FindAny(ValueChooser<> &ch, std::tstring &res);
	void FindByKeywords(ContactData *contactData, const std::vector<std::tstring> &keywords, std::tstring &res/*, std::tstring& ures*/, bool isQuestion);
	bool FindByOthers(ContactData *contactData, const std::vector<std::tstring> &otherwords, std::tstring &res, bool isQuestion);
	std::tstring AllReplies(const std::tstring &incomingMessage, ContactData *contactData, Level &maxValue, std::multimap<Level, std::tstring> &mm);
	std::tstring ReplaceAliases(const std::tstring &message);
	std::tstring ChooseResult(ContactData *contactData, Level maxValue, const std::multimap<Level, std::tstring> &mm);
	void RecordAnswer(ContactData *contactData, const TalkBot::MessageInfo &info);
#ifdef _DEBUG
public:
#endif
	void SplitSectences(const std::tstring &incomingMessage, std::vector<std::tstring> &vec);
	void SplitAndSortWords(std::tstring sentence, std::vector<std::tstring> &keywords,
		std::vector<std::tstring>& otherwords, bool& isQuestion);
public:
	TalkBot(const Mind& goodMind);
	~TalkBot();
	const Mind& GetMind() const;
	void SetSilent(const bool isSilent);
	void SetLowercase(const bool isLowercase);
	void SetUnderstandAlways(const bool understandAlways);
	//const MindData *GetData();
	std::tstring GetInitMessage(MCONTACT contact);
	MessageInfo* Reply(MCONTACT contact, const std::tstring incomingMessage, bool saveChoice);
	void AnswerGiven(MCONTACT contact, const MessageInfo &info);
};

#endif
