/*
            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                    Version 2, December 2004

 Copyright (C) 2014 Miranda NG project (http://miranda-ng.org)

 Everyone is permitted to copy and distribute verbatim or modified
 copies of this license document, and changing it is allowed as long
 as the name is changed.

            DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
   TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

  0. You just DO WHAT THE FUCK YOU WANT TO.
*/
#ifndef _CSEND_HOST_UPLOADPIE_H
#define _CSEND_HOST_UPLOADPIE_H
class CSendHost_UploadPie : public CSend {
	public:
		CSendHost_UploadPie(HWND Owner, MCONTACT hContact, bool bAsync, int expire);
		~CSendHost_UploadPie();
		int Send();
	protected:
		int m_expire;
		NETLIBHTTPREQUEST m_nlhr;
		static void SendThread(void* obj);
};
#endif
