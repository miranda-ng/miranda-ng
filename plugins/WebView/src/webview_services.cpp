/*
 * A plugin for Miranda IM which displays web page text in a window Copyright 
 * (C) 2005 Vincent Joyce.
 * 
 * Miranda IM: the free icq client for MS Windows  Copyright (C) 2000-2
 * Richard Hughes, Roland Rabien & Tristan Van de Vreede
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 59 
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 */

#include "stdafx.h"

static int      searchId = -1;
static char     sID[32];

/************************/
int DBSettingChanged(WPARAM wParam, LPARAM lParam)
{

   DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING *) lParam;
   char *invalidchar;

   // We can't upload changes to NULL contact
   if ((HANDLE) wParam == NULL)
      return 0;

   if (!strcmp(cws->szModule, "CList"))
   {
      HANDLE hContact;
      DBVARIANT       dbv;
      char*szProto;
      FILE  *pcachefile;
      char cachepath[MAX_PATH];
  
      char cachedirectorypath[MAX_PATH];
  
      char newcachepath[MAX_PATH + 50];
      char renamedcachepath[MAX_PATH + 50];
      char*cacheend;
      char OLDcontactname[100];
      char NEWcontactname[100];
       int ranNum;
      char ranStr[7];
      int invalidpresent = 0;

      
      hContact = (HANDLE) wParam;
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      if (szProto == NULL || strcmp(szProto, MODULENAME))
         return 0;

      // A contact is renamed
      if (!strcmp(cws->szSetting, "MyHandle"))
      {
         hContact = (HANDLE) wParam;
         szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
         if (szProto == NULL || strcmp(szProto, MODULENAME))
 return 0;
///////
         if (!DBGetContactSetting(hContact, MODULENAME, PRESERVE_NAME_KEY, &dbv))
         {
 char nick[100];

 ZeroMemory(&OLDcontactname, sizeof(OLDcontactname));
 ZeroMemory(&NEWcontactname, sizeof(NEWcontactname));

 DBFreeVariant(&dbv);

 ZeroMemory(&nick, sizeof(nick));

 DBGetContactSetting(hContact, "CList", "MyHandle", &dbv);
 _snprintf(nick, sizeof(nick), "%s", dbv.pszVal);
 DBFreeVariant(&dbv);

 DBGetContactSetting((HANDLE) wParam, MODULENAME, PRESERVE_NAME_KEY, &dbv);
 _snprintf(OLDcontactname, sizeof(OLDcontactname), "%s", dbv.pszVal);
 DBFreeVariant(&dbv);

  	if (strchr(nick, '\\') != NULL)
  	{
		     WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
		      invalidchar = strchr(nick, '\\');
  *invalidchar = '_';
  invalidpresent =1;
 }
 
 if (strchr(nick, '/') != NULL)
 {
  WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
		      invalidchar = strchr(nick, '/');
  *invalidchar = '_';
  invalidpresent =1;
 } 
 
 if (strchr(nick, ':') != NULL)
 {
  WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
		      invalidchar = strchr(nick, ':');
  *invalidchar = '_';
  invalidpresent =1;
  }

 if (strchr(nick, '*') != NULL)
 {
  WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
  invalidchar = strchr(nick, '*');
  *invalidchar = '_';
  invalidpresent =1;
  }

 if (strchr(nick, '?') != NULL)
 {
  WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
  invalidchar = strchr(nick, '?');
  *invalidchar = '_';
  invalidpresent =1;
  }

 if (strchr(nick, '\"') != NULL)
 {
 WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
 invalidchar = strchr(nick, '\"');
  *invalidchar = '_';
  invalidpresent =1;
 }

 if (strchr(nick, '<') != NULL)
 {
   WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
   invalidchar = strchr(nick, '<');
  *invalidchar = '_';
  invalidpresent =1;
   }
       
 if (strchr(nick, '>') != NULL)
 {
  WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
	 invalidchar = strchr(nick, '>');
  *invalidchar = '_';
  invalidpresent =1;
  }

 if (strchr(nick, '|') != NULL)
 {
   WErrorPopup("ERROR", Translate("Invalid symbol present in contact name."));
   invalidchar = strchr(nick, '|');
  *invalidchar = '_';
  invalidpresent =1;
   }
  
  if(invalidpresent)
  {
      srand((unsigned) time(NULL));
      ranNum = (int) 10000 *rand() / (RAND_MAX + 1.0);
      _snprintf(ranStr, sizeof(ranStr), "%d", ranNum);
      strcat(nick, ranStr); 
  }  
 
  

 if (((strchr(nick, '(')) == 0))
 {
   db_set_s(hContact, MODULENAME, PRESERVE_NAME_KEY, nick);
   db_set_s(hContact, MODULENAME, "Nick", nick);
   db_set_s(hContact, "CList", "MyHandle", nick);
 }
 DBGetContactSetting((HANDLE) wParam, MODULENAME, PRESERVE_NAME_KEY, &dbv);
 _snprintf(NEWcontactname, sizeof(NEWcontactname), "%s", dbv.pszVal);
 DBFreeVariant(&dbv);

//

 // TEST GET NAME FOR CACHE
 GetModuleFileName(hInst, cachepath, sizeof(cachepath));
 cacheend = strrchr(cachepath, '\\');
 cacheend++;
 *cacheend = '\0';
 _snprintf(cachedirectorypath, sizeof(cachedirectorypath), "%s%s%s", cachepath, MODULENAME, "cache\\");
 CreateDirectory(cachedirectorypath, NULL);
       
 
 
 _snprintf(newcachepath, sizeof(newcachepath), "%s%s%s%s%s", cachepath, MODULENAME, "cache\\", OLDcontactname, ".txt");
 _snprintf(renamedcachepath, sizeof(newcachepath), "%s%s%s%s%s", cachepath, MODULENAME, "cache\\", NEWcontactname, ".txt");
 
 
 
 // file exists?
 if ((_access(newcachepath, 0)) != -1)
 {
   if ((pcachefile = fopen(newcachepath, "r")) != NULL)
   {
      fclose(pcachefile);
      if (lstrcmp(newcachepath, renamedcachepath))
      {
         MoveFile(newcachepath, renamedcachepath);
         db_set_s(hContact, MODULENAME, CACHE_FILE_KEY, renamedcachepath);
      }
         
   }
 }
//

         }
////////     
      }
   }
   return 0;
}
/************************/

