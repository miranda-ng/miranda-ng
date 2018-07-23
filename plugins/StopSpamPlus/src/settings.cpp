#include "stdafx.h"

Settings g_sets;

/////////////////////////////////////////////////////////////////////////////////////////

Settings::Settings() :
	Question(MODULENAME, "Question"),
	AuthRepl(MODULENAME, "AuthReply"),
	Answer(MODULENAME, "Answer", L"nospam"),
	Congratulation(MODULENAME, "Congratulation"),
	DisabledProtoList(MODULENAME, "DisabledProtoList", "MetaContacts RSSNews"),
	InfTalkProtection(MODULENAME, "InfTalkProtection", 1),
	AddPermanent(MODULENAME, "AddPermanent", 0),
	HandleAuthReq(MODULENAME, "HandleAuthReq", 0),
	MaxQuestCount(MODULENAME, "MaxQuestCount", 2),
	AnswNotCaseSens(MODULENAME, "AnswNotCaseSens", 1),
	AnswSplitString(MODULENAME, "AnswSplitString", L"|"),
	RemTmpAll(MODULENAME, "RemTmpAll", 1),
	HistLog(MODULENAME, "HistLog", 0)
{
}

const wchar_t* Settings::getQuestion()
{
	const wchar_t *res = Question;
	return (res != nullptr) ? res : TranslateW(L"Spammers made me to install small anti-spam system you are now speaking with. Please reply \"nospam\" without quotes and spaces if you want to contact me.");
}

const wchar_t* Settings::getReply()
{
	const wchar_t *res = AuthRepl;
	return (res != nullptr) ? res : TranslateW(L"StopSpam: send a message and reply to an anti-spam bot question.");
}

const wchar_t* Settings::getCongrats()
{
	const wchar_t *res = Congratulation;
	return (res != nullptr) ? res : TranslateW(L"Congratulations! You just passed human/robot test. Now you can write me a message.");
}
