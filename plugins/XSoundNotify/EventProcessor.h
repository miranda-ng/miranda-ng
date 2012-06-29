#ifndef __XSN_EVENT_PROCESSOR
#define __XSN_EVENT_PROCESSOR

#include <windows.h>
#include "xsn_types.h"
#include <newpluginapi.h>


class XSN_EventProcessor
{
public:
	static void make(PLUGINLINK * pl);
	static XSN_EventProcessor & instance();

	void process(WPARAM wParam, LPARAM lParam);	

protected:
	XSN_EventProcessor(PLUGINLINK * pl);

	bool isReceiveMessage(LPARAM event);
	xsn_string getProtocol(WPARAM contact);
	xsn_string getNick(WPARAM contact, LPCTSTR protocol);

	PLUGINLINK * pluginLink;
};

#endif
