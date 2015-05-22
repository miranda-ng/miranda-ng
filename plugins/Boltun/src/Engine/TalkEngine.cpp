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

#include "..\stdafx.h"

#ifdef _DEBUG

//#define DEBUG_PREFIXES
//#define DEBUG_SHOW_LEVEL
//#define DEBUG_SHOW_VARIANTS
//#define DEBUG_SHOW_SOLUTION_REASON

#endif

//Enabling next define will make a bot more stupid:
//#define EXCLUDE_SPECIAL_WORDS

#ifdef DEBUG_SHOW_VARIANTS
extern void AddBotMessage(tstring s);
#endif

using namespace std;

void TalkBot::UpdateStartChar(tstring& str)
{
	if (!makeLowercase)
		return;
	size_t l = str.length();
	if (l)
	{
		//Answers starting with ' ' must remain unchanged.
		if (str[0] == _T(' '))
		{
			str = str.substr(1);
			return;
		}
		for (size_t i = 0; i < l; i++)
		{
			TCHAR cl = (TCHAR)CharLower((LPTSTR)(void*)(long)str[i]);
			TCHAR cu = (TCHAR)CharUpper((LPTSTR)(void*)(long)str[i]);
			if (i != l - 1)
			{
				//Do not react to BLONDE ANSWERS
				TCHAR ncl = (TCHAR)CharLower((LPTSTR)(void*)(long)str[i + 1]);
				TCHAR ncu = (TCHAR)CharUpper((LPTSTR)(void*)(long)str[i + 1]);
				if (ncl != ncu && str[i + 1] == ncu)
					break;
			}
			if (cl != cu)
			{
				str[i] = cl;
				break;
			}
		}
	}
}

TalkBot::TalkBot(const Mind& goodMind)
	:mind(goodMind), beSilent(false), makeLowercase(false),
	understandAlways(false)
{
	contactDatas = new PerContactData<Mind, ContactData, MCONTACT>(mind);
}

TalkBot::~TalkBot()
{
	delete contactDatas;
}

tstring TalkBot::GetInitMessage(MCONTACT contact)
{
	ContactData* d = contactDatas->GetData(contact);
	tstring s = d->initial.GetString();
	contactDatas->PutData(contact);
	return s;
}

tstring TalkBot::ReplaceAliases(const tstring &message)
{
	const TCHAR dividers[] = _T(" \t\n\r,./?\\|;:'\"~!#^&*()_-+=[{]}—\1");
	tstring sentence = message;
	tstring result;
	map<size_t, tstring> sm;
	//Find smiles
	for (size_t i = 0; i < sentence.length() - 1; i++)
	{
		unsigned max = (int)(sentence.length() - i);
		if (max > mind.GetData()->maxSmileLen)
			max = mind.GetData()->maxSmileLen;
		for (unsigned j = max; j > 0; j--)
		{
			tstring item = sentence.substr(i, j);
			if (mind.GetData()->smiles.find(item) != mind.GetData()->smiles.end())
			{
				sm[i] = item;
				sentence.replace(i, j, _T("\1"));
				break;
			}
		}
	}
	int len = (int)sentence.length();
	bool hadQuestionSigns = false;
	int it = 0;
	while (it != len)
	{
		while (it != len && _tcschr(dividers, sentence[it]))
		{
			if (sentence[it] == _T('?'))
				hadQuestionSigns = true;
			map<size_t, tstring>::iterator smit;
			if (sentence[it] == '\1')
			{
				smit = sm.find(it);
				result.append((*smit).second);
			}
			else
				result.push_back(sentence[it]);
			it++;
		}
		if (it == len)
			break;
		int start = it;
		while (true)
		{
			while (it != len && !_tcschr(dividers, sentence[it]))
				it++;
			if (it == len || sentence[it] != _T('-'))
				break;
			//If we have-a-word-with-minus, we shouldn't split it
			if (_tcschr(dividers, sentence[it + 1]))
				break;
			it += 2;
		}
		tstring str = sentence.substr(start, it - start);
		map<tstring, tstring>::const_iterator al = mind.GetData()->aliases.find(str);
		if (al != mind.GetData()->aliases.end())
			result.append((*al).second);
		else
			result.append(str);
	}
	return result;
}

