#include "stdafx.h"

void CNudge::Save(void)
{
	char SectionName[512];
	mir_snprintf(SectionName, "useByProtocol");
	g_plugin.setByte(SectionName, this->useByProtocol);
	mir_snprintf(SectionName, "RecvTimeSec");
	g_plugin.setDword(SectionName, this->recvTimeSec);
	mir_snprintf(SectionName, "SendTimeSec");
	g_plugin.setDword(SectionName, this->sendTimeSec);
	mir_snprintf(SectionName, "ResendDelaySec");
	g_plugin.setDword(SectionName, this->resendDelaySec);
}

void CNudge::Load(void)
{
	char SectionName[512];
	mir_snprintf(SectionName, "useByProtocol");
	this->useByProtocol = g_plugin.getByte(SectionName, FALSE) != 0;
	mir_snprintf(SectionName, "RecvTimeSec");
	this->recvTimeSec = g_plugin.getDword(SectionName, 30);
	mir_snprintf(SectionName, "SendTimeSec");
	this->sendTimeSec = g_plugin.getDword(SectionName, 30);
	mir_snprintf(SectionName, "ResendDelaySec");
	this->resendDelaySec = g_plugin.getDword(SectionName, 3);
}

void CNudgeElement::Save(void)
{
	char SectionName[512];
	mir_snprintf(SectionName, "%s-openMessageWindow", ProtocolName);
	g_plugin.setByte(SectionName, this->openMessageWindow);
	mir_snprintf(SectionName, "%s-openContactList", ProtocolName);
	g_plugin.setByte(SectionName, this->openContactList);
	mir_snprintf(SectionName, "%s-useIgnore", ProtocolName);
	g_plugin.setByte(SectionName, this->useIgnoreSettings);
	mir_snprintf(SectionName, "%s-showStatus", ProtocolName);
	g_plugin.setByte(SectionName, this->showStatus);
	mir_snprintf(SectionName, "%s-showPopup", ProtocolName);
	g_plugin.setByte(SectionName, this->showPopup);
	mir_snprintf(SectionName, "%s-shakeClist", ProtocolName);
	g_plugin.setByte(SectionName, this->shakeClist);
	mir_snprintf(SectionName, "%s-shakeChat", ProtocolName);
	g_plugin.setByte(SectionName, this->shakeChat);
	mir_snprintf(SectionName, "%s-enabled", ProtocolName);
	g_plugin.setByte(SectionName, this->enabled);
	mir_snprintf(SectionName, "%s-autoResend", ProtocolName);
	g_plugin.setByte(SectionName, this->autoResend);
	mir_snprintf(SectionName, "%s-statusFlags", ProtocolName);
	g_plugin.setDword(SectionName, this->statusFlags);
	mir_snprintf(SectionName, "%s-recText", ProtocolName);
	g_plugin.setWString(SectionName, this->recText);
	mir_snprintf(SectionName, "%s-senText", ProtocolName);
	g_plugin.setWString(SectionName, this->senText);
}

void CNudgeElement::Load(void)
{
	char SectionName[512];
	mir_snprintf(SectionName, "%s-openMessageWindow", ProtocolName);
	this->openMessageWindow = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-openContactList", ProtocolName);
	this->openContactList = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-useIgnore", ProtocolName);
	this->useIgnoreSettings = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-showStatus", ProtocolName);
	this->showStatus = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-showPopup", ProtocolName);
	this->showPopup = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-shakeClist", ProtocolName);
	this->shakeClist = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-shakeChat", ProtocolName);
	this->shakeChat = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-enabled", ProtocolName);
	this->enabled = g_plugin.getByte(SectionName, TRUE) != 0;
	mir_snprintf(SectionName, "%s-autoResend", ProtocolName);
	this->autoResend = g_plugin.getByte(SectionName, FALSE) != 0;
	mir_snprintf(SectionName, "%s-statusFlags", ProtocolName);
	this->statusFlags = g_plugin.getDword(SectionName, 967);

	mir_snprintf(SectionName, "%s-recText", ProtocolName);
	ptrW wszRecvText(g_plugin.getWStringA(SectionName));
	if (mir_wstrlen(wszRecvText) > 0)
		wcsncpy_s(this->recText, wszRecvText, _TRUNCATE);
	else
		wcsncpy_s(this->recText, TranslateT("You received a nudge"), _TRUNCATE);

	mir_snprintf(SectionName, "%s-senText", ProtocolName);
	ptrW wszSendText(g_plugin.getWStringA(SectionName));
	if (mir_wstrlen(wszSendText) > 0)
		wcsncpy_s(this->senText, wszSendText, _TRUNCATE);
	else
		wcsncpy_s(this->senText, TranslateT("You sent a nudge"), _TRUNCATE);
}
