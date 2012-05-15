#include "Mra.h"
#include "proto.h"





INT_PTR		MraXStatusMenu			(WPARAM wParam,LPARAM lParam,LPARAM param);

int			MraContactDeleted		(WPARAM wParam,LPARAM lParam);
int			MraDbSettingChanged		(WPARAM wParam,LPARAM lParam);
int			MraRebuildContactMenu	(WPARAM wParam,LPARAM lParam);
int			MraExtraIconsApply		(WPARAM wParam,LPARAM lParam);
int			MraExtraIconsRebuild	(WPARAM wParam,LPARAM lParam);
int			MraRebuildStatusMenu	(WPARAM wParam,LPARAM lParam);
int			MraMusicChanged			(WPARAM wParam,LPARAM lParam);

DWORD		MraSendNewStatus		(DWORD dwStatusMir,DWORD dwXStatusMir,LPWSTR lpwszStatusTitle,SIZE_T dwStatusTitleSize,LPWSTR lpwszStatusDesc,SIZE_T dwStatusDescSize);


HANDLE		AddToListByEmail		(MRA_LPS *plpsEMail,MRA_LPS *plpsNick,MRA_LPS *plpsFirstName,MRA_LPS *plpsLastName,DWORD dwFlags);



INT_PTR LoadServices(void)
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

	// Service creation
	for (SIZE_T i=0;i<SIZEOF(siPluginServices);i++)
	{
		memmove(pszServiceFunctionName,siPluginServices[i].lpszName,(lstrlenA(siPluginServices[i].lpszName)+1));
		CreateServiceFunction(szServiceFunction,siPluginServices[i].lpFunc);
	}

	DebugPrintCRLFW(L"MRA/LoadServices - DONE");
return(0);
}


INT_PTR LoadModules(void)
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

	IconsLoad();
	masMraSettings.hExtraXstatusIcon=ExtraIcon_Register("MRAXstatus","Mail.ru Xstatus","MRA_xstatus25",MraExtraIconsRebuild,MraExtraIconsApply,NULL,NULL);
	masMraSettings.hExtraInfo=ExtraIcon_Register("MRAStatus","Mail.ru extra info","MRA_xstatus49",MraExtraIconsRebuild,MraExtraIconsApply,NULL,NULL);


	masMraSettings.hHookOptInitialize=HookEvent(ME_OPT_INITIALISE,OptInit);
	masMraSettings.hHookContactDeleted=HookEvent(ME_DB_CONTACT_DELETED,MraContactDeleted);
	masMraSettings.hHookSettingChanged=HookEvent(ME_DB_CONTACT_SETTINGCHANGED,MraDbSettingChanged);
	masMraSettings.hHookRebuildCMenu=HookEvent(ME_CLIST_PREBUILDCONTACTMENU,MraRebuildContactMenu);
	if (ServiceExists(MS_NUDGE_SEND)) 
	{
		memmove(pszServiceFunctionName,MS_NUDGE,sizeof(MS_NUDGE));
		masMraSettings.heNudgeReceived=CreateHookableEvent(szServiceFunction);
	}
	masMraSettings.hHookExtraIconsApply=HookEvent(ME_CLIST_EXTRA_IMAGE_APPLY,MraExtraIconsApply);
	masMraSettings.hHookExtraIconsRebuild=HookEvent(ME_CLIST_EXTRA_LIST_REBUILD,MraExtraIconsRebuild);


	// Main menu initialization
	CListCreateMenu(2000060000,500085000,(HANDLE)LoadImage(masMraSettings.hInstance,MAKEINTRESOURCE(IDI_MRA),IMAGE_ICON,0,0,LR_SHARED),NULL,MS_CLIST_ADDMAINMENUITEM,gdiMenuItems,masMraSettings.hMainMenuIcons,SIZEOF(gdiMenuItems),masMraSettings.hMainMenuItems);

	// Contact menu initialization
	CListCreateMenu(2000060000,-500050000,NULL,NULL,MS_CLIST_ADDCONTACTMENUITEM,gdiContactMenuItems,masMraSettings.hContactMenuIcons,(SIZEOF(gdiContactMenuItems) - ((masMraSettings.heNudgeReceived==NULL)? 0:1)),masMraSettings.hContactMenuItems);

	// xstatus menu
	if (ServiceExists(MS_CLIST_ADDSTATUSMENUITEM))
	{
		InitXStatusIcons();
		for(SIZE_T i=0;i<MRA_XSTATUS_COUNT;i++) 
		{
			mir_snprintf(pszServiceFunctionName,(SIZEOF(szServiceFunction)-PROTOCOL_NAME_LEN),"/menuXStatus%ld",i);
			CreateServiceFunctionParam(szServiceFunction,MraXStatusMenu,i);
		}

		masMraSettings.bHideXStatusUI=FALSE;
		masMraSettings.dwXStatusMode=DB_Mra_GetByte(NULL,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE);
		if (IsXStatusValid(masMraSettings.dwXStatusMode)==FALSE) masMraSettings.dwXStatusMode=MRA_MIR_XSTATUS_NONE;

		masMraSettings.hHookRebuildStatusMenu=HookEvent(ME_CLIST_PREBUILDSTATUSMENU,MraRebuildStatusMenu);
		MraRebuildStatusMenu(0,0);
	}

	MraExtraIconsRebuild(0,0);

	masMraSettings.hWATrack=HookEvent(ME_WAT_NEWSTATUS,MraMusicChanged);


	mir_snprintf(masMraSettings.szNewMailSound,SIZEOF(masMraSettings.szNewMailSound),"%s: %s",PROTOCOL_NAMEA,MRA_SOUND_NEW_EMAIL);
	SkinAddNewSoundEx(masMraSettings.szNewMailSound,PROTOCOL_NAMEA,MRA_SOUND_NEW_EMAIL);

	masMraSettings.bChatExist=MraChatRegister();


	DebugPrintCRLFW(L"MRA/LoadModules - DONE");
return(0);
}


void UnloadModules()
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;

	if (masMraSettings.bChatExist)
	{// destroy all chat sessions
		MraChatSessionDestroy(NULL);
	}

	// xstatus menu destroy
	if (ServiceExists(MS_CLIST_ADDSTATUSMENUITEM))
	{
		if (masMraSettings.hHookRebuildStatusMenu)	{UnhookEvent(masMraSettings.hHookRebuildStatusMenu);	masMraSettings.hHookRebuildStatusMenu=NULL;}

		memset(masMraSettings.hXStatusMenuItems, 0, sizeof(masMraSettings.hXStatusMenuItems));
		// Service deletion
		for(SIZE_T i=0;i<MRA_XSTATUS_COUNT;i++) 
		{
			mir_snprintf(pszServiceFunctionName,(SIZEOF(szServiceFunction)-PROTOCOL_NAME_LEN),"/menuXStatus%ld",i);
			DestroyServiceFunction(szServiceFunction);
		}
		DestroyXStatusIcons();
	}

	// Main menu destroy
	CListDestroyMenu(gdiMenuItems,SIZEOF(gdiMenuItems));
	memset(masMraSettings.hMainMenuItems, 0, sizeof(masMraSettings.hMainMenuItems));

	// Contact menu destroy
	CListDestroyMenu(gdiContactMenuItems,(SIZEOF(gdiContactMenuItems) - ((masMraSettings.heNudgeReceived==NULL)? 0:1)));
	memset(masMraSettings.hContactMenuItems, 0, sizeof(masMraSettings.hContactMenuItems));

	if (masMraSettings.heNudgeReceived)			{DestroyHookableEvent(masMraSettings.heNudgeReceived);	masMraSettings.heNudgeReceived=NULL;}
	if (masMraSettings.hWATrack)				{UnhookEvent(masMraSettings.hWATrack);					masMraSettings.hWATrack=NULL;}
	if (masMraSettings.hHookIconsChanged)		{UnhookEvent(masMraSettings.hHookIconsChanged);			masMraSettings.hHookIconsChanged=NULL;}
	if (masMraSettings.hHookExtraIconsRebuild)	{UnhookEvent(masMraSettings.hHookExtraIconsRebuild);	masMraSettings.hHookExtraIconsRebuild=NULL;}
	if (masMraSettings.hHookExtraIconsApply)	{UnhookEvent(masMraSettings.hHookExtraIconsApply);		masMraSettings.hHookExtraIconsApply=NULL;}
	if (masMraSettings.hHookRebuildCMenu)		{UnhookEvent(masMraSettings.hHookRebuildCMenu);			masMraSettings.hHookRebuildCMenu=NULL;}
	if (masMraSettings.hHookSettingChanged)		{UnhookEvent(masMraSettings.hHookSettingChanged);		masMraSettings.hHookSettingChanged=NULL;}
	if (masMraSettings.hHookContactDeleted)		{UnhookEvent(masMraSettings.hHookContactDeleted);		masMraSettings.hHookContactDeleted=NULL;}
	if (masMraSettings.hHookOptInitialize)		{UnhookEvent(masMraSettings.hHookOptInitialize);		masMraSettings.hHookOptInitialize=NULL;}

	IconsUnLoad();


	DebugPrintCRLFW(L"MRA/UnloadModules - DONE");
}


void UnloadServices()
{
	CHAR szServiceFunction[MAX_PATH],*pszServiceFunctionName;

	memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;


	// destroy plugin services
	for (SIZE_T i=0;i<SIZEOF(siPluginServices);i++)
	{
		memmove(pszServiceFunctionName,siPluginServices[i].lpszName,(lstrlenA(siPluginServices[i].lpszName)+1));
		DestroyServiceFunction(szServiceFunction);
	}

	DebugPrintCRLFW(L"MRA/UnloadServices - DONE");
}



void SetExtraIcons(HANDLE hContact)
{
	if(masMraSettings.hHookExtraIconsApply)
	{
		DWORD dwID,dwGroupID,dwContactSeverFlags;

		if (GetContactBasicInfoW(hContact,&dwID,&dwGroupID,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL)==NO_ERROR)
		{
			DWORD dwIconID=-1;
			DWORD dwXStatus=MRA_MIR_XSTATUS_NONE;

			if(masMraSettings.bLoggedIn)
			{
				dwXStatus=DB_Mra_GetByte(hContact,DBSETTING_XSTATUSID,MRA_MIR_XSTATUS_NONE);

				if(dwID==-1)
				{
					if(dwContactSeverFlags==-1)
					{
						dwIconID=ADV_ICON_DELETED;
					}else{
						dwIconID=ADV_ICON_NOT_ON_SERVER;
					}
				}else{
					if (dwGroupID==103)
					{//***deb
						dwIconID=ADV_ICON_PHONE;
					}else{
						if(dwContactSeverFlags)
						if(dwContactSeverFlags==-1)
						{
							dwIconID=ADV_ICON_DELETED;
						}else{
							dwIconID=ADV_ICON_NOT_AUTHORIZED;
						}
					}
				}
			}

			if (dwIconID==-1)
			{
				SIZE_T dwBlogStatusMsgSize=0;

				DB_Mra_GetStaticStringW(hContact,DBSETTING_BLOGSTATUS,NULL,0,&dwBlogStatusMsgSize);
				if (dwBlogStatusMsgSize) dwIconID=ADV_ICON_BLOGSTATUS;
			}

			ExtraSetIcon(masMraSettings.hExtraXstatusIcon,hContact,((IsXStatusValid(dwXStatus) || dwXStatus==MRA_MIR_XSTATUS_UNKNOWN)? masMraSettings.hXStatusAdvancedStatusItems[dwXStatus]:NULL),EXTRA_ICON_ADV1);
			ExtraSetIcon(masMraSettings.hExtraInfo,hContact,((dwIconID!=-1)? masMraSettings.hAdvancedStatusItems[dwIconID]:NULL),EXTRA_ICON_ADV2);
		}
	}
}



INT_PTR MraXStatusMenu(WPARAM wParam,LPARAM lParam,LPARAM param)
{
	if (MraRequestXStatusDetails(param)==FALSE) MraSetXStatusInternal(param);
return(0);
}


INT_PTR MraGotoInbox(WPARAM wParam,LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(masMraSettings.hMPopSessionQueue,MRA_WIN_INBOX_URL,sizeof(MRA_WIN_INBOX_URL));
return(0);
}

INT_PTR MraShowInboxStatus(WPARAM wParam,LPARAM lParam)
{
	MraUpdateEmailStatus(NULL,0,NULL,0,0,0);
return(0);
}

