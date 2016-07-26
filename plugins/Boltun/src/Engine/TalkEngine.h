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
		std::wstring Question;
		MessageInfo(std::wstring q, std::wstring a)
			:Question(q), Answer(a)
		{
		}
	public:
		std::wstring Answer;
		MessageInfo(std::wstring q)
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
	friend std::wstring LevelToStr(TalkBot::Level target);

	struct ContactData
	{
		ValueChooser<> initial;
		//ValueChooser<std::set<std::wstring> > question;
		//ValueChooser<std::set<std::wstring> > special;
		ValueChooser<> escape;
		ValueChooser<> failure;
		UnRecentChooser chooser;
		std::wstring lastMessage;
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
	void UpdateStartChar(std::wstring &str);
	typedef std::multimap<std::wstring, std::wstring>::const_iterator mm_cit;
	bool FindExact(ContactData *contactData, const std::wstring &incomingMessage, const std::multimap<std::wstring, std::wstring> &map, std::wstring &res);
	bool FindAny(ValueChooser<> &ch, std::wstring &res);
	void FindByKeywords(ContactData *contactData, const std::vector<std::wstring> &keywords, std::wstring &res/*, std::wstring& ures*/, bool isQuestion);
	bool FindByOthers(ContactData *contactData, const std::vector<std::wstring> &otherwords, std::wstring &res, bool isQuestion);
	std::wstring AllReplies(const std::wstring &incomingMessage, ContactData *contactData, Level &maxValue, std::multimap<Level, std::wstring> &mm);
	std::wstring ReplaceAliases(const std::wstring &message);
	std::wstring ChooseResult(ContactData *contactData, Level maxValue, const std::multimap<Level, std::wstring> &mm);
	void RecordAnswer(ContactData *contactData, const TalkBot::MessageInfo &info);
#ifdef _DEBUG
public:
#endif
	void SplitSectences(const std::wstring &incomingMessage, std::vector<std::wstring> &vec);
	void SplitAndSortWords(std::wstring sentence, std::vector<std::wstring> &keywords,
		std::vector<std::wstring>& otherwords, bool& isQuestion);
public:
	TalkBot(const Mind& goodMind);
	~TalkBot();
	const Mind& GetMind() const;
	void SetSilent(const bool isSilent);
	void SetLowercase(const bool isLowercase);
	void SetUnderstandAlways(const bool understandAlways);
	//const MindData *GetData();
	std::wstring GetInitMessage(MCONTACT contact);
	MessageInfo* Reply(MCONTACT contact, const std::wstring incomingMessage, bool saveChoice);
	void AnswerGiven(MCONTACT contact, const MessageInfo &info);
};

#endif
