#ifndef __8C9706FF_6B05_4d0d_85B8_5724E5DC0BA4_HTTPSession_h__
#define __8C9706FF_6B05_4d0d_85B8_5724E5DC0BA4_HTTPSession_h__

class CHTTPSession
{
	static HNETLIBUSER g_hNetLib;
	CMStringA m_szUrl;
	mir_cs m_mx;

public:
	CHTTPSession() {}
	~CHTTPSession() {}

	static bool Init();

	bool OpenURL(const std::wstring &rsURL);
	bool ReadResponce(std::wstring &rsResponce);
};

#endif //__8C9706FF_6B05_4d0d_85B8_5724E5DC0BA4_HTTPSession_h__