INT_PTR MraSendSMS(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.bLoggedIn && wParam && lParam)
	{
		LPSTR lpszMessageUTF;
		LPWSTR lpwszMessageXMLEncoded,lpwszMessageXMLDecoded;
		SIZE_T dwMessageUTFSize,dwBuffLen,dwMessageXMLEncodedSize,dwMessageXMLDecodedSize;

		lpszMessageUTF=(LPSTR)lParam;
		dwMessageUTFSize=lstrlenA(lpszMessageUTF);
		dwBuffLen=(dwMessageUTFSize+MAX_PATH);
		lpwszMessageXMLEncoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
		lpwszMessageXMLDecoded=(LPWSTR)MEMALLOC((dwBuffLen*sizeof(WCHAR)));
		if (lpwszMessageXMLEncoded && lpwszMessageXMLDecoded)
		{
			dwMessageXMLEncodedSize=MultiByteToWideChar(CP_UTF8,0,lpszMessageUTF,dwMessageUTFSize,lpwszMessageXMLEncoded,dwBuffLen);
			DecodeXML(lpwszMessageXMLEncoded,dwMessageXMLEncodedSize,lpwszMessageXMLDecoded,dwBuffLen,&dwMessageXMLDecodedSize);

			if (dwMessageXMLDecodedSize)
			{
				iRet=MraSendCommand_SMSW(NULL,(LPSTR)wParam,lstrlenA((LPSTR)wParam),lpwszMessageXMLDecoded,dwMessageXMLDecodedSize);
				/*{// имитируем получение смс
					char szBuff[16384];
					DWORD dwFlags;
					MRA_LPS lpsEMail,lpsText,lpsRTFText={0};

					dwFlags=(MESSAGE_FLAG_SMS|MESSAGE_FLAG_CP1251);
					lpsEMail.lpszData=(LPSTR)wParam;
					lpsEMail.dwSize=lstrlenA(lpsEMail.lpszData);
					lpsText.lpszData=szBuff;//LPS ## message ## текстовая версия сообщения
					lpsText.dwSize=WideCharToMultiByte(MRA_CODE_PAGE,0,lpwszMessageXMLDecoded,dwMessageXMLDecodedSize,szBuff,sizeof(szBuff),NULL,NULL);

					MraRecvCommand_Message((DWORD)_time32(NULL),dwFlags,&lpsEMail,&lpsText,&lpsRTFText,NULL);
				}//*/
			}else{// conversion failed?
				DebugBreak();
			}
		}
		MEMFREE(lpwszMessageXMLDecoded);
		MEMFREE(lpwszMessageXMLEncoded);
	}
return(iRet);
}

INT_PTR MraEditProfile(WPARAM wParam,LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(masMraSettings.hMPopSessionQueue,MRA_EDIT_PROFILE_URL,sizeof(MRA_EDIT_PROFILE_URL));
return(0);
}

INT_PTR MyAlbum(WPARAM wParam,LPARAM lParam)
{
return(MraViewAlbum(0,0));
}

INT_PTR MyBlog(WPARAM wParam,LPARAM lParam)
{
return(MraReadBlog(0,0));
}

INT_PTR MyBlogStatus(WPARAM wParam,LPARAM lParam)
{
return(MraReplyBlogStatus(0,0));
}

INT_PTR MyVideo(WPARAM wParam,LPARAM lParam)
{
return(MraViewVideo(0,0));
}

INT_PTR MyAnswers(WPARAM wParam,LPARAM lParam)
{
return(MraAnswers(0,0));
}

INT_PTR MyWorld(WPARAM wParam,LPARAM lParam)
{
return(MraWorld(0,0));
}

INT_PTR MraZhuki(WPARAM wParam,LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(masMraSettings.hMPopSessionQueue,MRA_ZHUKI_URL,sizeof(MRA_ZHUKI_URL));
return(0);
}

INT_PTR MraChat(WPARAM wParam,LPARAM lParam)
{
	MraMPopSessionQueueAddUrl(masMraSettings.hMPopSessionQueue,MRA_CHAT_URL,sizeof(MRA_CHAT_URL));
return(0);
}

INT_PTR MraWebSearch(WPARAM wParam,LPARAM lParam)
{
	CallService(MS_UTILS_OPENURL,TRUE,(LPARAM)MRA_SEARCH_URL);
return(0);
}

INT_PTR MraUpdateAllUsersInfo(WPARAM wParam,LPARAM lParam)
{
	if (MessageBox(NULL,TranslateW(L"Are you sure?"),TranslateW(MRA_UPD_ALL_USERS_INFO_STR),(MB_YESNO|MB_ICONQUESTION))==IDYES)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;
		HANDLE hContact;

		for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact))
			if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraSendCommand_WPRequestByEMail(hContact,ACKTYPE_GETINFO,szEMail,dwEMailSize);
			}
		}
	}
return(0);
}

INT_PTR MraCheckUpdatesUsersAvt(WPARAM wParam,LPARAM lParam)
{
	if (MessageBox(NULL,TranslateW(L"Are you sure?"),TranslateW(MRA_CHK_UPDATES_USERS_AVATARS_STR),(MB_YESNO|MB_ICONQUESTION))==IDYES)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;
		HANDLE hContact;

		for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (IsContactMra(hContact))
			if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			if (IsEMailChatAgent(szEMail,dwEMailSize)==FALSE)// только для оптимизации, MraAvatarsQueueGetAvatarSimple сама умеет фильтровать чатконтакты
			{
				MraAvatarsQueueGetAvatarSimple(masMraSettings.hAvatarsQueueHandle,0/*GAIF_FORCE*/,hContact,0);
			}
		}
	}
return(0);
}

INT_PTR MraRequestAuthForAll(WPARAM wParam,LPARAM lParam)
{
	if (MessageBox(NULL,TranslateW(L"Are you sure?"),TranslateW(MRA_REQ_AUTH_FOR_ALL_STR),(MB_YESNO|MB_ICONQUESTION))==IDYES)
	{
		DWORD dwContactSeverFlags;
		HANDLE hContact;

		for(hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
		{
			if (GetContactBasicInfoW(hContact,NULL,NULL,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL)==NO_ERROR)
			if (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED && dwContactSeverFlags!=-1)
			{
				MraRequestAuthorization((WPARAM)hContact,0);
			}

			/*if (IsContactMra(hContact))// inviz check
			//if ((MraContactCapabilitiesGet(hContact)&MRACCF_INVIS_ALLOWED)==0)
			if (MraGetContactStatus(hContact)==ID_STATUS_OFFLINE || MraGetContactStatus(hContact)==ID_STATUS_INVISIBLE)
			{
				CHAR szEMail[MAX_EMAIL_LEN];
				SIZE_T dwEMailSize;

				if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
				{
					MraSetContactStatus(hContact,ID_STATUS_OFFLINE);
					MraSendCommand_Game(szEMail,dwEMailSize,111,GAME_CONNECTION_INVITE,222,NULL,0);
				}
			}*/
		}
	}
return(0);
}




INT_PTR MraRequestAuthorization(WPARAM wParam,LPARAM lParam)
{//**deb add dialog?
	INT_PTR iRet=0;

	if (wParam)
	{
		WCHAR wszAuthMessage[MAX_PATH];
		CCSDATA cs={0};

		if (DB_Mra_GetStaticStringW(NULL,"AuthMessage",wszAuthMessage,SIZEOF(wszAuthMessage),NULL)==FALSE)
		{// def auth message
			lstrcpynW(wszAuthMessage,TranslateW(MRA_DEFAULT_AUTH_MESSAGE),SIZEOF(wszAuthMessage));
		}

		cs.hContact=(HANDLE)wParam;
		cs.szProtoService=PSS_AUTHREQUESTW;
		cs.wParam=PREF_UNICODE;
		cs.lParam=(LPARAM)wszAuthMessage;
		iRet=MraSendAuthRequest(0,(LPARAM)&cs);
	}
return(iRet);
}

INT_PTR MraGrantAuthorization(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn && wParam)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;

		if (DB_Mra_GetStaticStringA((HANDLE)wParam,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{// send without reason, do we need any ?
			MraSendCommand_Authorize(szEMail,dwEMailSize);

			//MraChatSessionJoinUser((HANDLE)wParam,"123",3,(DWORD)_time32(NULL));

			/*//if ((MraContactCapabilitiesGet((HANDLE)wParam)&MRACCF_INVIS_ALLOWED)==0)
			if (MraGetContactStatus((HANDLE)wParam)==ID_STATUS_OFFLINE || MraGetContactStatus((HANDLE)wParam)==ID_STATUS_INVISIBLE)
			{// inviz check
				MraSetContactStatus((HANDLE)wParam,ID_STATUS_OFFLINE);
				MraSendCommand_Game(szEMail,dwEMailSize,111,GAME_CONNECTION_INVITE,222,NULL,0);
			}*/

			/*MRA_GUID mguidSessionID={0};
			for(DWORD i=0;i<8;i++)
			{
				MraSendCommand_Proxy(szEMail,dwEMailSize,211+i,MRIM_PROXY_TYPE_FILES,"1.txt;111;",10,"172.0.0.1:111;",15,mguidSessionID);
				MraSendCommand_ProxyAck(j,szEMail,dwEMailSize,111+i,MRIM_PROXY_TYPE_FILES,"1.txt;111;",10,"127.0.0.1:111;",15,mguidSessionID);
			}//*/
		}
	}
return(0);
}

INT_PTR MraSendPostcard(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		DWORD dwContactEMailCount=GetContactEMailCount((HANDLE)wParam,FALSE);
		
		if (dwContactEMailCount)
		if (dwContactEMailCount==1)
		{
			SIZE_T dwUrlSize,dwEMailSize;
			CHAR szUrl[BUFF_SIZE_URL],szEMail[MAX_EMAIL_LEN];

			if (GetContactFirstEMail((HANDLE)wParam,FALSE,szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				BuffToLowerCase(szEMail,szEMail,dwEMailSize);
				dwUrlSize=mir_snprintf(szUrl,SIZEOF(szUrl),"http://cards.mail.ru/event.html?rcptname=%s&rcptemail=%s",GetContactNameA((HANDLE)wParam),szEMail);
				MraMPopSessionQueueAddUrl(masMraSettings.hMPopSessionQueue,szUrl,dwUrlSize);
			}
		}else{// show dialog box
			MraSelectEMailDlgShow((HANDLE)wParam,MRA_SELECT_EMAIL_TYPE_SEND_POSTCARD);
		}
	}
return(0);
}

INT_PTR MraViewAlbum(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		DWORD dwContactEMailMRCount=GetContactEMailCount((HANDLE)wParam,TRUE);
		
		if (dwContactEMailMRCount)
		if (dwContactEMailMRCount==1)
		{
			SIZE_T dwEMailSize;
			CHAR szEMail[MAX_EMAIL_LEN];

			if (GetContactFirstEMail((HANDLE)wParam,TRUE,szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_FOTO_URL,sizeof(MRA_FOTO_URL),szEMail,dwEMailSize);
			}
		}else{// show dialog box
			MraSelectEMailDlgShow((HANDLE)wParam,MRA_SELECT_EMAIL_TYPE_VIEW_ALBUM);
		}
	}
return(0);
}

INT_PTR MraReadBlog(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		DWORD dwContactEMailMRCount=GetContactEMailCount((HANDLE)wParam,TRUE);
		
		if (dwContactEMailMRCount)
		if (dwContactEMailMRCount==1)
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;

			if (GetContactFirstEMail((HANDLE)wParam,TRUE,szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_BLOGS_URL,sizeof(MRA_BLOGS_URL),szEMail,dwEMailSize);
			}
		}else{// show dialog box
			MraSelectEMailDlgShow((HANDLE)wParam,MRA_SELECT_EMAIL_TYPE_READ_BLOG);
		}
	}
return(0);
}

INT_PTR MraReplyBlogStatus(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		SIZE_T dwBlogStatusMsgSize=0;
		
		DB_Mra_GetStaticStringW((HANDLE)wParam,DBSETTING_BLOGSTATUS,NULL,0,&dwBlogStatusMsgSize);
		if (dwBlogStatusMsgSize || wParam==0)
		{
			MraSendReplyBlogStatus((HANDLE)wParam);
		}
	}
return(0);
}