/************************/
int SiteDeleted(WPARAM wParam, LPARAM lParam)
{
   FILE  *pcachefile;
   char cachepath[MAX_PATH];
   char cachedirectorypath[MAX_PATH];
   char newcachepath[MAX_PATH + 50];
   char*cacheend;
   char contactname[100];
   DBVARIANT       dbv;

   ZeroMemory(&contactname, sizeof(contactname));
   ZeroMemory(&dbv, sizeof(dbv));

   if (lstrcmp((char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, wParam, 0), MODULENAME))
      return 0;

   DBGetContactSetting((HANDLE) wParam, MODULENAME, PRESERVE_NAME_KEY, &dbv);
   _snprintf(contactname, sizeof(contactname), "%s", dbv.pszVal);
   DBFreeVariant(&dbv);

   // TEST GET NAME FOR CACHE
   GetModuleFileName(hInst, cachepath, sizeof(cachepath));
   cacheend = strrchr(cachepath, '\\');
   cacheend++;
   *cacheend = '\0';

   _snprintf(cachedirectorypath, sizeof(cachedirectorypath), "%s%s%s", cachepath, MODULENAME, "cache\\");
   CreateDirectory(cachedirectorypath, NULL);
   _snprintf(newcachepath, sizeof(newcachepath), "%s%s%s%s%s", cachepath, MODULENAME, "cache\\", contactname, ".txt");
   // file exists?
   if ((_access(newcachepath, 0)) != -1)
   {
      if ((pcachefile = fopen(newcachepath, "r")) != NULL)
      {
         fclose(pcachefile);
         DeleteFile(newcachepath);
         db_set_s((HANDLE) wParam, MODULENAME, CACHE_FILE_KEY, "");
      }
   }
   return 0;
}
/************************/

