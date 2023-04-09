#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

const wchar_t* CMPlugin::getQuestion()
{
	const wchar_t *res = Question;
	return (res != nullptr) ? res : TranslateW(L"Spammers made me to install small anti-spam system you are now speaking with. Please reply \"nospam\" without quotes and spaces if you want to contact me.");
}

const wchar_t* CMPlugin::getReply()
{
	const wchar_t *res = AuthRepl;
	return (res != nullptr) ? res : TranslateW(L"StopSpam: send a message and reply to an anti-spam bot question.");
}

const wchar_t* CMPlugin::getCongrats()
{
	const wchar_t *res = Congratulation;
	return (res != nullptr) ? res : TranslateW(L"Congratulations! You just passed human/robot test. Now you can write me a message.");
}