INT_PTR MraViewVideo(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		DWORD dwContactEMailMRCount=GetContactEMailCount((HANDLE)wParam,TRUE);
		
		if (dwContactEMailMRCount)
		if (dwContactEMailMRCount==1)
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;

			if (GetContactFirstEMail((HANDLE)wParam,TRUE,szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_VIDEO_URL,sizeof(MRA_VIDEO_URL),szEMail,dwEMailSize);
			}
		}else{// show dialog box
			MraSelectEMailDlgShow((HANDLE)wParam,MRA_SELECT_EMAIL_TYPE_VIEW_VIDEO);
		}
	}
return(0);
}

INT_PTR MraAnswers(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		DWORD dwContactEMailMRCount=GetContactEMailCount((HANDLE)wParam,TRUE);
		
		if (dwContactEMailMRCount)
		if (dwContactEMailMRCount==1)
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;

			if (GetContactFirstEMail((HANDLE)wParam,TRUE,szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_ANSWERS_URL,sizeof(MRA_ANSWERS_URL),szEMail,dwEMailSize);
			}
		}else{// show dialog box
			MraSelectEMailDlgShow((HANDLE)wParam,MRA_SELECT_EMAIL_TYPE_ANSWERS);
		}
	}
return(0);
}

INT_PTR MraWorld(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn)
	{
		DWORD dwContactEMailMRCount=GetContactEMailCount((HANDLE)wParam,TRUE);
		
		if (dwContactEMailMRCount)
		if (dwContactEMailMRCount==1)
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			SIZE_T dwEMailSize;

			if (GetContactFirstEMail((HANDLE)wParam,TRUE,szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				MraMPopSessionQueueAddUrlAndEMail(masMraSettings.hMPopSessionQueue,MRA_WORLD_URL,sizeof(MRA_WORLD_URL),szEMail,dwEMailSize);
			}
		}else{// show dialog box
			MraSelectEMailDlgShow((HANDLE)wParam,MRA_SELECT_EMAIL_TYPE_WORLD);
		}
	}
return(0);
}







int MraContactDeleted(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn && wParam)
	{
		HANDLE hContact=(HANDLE)wParam;
		if (IsContactMra(hContact))
		{
			CHAR szEMail[MAX_EMAIL_LEN];
			DWORD dwID,dwGroupID;
			SIZE_T dwEMailSize;

			GetContactBasicInfoW(hContact,&dwID,&dwGroupID,NULL,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,NULL,0,NULL,NULL,0,NULL);
			
			MraSetContactStatus(hContact,ID_STATUS_OFFLINE);
			if (DBGetContactSettingByte(hContact,"CList","NotOnList",0)==0 || dwID!=-1) MraSendCommand_ModifyContactW(hContact,dwID,CONTACT_FLAG_REMOVED,dwGroupID,szEMail,dwEMailSize,NULL,0,NULL,0);
			MraAvatarsDeleteContactAvatarFile(masMraSettings.hAvatarsQueueHandle,hContact);
		}
	}
return(0);
}


int MraDbSettingChanged(WPARAM wParam,LPARAM lParam)
{
	if (masMraSettings.bLoggedIn && lParam)
	{
		HANDLE hContact=(HANDLE)wParam;
		DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;

		if (hContact)
		{
			if (IsContactMra(hContact) && DBGetContactSettingByte(hContact,"CList","NotOnList",0)==0 && DB_Mra_GetDword(hContact,"HooksLocked",FALSE)==FALSE)
			{// это наш контакт, он не временный (есть в списке на сервере) и его обновление разрешено
				CHAR szEMail[MAX_EMAIL_LEN],szPhones[MAX_EMAIL_LEN];
				WCHAR wszNick[MAX_EMAIL_LEN];
				DWORD dwID,dwGroupID,dwContactFlag;
				SIZE_T dwEMailSize,dwNickSize,dwPhonesSize;

				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szModule,-1,"CList",5)==CSTR_EQUAL)
				{// CList section
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"MyHandle",8)==CSTR_EQUAL)
					{// MyHandle setting
						LPWSTR lpwszNewNick;

						if (cws->value.type==DBVT_DELETED)
						{// allways store custom nick
							lstrcpynW(wszNick,GetContactNameW(hContact),SIZEOF(wszNick));
							lpwszNewNick=wszNick;
							dwNickSize=lstrlenW(lpwszNewNick);
							DB_SetStringExW(hContact,"CList","MyHandle",lpwszNewNick,dwNickSize);
						}else{
							if (cws->value.pszVal)
							{
								switch(cws->value.type){
								case DBVT_WCHAR:
									lpwszNewNick=cws->value.pwszVal;
									dwNickSize=lstrlenW(lpwszNewNick);
									break;
								case DBVT_UTF8:
									lpwszNewNick=wszNick;
									dwNickSize=MultiByteToWideChar(CP_UTF8,0,cws->value.pszVal,-1,wszNick,SIZEOF(wszNick));
									break;
								case DBVT_ASCIIZ:
									lpwszNewNick=wszNick;
									dwNickSize=MultiByteToWideChar(MRA_CODE_PAGE,0,cws->value.pszVal,-1,wszNick,SIZEOF(wszNick));
									break;
								default:
									lpwszNewNick=NULL;
									dwNickSize=0;
									break;
								}
								if (lpwszNewNick)
								if (GetContactBasicInfoW(hContact,&dwID,&dwGroupID,&dwContactFlag,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,NULL,0,NULL,szPhones,SIZEOF(szPhones),&dwPhonesSize)==NO_ERROR)
								{
									MraSendCommand_ModifyContactW(hContact,dwID,dwContactFlag,dwGroupID,szEMail,dwEMailSize,lpwszNewNick,dwNickSize,szPhones,dwPhonesSize);
								}
							}
						}
					}else
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"Group",5)==CSTR_EQUAL)
					{// Group setting //***deb
						if (TRUE)
						{// manage group on server
							switch(cws->value.type){
							case DBVT_ASCIIZ:

								break;
							case DBVT_DELETED:

								break;
							}
						}
					}else
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"NotOnList",9)==CSTR_EQUAL)
					{// NotOnList setting // Has a temporary contact just been added permanently?
						if (cws->value.type==DBVT_DELETED || (cws->value.type==DBVT_BYTE && cws->value.bVal==0))
						{
							WCHAR wszAuthMessage[MAX_PATH];
							SIZE_T dwAuthMessageSize;

							if (DB_Mra_GetStaticStringW(NULL,"AuthMessage",wszAuthMessage,SIZEOF(wszAuthMessage),&dwAuthMessageSize)==FALSE)
							{// def auth message
								lstrcpynW(wszAuthMessage,TranslateW(MRA_DEFAULT_AUTH_MESSAGE),SIZEOF(wszAuthMessage));
								dwAuthMessageSize=lstrlenW(wszAuthMessage);
							}

							DBDeleteContactSetting(hContact,"CList","Hidden");
							GetContactBasicInfoW(hContact,NULL,&dwGroupID,&dwContactFlag,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,wszNick,SIZEOF(wszNick),&dwNickSize,szPhones,SIZEOF(szPhones),&dwPhonesSize);
							MraSendCommand_AddContactW(hContact,dwContactFlag,dwGroupID,szEMail,dwEMailSize,wszNick,dwNickSize,szPhones,dwPhonesSize,wszAuthMessage,dwAuthMessageSize,0);
						}
					}else
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"Hidden",6)==CSTR_EQUAL)
					{// Hidden setting
						GetContactBasicInfoW(hContact,&dwID,&dwGroupID,&dwContactFlag,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,wszNick,SIZEOF(wszNick),&dwNickSize,szPhones,SIZEOF(szPhones),&dwPhonesSize);
						if(cws->value.type==DBVT_DELETED || (cws->value.type==DBVT_BYTE && cws->value.bVal==0))
						{
							dwContactFlag&=~CONTACT_FLAG_SHADOW;
						}else{
							dwContactFlag|=CONTACT_FLAG_SHADOW;
						}
						MraSendCommand_ModifyContactW(hContact,dwID,dwContactFlag,dwGroupID,szEMail,dwEMailSize,wszNick,dwNickSize,szPhones,dwPhonesSize);
					}
				}else
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szModule,-1,"Ignore",6)==CSTR_EQUAL)
				{// Ignore section
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"Mask1",5)==CSTR_EQUAL)
					{
						GetContactBasicInfoW(hContact,&dwID,&dwGroupID,&dwContactFlag,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,wszNick,SIZEOF(wszNick),&dwNickSize,szPhones,SIZEOF(szPhones),&dwPhonesSize);
						if(cws->value.type==DBVT_DELETED || (cws->value.type==DBVT_DWORD && cws->value.dVal&IGNOREEVENT_MESSAGE)==0)
						{
							dwContactFlag&=~CONTACT_FLAG_IGNORE;
						}else{
							dwContactFlag|=CONTACT_FLAG_IGNORE;
						}
						MraSendCommand_ModifyContactW(hContact,dwID,dwContactFlag,dwGroupID,szEMail,dwEMailSize,wszNick,dwNickSize,szPhones,dwPhonesSize);
					}
				}else
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szModule,-1,"UserInfo",8)==CSTR_EQUAL)
				{// User info section
					if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"MyPhone0",8)==CSTR_EQUAL
						|| CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"MyPhone1",8)==CSTR_EQUAL
						|| CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szSetting,-1,"MyPhone2",8)==CSTR_EQUAL)
					{
						//if(cws->value.type!=DBVT_DELETED)
						{
							GetContactBasicInfoW(hContact,&dwID,&dwGroupID,&dwContactFlag,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,wszNick,SIZEOF(wszNick),&dwNickSize,szPhones,SIZEOF(szPhones),&dwPhonesSize);
							MraSendCommand_ModifyContactW(hContact,dwID,dwContactFlag,dwGroupID,szEMail,dwEMailSize,wszNick,dwNickSize,szPhones,dwPhonesSize);
						}
					}
				}
			}
		}else{// not contact
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,cws->szModule,-1,"CListGroups",11)==CSTR_EQUAL)
			{// CList section //***deb
				if (TRUE)
				{// manage group on server
					switch(cws->value.type){
					case DBVT_ASCIIZ:

						break;
					case DBVT_DELETED:

						break;
					}
				}
			}
		}


		/*if ( hContact == NULL && MyOptions.ManageServer && !strcmp( cws->szModule, "CListGroups" )) {
			int iNumber = atol( cws->szSetting );
			LPCSTR szId = MSN_GetGroupByNumber( iNumber );
			if ( szId != NULL ) {
				if ( cws->value.type == DBVT_DELETED ) {
					msnNsThread->sendPacket( "RMG", szId );
				}
				else if ( cws->value.type == DBVT_ASCIIZ ) {
					LPCSTR oldId = MSN_GetGroupByName( cws->value.pszVal+1 );
					if ( oldId == NULL ) {
						CHAR* p = Utf8Encode( cws->value.pszVal+1 ), szNewName[ 200 ];
						UrlEncode( p, szNewName, sizeof szNewName );
						msnNsThread->sendPacket( "REG", "%s %s", szId, szNewName );
						free( p );
					}
					else MSN_SetGroupNumber( oldId, iNumber );
			}	}
			else if ( cws->value.type == DBVT_ASCIIZ )
				MSN_AddServerGroup( cws->value.pszVal+1 );

			return 0;
		}

		if ( !strcmp( cws->szModule, "CList" ) && MyOptions.ManageServer ) {
			CHAR* szProto = ( CHAR* )MSN_CallService( MS_PROTO_GETCONTACTBASEPROTO, ( WPARAM ) hContact, 0 );
			if ( szProto == NULL || strcmp( szProto, msnProtocolName ))
				return 0;

			if ( !strcmp( cws->szSetting, "Group" )) {
				if ( cws->value.type == DBVT_DELETED )
					MSN_MoveContactToGroup( hContact, NULL );
				else if ( cws->value.type == DBVT_ASCIIZ ) {
					LPCSTR p = MSN_GetGroupByName( cws->value.pszVal );
					if ( p == NULL )
						MSN_AddServerGroup( cws->value.pszVal );

					MSN_MoveContactToGroup( hContact, cws->value.pszVal );
				}
				return 0;
			}

	}	}*/

	}
return(0);
}