tstring TalkBot::AllReplies(const tstring &incomingMessage, ContactData *contactData, Level &maxValue, std::multimap<Level, tstring> &mm)
{
	tstring res;
	//Part 1
	if (FindExact(contactData, incomingMessage, mind.GetData()->widelyUsed, res)) //widelyUsed
	{
		return res;
	}
	//Part 2
	if (FindExact(contactData, incomingMessage, mind.GetData()->study, res)) //study
	{
#ifdef DEBUG_PREFIXES
		mm.insert(make_pair(LOOKSLIKE, _T("(study_all) ")+res));
#else
		mm.insert(make_pair(LOOKSLIKE, res));
#endif
		maxValue = LOOKSLIKE;
	}
	//Part 3
	vector<tstring> sentences;
	SplitSectences(incomingMessage, sentences);
	ValueChooser<> ch(sentences, true); //Using random order of sentences.
	while ((res = ch.GetString()) != _T(""))
	{
		//Part 4
		if (FindExact(contactData, res, mind.GetData()->widelyUsed, res)) //widelyUsed
		{
#ifdef DEBUG_PREFIXES
			mm.insert(make_pair(BEST, _T("(widelyused_sent) ")+res));
#else
			mm.insert(make_pair(BEST, res));
#endif
			if (maxValue > BEST)
				maxValue = BEST;
		}
		//Part 5
		if (FindExact(contactData, res, mind.GetData()->study, res)) //study
		{
#ifdef DEBUG_PREFIXES
			mm.insert(make_pair(LOOKSLIKE, _T("(study_sent) ")+res));
#else
			mm.insert(make_pair(LOOKSLIKE, res));
#endif
			if (maxValue > LOOKSLIKE)
				maxValue = LOOKSLIKE;
		}
		//Part 6
		vector<tstring> keywords, otherwords;
		bool isQuestion;
		SplitAndSortWords(res, keywords, otherwords, isQuestion);
		//Part 7, 8
		res = _T("");
		FindByKeywords(contactData, keywords, res/*, ures*/, isQuestion); //keywords
		if (res != _T(""))
		{
#ifdef DEBUG_PREFIXES
			mm.insert(make_pair(LOOKSLIKE, _T("(keywords) ")+res));
#else
			mm.insert(make_pair(LOOKSLIKE, res));
#endif
			if (maxValue > LOOKSLIKE)
				maxValue = LOOKSLIKE;
		}
		/*		if (ures != _T(""))
				{
				#ifdef DEBUG_PREFIXES
				mm.insert(make_pair(LOOKSLIKE2, _T("(keywords_unstrict) ")+ures));
				#else
				mm.insert(make_pair(LOOKSLIKE2, ures));
				#endif
				if (maxValue > LOOKSLIKE2)
				maxValue = LOOKSLIKE2;
				}*/
		//Part 9
		if (FindByOthers(contactData, otherwords, res, isQuestion)) //specialEscapes
		{
#ifdef DEBUG_PREFIXES
			mm.insert(make_pair(BAD, _T("(otherwords) ")+res));
#else
			mm.insert(make_pair(BAD, res));
#endif
			if (maxValue > BAD)
				maxValue = BAD;
		}
	}
	if (!beSilent)
	{
		//Part 10
		if (FindAny(contactData->escape, res)) //escape
		{
#ifdef DEBUG_PREFIXES
			mm.insert(make_pair(FAIL, _T("(escape) ") + res));
#else
			mm.insert(make_pair(FAIL, res));
#endif
			if (maxValue > FAIL)
				maxValue = FAIL;
		}
		//Part 11
		if (!understandAlways && FindAny(contactData->failure, res)) //failure
		{
#ifdef DEBUG_PREFIXES
			mm.insert(make_pair(FAIL, _T("(failure) ") + res));
#else
			mm.insert(make_pair(FAIL, res));
#endif
			if (maxValue > FAIL)
				maxValue = FAIL;
		}
	}
	return tstring();
}

