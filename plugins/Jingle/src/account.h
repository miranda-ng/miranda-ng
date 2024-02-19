#ifndef _ACCOUNT_H
#define _ACCOUNT_H

struct CJabberAccount : public MZeroedObject
{
	CJabberAccount(IJabberInterface *_1);
	~CJabberAccount();

	IJabberInterface *m_api;

	CMOption<bool> m_bEnableVOIP;
	CMOption<wchar_t *> m_szStunServer;

	void Init();
	static void InitHooks();

	void InitVoip(bool bEnable);
	bool OnICECandidate(const TiXmlElement *Node);
	bool OnRTPDescription(const TiXmlElement *Node);
	bool VOIPCreatePipeline();
	bool VOIPTerminateSession(const char *reason = "cancel");
	bool VOIPCallIinitiate(MCONTACT hContact);

	const char *m_szModuleName;
	CMStringA m_voipSession, m_voipPeerJid;
	CMStringA m_voipICEPwd, m_voipICEUfrag, m_medianame;
	bool m_isOutgoing = false;
	TiXmlDocument m_offerDoc;
	const TiXmlElement *m_offerNode = 0;
	HANDLE m_hVoiceEvent = 0;
	struct _GstElement *m_pipe1 = 0;
	struct _GstElement *m_webrtc1 = 0;
};

extern OBJLIST<CJabberAccount> g_arJabber;

#endif //_ACCOUNT_H