int MraRebuildContactMenu(WPARAM wParam,LPARAM lParam)
{
	BOOL bIsContactMRA,bHasEMail,bHasEMailMR,bChatAgent;
	DWORD dwContactSeverFlags=0;
	SIZE_T dwBlogStatusMsgSize=0;
	HANDLE hContact=(HANDLE)wParam;

	bIsContactMRA=IsContactMra(hContact);
	if (bIsContactMRA)//**deb телефонные контакты!
	{// proto own contact
		bHasEMail=TRUE;
		bHasEMailMR=TRUE;
		bChatAgent=IsContactChatAgent(hContact);
		GetContactBasicInfoW(hContact,NULL,NULL,NULL,&dwContactSeverFlags,NULL,NULL,0,NULL,NULL,0,NULL,NULL,0,NULL);
		DB_Mra_GetStaticStringW(hContact,DBSETTING_BLOGSTATUS,NULL,0,&dwBlogStatusMsgSize);
	}else{// non proto contact
		bHasEMail=FALSE;
		bHasEMailMR=FALSE;
		bChatAgent=FALSE;
		if (DB_Mra_GetByte(NULL,"HideMenuItemsForNonMRAContacts",MRA_DEFAULT_HIDE_MENU_ITEMS_FOR_NON_MRA)==FALSE)
		if (IsContactMraProto(hContact)==FALSE)// избегаем добавления менюшек в контакты других копий MRA
		if (GetContactEMailCount(hContact,FALSE))
		{
			bHasEMail=TRUE;
			if (GetContactEMailCount(hContact,TRUE)) bHasEMailMR=TRUE;
		}
	}

	//"Request authorization"
	CListShowMenuItem(masMraSettings.hContactMenuItems[0],(masMraSettings.bLoggedIn && bIsContactMRA));// && (dwContactSeverFlags&CONTACT_INTFLAG_NOT_AUTHORIZED)

	//"Grant authorization"
	CListShowMenuItem(masMraSettings.hContactMenuItems[1],(masMraSettings.bLoggedIn && bIsContactMRA && bChatAgent==FALSE));

	//"&Send postcard"
	CListShowMenuItem(masMraSettings.hContactMenuItems[2],(masMraSettings.bLoggedIn && bHasEMail && bChatAgent==FALSE));

	//"&View Album"
	CListShowMenuItem(masMraSettings.hContactMenuItems[3],(masMraSettings.bLoggedIn && bHasEMailMR && bChatAgent==FALSE));

	//"&Read Blog"
	CListShowMenuItem(masMraSettings.hContactMenuItems[4],(masMraSettings.bLoggedIn && bHasEMailMR && bChatAgent==FALSE));

	//"Reply Blog Status"
	CListShowMenuItem(masMraSettings.hContactMenuItems[5],(masMraSettings.bLoggedIn && dwBlogStatusMsgSize && bChatAgent==FALSE));
		
	//"View Video"
	CListShowMenuItem(masMraSettings.hContactMenuItems[6],(masMraSettings.bLoggedIn && bHasEMailMR && bChatAgent==FALSE));

	//"Answers"
	CListShowMenuItem(masMraSettings.hContactMenuItems[7],(masMraSettings.bLoggedIn && bHasEMailMR && bChatAgent==FALSE));

	//"World"
	CListShowMenuItem(masMraSettings.hContactMenuItems[8],(masMraSettings.bLoggedIn && bHasEMailMR && bChatAgent==FALSE));

	//"Send &Nudge"
	if (masMraSettings.heNudgeReceived==NULL)
	{
		CListShowMenuItem(masMraSettings.hContactMenuItems[9],(masMraSettings.bLoggedIn && bIsContactMRA));
	}

return(0);
}


int MraExtraIconsApply(WPARAM wParam,LPARAM lParam)
{
	SetExtraIcons((HANDLE)wParam);
return(0);
}


int MraExtraIconsRebuild(WPARAM wParam,LPARAM lParam)
{
	for(SIZE_T i=0;i<ADV_ICON_MAX;i++)			masMraSettings.hAdvancedStatusItems[i]=(HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON,(WPARAM)IconLibGetIcon(masMraSettings.hAdvancedStatusIcons[i]),0);
	for(SIZE_T i=0;i<(MRA_XSTATUS_COUNT+1);i++)	masMraSettings.hXStatusAdvancedStatusItems[i]=(HANDLE)CallService(MS_CLIST_EXTRA_ADD_ICON,(WPARAM)IconLibGetIcon(masMraSettings.hXStatusAdvancedStatusIcons[i]),0);

return(0);
}

int MraRebuildStatusMenu(WPARAM wParam,LPARAM lParam)
{
	CHAR szServiceFunction[MAX_PATH*2],*pszServiceFunctionName,szValueName[MAX_PATH];
	WCHAR szItem[MAX_PATH+64],szStatusTitle[STATUS_TITLE_MAX+4];
	HANDLE hXStatusRoot;
	CLISTMENUITEM mi={0};

	memmove(szServiceFunction,PROTOCOL_NAMEA,PROTOCOL_NAME_SIZE);
	pszServiceFunctionName=szServiceFunction+PROTOCOL_NAME_LEN;
	//memmove(pszServiceFunctionName,MRA_XSTATUS_MENU,sizeof(MRA_XSTATUS_MENU));

	mir_sntprintf(szItem,SIZEOF(szItem),L"%s Custom Status",PROTOCOL_NAMEW);
	mi.cbSize=sizeof(mi);
	mi.position=2000060000;
	mi.popupPosition=500085000;
	mi.ptszPopupName=szItem;
	//mi.ptszName=szStatusTitle;
	mi.flags=(CMIF_UNICODE|CMIF_ICONFROMICOLIB);
	mi.pszService=szServiceFunction;
	mi.pszContactOwner=PROTOCOL_NAMEA;

	for(SIZE_T i=0;i<MRA_XSTATUS_COUNT;i++) 
	{
		mir_snprintf(pszServiceFunctionName,(SIZEOF(szServiceFunction)-PROTOCOL_NAME_LEN),"/menuXStatus%ld",i);
		mi.position++;
		if (i)
		{
			mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldName",i);
			if (DB_Mra_GetStaticStringW(NULL,szValueName,szStatusTitle,(STATUS_TITLE_MAX+1),NULL))
			{
				szStatusTitle[STATUS_TITLE_MAX]=0;
				mi.ptszName=szStatusTitle;
			}else{// default xstatus name
				mi.ptszName=lpcszXStatusNameDef[i];
			}
			mi.icolibItem=masMraSettings.hXStatusAdvancedStatusIcons[i];
		}else{
			mi.ptszName=lpcszXStatusNameDef[i];
			mi.hIcon=NULL;
		}
		masMraSettings.hXStatusMenuItems[i]=(HANDLE)CallService(MS_CLIST_ADDSTATUSMENUITEM,(WPARAM)&hXStatusRoot,(LPARAM)&mi);
	}
return(0);
}


INT_PTR MraSetListeningTo(WPARAM wParam,LPARAM lParam)
{
	LISTENINGTOINFO *pliInfo=(LISTENINGTOINFO*)lParam;

	if ( pliInfo == NULL || pliInfo->cbSize != sizeof(LISTENINGTOINFO) )
	{
		MraSendCommand_ChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC,NULL,0,0);
		DB_Mra_DeleteValue(NULL,DBSETTING_BLOGSTATUSMUSIC);
	}else
	if (pliInfo->dwFlags & LTI_UNICODE)
	{
		LPWSTR pwszListeningTo;
		WCHAR wszListeningTo[MICBLOG_STATUS_MAX+4];
		SIZE_T dwListeningToSize;

		if (ServiceExists(MS_LISTENINGTO_GETPARSEDTEXT))
		{
			pwszListeningTo=(LPWSTR)CallService(MS_LISTENINGTO_GETPARSEDTEXT,(WPARAM)L"%track%. %title% - %artist% - %player%",(LPARAM)pliInfo);
			dwListeningToSize=lstrlenW(pwszListeningTo);
		}else{
			pwszListeningTo=wszListeningTo;
			dwListeningToSize=mir_sntprintf(pwszListeningTo,SIZEOF(wszListeningTo),L"%s. %s - %s - %s", pliInfo->ptszTrack?pliInfo->ptszTrack:L"", pliInfo->ptszTitle?pliInfo->ptszTitle:L"", pliInfo->ptszArtist?pliInfo->ptszArtist:L"", pliInfo->ptszPlayer?pliInfo->ptszPlayer:L"");
		}

		DB_Mra_SetStringExW(NULL,DBSETTING_BLOGSTATUSMUSIC,pwszListeningTo,dwListeningToSize);
		MraSendCommand_ChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC,pwszListeningTo,dwListeningToSize,0);

		if (pwszListeningTo!=wszListeningTo) mir_free(pwszListeningTo);
	}

return(0);
}


int MraMusicChanged(WPARAM wParam,LPARAM lParam)
{
	switch (wParam){
	case WAT_EVENT_PLAYERSTATUS:
		if (1==lParam)
		{// stopped
			DB_Mra_DeleteValue(NULL,DBSETTING_BLOGSTATUSMUSIC);
			MraSendCommand_ChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC,NULL,0,0);
		}
		break;
	case WAT_EVENT_NEWTRACK:
		{
			SONGINFO *psiSongInfo;

			if (WAT_RES_OK==CallService(MS_WAT_GETMUSICINFO,WAT_INF_UNICODE,(LPARAM)&psiSongInfo))
			{
				WCHAR wszMusic[MICBLOG_STATUS_MAX+4];
				SIZE_T dwMusicSize;

				dwMusicSize=mir_sntprintf(wszMusic,SIZEOF(wszMusic),L"%ld. %s - %s - %s",psiSongInfo->track,psiSongInfo->artist,psiSongInfo->title,psiSongInfo->player);
				DB_Mra_SetStringExW(NULL,DBSETTING_BLOGSTATUSMUSIC,wszMusic,dwMusicSize);
				MraSendCommand_ChangeUserBlogStatus(MRIM_BLOG_STATUS_MUSIC,wszMusic,dwMusicSize,0);
			}
		}
		break;
	default:
		break;
	}

return(0);
}


INT_PTR MraGetCaps(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet;

	switch(wParam){
	case PFLAGNUM_1:
		iRet=PF1_IM|PF1_FILE|PF1_MODEMSG|PF1_SERVERCLIST|PF1_AUTHREQ|PF1_ADDED|PF1_VISLIST|PF1_INVISLIST|PF1_INDIVSTATUS|PF1_PEER2PEER|/*PF1_NEWUSER|*/PF1_CHAT|PF1_BASICSEARCH|PF1_EXTSEARCH|PF1_CANRENAMEFILE|PF1_FILERESUME|PF1_ADDSEARCHRES|PF1_CONTACT|PF1_SEARCHBYEMAIL|PF1_USERIDISEMAIL|PF1_SEARCHBYNAME|PF1_EXTSEARCHUI;
		break;
	case PFLAGNUM_2:
		iRet=PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_HEAVYDND|PF2_FREECHAT;
		break;
	case PFLAGNUM_3:
		iRet=PF2_ONLINE|PF2_INVISIBLE|PF2_SHORTAWAY|PF2_HEAVYDND|PF2_FREECHAT;
		break;
	case PFLAGNUM_4:
		iRet=PF4_FORCEAUTH|PF4_FORCEADDED|/*PF4_NOCUSTOMAUTH|*/PF4_SUPPORTTYPING|PF4_AVATARS|PF4_IMSENDUTF;
		break;
	case PFLAG_UNIQUEIDTEXT:
		iRet=(INT_PTR)Translate("E-mail address");
		break;
	case PFLAG_MAXCONTACTSPERPACKET:
		iRet=MRA_MAXCONTACTSPERPACKET;
		break;
	case PFLAG_UNIQUEIDSETTING:
		iRet=(INT_PTR)"e-mail";
		break;
	case PFLAG_MAXLENOFMESSAGE:
		iRet=MRA_MAXLENOFMESSAGE;
		break;
	default:
		iRet=0;
		break;
	}
return(iRet);
}


INT_PTR MraGetName(WPARAM wParam,LPARAM lParam)
{
	lstrcpynA((LPSTR)lParam,PROTOCOL_DISPLAY_NAMEA,wParam);
return(0);
}


INT_PTR MraLoadIcon(WPARAM wParam,LPARAM lParam)
{
	UINT id;

	switch(wParam&0xFFFF){
	case PLI_PROTOCOL:id=IDI_MRA;break;// IDI_TM is the main icon for the protocol
	default:return(0);	
	}
return((INT_PTR)LoadImage(masMraSettings.hInstance,MAKEINTRESOURCE(id),IMAGE_ICON,GetSystemMetrics((wParam&PLIF_SMALL)?SM_CXSMICON:SM_CXICON),GetSystemMetrics((wParam&PLIF_SMALL)?SM_CYSMICON:SM_CYICON),0));
}