TalkBot::MessageInfo* TalkBot::Reply(MCONTACT contact, tstring incomingMessage, bool saveChoice)
{
	TCHAR* str = new TCHAR[incomingMessage.length() + 1];
	mir_tstrcpy(str, incomingMessage.c_str());
	CharLower(str);
	incomingMessage = str;
	delete[] str;
	ContactData *contactData = contactDatas->GetData(contact);

	if (incomingMessage == contactData->lastMessage && GetTickCount() < contactData->lastMessageTime + 30 * 60 * 1000)
	{
		MessageInfo *info;
		//only 2-3 repeats
		if (contactData->repeatCount < 2 || contactData->repeatCount == 2 && (rand() % 2))
		{
			const vector<tstring>& v = mind.GetData()->repeats;
			tstring res = v[rand() % v.size()];
#ifdef DEBUG_PREFIXES
			info = new MessageInfo(incomingMessage, _T("(repeat_norm) ") + res);
#else
			info = new MessageInfo(incomingMessage, res);
#endif
		}
		else
#ifdef DEBUG_PREFIXES
			info = new MessageInfo(incomingMessage, _T("(repeat_silence)"));
#else
			info = new MessageInfo(incomingMessage, _T(""));
#endif
		if (saveChoice)
			RecordAnswer(contactData, *info);
		contactDatas->PutData(contact);
		return info;
	}

	multimap<Level, tstring> mm;
	Level maxValue = NOTHING;

	tstring res = AllReplies(incomingMessage, contactData, maxValue, mm);
	if (!res.empty())
	{
		UpdateStartChar(res);
#ifdef DEBUG_PREFIXES
		MessageInfo *info = new MessageInfo(incomingMessage, _T("(widelyused_all) ") + res);
#else
		MessageInfo *info = new MessageInfo(incomingMessage, res);
#endif
		if (saveChoice)
			RecordAnswer(contactData, *info);
		contactDatas->PutData(contact);
		return info;
	}

	incomingMessage = ReplaceAliases(incomingMessage);

	res = AllReplies(incomingMessage, contactData, maxValue, mm);
	if (!res.empty())
	{
		UpdateStartChar(res);
#ifdef DEBUG_PREFIXES
		MessageInfo *info = new MessageInfo(incomingMessage, _T("(widelyused_all) ") + res);
#else
		MessageInfo *info = new MessageInfo(incomingMessage, res);
#endif
		if (saveChoice)
			RecordAnswer(contactData, *info);
		contactDatas->PutData(contact);
		return info;
	}

	//Also does Part 12
	tstring final = ChooseResult(contactData, maxValue, mm);
	MessageInfo *info = new MessageInfo(incomingMessage, final);
	UpdateStartChar(final);
	if (saveChoice)
		RecordAnswer(contactData, *info);
	contactDatas->PutData(contact);
	return info;
}

bool TalkBot::FindExact(ContactData *contactData, const tstring &incomingMessage,
	const multimap<tstring, tstring>& map, tstring& res)
{
	int max = (int)map.count(incomingMessage);
	if (!max)
	{
		TCHAR c = incomingMessage[incomingMessage.length() - 1];
		if (c != _T('?') && c != _T('.') && c != _T('!'))
			return FindExact(contactData, incomingMessage + _T('.'), map, res);
		return false;
	}
	pair<mm_cit, mm_cit> range = map.equal_range(incomingMessage);
	for (mm_cit it = range.first; it != range.second; ++it)
		contactData->chooser.AddChoice((*it).second);
	res = contactData->chooser.Choose();
	return true;
}