/************************/
INT_PTR OpenCacheDir(WPARAM wParam, LPARAM lParam)
{
      char cachepath[MAX_PATH];
      char cachedirectorypath[MAX_PATH];
      char*cacheend;
      DIR *pdir = NULL;
  

 //GET NAME FOR CACHE
 GetModuleFileName(hInst, cachepath, sizeof(cachepath));
 cacheend = strrchr(cachepath, '\\');
 cacheend++;
 *cacheend = '\0';

 _snprintf(cachedirectorypath, sizeof(cachedirectorypath), "%s%s%s%s", cachepath, MODULENAME, "cache\\", cacheend);
 
 	  if((pdir = opendir(cachedirectorypath)) == NULL)
       {
		         WErrorPopup("ERROR", Translate("Cache folder does not exist."));
     return 0;
       }
 
 ShellExecute(NULL, "open", cachedirectorypath, NULL, NULL, SW_SHOWNORMAL);

   return 0;
}
/************************/
/************************/
INT_PTR PingWebsiteMenuCommand(WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;
   char url[300];
   static char    *Newnick;
   static char    *Oldnick;
   char*Nend;
   char Cnick[255];
   FILE  *pfile = NULL;
  
  	if ((pfile = fopen("psite.bat", "r")) == NULL)
  	{
    WErrorPopup("ERROR", Translate("Missing \"psite.bat\" file."));
    return 0;
    }

   ZeroMemory(&Cnick, sizeof(Cnick));
   ZeroMemory(&url, sizeof(url));
   
   DBGetContactSetting((HANDLE) wParam, MODULENAME, "URL", &dbv);
    wsprintf(url, "%s", dbv.pszVal);
	DBFreeVariant(&dbv);
	
	   strncpy(Cnick, url, sizeof(Cnick));
   Oldnick = strdup(Cnick);

    /**/
      if ((strstr(Cnick, "://")) != 0)
   {
      Oldnick = strstr(Cnick, "://");
      Oldnick = Oldnick + 3;
   } else if ((strstr(Cnick, "://")) == 0)
   {
      strcpy(Oldnick, Cnick);
   }
   
      Newnick = strdup(Oldnick);
      if (strchr(Newnick, '/'))
      {
         Nend = strchr(Newnick, '/');
         *Nend = '\0';
      }
   
    //MessageBox(NULL, Newnick, MODULENAME, MB_OK);   
 
  ShellExecute(NULL, "open", "psite.bat", Newnick, NULL, SW_HIDE);
  

   return 0;
}
/************************/


/************************/
INT_PTR StpPrcssMenuCommand(WPARAM wParam, LPARAM lParam)
{
   DBVARIANT       dbv;

  db_set_b((HANDLE) wParam, MODULENAME, STOP_KEY, 1);  

   return 0;
}
/************************/



//=======================================================
// GetCaps
// =======================================================

int GetCaps(WPARAM wParam, LPARAM lParam)
{
   int ret = 0;

   if (wParam == PFLAGNUM_1)
      ret = PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_VISLIST;

   if (wParam == PFLAGNUM_2)
   {
      if (!(db_get_b(NULL, MODULENAME, HIDE_STATUS_ICON_KEY, 0)))
         return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND; // add 
      // the
      // possible
      // statuses here

   }
   if (wParam == PFLAG_UNIQUEIDTEXT)
      ret = (int) Translate("Site URL");

   if (wParam == PFLAGNUM_3)
      ret = 0;
      
      
      if (wParam == PFLAGNUM_5)
			ret = PF2_INVISIBLE|PF2_SHORTAWAY|PF2_LONGAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|
				PF2_OUTTOLUNCH|PF2_ONTHEPHONE;
				
   if (wParam == PFLAG_UNIQUEIDSETTING)
      ret = (int) "PreserveName";

   return ret;
}

//=======================================================
// =======================================================
// GetName
// =======================================================
int GetName(WPARAM wParam, LPARAM lParam)
{
   lstrcpyn((char *) lParam, MODULENAME, wParam);
   return 0;
}
//=======================================================
// SetStatus
// =======================================================
int SetStatus(WPARAM wParam, LPARAM lParam)
{
   int oldStatus;
   DBVARIANT       dbv;

   oldStatus = bpStatus;
   char*szProto;
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);

   if (wParam == ID_STATUS_ONLINE)
   {
      wParam = ID_STATUS_ONLINE;
   } else if (wParam == ID_STATUS_OFFLINE)
   {
      wParam = ID_STATUS_OFFLINE;
   } else
   {
      wParam = ID_STATUS_ONLINE;
   }
   // broadcast the message
   bpStatus = wParam;

   ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE) oldStatus, wParam);

    /**/
   // Make sure no contact has offline status for any reason on first time
   // run     
      if (DBGetContactSetting(NULL, MODULENAME, "FirstTime", &dbv))
   {
      while (hContact)
      {
         szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
         if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
         {
 db_set_w(hContact, MODULENAME, "Status", ID_STATUS_ONLINE);
         }
         hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
      }
      if (DBGetContactSetting(NULL, MODULENAME, "FirstTime", &dbv))
         db_set_b(NULL, MODULENAME, "FirstTime", 1);
   }
   DBFreeVariant(&dbv);
    /**/

      if (bpStatus == ID_STATUS_OFFLINE)
      db_set_b(NULL, MODULENAME, OFFLINE_STATUS, 1);
   else
      db_set_b(NULL, MODULENAME, OFFLINE_STATUS, 0);

   return 0;
}
//=======================================================
// GetStatus
// =======================================================