INT_PTR MraSetStatus(WPARAM wParam,LPARAM lParam)
{
	// remap global statuses to local supported
	switch(wParam){
	case ID_STATUS_OFFLINE:
		wParam=ID_STATUS_OFFLINE;
		break;
	case ID_STATUS_ONLINE:
		wParam=ID_STATUS_ONLINE;
		break;
	case ID_STATUS_AWAY:
		wParam=ID_STATUS_AWAY;
		break;
	case ID_STATUS_DND:
	case ID_STATUS_OCCUPIED:
		wParam=ID_STATUS_DND;
		break;
	case ID_STATUS_FREECHAT:
		wParam=ID_STATUS_FREECHAT;
		break;
	case ID_STATUS_INVISIBLE:
		wParam=ID_STATUS_INVISIBLE;
		break;
	case ID_STATUS_NA:
	case ID_STATUS_ONTHEPHONE:
	case ID_STATUS_OUTTOLUNCH:
		wParam=ID_STATUS_AWAY;
		break;
	default:
		wParam=ID_STATUS_OFFLINE;
		break;
	}


	if (InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwStatusMode,0)==wParam && wParam!=lParam)
	{// nothink to change
		ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)wParam,wParam,0);
	}else{
		DWORD dwOldStatusMode;

		if ((masMraSettings.dwDesiredStatusMode=wParam)==ID_STATUS_OFFLINE)
		{//set all contacts to offline
			masMraSettings.bLoggedIn=FALSE;
			dwOldStatusMode=InterlockedExchange((volatile LONG*)&masMraSettings.dwStatusMode,masMraSettings.dwDesiredStatusMode);

			if (dwOldStatusMode>ID_STATUS_OFFLINE)
			{// всех в offline, только если мы бывали подключены
				for(HANDLE hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);hContact!=NULL;hContact=(HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)hContact,0))
				{// функция сама проверяет принадлежность контакта к MRA
					SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,(SCBIF_ID|SCBIF_GROUP_ID|SCBIF_SERVER_FLAG|SCBIF_STATUS),-1,-1,0,0,ID_STATUS_OFFLINE,NULL,0,NULL,0,NULL,0);
				}
			}
			Netlib_CloseHandle(masMraSettings.hConnection);
		}else{
			// если offline то сразу ставим connecting, но обработка как offline
			dwOldStatusMode=InterlockedCompareExchange((volatile LONG*)&masMraSettings.dwStatusMode,ID_STATUS_CONNECTING,ID_STATUS_OFFLINE);

			switch(dwOldStatusMode){
			case ID_STATUS_OFFLINE:// offline, connecting
				if (StartConnect()!=NO_ERROR)
				{//err
					masMraSettings.bLoggedIn=FALSE;
					masMraSettings.dwDesiredStatusMode=ID_STATUS_OFFLINE;
					dwOldStatusMode=InterlockedExchange((volatile LONG*)&masMraSettings.dwStatusMode,masMraSettings.dwDesiredStatusMode);
				}
				break;
			case ID_STATUS_ONLINE:// connected, change status
			case ID_STATUS_AWAY:
			case ID_STATUS_DND:
			case ID_STATUS_FREECHAT:
			case ID_STATUS_INVISIBLE:
				MraSendNewStatus(masMraSettings.dwDesiredStatusMode,MraGetXStatusInternal(),NULL,0,NULL,0);
			case ID_STATUS_CONNECTING:
				if (dwOldStatusMode==ID_STATUS_CONNECTING && wParam!=lParam)
				{// предотвращаем переход в любой статус (кроме offline) из статуса connecting, если он не вызван самим плагином
					break;
				}
			default:
				dwOldStatusMode=InterlockedExchange((volatile LONG*)&masMraSettings.dwStatusMode,masMraSettings.dwDesiredStatusMode);
				//MraSendNewStatus(masMraSettings.dwDesiredStatusMode,MraGetXStatusInternal(),NULL,0,NULL,0);
				break;
			}
		}
		MraSetContactStatus(NULL,masMraSettings.dwStatusMode);
		ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)dwOldStatusMode,masMraSettings.dwStatusMode,0);
	}
return(0);
}


INT_PTR MraGetStatus(WPARAM wParam,LPARAM lParam)
{
return(InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwStatusMode,0));
}


DWORD MraSetXStatusInternal(DWORD dwXStatus)
{
	DWORD dwOldStatusMode;

	if (IsXStatusValid(dwXStatus))
	{
		CHAR szValueName[MAX_PATH];
		WCHAR szBuff[4096];
		SIZE_T dwBuffSize;

		// obsolete (TODO: remove in next version)
		mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldName",dwXStatus);
		if (DB_Mra_GetStaticStringW(NULL,szValueName,szBuff,SIZEOF(szBuff),&dwBuffSize)==FALSE)
		{// default xstatus name
			lstrcpynW(szBuff,lpcszXStatusNameDef[dwXStatus],SIZEOF(szBuff));
			dwBuffSize=lstrlenW(szBuff);
		}
		if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize=STATUS_TITLE_MAX;
		DB_Mra_SetStringExW(NULL,DBSETTING_XSTATUSNAME,szBuff,dwBuffSize);

		// obsolete (TODO: remove in next version)
		mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldMsg",dwXStatus);
		if (DB_Mra_GetStaticStringW(NULL,szValueName,szBuff,SIZEOF(szBuff),&dwBuffSize))
		{// custom xstatus description
			if (dwBuffSize>STATUS_DESC_MAX) dwBuffSize=STATUS_DESC_MAX;
			DB_Mra_SetStringExW(NULL,DBSETTING_XSTATUSMSG,szBuff,dwBuffSize);
		}else{// default xstatus description
			DB_Mra_DeleteValue(NULL,DBSETTING_XSTATUSMSG);
		}
	}else{
		DB_Mra_DeleteValue(NULL,DBSETTING_XSTATUSNAME);
		DB_Mra_DeleteValue(NULL,DBSETTING_XSTATUSMSG);
		dwXStatus=MRA_MIR_XSTATUS_NONE;
	}

	dwOldStatusMode=InterlockedExchange((volatile LONG*)&masMraSettings.dwXStatusMode,dwXStatus);
	DB_Mra_SetByte(NULL,DBSETTING_XSTATUSID,(BYTE)dwXStatus);

	MraSendNewStatus(MraGetStatus(0,0),dwXStatus,NULL,0,NULL,0);

return(dwOldStatusMode);
}

DWORD MraGetXStatusInternal()
{
return(InterlockedExchangeAdd((volatile LONG*)&masMraSettings.dwXStatusMode,0));
}



INT_PTR MraSetXStatus(WPARAM wParam,LPARAM lParam)
{// obsolete (TODO: remove in next version)
	INT_PTR iRet=0;

	if (IsXStatusValid(wParam) || wParam==MRA_MIR_XSTATUS_NONE)
	{
		MraSetXStatusInternal(wParam);
		iRet=wParam;
	}
return(iRet);
}

INT_PTR MraSetXStatusEx(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;
	DWORD dwXStatus;
	ICQ_CUSTOM_STATUS *pData=(ICQ_CUSTOM_STATUS*)lParam;

	if (pData->cbSize>=sizeof(ICQ_CUSTOM_STATUS))
	{
		iRet=0;

		if (pData->flags&CSSF_MASK_STATUS)
		{// fill status member
			dwXStatus=(*pData->status);
			if (IsXStatusValid(dwXStatus)==FALSE && dwXStatus!=MRA_MIR_XSTATUS_NONE) iRet=1;
		}else{
			dwXStatus=MraGetXStatusInternal();
		}

		if (pData->flags&(CSSF_MASK_NAME|CSSF_MASK_MESSAGE) && iRet==0)
		{//
			if (IsXStatusValid(dwXStatus) || dwXStatus==MRA_MIR_XSTATUS_NONE)
			{
				CHAR szValueName[MAX_PATH];
				SIZE_T dwBuffSize;

				if (pData->flags&CSSF_MASK_NAME)
				{// set custom status name
					mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldName",dwXStatus);
					if (pData->flags&CSSF_UNICODE)
					{
						dwBuffSize=lstrlenW(pData->pwszName);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize=STATUS_TITLE_MAX;

						DB_Mra_SetStringExW(NULL,szValueName,pData->pwszName,dwBuffSize);
						DB_Mra_SetStringExW(NULL,DBSETTING_XSTATUSNAME,pData->pwszName,dwBuffSize);
					}else{
						dwBuffSize=lstrlenA(pData->pszName);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize=STATUS_TITLE_MAX;

						DB_Mra_SetStringExA(NULL,szValueName,pData->pszName,dwBuffSize);
						DB_Mra_SetStringExA(NULL,DBSETTING_XSTATUSNAME,pData->pszName,dwBuffSize);
					}
				}

				if (pData->flags&CSSF_MASK_MESSAGE)
				{// set custom status message
					mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldMsg",dwXStatus);
					if (pData->flags&CSSF_UNICODE)
					{
						dwBuffSize=lstrlenW(pData->pwszMessage);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize=STATUS_DESC_MAX;

						DB_Mra_SetStringExW(NULL,szValueName,pData->pwszMessage,dwBuffSize);
						DB_Mra_SetStringExW(NULL,DBSETTING_XSTATUSMSG,pData->pwszMessage,dwBuffSize);
					}else{
						dwBuffSize=lstrlenA(pData->pszMessage);
						if (dwBuffSize>STATUS_TITLE_MAX) dwBuffSize=STATUS_DESC_MAX;

						DB_Mra_SetStringExA(NULL,szValueName,pData->pszMessage,dwBuffSize);
						DB_Mra_SetStringExA(NULL,DBSETTING_XSTATUSMSG,pData->pszMessage,dwBuffSize);
					}
				}
			}else{// неудача только если мы не ставили Хстатус и попытались записать сообщения для "нет" статуса
				if ((pData->flags&CSSF_MASK_STATUS)==0) iRet=1;
			}
		}

		if (pData->flags&(CSSF_MASK_STATUS|CSSF_MASK_NAME|CSSF_MASK_MESSAGE) && iRet==0)
		{// set/update xstatus code and/or message
			MraSetXStatusInternal(dwXStatus);
		}

		if (pData->flags&CSSF_DISABLE_UI)
		{// hide menu items
			masMraSettings.bHideXStatusUI=(*pData->wParam)? FALSE:TRUE;
			for (DWORD i=0;i<MRA_XSTATUS_COUNT;i++) CListShowMenuItem(masMraSettings.hXStatusMenuItems[i],!masMraSettings.bHideXStatusUI);
		}
	}
return(iRet);
}

INT_PTR MraGetXStatus(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet;

	if (masMraSettings.bLoggedIn)
	{
		if (wParam) *((CHAR**)wParam)=DBSETTING_XSTATUSNAME;
		if (lParam) *((CHAR**)lParam)=DBSETTING_XSTATUSMSG;
		iRet=MraGetXStatusInternal();
	}else{
		iRet=0;
	}

return(iRet);
}

