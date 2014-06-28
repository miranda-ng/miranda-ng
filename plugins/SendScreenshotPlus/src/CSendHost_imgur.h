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
#ifndef _CSEND_HOST_IMGUR_H
#define _CSEND_HOST_IMGUR_H
class CSendHost_Imgur : public CSend {
// API: http://api.imgur.com/endpoints/image
	public:
		CSendHost_Imgur(HWND Owner, MCONTACT hContact, bool bAsync);
		~CSendHost_Imgur();
		int Send();
	protected:
		NETLIBHTTPREQUEST m_nlhr;
		static void SendThread(void* obj);
};
#endif