void TalkBot::AnswerGiven(MCONTACT contact, const TalkBot::MessageInfo& info)
{
	ContactData *contactData = contactDatas->GetData(contact);
	RecordAnswer(contactData, info);
	contactDatas->PutData(contact);
}

void TalkBot::RecordAnswer(ContactData *contactData, const TalkBot::MessageInfo& info)
{
	contactData->chooser.SaveChoice(info.Answer);
	if (contactData->lastMessage == info.Question)
		contactData->repeatCount++;
	else
		contactData->repeatCount = 0;
	contactData->lastMessageTime = GetTickCount();
	contactData->lastMessage = info.Question;
}

bool TalkBot::FindAny(ValueChooser<> &ch, tstring& res)
{
	if (!ch.GetContainer().size())
		return false;
	res = ch.GetString();
	return true;
}

void TalkBot::SplitSectences(const tstring &incomingMessage, vector<tstring>& vec)
{
	//FIXME: (THINK ABOUT IT:-))these chars not always mark the end of sentence.
	const TCHAR symbols[] = _T(".?!");
	int it = 0, len = (int)incomingMessage.length();
	while (it != len)
	{
		while (it != len && _istspace(incomingMessage[it]))
			it++;
		int start = it;
		while (it != len)
		{
			if (_tcschr(symbols, incomingMessage[it++]))
			{
				//Test for a :-! smile
				if (it > 2 && incomingMessage[it - 1] == _T('!')
					&& incomingMessage[it - 2] == _T('-')
					&& incomingMessage[it - 3] == _T(':'))
					continue;
				while (it != len && _tcschr(symbols, incomingMessage[it]))
					it++;
				break;
			}
		}
		vec.insert(vec.end(), incomingMessage.substr(start, it - start));
	}
}

#ifdef _DEBUG
tstring LevelToStr(TalkBot::Level target)
{
	tstring lev;
	switch (target)
	{
	case TalkBot::BEST: lev = _T("BEST(0)"); break;
	case TalkBot::LOOKSLIKE: lev = _T("LOOKSLIKE(1)"); break;
	case TalkBot::BAD: lev = _T("BAD(2)"); break;
	case TalkBot::FAIL: lev = _T("FAIL(3)"); break;
	case TalkBot::NOTHING: lev = _T("NOTHING(4)"); break;
	}
	return lev;
}
#endif

tstring TalkBot::ChooseResult(ContactData *contactData, Level maxValue, const multimap<Level, tstring> &mm)
{
#ifdef DEBUG_SHOW_VARIANTS
	AddBotMessage(_T(">>Availabe:"));
	for (multimap<Level, tstring>::iterator it = mm.begin(); it != mm.end(); it++)
		AddBotMessage(LevelToStr((*it).first) + _T(": ") + (*it).second);
	AddBotMessage(_T(">>Result:"));
#endif
	if (maxValue == NOTHING)
		return _T("");
	Level target = maxValue;
	typedef multimap<Level, tstring>::const_iterator lt_cit;
	pair<lt_cit, lt_cit> range = mm.equal_range(target);
	for (lt_cit it = range.first; it != range.second; ++it)
		contactData->chooser.AddChoice((*it).second);
#ifdef DEBUG_SHOW_LEVEL
	tstring lev = LevelToStr(target);
	return lev + _T(": ") + contactData->chooser.Choose();
#else
	return contactData->chooser.Choose();
#endif
}

void TalkBot::FindByKeywords(ContactData *contactData, const vector<tstring> &keywords, tstring& res/*, tstring& ures*/, 
	bool isQuestion)
{
	if (keywords.size() == 0)
		return;
	const multimap<WordsList, tstring> &keys = isQuestion ? mind.GetData()->qkeywords :
		mind.GetData()->keywords;
	for (multimap<WordsList, tstring>::const_iterator it = keys.begin(); it != keys.end(); ++it)
	{
		float prio;
		if ((*it).first.MatchesAll(keywords/*, strict*/, prio))
#ifdef DEBUG_SHOW_SOLUTION_REASON
			contactData->chooser.AddChoice((tstring)(*it).first + _T(": - ") + (*it).second, prio);
#else
			contactData->chooser.AddChoice((*it).second, prio);
#endif
	}
	res = contactData->chooser.Choose();
}