INT_PTR MraGetXStatusEx(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;
	HANDLE hContact=(HANDLE)wParam;
	ICQ_CUSTOM_STATUS *pData=(ICQ_CUSTOM_STATUS*)lParam;

	if (pData->cbSize>=sizeof(ICQ_CUSTOM_STATUS))
	{
		DWORD dwXStatus;

		iRet=0;

		if (pData->flags&CSSF_MASK_STATUS)
		{// fill status member
			*pData->status=MraGetXStatusInternal();
		}

		if (pData->flags&CSSF_MASK_NAME)
		{// fill status name member
			if (pData->flags&CSSF_DEFAULT_NAME)
			{
				dwXStatus=(*pData->wParam);
				if (IsXStatusValid(dwXStatus))
				{
					if (pData->flags&CSSF_UNICODE)
					{
						lstrcpynW(pData->pwszName,lpcszXStatusNameDef[dwXStatus],(STATUS_TITLE_MAX+1));
					}else{
						SIZE_T dwStatusTitleSize;

						dwStatusTitleSize=lstrlenW(lpcszXStatusNameDef[dwXStatus]);
						if (dwStatusTitleSize>STATUS_TITLE_MAX) dwStatusTitleSize=STATUS_TITLE_MAX;

						WideCharToMultiByte(MRA_CODE_PAGE,0,lpcszXStatusNameDef[dwXStatus],dwStatusTitleSize,pData->pszName,MAX_PATH,NULL,NULL );
						(*((CHAR*)(pData->pszName+dwStatusTitleSize)))=0;
					}
				}else{// failure
					iRet=1;
				}
			}else{
				if (pData->flags&CSSF_UNICODE)
				{
					DB_Mra_GetStaticStringW(hContact,DBSETTING_XSTATUSNAME,pData->pwszName,(STATUS_TITLE_MAX+1),NULL);
				}else{
					DB_Mra_GetStaticStringA(hContact,DBSETTING_XSTATUSNAME,pData->pszName,(STATUS_TITLE_MAX+1),NULL);
				}
			}
		}

		if (pData->flags&CSSF_MASK_MESSAGE)
		{// fill status message member
			if (pData->flags&CSSF_UNICODE)
			{
				DB_Mra_GetStaticStringW(hContact,DBSETTING_XSTATUSMSG,pData->pwszMessage,(STATUS_DESC_MAX+1),NULL);
			}else{
				DB_Mra_GetStaticStringA(hContact,DBSETTING_XSTATUSMSG,pData->pszMessage,(STATUS_DESC_MAX+1),NULL);
			}
		}

		if (pData->flags&CSSF_DISABLE_UI)
		{
			if (pData->wParam) (*pData->wParam)=masMraSettings.bHideXStatusUI;
		}

		if (pData->flags&CSSF_STATUSES_COUNT)
		{
			if (pData->wParam) (*pData->wParam)=(MRA_XSTATUS_COUNT-1);
		}

		if (pData->flags&CSSF_STR_SIZES)
		{//**deb можно оптимизировать, данный параметр возможно уже был вычислен при получении самих текстов
			if (pData->wParam) DB_Mra_GetStaticStringW(hContact,DBSETTING_XSTATUSNAME,NULL,0,(SIZE_T*)pData->wParam);
			if (pData->lParam) DB_Mra_GetStaticStringW(hContact,DBSETTING_XSTATUSMSG,NULL,0,(SIZE_T*)pData->lParam);
		}
	}
return(iRet);
}

INT_PTR MraGetXStatusIcon(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet;

	if (wParam==0) wParam=MraGetXStatusInternal();
	iRet=(INT_PTR)IconLibGetIconEx(masMraSettings.hXStatusAdvancedStatusIcons[wParam],lParam);

return(iRet);
}


DWORD MraSendNewStatus(DWORD dwStatusMir,DWORD dwXStatusMir,LPWSTR lpwszStatusTitle,SIZE_T dwStatusTitleSize,LPWSTR lpwszStatusDesc,SIZE_T dwStatusDescSize)
{
	if (masMraSettings.bLoggedIn)
	{
		CHAR szValueName[MAX_PATH];
		WCHAR wszStatusTitle[STATUS_TITLE_MAX+4],wszStatusDesc[STATUS_DESC_MAX+4];
		DWORD dwStatus,dwXStatus;

		dwStatus=GetMraStatusFromMiradaStatus(dwStatusMir,dwXStatusMir,&dwXStatus);
		if (IsXStatusValid(dwXStatusMir))
		{// xstatuses
			if (lpwszStatusTitle==NULL || dwStatusTitleSize==0)
			{
				mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldName",dwXStatusMir);
				if (DB_Mra_GetStaticStringW(NULL,szValueName,wszStatusTitle,(STATUS_TITLE_MAX+1),&dwStatusTitleSize))
				{// custom xstatus name
					lpwszStatusTitle=wszStatusTitle;
				}else{// default xstatus name
					lpwszStatusTitle=TranslateW(lpcszXStatusNameDef[dwXStatusMir]);
					dwStatusTitleSize=lstrlenW(lpwszStatusTitle);
				}
			}

			if (lpwszStatusDesc==NULL || dwStatusDescSize==0)
			{
				mir_snprintf(szValueName,SIZEOF(szValueName),"XStatus%ldMsg",dwXStatusMir);
				if (DB_Mra_GetStaticStringW(NULL,szValueName,wszStatusDesc,(STATUS_DESC_MAX+1),&dwStatusDescSize))
				{// custom xstatus description
					lpwszStatusDesc=wszStatusDesc;
				}else{// default xstatus description
					lpwszStatusDesc=NULL;
					dwStatusDescSize=0;
				}
			}
		}else{// not xstatuses
			if (lpwszStatusTitle==NULL || dwStatusTitleSize==0)
			{
				lpwszStatusTitle=GetStatusModeDescriptionW(dwStatusMir);
				dwStatusTitleSize=lstrlenW(lpwszStatusTitle);
			}
		}

		MraSendCommand_ChangeStatusW(dwStatus,lpcszStatusUri[dwXStatus],lstrlenA(lpcszStatusUri[dwXStatus]),lpwszStatusTitle,dwStatusTitleSize,lpwszStatusDesc,dwStatusDescSize,((DB_Mra_GetByte(NULL,"RTFReceiveEnable",MRA_DEFAULT_RTF_RECEIVE_ENABLE)? FEATURE_FLAG_RTF_MESSAGE:0)|MRA_FEATURE_FLAGS));
	}

return(0);
}



INT_PTR MraSetAwayMsgA(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn)
	{
		WCHAR wszStatusDesc[STATUS_DESC_MAX+1];
		SIZE_T dwStatusDescSize;

		if (lParam)
		{
			dwStatusDescSize=MultiByteToWideChar(MRA_CODE_PAGE,0,(LPSTR)lParam,-1,wszStatusDesc,(SIZEOF(wszStatusDesc)-1));
		}else{
			dwStatusDescSize=0;
		}
		wszStatusDesc[dwStatusDescSize]=0;

		iRet=MraSetAwayMsg(wParam,(LPARAM)wszStatusDesc);
	}
return(iRet);
}

INT_PTR MraSetAwayMsg(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn)
	{
		LPWSTR lpwszStatusDesc;
		DWORD dwStatus,dwXStatus;
		SIZE_T dwStatusDescSize;

		lpwszStatusDesc=(LPWSTR)lParam;
		dwStatusDescSize=lstrlenW(lpwszStatusDesc);
		dwStatus=MraGetStatus(0,0);
		dwXStatus=MraGetXStatusInternal();

		if (dwStatus!=ID_STATUS_ONLINE || IsXStatusValid(dwXStatus)==FALSE)
		{// не отправляем новый статусный текст для хстатусов, для хстатусов только эвей сообщения
			dwStatusDescSize=min(dwStatusDescSize,STATUS_DESC_MAX);
			MraSendNewStatus(dwStatus,dwXStatus,NULL,0,lpwszStatusDesc,dwStatusDescSize);
		}
		iRet=0;
	}
return(iRet);
}


INT_PTR MraGetAwayMsg(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.bLoggedIn && lParam)
	{
		CHAR szStatusDesc[MICBLOG_STATUS_MAX+MICBLOG_STATUS_MAX+MAX_PATH],szBlogStatus[MICBLOG_STATUS_MAX+4],szTime[64];
		DWORD dwTime;
		SIZE_T dwStatusDescSize;
		CCSDATA* ccs=(CCSDATA*)lParam;
		SYSTEMTIME stBlogStatusTime={0};

		if (DB_Mra_GetStaticStringA(ccs->hContact,DBSETTING_BLOGSTATUS,szBlogStatus,SIZEOF(szBlogStatus),NULL))
		{
			dwTime=DB_Mra_GetDword(ccs->hContact,DBSETTING_BLOGSTATUSTIME,0);
			if (dwTime && MakeLocalSystemTimeFromTime32(dwTime,&stBlogStatusTime))
			{
				mir_snprintf(szTime,SIZEOF(szTime),"%04ld.%02ld.%02ld %02ld:%02ld: ",stBlogStatusTime.wYear,stBlogStatusTime.wMonth,stBlogStatusTime.wDay,stBlogStatusTime.wHour,stBlogStatusTime.wMinute);
			}else{
				szTime[0]=0;
			}
			
			dwStatusDescSize=mir_snprintf(szStatusDesc,SIZEOF(szStatusDesc),"%s%s",szTime,szBlogStatus);

			iRet=GetTickCount();
			ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_AWAYMSG,ACKRESULT_SUCCESS,(HANDLE)iRet,(LPARAM)szStatusDesc,dwStatusDescSize);
		}
	}
return(iRet);
}


INT_PTR MraAuthAllow(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn)
	{
		DBEVENTINFO dbei={0};

		dbei.cbSize=sizeof(dbei);
		if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,wParam,0))!=-1)
		{
			dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
			if (dbei.pBlob)
			{
				if (CallService(MS_DB_EVENT_GET,wParam,(LPARAM)&dbei)==0)
				if (dbei.eventType==EVENTTYPE_AUTHREQUEST)
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,dbei.szModule,-1,PROTOCOL_NAMEA,PROTOCOL_NAME_LEN)==CSTR_EQUAL)
				{
					LPSTR lpszNick,lpszFirstName,lpszLastName,lpszEMail,lpszRequestReason;
					
					lpszNick=(LPSTR)(dbei.pBlob+sizeof(DWORD)*2);
					lpszFirstName=lpszNick+lstrlenA(lpszNick)+1;
					lpszLastName=lpszFirstName+lstrlenA(lpszFirstName)+1;
					lpszEMail=lpszLastName+lstrlenA(lpszLastName)+1;
					lpszRequestReason=lpszEMail+lstrlenA(lpszEMail)+1;

					MraSendCommand_Authorize(lpszEMail,lstrlenA(lpszEMail));
					iRet=0;
				}
				MEMFREE(dbei.pBlob);
			}
		}
	}
return(iRet);
}


INT_PTR MraAuthDeny(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (lParam && masMraSettings.bLoggedIn)
	{
		DBEVENTINFO dbei={0};

		dbei.cbSize=sizeof(dbei);
		if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,wParam,0))!=-1)
		{
			dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
			if (dbei.pBlob)
			{
				if (CallService(MS_DB_EVENT_GET,wParam,(LPARAM)&dbei)==0)
				if (dbei.eventType==EVENTTYPE_AUTHREQUEST)
				if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,dbei.szModule,-1,PROTOCOL_NAMEA,PROTOCOL_NAME_LEN)==CSTR_EQUAL)
				{
					LPSTR lpszNick,lpszFirstName,lpszLastName,lpszEMail,lpszRequestReason;
					SIZE_T dwEMailSize,dwRequestReasonSize;
					//HANDLE hContact;
					
					lpszNick=(LPSTR)(dbei.pBlob+sizeof(DWORD)*2);
					lpszFirstName=lpszNick+lstrlenA(lpszNick)+1;
					lpszLastName=lpszFirstName+lstrlenA(lpszFirstName)+1;
					lpszEMail=lpszLastName+lstrlenA(lpszLastName)+1;
					dwEMailSize=lstrlenA(lpszEMail);
					lpszRequestReason=(LPSTR)lParam;
					dwRequestReasonSize=lstrlenA(lpszRequestReason);

					if (dwRequestReasonSize) MraSendCommand_MessageW(FALSE,NULL,0,0,lpszEMail,dwEMailSize,(LPWSTR)lpszRequestReason,dwRequestReasonSize,NULL,0);
					//hContact=MraHContactFromEmail(lpszEMail,dwEMailSize,FALSE,TRUE,NULL);
					//if (DBGetContactSettingByte(hContact,"CList","NotOnList",0)) CallService(MS_DB_CONTACT_DELETE,(WPARAM)hContact,0);

					iRet=0;
				}
				MEMFREE(dbei.pBlob);
			}
		}
	}
return(iRet);
}