int GetStatus(WPARAM wParam, LPARAM lParam)
{

   if (bpStatus == ID_STATUS_ONLINE)
      return ID_STATUS_ONLINE;
   else if (bpStatus == ID_STATUS_AWAY)
      return ID_STATUS_AWAY;
   else if (bpStatus == ID_STATUS_NA)
      return ID_STATUS_NA;
   else if (bpStatus == ID_STATUS_OCCUPIED)
      return ID_STATUS_OCCUPIED;
   else if (bpStatus == ID_STATUS_DND)
      return ID_STATUS_DND;
   else
      return ID_STATUS_OFFLINE;

}

//=======================================================
// BPLoadIcon
// =======================================================

int BPLoadIcon(WPARAM wParam, LPARAM lParam)
{
   UINT id;

   switch (wParam & 0xFFFF)
   {
   case PLI_PROTOCOL:
      id = IDI_SITE;
      break;
   default:
      return (int) (HICON) NULL;
   }
   return (int) LoadImage(hInst, MAKEINTRESOURCE(id), IMAGE_ICON, GetSystemMetrics(wParam & PLIF_SMALL ? SM_CXSMICON : SM_CXICON), GetSystemMetrics(wParam & PLIF_SMALL ? SM_CYSMICON : SM_CYICON), 0);
}

/*******************/
static void __cdecl BasicSearchTimerProc(void *pszNick)
{
   PROTOSEARCHRESULT psr;

   ZeroMemory(&psr, sizeof(psr));
   psr.cbSize = sizeof(psr);
   psr.nick = (char *) pszNick;
   // 

   // broadcast the search result
   ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_DATA, (HANDLE) 1, (LPARAM) & psr);
   ProtoBroadcastAck(MODULENAME, NULL, ACKTYPE_SEARCH, ACKRESULT_SUCCESS, (HANDLE) 1, 0);

   // exit the search
   searchId = -1;
}

/*******************/

/*******************/
int BasicSearch(WPARAM wParam, LPARAM lParam)
{
   static char     buf[300];
   

   if (lParam)
      lstrcpyn(buf, (const char *) lParam, 256);
   // 

   if (searchId != -1)
      return 0; // only one search at a time

   lstrcpyn(sID, (char *) lParam, sizeof(sID));
   searchId = 1;
   // create a thread for the ID search
   forkthread(BasicSearchTimerProc, 0, &buf);

   return searchId;
}
/*******************/

/*******************/
int AddToList(WPARAM wParam, LPARAM lParam)
{
   PROTOSEARCHRESULT *psr = (PROTOSEARCHRESULT *) lParam;
   DBVARIANT       dbv;
   static char    *Newnick;
   static char    *Oldnick;
   char*Nend;
   char Cnick[255];
   int sameurl = 0;
   int samename = 0;


   ZeroMemory(&Cnick, sizeof(Cnick));


   // search for existing contact
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   HANDLE         hContact2 = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char*szProto;


    
 

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // check if it is a webview contact
      if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
      {
         // check ID to see if the contact already exist in the database

         if (!DBGetContactSetting(hContact, MODULENAME, "URL", &dbv))
         {
 if (!lstrcmpi(psr->nick, dbv.pszVal))
 {
   // remove the flag for not on list and hidden, thus make the
   // contact visible
   // and add them on the list
   sameurl++;
   if (db_get_b(hContact, "CList", "NotOnList", 1))
   {
      DBDeleteContactSetting(hContact, "CList", "NotOnList");
      DBDeleteContactSetting(hContact, "CList", "Hidden");
   }
   DBFreeVariant(&dbv);
 }
         }
         DBFreeVariant(&dbv);
         
      }
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }


 
   if (psr->nick == NULL)
  {
     WErrorPopup("ERROR", Translate("Please select site in Find/Add Contacts..."));
     return;
  }   
 
   // if contact with the same ID was not found, add it
   if (psr->cbSize != sizeof(PROTOSEARCHRESULT))
      return (int) (HANDLE) NULL;
   hContact = (HANDLE) CallService(MS_DB_CONTACT_ADD, 0, 0);
   CallService(MS_PROTO_ADDTOCONTACT, (WPARAM) hContact, (LPARAM) MODULENAME);
   

