#include "EventProcessor.h"
#include <m_database.h>
#include "m_protocols.h"
#include <memory>
#include "DebugLogger.hpp"

// static area

static std::unique_ptr<XSN_EventProcessor> XSN_EventProcessorPtr;

void XSN_EventProcessor::make(PLUGINLINK * pl)
{
	XSN_EventProcessorPtr.reset(new XSN_EventProcessor(pl));
}

XSN_EventProcessor & XSN_EventProcessor::instance()
{	
	return *XSN_EventProcessorPtr;
}

// class methods

XSN_EventProcessor::XSN_EventProcessor(PLUGINLINK * pl) : pluginLink(pl)
{

}

void XSN_EventProcessor::process(WPARAM wParam, LPARAM lParam)
{	
	if (!wParam || !lParam || !isReceiveMessage(lParam))
		return ;	
	try
	{
		MessageBox(0, "Receive message", "INFO", MB_OK);
		/*GRS_DEBUG_LOG("Receive message");
		HANDLE contact = (HANDLE)wParam;
		MessageBox(0, "Get protocol", "INFO", MB_OK);
		GRS_DEBUG_LOG("Protocol : ");
		xsn_string proto = getProtocol(wParam);
		MessageBox(0, "Protocol is", proto.c_str(), MB_OK);
		GRS_DEBUG_FORMAT_LOG(proto)
		//xsn_string nick = getNick(wParam, proto.c_str());
		XSN_Variant sound;
		DBGetContactSettingTString(contact, proto.c_str(), "XSNPlugin_sound", &sound);
		if (!sound.empty())
		{
			GRS_DEBUG_LOG("Sound for user exist");
			GRS_DEBUG_FORMAT_LOG("Playing sound : " << sound.ptszVal << ", for user : " << getNick(wParam, proto.c_str()));
			PlaySound(sound.toString().c_str(), nullptr, SND_FILENAME | SND_ASYNC);			
		}*/
	}
	catch (std::runtime_error &)
	{
		//MessageBoxA(0, e.what(), "Runtime error", MB_OK);
	}
	catch (...)
	{
		//MessageBoxA(0, "Unknown error occured", "Exception", MB_OK);
	}
}

bool XSN_EventProcessor::isReceiveMessage(LPARAM event)
{
	DBEVENTINFO info ={ sizeof(info) };	
	CallService(MS_DB_EVENT_GET, event, (LPARAM)&info);
	
	// TODO : rec msg flag : 16, send msg flag : 18 - WTF?
	// return (info.eventType == EVENTTYPE_MESSAGE) && (info.flags & DBEF_READ);
	// It's condition work well - magic?
	return  !(((info.eventType != EVENTTYPE_MESSAGE) && !(info.flags & DBEF_READ)) || (info.flags & DBEF_SENT));
}

xsn_string XSN_EventProcessor::getProtocol(WPARAM contact)
{
	char *pszProto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, contact, 0);
	return xsn_string(pszProto);
}

xsn_string XSN_EventProcessor::getNick(WPARAM contact, LPCTSTR protocol)
{
	XSN_Variant nick;
	DBGetContactSettingTString((HANDLE)contact, protocol, "Nick", &nick);
	return nick.toString();
}