bool TalkBot::FindByOthers(ContactData *contactData, const vector<tstring> &otherwords, tstring& res, bool isQuestion)
{
	//vector<tstring> results;
	const multimap<WordsList, tstring> &specs = isQuestion ? mind.GetData()->qspecialEscapes :
		mind.GetData()->specialEscapes;
	for (multimap<WordsList, tstring>::const_iterator it = specs.begin();
		it != specs.end(); ++it)
		if ((*it).first.MatchesAny(otherwords))
		{
#ifdef DEBUG_SHOW_SOLUTION_REASON
			contactData->chooser.AddChoice((tstring)(*it).first + _T(": - ") + (*it).second);
#else
			contactData->chooser.AddChoice((*it).second);
#endif
		}
	res = contactData->chooser.Choose();
	if (res.empty())
		return false;
	return true;
}

const Mind& TalkBot::GetMind() const
{
	return mind;
}

void TalkBot::SplitAndSortWords(tstring sentence, vector<tstring>& keywords,
	vector<tstring>& otherwords, bool& isQuestion)
{
	const TCHAR dividers[] = _T(" \t\n\r,./?\\|;:'\"~!#^&*()_-+=[{]}—");
	int len = (int)sentence.length();
	vector<tstring> words;
	map<size_t, tstring> sm;
	//Find smiles
	for (size_t i = 0; i < sentence.length() - 1; i++)
	{
		unsigned max = (int)(sentence.length() - i);
		if (max > mind.GetData()->maxSmileLen)
			max = mind.GetData()->maxSmileLen;
		for (unsigned j = max; j > 0; j--)
		{
			tstring item = sentence.substr(i, j);
			if (mind.GetData()->smiles.find(item)
				!= mind.GetData()->smiles.end())
			{
				sm[i] = item;
				sentence.replace(i, j, _T(" "));
				break;
			}
		}
	}
	len = (int)sentence.length();
	bool hadQuestionSigns = false;
	int it = 0;
	while (it != len)
	{
		while (it != len && _tcschr(dividers, sentence[it]))
		{
			if (sentence[it] == _T('?'))
				hadQuestionSigns = true;
			map<size_t, tstring>::iterator smit;
			if (_istspace(sentence[it]) && (smit = sm.find(it)) != sm.end())
				words.push_back((*smit).second);
			it++;
		}
		if (it == len)
			break;
		hadQuestionSigns = false;
		int start = it;
		while (true)
		{
			while (it != len && !_tcschr(dividers, sentence[it]))
				it++;
			if (it == len || sentence[it] != _T('-'))
				break;
			//If we have-a-word-with-minus, we shouldn't split it
			if (_tcschr(dividers, sentence[it + 1]))
				break;
			it += 2;
		}
		tstring str = sentence.substr(start, it - start);
		words.push_back(str);
	}
	isQuestion = hadQuestionSigns;
	for (vector<tstring>::iterator it = words.begin(); it != words.end(); ++it)
	{
		if (!isQuestion)
		{
			if (mind.GetData()->question.find(*it) != mind.GetData()->question.end())
				isQuestion = true;
		}
		if (mind.GetData()->special.find(*it) != mind.GetData()->special.end())
			otherwords.push_back(*it);
#ifdef EXCLUDE_SPECIAL_WORDS
		else
#endif
			keywords.push_back(*it);
	}
}

void TalkBot::SetSilent(const bool isSilent)
{
	beSilent = isSilent;
}

void TalkBot::SetLowercase(const bool isLowercase)
{
	makeLowercase = isLowercase;
}

void TalkBot::SetUnderstandAlways(const bool understandAlways)
{
	this->understandAlways = understandAlways;
}