HANDLE AddToListByEmail(MRA_LPS *plpsEMail,MRA_LPS *plpsNick,MRA_LPS *plpsFirstName,MRA_LPS *plpsLastName,DWORD dwFlags)
{
	HANDLE hContact=NULL;

	if (plpsEMail)
	if (plpsEMail->dwSize)
	{
		BOOL bAdded;

		hContact=MraHContactFromEmail(plpsEMail->lpszData,plpsEMail->dwSize,TRUE,TRUE,&bAdded);
		if (hContact)
		{
			if (plpsNick)
			if (plpsNick->dwSize)
			{
				DB_Mra_SetLPSStringA(hContact,"Nick",plpsNick);
				if (bAdded)
				{// впервые добавляется контакт в базу///***deb
					//SetContactBasicInfoW(hContact,SCBIFSI_LOCK_CHANGES_EVENTS,SCBIF_NICK,0,0,0,0,0,NULL,0,plpsNick->lpszData,plpsNick->dwSize,NULL,0);
				}
			}

			if (plpsFirstName)
			if (plpsFirstName->dwSize) DB_Mra_SetLPSStringA(hContact,"FirstName",plpsFirstName);

			if (plpsLastName)
			if (plpsLastName->dwSize) DB_Mra_SetLPSStringA(hContact,"LastName",plpsLastName);

			if (dwFlags&PALF_TEMPORARY)
			{
				DBWriteContactSettingByte(hContact,"CList","Hidden",1);
			}else{
				DBDeleteContactSetting(hContact,"CList","NotOnList");
			}

			if (bAdded) MraUpdateContactInfo(hContact);
		}
	}
return(hContact);
}


INT_PTR MraAddToList(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet;
	PROTOSEARCHRESULT *psr=(PROTOSEARCHRESULT*)lParam;

	if (psr->cbSize==sizeof(PROTOSEARCHRESULT))
	{
		MRA_LPS lpsEMail,lpsNick,lpsFirstName,lpsLastName;
		
		lpsEMail.dwSize=lstrlenA((LPSTR)psr->email);
		lpsEMail.lpwszData=psr->email;
		lpsNick.dwSize=lstrlenA((LPSTR)psr->nick);
		lpsNick.lpwszData=psr->nick;
		lpsFirstName.dwSize=lstrlenA((LPSTR)psr->firstName);
		lpsFirstName.lpwszData=psr->firstName;
		lpsLastName.dwSize=lstrlenA((LPSTR)psr->lastName);
		lpsLastName.lpwszData=psr->lastName;

		iRet=(INT_PTR)AddToListByEmail(&lpsEMail,&lpsNick,&lpsFirstName,&lpsLastName,wParam);
	}else{
		iRet=0;
	}
return(iRet);
}


INT_PTR MraAddToListByEvent(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;
	DBEVENTINFO dbei={0};

	dbei.cbSize=sizeof(dbei);
	if ((dbei.cbBlob=CallService(MS_DB_EVENT_GETBLOBSIZE,lParam,0))!=-1)
	{
		dbei.pBlob=(PBYTE)MEMALLOC(dbei.cbBlob);
		if (dbei.pBlob)
		{
			if (CallService(MS_DB_EVENT_GET,lParam,(LPARAM)&dbei)==0)
			if (CompareStringA(MAKELANGID(LANG_ENGLISH,SUBLANG_ENGLISH_US),NORM_IGNORECASE,dbei.szModule,-1,PROTOCOL_NAMEA,PROTOCOL_NAME_LEN)==CSTR_EQUAL)
			if (dbei.eventType==EVENTTYPE_AUTHREQUEST || dbei.eventType==EVENTTYPE_CONTACTS)
			{
				LPSTR lpszRequestReason;
				MRA_LPS lpsEMail,lpsNick,lpsFirstName,lpsLastName;

				lpsNick.lpszData=(LPSTR)(dbei.pBlob+ ((dbei.eventType==EVENTTYPE_AUTHREQUEST)?(sizeof(DWORD)*2):0) );
				lpsNick.dwSize=lstrlenA(lpsNick.lpszData);
				lpsFirstName.lpszData=lpsNick.lpszData+lpsNick.dwSize+1;
				lpsFirstName.dwSize=lstrlenA(lpsFirstName.lpszData);
				lpsLastName.lpszData=lpsFirstName.lpszData+lpsFirstName.dwSize+1;
				lpsLastName.dwSize=lstrlenA(lpsLastName.lpszData);
				lpsEMail.lpszData=lpsLastName.lpszData+lpsLastName.dwSize+1;
				lpsEMail.dwSize=lstrlenA(lpsEMail.lpszData);
				lpszRequestReason=lpsEMail.lpszData+lpsEMail.dwSize+1;

				iRet=(INT_PTR)AddToListByEmail(&lpsEMail,&lpsNick,&lpsFirstName,&lpsLastName,0);
			}else{
				dbei.eventType=dbei.eventType;
				DebugBreak();
			}
			MEMFREE(dbei.pBlob);
		}
	}
return(iRet);
}


/*
   ---------------------------------
   |           Receiving           |
   ---------------------------------
*/
INT_PTR MraRecvMessage(WPARAM wParam,LPARAM lParam)
{
	CallService(MS_PROTO_RECVMSG,0,(LPARAM)lParam);
return(0);
}


INT_PTR MraRecvContacts(WPARAM wParam,LPARAM lParam)
{
	CCSDATA* ccs=(CCSDATA*)lParam;
	DBEVENTINFO dbei={0};
	PROTORECVEVENT* pre=(PROTORECVEVENT*)ccs->lParam;

	dbei.cbSize=sizeof(dbei);
	dbei.szModule=PROTOCOL_NAMEA;
	dbei.timestamp=pre->timestamp;
	dbei.flags=(pre->flags&PREF_CREATEREAD)?DBEF_READ:0;
	dbei.eventType=EVENTTYPE_CONTACTS;
	dbei.cbBlob=pre->lParam;
	dbei.pBlob=(PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD,(WPARAM)ccs->hContact,(LPARAM)&dbei);
return(0);
}


INT_PTR MraRecvFile(WPARAM wParam,LPARAM lParam)
{
	CCSDATA* ccs=(CCSDATA*)lParam;
	PROTORECVFILET* pre=(PROTORECVFILET*)ccs->lParam;

	CallService(MS_PROTO_RECVFILET,0,(LPARAM)lParam);
return(0);
}


INT_PTR MraRecvAuth(WPARAM wParam,LPARAM lParam)
{
	CCSDATA* ccs=(CCSDATA*)lParam;
	DBEVENTINFO dbei={0};
	PROTORECVEVENT* pre=(PROTORECVEVENT*)ccs->lParam;

	dbei.cbSize=sizeof(dbei);
	dbei.szModule=PROTOCOL_NAMEA;
	dbei.timestamp=pre->timestamp;
	dbei.flags=(pre->flags&PREF_CREATEREAD)?DBEF_READ:0;
	dbei.eventType=EVENTTYPE_AUTHREQUEST;
	dbei.cbBlob=pre->lParam;
	dbei.pBlob=(PBYTE)pre->szMessage;

	CallService(MS_DB_EVENT_ADD,(WPARAM)NULL,(LPARAM)&dbei);
return(0);
}


/*
   ---------------------------------
   |           Sending             |
   ---------------------------------
*/
INT_PTR MraSendAuthRequest(WPARAM wParam,LPARAM lParam)
{// internal only
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn && lParam)
	{
		BOOL bSlowSend;
		CHAR szEMail[MAX_EMAIL_LEN];
		LPWSTR lpwszMessage;
		SIZE_T dwEMailSize,dwMessageSize;
		CCSDATA* ccs=(CCSDATA*)lParam;

		if (ccs->wParam&PREF_UNICODE)//flags
		{
			lpwszMessage=(LPWSTR)ccs->lParam;
			dwMessageSize=lstrlenW(lpwszMessage);
		}else{
			dwEMailSize=lstrlenA((LPSTR)ccs->lParam);
			lpwszMessage=(LPWSTR)MEMALLOC(((dwEMailSize+MAX_PATH)*sizeof(WCHAR)));
			if (lpwszMessage)
			{
				dwMessageSize=MultiByteToWideChar(MRA_CODE_PAGE,0,(LPSTR)ccs->lParam,dwEMailSize,lpwszMessage,(dwEMailSize+MAX_PATH));
			}
		}

		if (lpwszMessage)
		{
			if (DB_Mra_GetStaticStringA(ccs->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				bSlowSend=DB_Mra_GetByte(NULL,"SlowSend",MRA_DEFAULT_SLOW_SEND);
				iRet=MraSendCommand_MessageW(bSlowSend,ccs->hContact,ACKTYPE_AUTHREQ,MESSAGE_FLAG_AUTHORIZE,szEMail,dwEMailSize,lpwszMessage,dwMessageSize,NULL,0);
				if (bSlowSend==FALSE)
				{
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_AUTHREQ,ACKRESULT_SUCCESS,(HANDLE)iRet,(LPARAM)NULL,0);
				}
				iRet=0;
			}

			if (lpwszMessage!=(LPWSTR)ccs->lParam) MEMFREE(lpwszMessage);
		}
	}
return(iRet);
}


INT_PTR MraSendMessage(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;
	CCSDATA* ccs=(CCSDATA*)lParam;

	if (masMraSettings.bLoggedIn)
	{
		BOOL bSlowSend,bMemAllocated=FALSE;
		CHAR szEMail[MAX_EMAIL_LEN];
		DWORD dwFlags=0;
		LPSTR lpszMessage;
		LPWSTR lpwszMessage=NULL;
		SIZE_T dwEMailSize,dwMessageSize=0;

		lpszMessage=(LPSTR)ccs->lParam;
		dwMessageSize=lstrlenA(lpszMessage);
		if (ccs->wParam&PREF_UNICODE)//flags
		{
			lpwszMessage=(LPWSTR)(lpszMessage+dwMessageSize+1);
		}else
		if (ccs->wParam&PREF_UTF){// convert to unicode from utf8
			lpwszMessage=(LPWSTR)MEMALLOC(((dwMessageSize+MAX_PATH)*sizeof(WCHAR)));
			if (lpwszMessage)
			{
				dwMessageSize=MultiByteToWideChar(CP_UTF8,0,lpszMessage,dwMessageSize,lpwszMessage,(dwMessageSize+MAX_PATH));
				bMemAllocated=TRUE;
			}
		}else{// convert to unicode from ansi
			MraPopupShowFromAgentW(MRA_POPUP_TYPE_DEBUG,0,TranslateW(L"Send ANSI"));
			lpwszMessage=(LPWSTR)MEMALLOC(((dwMessageSize+MAX_PATH)*sizeof(WCHAR)));
			if (lpwszMessage)
			{
				dwMessageSize=MultiByteToWideChar(MRA_CODE_PAGE,0,lpszMessage,dwMessageSize,lpwszMessage,(dwMessageSize+MAX_PATH));
				bMemAllocated=TRUE;
			}
		}

		if (lpwszMessage)
		{
			if (DB_Mra_GetStaticStringA(ccs->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				bSlowSend=DB_Mra_GetByte(NULL,"SlowSend",MRA_DEFAULT_SLOW_SEND);

				if (DB_Mra_GetByte(NULL,"RTFSendEnable",MRA_DEFAULT_RTF_SEND_ENABLE) && (MraContactCapabilitiesGet(ccs->hContact)&FEATURE_FLAG_RTF_MESSAGE))
				{
					dwFlags|=MESSAGE_FLAG_RTF;
				}

				iRet=MraSendCommand_MessageW(bSlowSend,ccs->hContact,ACKTYPE_MESSAGE,dwFlags,szEMail,dwEMailSize,lpwszMessage,dwMessageSize,NULL,0);
				if (bSlowSend==FALSE)
				{
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_MESSAGE,ACKRESULT_SUCCESS,(HANDLE)iRet,(LPARAM)NULL,0);
				}
			}

			if (bMemAllocated) MEMFREE(lpwszMessage);
		}else{
			ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_MESSAGE,ACKRESULT_FAILED,NULL,(LPARAM)"Cant allocate buffer for convert to unicode.",-1);
		}
	}else{
		ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_MESSAGE,ACKRESULT_FAILED,NULL,(LPARAM)"You cannot send when you are offline.",-1);
	}
return(iRet);
}


