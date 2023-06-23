#ifndef _COMMONHEADERS_H
#define _COMMONHEADERS_H

#include <gst/gst.h>
#include <gst/sdp/sdp.h>
#include <gst/rtp/rtp.h>

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

#pragma comment(lib, "glib-2.0.lib")
#pragma comment(lib, "gobject-2.0.lib")
#pragma comment(lib, "gstreamer-1.0.lib")
#pragma comment(lib, "gstrtp-1.0.lib")
#pragma comment(lib, "gstsdp-1.0.lib")
#pragma comment(lib, "gstwebrtc-1.0.lib")

#include <windows.h>

#include <list>

#include <newpluginapi.h>
#include <m_jabber.h>
#include <m_jingle.h>
#include <m_langpack.h>
#include <m_voice.h>
#include <m_voiceservice.h>

#include "account.h"
#include "resource.h"
#include "version.h"

struct CMPlugin : public PLUGIN<CMPlugin>
{
	CMPlugin();

	int Load() override;
};

#endif //_COMMONHEADERS_H