#pragma once

struct Settings
{
	Settings();

	CMOption<bool> InfTalkProtection, AddPermanent, HandleAuthReq, AnswNotCaseSens, RemTmpAll, HistLog;
	CMOption<wchar_t*> Question, AuthRepl, Answer, Congratulation, AnswSplitString;
	CMOption<char*> DisabledProtoList;
	CMOption<DWORD> MaxQuestCount;

	bool ProtoDisabled(const char *proto)
	{
		std::string temp(proto); temp += ' ';
		return strstr(DisabledProtoList, temp.c_str()) != nullptr;
	}
};

extern Settings g_sets;