INT_PTR MraSendContacts(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;
	CCSDATA* ccs=(CCSDATA*)lParam;

	if (masMraSettings.bLoggedIn && lParam)
	{
		BOOL bSlowSend;
		CHAR szEMail[MAX_EMAIL_LEN];
		LPWSTR lpwszData,lpwszDataCurrent,lpwszNick;
		SIZE_T i,dwContacts,dwDataBuffSize,dwEMailSize,dwStringSize,dwNickSize;
		HANDLE *hContactsList=(HANDLE*)ccs->lParam;

		dwContacts=HIWORD(ccs->wParam);
		dwDataBuffSize=(dwContacts*(MAX_EMAIL_LEN*2));
		lpwszData=(LPWSTR)MEMALLOC((dwDataBuffSize*sizeof(WCHAR)));
		if (lpwszData)
		{
			lpwszDataCurrent=lpwszData;
			if (DB_Mra_GetStaticStringA(ccs->hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
			{
				for(i=0;i<dwContacts;i++)
				{
					if (IsContactMra(hContactsList[i]))
					if (DB_Mra_GetStaticStringW(hContactsList[i],"e-mail",lpwszDataCurrent,(dwDataBuffSize-(lpwszDataCurrent-lpwszData)),&dwStringSize))
					{
						lpwszDataCurrent+=dwStringSize;
						(*lpwszDataCurrent)=';';
						lpwszDataCurrent++;

						lpwszNick=GetContactNameW(hContactsList[i]);
						dwNickSize=lstrlenW(lpwszNick);
						memmove(lpwszDataCurrent,lpwszNick,(dwNickSize*sizeof(WCHAR)));
						lpwszDataCurrent+=dwNickSize;
						(*lpwszDataCurrent)=';';
						lpwszDataCurrent++;
					}
				}

				bSlowSend=DB_Mra_GetByte(NULL,"SlowSend",MRA_DEFAULT_SLOW_SEND);
				iRet=MraSendCommand_MessageW(bSlowSend,ccs->hContact,ACKTYPE_CONTACTS,MESSAGE_FLAG_CONTACT,szEMail,dwEMailSize,lpwszData,(lpwszDataCurrent-lpwszData),NULL,0);
				if (bSlowSend==FALSE)
				{
					ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_CONTACTS,ACKRESULT_SUCCESS,(HANDLE)iRet,(LPARAM)NULL,0);
				}
			}
			MEMFREE(lpwszData);
		}
	}else{
		ProtoBroadcastAckAsynchEx(PROTOCOL_NAMEA,ccs->hContact,ACKTYPE_CONTACTS,ACKRESULT_FAILED,NULL,(LPARAM)"You cannot send when you are offline.",-1);
	}
return(iRet);
}


INT_PTR MraSendUserIsTyping(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn && wParam && lParam!=PROTOTYPE_SELFTYPING_OFF)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		SIZE_T dwEMailSize;
		HANDLE hContact=(HANDLE)wParam;

		if (MraGetContactStatus(hContact)!=ID_STATUS_OFFLINE && MraGetStatus(0,0)!=ID_STATUS_INVISIBLE)
		if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			if (MraSendCommand_MessageW(FALSE,hContact,0,MESSAGE_FLAG_NOTIFY,szEMail,dwEMailSize,L" ",1,NULL,0)) iRet=0;
		}
	}
return(iRet);
}


INT_PTR MraSendNudge(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn && wParam)
	{
		CHAR szEMail[MAX_EMAIL_LEN];
		LPWSTR lpwszAlarmMessage=TranslateW(MRA_ALARM_MESSAGE);
		SIZE_T dwEMailSize;
		HANDLE hContact=(HANDLE)wParam;

		if (DB_Mra_GetStaticStringA(hContact,"e-mail",szEMail,SIZEOF(szEMail),&dwEMailSize))
		{
			if (MraSendCommand_MessageW(FALSE,hContact,0,(MESSAGE_FLAG_RTF|MESSAGE_FLAG_ALARM),szEMail,dwEMailSize,lpwszAlarmMessage,lstrlenW(lpwszAlarmMessage),NULL,0)) iRet=0;
		}
	}
return(iRet);
}


INT_PTR MraSetApparentMode(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (masMraSettings.bLoggedIn && lParam)
	{
		CCSDATA *ccs=(CCSDATA*)lParam;

		if (ccs->hContact && ccs->wParam==0 || ccs->wParam==ID_STATUS_ONLINE || ccs->wParam==ID_STATUS_OFFLINE)
		{// Only 3 modes are supported
			DWORD dwOldMode=DB_Mra_GetWord(ccs->hContact,"ApparentMode",0);

			// Dont send redundant updates
			if (ccs->wParam!=dwOldMode)
			{
				CHAR szEMail[MAX_EMAIL_LEN],szPhones[MAX_EMAIL_LEN];
				WCHAR wszNick[MAX_EMAIL_LEN];
				DWORD dwID,dwGroupID,dwContactFlag=0;
				SIZE_T dwEMailSize,dwNickSize,dwPhonesSize;

				GetContactBasicInfoW(ccs->hContact,&dwID,&dwGroupID,&dwContactFlag,NULL,NULL,szEMail,SIZEOF(szEMail),&dwEMailSize,wszNick,SIZEOF(wszNick),&dwNickSize,szPhones,SIZEOF(szPhones),&dwPhonesSize);

				dwContactFlag&=~(CONTACT_FLAG_INVISIBLE|CONTACT_FLAG_VISIBLE);
				switch(ccs->wParam){
				case ID_STATUS_OFFLINE:
					dwContactFlag|=CONTACT_FLAG_INVISIBLE;
					break;
				case ID_STATUS_ONLINE:
					dwContactFlag|=CONTACT_FLAG_VISIBLE;
					break;
				}

				if (MraSendCommand_ModifyContactW(ccs->hContact,dwID,dwContactFlag,dwGroupID,szEMail,dwEMailSize,wszNick,dwNickSize,szPhones,dwPhonesSize))
				{
					SetContactBasicInfoW(ccs->hContact,0,SCBIF_FLAG,0,0,dwContactFlag,0,0,NULL,0,NULL,0,NULL,0);
					iRet=0;// Success
				}
			}
		}
	}
return(iRet);
}


INT_PTR MraGetInfo(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (MraUpdateContactInfo(((CCSDATA*)lParam)->hContact))
	{
		iRet=0;
	}
return(iRet);
}


INT_PTR MraGetAvatarCaps(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	switch(wParam){
	case AF_MAXSIZE:
		iRet=-1;
		break;
	case AF_PROPORTION:
		iRet=PIP_NONE;
		break;
	case AF_FORMATSUPPORTED:
		iRet=0;// no formats to set
		break;
	case AF_ENABLED:
		iRet=1;// allways on
		break;
	case AF_DONTNEEDDELAYS:
		iRet=0;// need delay
		break;
	case AF_MAXFILESIZE:
		iRet=0;// 
		break;
	case AF_DELAYAFTERFAIL:
		iRet=5000;
		break;
	}

return(iRet);
}


INT_PTR MraGetAvatarInfo(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=GAIR_NOAVATAR;

	if (lParam)
	{//(DWORD)wParam&GAIF_FORCE
		PROTO_AVATAR_INFORMATION *ppai=(PROTO_AVATAR_INFORMATION*)lParam;
		iRet=(INT_PTR)MraAvatarsQueueGetAvatar(masMraSettings.hAvatarsQueueHandle,(DWORD)wParam,ppai->hContact,NULL,(DWORD*)&ppai->format,ppai->filename);
	}
return(iRet);
}


INT_PTR MraGetMyAvatar(WPARAM wParam,LPARAM lParam)
{// need call MS_AV_REPORTMYAVATARCHANGED hook
	INT_PTR iRet=1;

	if (MraAvatarsGetFileName(masMraSettings.hAvatarsQueueHandle,NULL,GetContactAvatarFormat(NULL,PA_FORMAT_DEFAULT),(LPSTR)wParam,(SIZE_T)lParam,NULL)==NO_ERROR)
	{
		LPSTR lpsz=(LPSTR)wParam;
		iRet=0;
	}
return(iRet);
}


INT_PTR MraBasicSearch(WPARAM wParam,LPARAM lParam)
{
return(MraSearchByEmail(wParam,lParam));
}

INT_PTR MraSearchByEmail(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.bLoggedIn && lParam)
	{
		CHAR szEMail[MAX_EMAIL_LEN]={0};
		SIZE_T dwEMailSize;

		dwEMailSize=(WideCharToMultiByte(MRA_CODE_PAGE,0,(LPWSTR)lParam,-1,szEMail,SIZEOF(szEMail),NULL,NULL)-1);
		iRet=MraSendCommand_WPRequestByEMail(NULL,ACKTYPE_SEARCH,szEMail,dwEMailSize);
	}
return(iRet);
}

INT_PTR MraSearchByName(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.bLoggedIn && lParam)
	{
		PROTOSEARCHBYNAME *psbn=(PROTOSEARCHBYNAME*)lParam;

		if (psbn->pszNick || psbn->pszFirstName || psbn->pszLastName)
		{
			DWORD dwRequestFlags=0;
			SIZE_T dwNickSize=0,dwFirstNameSize=0,dwLastNameSize=0;

			if (psbn->pszNick)		dwNickSize=lstrlenW(psbn->pszNick);
			if (psbn->pszFirstName)	dwFirstNameSize=lstrlenW(psbn->pszFirstName);
			if (psbn->pszLastName)	dwLastNameSize=lstrlenW(psbn->pszLastName);

			if (dwNickSize)			SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_NICKNAME);
			if (dwFirstNameSize)	SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_FIRSTNAME);
			if (dwLastNameSize)		SetBit(dwRequestFlags,MRIM_CS_WP_REQUEST_PARAM_LASTNAME);

			iRet=MraSendCommand_WPRequestW(NULL,ACKTYPE_SEARCH,dwRequestFlags,NULL,0,NULL,0,psbn->pszNick,dwNickSize,psbn->pszFirstName,dwFirstNameSize,psbn->pszLastName,dwLastNameSize,0,0,0,0,0,0,0,0,0);
		}
	}
return(iRet);
}

INT_PTR MraCreateAdvSearchUI(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.hInstance && lParam)
	{
		iRet=(INT_PTR)CreateDialogParam(masMraSettings.hInstance,MAKEINTRESOURCE(IDD_MRAADVANCEDSEARCH),(HWND)lParam,AdvancedSearchDlgProc,0);
	}
return(iRet);
}

INT_PTR MraSearchByAdvanced(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.bLoggedIn && lParam)
	{
		iRet=(INT_PTR)AdvancedSearchFromDlg((HWND)lParam);
	}
return(iRet);
}





INT_PTR MraFileResume(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (wParam && lParam)
	{
		PROTOFILERESUME *pfr=(PROTOFILERESUME*)lParam;
		//icq_sendFileResume((filetransfer*)wParam,pfr->action,pfr->szFilename);
		iRet=0;// Success
	}
return(iRet);
}

INT_PTR MraFileAllow(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (lParam)
	if (((CCSDATA*)lParam)->wParam) 
	{
		LPWSTR lpwszPath;
		SIZE_T dwPathSize;
		CCSDATA *pcds=(CCSDATA*)lParam;
		PROTORECVEVENT *prce=(PROTORECVEVENT*)pcds->lParam;
		PROTORECVFILET *prcf=(PROTORECVFILET*)pcds->lParam;

		lpwszPath=(LPWSTR)((CCSDATA*)lParam)->lParam;
		dwPathSize=lstrlenW(lpwszPath);

		if (MraFilesQueueAccept(masMraSettings.hFilesQueueHandle,((CCSDATA*)lParam)->wParam,lpwszPath,dwPathSize)==NO_ERROR)
		{
			iRet=((CCSDATA*)lParam)->wParam; // Success
		}
	}
return(iRet);
}

INT_PTR MraFileDenyCancel(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=1;

	if (lParam)
	if (((CCSDATA*)lParam)->wParam) 
	{// description: (LPSTR)((CCSDATA*)lParam)->lParam
		MraFilesQueueCancel(masMraSettings.hFilesQueueHandle,((CCSDATA*)lParam)->wParam,TRUE);
		iRet=0; // Success
	}
return(iRet);
}



INT_PTR MraFileSend(WPARAM wParam,LPARAM lParam)
{
	INT_PTR iRet=0;

	if (masMraSettings.bLoggedIn && wParam && lParam)
	if (((CCSDATA*)lParam)->hContact && ((CCSDATA*)lParam)->lParam) 
	{// (LPSTR)((CCSDATA*)lParam)->wParam,lstrlenA((LPSTR)((CCSDATA*)lParam)->wParam) - description
		LPWSTR *plpwszFiles=(LPWSTR*)((CCSDATA*)lParam)->lParam;
		SIZE_T dwFilesCount;

		for(dwFilesCount=0;plpwszFiles[dwFilesCount];dwFilesCount++);

		MraFilesQueueAddSend(masMraSettings.hFilesQueueHandle,0,((CCSDATA*)lParam)->hContact,plpwszFiles,dwFilesCount,(DWORD*)&iRet);
	}
return(iRet);
}