/////////write to db
   db_set_b(hContact, MODULENAME, ON_TOP_KEY, 0);
   db_set_b(hContact, MODULENAME, DBLE_WIN_KEY, 1);
   db_set_s(hContact, MODULENAME, END_STRING_KEY, "");
   db_set_b(hContact, MODULENAME, RWSPACE_KEY, 1);
  
 


//Convert url into a name for contact
///
  if (psr->nick != NULL)
  strncpy(Cnick, psr->nick, sizeof(Cnick));// this crashes
///   
 
   Oldnick = strdup(Cnick);


    /**/
      if ((strstr(Cnick, "://")) != 0)
   {
      Oldnick = strstr(Cnick, "://");
      Oldnick = Oldnick + 3;

   } else if ((strstr(Cnick, "://")) == 0)
   {
      strcpy(Oldnick, Cnick);
 
   }
   if ((strstr(Oldnick, "www.")) != 0)
   {
      Newnick = strstr(Oldnick, "www.");
      Newnick = Newnick + 4;
      Nend = strchr(Newnick, '.');
      *Nend = '\0';
    
   } else if ((strstr(Oldnick, "WWW.")) != 0)
   {
      Newnick = strstr(Oldnick, "WWW.");
      Newnick = Newnick + 4;
      Nend = strchr(Newnick, '.');
      *Nend = '\0';
       
   } else
   {
       
      Newnick = strdup(Oldnick);
      if (strchr(Newnick, '.'))
      {
         Nend = strchr(Newnick, '.');
         *Nend = '\0';
  
      }
   }
   

   while (hContact2 != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact2, 0);
      // check if it is a webview contact
      if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
      {
         // check ID to see if the contact already exist in the database

   
		 
		  if (!DBGetContactSetting(hContact2, MODULENAME, PRESERVE_NAME_KEY, &dbv))
         {
 if (!lstrcmpi(Newnick, dbv.pszVal))
 {
   // remove the flag for not on list and hidden, thus make the
   // contact visible
   // and add them on the list
   samename++;
   if (db_get_b(hContact2, "CList", "NotOnList", 1))
   {
      DBDeleteContactSetting(hContact2, "CList", "NotOnList");
      DBDeleteContactSetting(hContact2, "CList", "Hidden");
   }
   DBFreeVariant(&dbv);
 }
         }
         DBFreeVariant(&dbv);

      }
      hContact2 = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact2, 0);
   }


   
   if ((sameurl > 0) || (samename > 0)) // contact has the same url or name as another contact, add rand
        // num to name

   {
      int ranNum;
      char ranStr[7];

      srand((unsigned) time(NULL));

      ranNum = (int) 10000 *rand() / (RAND_MAX + 1.0);

      _snprintf(ranStr, sizeof(ranStr), "%d", ranNum);
      strcat(Newnick, ranStr);
   }
//end convert

   db_set_s(hContact, "CList", "MyHandle", Newnick);
   db_set_s(hContact, MODULENAME, PRESERVE_NAME_KEY, Newnick);
   db_set_s(hContact, MODULENAME, "Nick", Newnick);
   db_set_b(hContact, MODULENAME, CLEAR_DISPLAY_KEY, 1);
   db_set_s(hContact, MODULENAME, START_STRING_KEY, "");
   db_set_s(hContact, MODULENAME, URL_KEY, psr->nick);
   db_set_s(hContact, MODULENAME, "Homepage", psr->nick);
   db_set_b(hContact, MODULENAME, U_ALLSITE_KEY, 1);
   db_set_w(hContact, MODULENAME, "Status", ID_STATUS_ONLINE);
////////////////////////



   // ignore status change
   db_set_dw(hContact, "Ignore", "Mask", 8);

   Sleep(2);

   DBFreeVariant(&dbv);

   
   return (int) hContact;
}
/*******************/

/*******************/
int GetInfo(WPARAM wParam, LPARAM lParam)
{
   forkthread(AckFunc, 0, NULL);
   return 1;
}
/*******************/
/*******************/
void AckFunc(void *dummy)
{
   HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
   char*szProto;

   while (hContact != NULL)
   {
      szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
      // 
      if (szProto != NULL && !lstrcmp(MODULENAME, szProto))
         ProtoBroadcastAck(MODULENAME, hContact, ACKTYPE_GETINFO, ACKRESULT_SUCCESS, (HANDLE) 1, 0);
      // 
      hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM) hContact, 0);
   }

}
/*******************/

