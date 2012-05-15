// ---------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
// 
// Copyright © 2002 Dominus Procellarum 
// Copyright © 2004-2008 Joe Kucera
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ---------------------------------------------------------------------------

#include "contacts.h"


int SRCCallProtoService(const char *szModule, const char *szService, WPARAM wParam, LPARAM lParam)
{
  if (!g_NewProtoAPI)
  {
    char str[MAXMODULELABELLENGTH];
    strcpy(str,szModule);
    strcat(str,szService);
    return CallService(str,wParam,lParam);
  }
  else // this will call pluginLink.CallProtoService (only 0.8.0.8+)
    return CallProtoService(szModule, szService, wParam, lParam);
}


int SRCCallContactService(HANDLE hContact, const char *szProtoService, WPARAM wParam, LPARAM lParam)
{
  if (!g_NewProtoAPI)
  {
    CCSDATA ccs;
    ccs.hContact=hContact;
    ccs.szProtoService=szProtoService;
    ccs.wParam=wParam;
    ccs.lParam=lParam;
    return CallService(MS_PROTO_CALLCONTACTSERVICE,0,(LPARAM)&ccs);
  }
  else // this will call pluginLink.CallContactService (only 0.8.0.8+)
    return CallContactService(hContact, szProtoService, wParam, lParam);
}


int utf8_decode(const unsigned char *from, char **to);


/* a strlennull() that likes NULL */
size_t __fastcall strlennull(const char *string)
{
  if (string)
    return strlen(string);

  return 0;
}


int __fastcall strcmpnull(const char *str1, const char *str2)
{
  if (!str1 || !str2) return 1;

  return strcmp(str1, str2);
}


char* __fastcall null_strdup(const char *string)
{
  if (string)
    return strdup(string);

  return NULL;
}


void __fastcall SAFE_FREE(void** p)
{
  if (*p)
  {
    free(*p);
    *p = NULL;
  }
}


char *GetContactProto(HANDLE hContact)
{
  return (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
}


TCHAR *GetContactDisplayNameT(HANDLE hContact)
{
  return (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, g_UnicodeCore ? GCDNF_UNICODE : 0);
}


char *GetContactUID(HANDLE hContact, int bTchar)
{
  DBVARIANT vrUid;
  char *szUid = NULL;

  char *szProto = GetContactProto(hContact);
  char *uid = (char*)SRCCallProtoService(szProto, PS_GETCAPS, PFLAG_UNIQUEIDSETTING, 0); // v0.3+ only 

  if (((int)uid != CALLSERVICE_NOTFOUND) && uid)
  { // it worked, yeah :)
    if (!DBGetContactSettingT(hContact, szProto, uid, &vrUid)) 
    {
      if (vrUid.type == DBVT_DWORD) 
      {
        szUid = (char*)_alloca(17);
        szUid[0] = 0; // empty string
        _itoa(vrUid.dVal, szUid, 10);
      }  
      else if (vrUid.type == DBVT_ASCIIZ) 
      {
        szUid = (char*)_alloca(strlennull(vrUid.pszVal) + 1);
        strcpy(szUid, vrUid.pszVal);
      }
      else if (vrUid.type == DBVT_UTF8)
      { // yeah, jabber gives this!
        char *szAnsi = NULL;

        if (utf8_decode((unsigned char*)vrUid.pszVal, &szAnsi))
        {
          szUid = (char*)_alloca(strlennull(szAnsi) + 1);
          strcpy(szUid, szAnsi);
          SAFE_FREE((void**)&szAnsi);
        }
      }
      DBFreeVariant(&vrUid);
    }
  }
  if (bTchar)
    return ansi_to_tchar(szUid);
  else
    return null_strdup(szUid);
}


TCHAR* MirandaStatusToStringT(int mirandaStatus)
{
  return (TCHAR *)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION, mirandaStatus, g_UnicodeCore ? GSMDF_UNICODE : 0);
}

HANDLE __fastcall SRCFindFirstContact()
{
  return (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
}

HANDLE __fastcall SRCFindNextContact(HANDLE hContact)
{
  return (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
}

int DBGetContactSettingT(HANDLE hContact, const char *szModule, const char* szSetting, DBVARIANT *dbv)
{
  if (ServiceExists(MS_DB_CONTACT_GETSETTING_STR))
    return DBGetContactSettingW(hContact, szModule, szSetting, dbv);
  else
    return DBGetContactSetting(hContact, szModule, szSetting, dbv);
}


TCHAR* DBGetContactSettingStringT(HANDLE hContact, const char *szModule, const char* szSetting, TCHAR* szDef)
{
  DBVARIANT dbv = {DBVT_DELETED};
  TCHAR* szRes;

  if (g_UnicodeCore)
  {
    if (DBGetContactSettingWString(hContact, szModule, szSetting, &dbv))
      return strdupT(szDef);
    
    szRes = strdupT(dbv.ptszVal);
    DBFreeVariant(&dbv);
  }
  else
  { 
    if (DBGetContactSettingT(hContact, szModule, szSetting, &dbv))
      return strdupT(szDef);

    if (dbv.type == DBVT_ASCIIZ)
      szRes = ansi_to_tchar(dbv.pszVal);
    else if (dbv.type == DBVT_UTF8)
      szRes = utf8_to_tchar((unsigned char*)dbv.pszVal);
    else
      szRes = strdupT(szDef);
    DBFreeVariant(&dbv);
  }
  return szRes;
}

int DBWriteContactSettingStringT(HANDLE hContact, const char *szModule, const char* szSetting, TCHAR* szValue)
{
  if (g_UnicodeCore)
    return DBWriteContactSettingWString(hContact, szModule, szSetting, (WCHAR*)szValue);
  else
    return DBWriteContactSettingString(hContact, szModule, szSetting, (char*)szValue);
}



int __fastcall SRCTranslateDialog(HWND hwndDlg)
{
  LANGPACKTRANSLATEDIALOG lptd;

  lptd.cbSize=sizeof(lptd);
  lptd.flags=0;
  lptd.hwndDlg=hwndDlg;
  lptd.ignoreControls=NULL;
  return CallService(MS_LANGPACK_TRANSLATEDIALOG,0,(LPARAM)&lptd);
}


void DialogAddContactExecute(HWND hwndDlg, HANDLE hNewContact) 
{
  ADDCONTACTSTRUCT acs={0};

  acs.handle = hNewContact;
  acs.handleType = HANDLE_CONTACT;

  CallService(MS_ADDCONTACT_SHOW, (WPARAM)hwndDlg, (LPARAM)&acs);
}


void DrawProtocolIcon(HWND hwndDlg, LPARAM lParam, HANDLE hContact)
{
  LPDRAWITEMSTRUCT dis=(LPDRAWITEMSTRUCT)lParam;

  if (dis->hwndItem==GetDlgItem(hwndDlg, IDC_PROTOCOL)) 
  {
    HICON hIcon = LoadContactProtoIcon(hContact);
    if (hIcon)
    {
      DrawIconEx(dis->hDC,dis->rcItem.left,dis->rcItem.top,hIcon,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0,NULL,DI_NORMAL);
      DestroyIcon(hIcon);
    }
  }
}


void UpdateDialogTitle(HWND hwndDlg, HANDLE hContact, char* pszTitleStart)
{
  TCHAR newtitle[512];
  WCHAR str[MAX_PATH];
  TCHAR *oldTitle;
  TCHAR *szStatus;
  char *szProto;

  if (hContact) 
  {
    szProto = GetContactProto(hContact);
    if (szProto) 
    {
      TCHAR *uid = GetContactUID(hContact, TRUE);
      TCHAR *contactName = GetContactDisplayNameT(hContact);

      oldTitle = GetDlgItemTextT(hwndDlg, IDC_NAME);

      if (strcmpT(uid?uid:contactName, oldTitle))
        SetDlgItemTextT(hwndDlg, IDC_NAME, uid?uid:contactName);

      szStatus = MirandaStatusToStringT(szProto==NULL ? ID_STATUS_OFFLINE:DBGetContactSettingWord(hContact,szProto,"Status",ID_STATUS_OFFLINE));
      _snprintfT(newtitle, 256, "%s %s (%s)", SRCTranslateT(pszTitleStart, str), contactName, szStatus);

      SAFE_FREE((void**)&uid);
      SAFE_FREE((void**)&oldTitle);
    }
    else
      strncpyT(newtitle, SRCTranslateT(pszTitleStart, str), 256);
  }
  else
    strncpyT(newtitle, SRCTranslateT(pszTitleStart, str), 256);

  oldTitle = GetWindowTextT(hwndDlg);

  if (strcmpT(newtitle, oldTitle))
    SetWindowTextT(hwndDlg, newtitle);

  SAFE_FREE((void**)&oldTitle);
}


void UpdateDialogAddButton(HWND hwndDlg, HANDLE hContact)
{
  int bVisible = DBGetContactSettingByte(hContact,"CList","NotOnList",0); 

  ShowWindow(GetDlgItem(hwndDlg, IDC_ADD), bVisible?SW_SHOW:SW_HIDE);
}


HICON InitMButton(HWND hDlg, int idButton, LPCSTR szIcon, char* szTip)
{
  HWND hButton = GetDlgItem(hDlg, idButton);
  HICON hIcon = (HICON)LoadImage(GetModuleHandle(NULL),szIcon,IMAGE_ICON,GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON),0);

  SendMessageT(hButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
  SendMessageT(hButton, BUTTONSETASFLATBTN, 0, 0);
  SendMessageT(hButton, BUTTONADDTOOLTIP, (WPARAM)Translate(szTip), 0);

  return hIcon;
}


HICON LoadContactProtoIcon(HANDLE hContact)
{
  char* szProto = GetContactProto(hContact);
  if (szProto)
    return (HICON)SRCCallProtoService(szProto, PS_LOADICON, PLI_PROTOCOL|PLIF_SMALL, 0);
  return NULL;
}


void EnableDlgItem(HWND hwndDlg, UINT control, int state)
{
  EnableWindow(GetDlgItem(hwndDlg, control), state);
}


LRESULT SendMessageT(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  if (g_UnicodeCore)
    return SendMessageW(hWnd, Msg, wParam, lParam);
  else
    return SendMessageA(hWnd, Msg, wParam, lParam);
}

TCHAR* GetWindowTextT(HWND hWnd)
{
  if (g_UnicodeCore)
  {
    int len = GetWindowTextLengthW(hWnd) + 1;
    WCHAR* txt = (WCHAR*)malloc(len * sizeof(WCHAR));
    if (txt) {
      txt[0] = 0;
      GetWindowTextW(hWnd, txt, len);
    }
    return (TCHAR*)txt;
  }
  else
  {
    int len = GetWindowTextLengthA(hWnd) + 1;
    char* txt = (char*)malloc(len * sizeof(char));
    if (txt) {
      txt[0] = 0;
      GetWindowTextA(hWnd, txt, len);
    }
    return (TCHAR*)txt;
  }
}

BOOL SetWindowTextT(HWND hWnd, TCHAR* lpString)
{
  if (g_UnicodeCore)
    return SetWindowTextW(hWnd, (WCHAR*)lpString);
  else
    return SetWindowTextA(hWnd, lpString);
}

TCHAR* GetDlgItemTextT(HWND hDlg, int nIDDlgItem)
{
  return GetWindowTextT(GetDlgItem(hDlg, nIDDlgItem));
}

BOOL SetDlgItemTextT(HWND hDlg, int nIDDlgItem, TCHAR* lpString)
{
  return SetWindowTextT(GetDlgItem(hDlg, nIDDlgItem), lpString);
}

HWND CreateDialogParamT(HINSTANCE hInstance, const char* szTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam)
{
  if (g_UnicodeCore)
    return CreateDialogParamW(hInstance, (LPCWSTR)szTemplate, hWndParent, lpDialogFunc, dwInitParam);
  else
    return CreateDialogParamA(hInstance, szTemplate, hWndParent, lpDialogFunc, dwInitParam);
}

int ListView_InsertColumnT(HWND hwnd, int iCol, const LPLVCOLUMN pcol)
{
  return SendMessageT(hwnd, g_UnicodeCore ? LVM_INSERTCOLUMNW : LVM_INSERTCOLUMNA, (WPARAM)iCol, (LPARAM)pcol);
}

void ListView_SetItemTextT(HWND hwnd, int i, int iSubItem, TCHAR* pszText)
{
  LV_ITEM lvi = {0};

  lvi.iSubItem = iSubItem;
  lvi.pszText = pszText;
  SendMessageT(hwnd, g_UnicodeCore ? LVM_SETITEMTEXTW : LVM_SETITEMTEXTA, (WPARAM)i, (LPARAM)&lvi);
}



size_t __fastcall strlenT(const TCHAR *string)
{
  if (string)
  {
    if (g_UnicodeCore)
      return wcslen((WCHAR*)string);
    else
      return strlen((char*)string);
  }
  return 0;
}

TCHAR* __fastcall strdupT(const TCHAR *string)
{
  if (string)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcsdup((WCHAR*)string);
    else
      return (TCHAR*)strdup((char*)string);
  }
  return NULL;
}

int __fastcall strcmpT(const TCHAR *string1, const TCHAR *string2)
{
  if (!string1 || !string2) return 1;

  if (g_UnicodeCore)
    return wcscmp((WCHAR*)string1, (WCHAR*)string2);
  else
    return strcmp((char*)string1, (char*)string2);
}

TCHAR* __fastcall strcpyT(TCHAR* dest, const TCHAR* src)
{
  if (src)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcscpy((WCHAR*)dest, (WCHAR*)src);
    else
      return (TCHAR*)strcpy((char*)dest, (char*)src);
  }
  return dest;
}

TCHAR* __fastcall strncpyT(TCHAR* dest, const TCHAR* src, size_t len)
{
  if (src)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcsncpy((WCHAR*)dest, (WCHAR*)src, len);
    else
      return (TCHAR*)strncpy((char*)dest, (char*)src, len);
  }
  return dest;
}

TCHAR* __fastcall strcatT(TCHAR* dest, const TCHAR* src)
{
  if (src)
  {
    if (g_UnicodeCore)
      return (TCHAR*)wcscat((WCHAR*)dest, (WCHAR*)src);
    else
      return (TCHAR*)strcat((char*)dest, (char*)src);
  }
  return dest;
}

int _snprintfT(TCHAR *buffer, size_t count, const char* fmt, ...)
{
  va_list va;
  int len;

  va_start(va, fmt);
  if (g_UnicodeCore)
  {
    TCHAR* wfmt = ansi_to_tchar(fmt);

    len = _vsnwprintf((WCHAR*)buffer, count-1, (WCHAR*)wfmt, va);
    ((WCHAR*)buffer)[count-1] = 0;
    SAFE_FREE((void**)&wfmt);
  }
  else
  {
    len = _vsnprintf((char*)buffer, count-1, fmt, va);
    ((char*)buffer)[count-1] = 0;
  }
  va_end(va);
  return len;
}

TCHAR* __fastcall SRCTranslateT(const char* src, const WCHAR* unibuf)
{ // this takes Ascii strings only!!!
  char* szRes = NULL;

  if (!strlennull(src))
  { // for the case of empty strings
    return "";
  }

  if (g_UnicodeCore)
  { // we give WCHAR
    WCHAR *unicode;
    int wchars, err;

    wchars = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src,
      (int)strlennull(src), NULL, 0);

    if (wchars == 0) return NULL; // Failure

    unicode = (WCHAR*)unibuf;
    if (!unicode)
      unicode = (WCHAR*)malloc((wchars + 1) * sizeof(WCHAR));

    unicode[wchars] = 0;

    err = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, src,
      (int)strlennull(src), unicode, wchars);
    if (err != wchars) return NULL; // Failure

    return (TCHAR*)TranslateW(unicode);
  }
  else
    return (TCHAR*)Translate(src);
}

static BOOL bHasCP_UTF8 = FALSE;


void InitI18N(void)
{
  CPINFO CPInfo;


  bHasCP_UTF8 = GetCPInfo(CP_UTF8, &CPInfo);
}


// Scans a string encoded in UTF-8 to verify that it contains
// only valid sequences. It will return 1 if the string contains
// only legitimate encoding sequences; otherwise it will return 0;
// From 'Secure Programming Cookbook', John Viega & Matt Messier, 2003
int UTF8_IsValid(const unsigned char* pszInput)
{
  int nb, i;
  const unsigned char* c = pszInput;

  if (!pszInput) return 0;

  for (c = pszInput; *c; c += (nb + 1))
  {
    if (!(*c & 0x80))
      nb = 0;
    else if ((*c & 0xc0) == 0x80) return 0;
    else if ((*c & 0xe0) == 0xc0) nb = 1;
    else if ((*c & 0xf0) == 0xe0) nb = 2;
    else if ((*c & 0xf8) == 0xf0) nb = 3;
    else if ((*c & 0xfc) == 0xf8) nb = 4;
    else if ((*c & 0xfe) == 0xfc) nb = 5;

    for (i = 1; i<=nb; i++) // we this forward, do not cross end of string
      if ((*(c + i) & 0xc0) != 0x80)
        return 0;
  }

  return 1;
}


/*
 * The following UTF8 routines are
 *
 * Copyright (C) 2001 Peter Harris <peter.harris@hummingbird.com>
 * Copyright (C) 2001 Edmund Grimley Evans <edmundo@rano.org>
 *
 * under a GPL license
 *
 * --------------------------------------------------------------
 * Convert a string between UTF-8 and the locale's charset.
 * Invalid bytes are replaced by '#', and characters that are
 * not available in the target encoding are replaced by '?'.
 *
 * If the locale's charset is not set explicitly then it is
 * obtained using nl_langinfo(CODESET), where available, the
 * environment variable CHARSET, or assumed to be US-ASCII.
 *
 * Return value of conversion functions:
 *
 *  -1 : memory allocation failed
 *   0 : data was converted exactly
 *   1 : valid data was converted approximately (using '?')
 *   2 : input was invalid (but still converted, using '#')
 *   3 : unknown encoding (but still converted, using '?')
 */



/*
 * Convert a string between UTF-8 and the locale's charset.
 */
unsigned char *make_utf8_string(const wchar_t *unicode)
{
  int size = 0;
  int index = 0;
  int out_index = 0;
  unsigned char* out;
  unsigned short c;

  if (!unicode) return NULL;

  /* first calculate the size of the target string */
  c = unicode[index++];
  while (c)
  {
    if (c < 0x0080) 
      size += 1;
    else if (c < 0x0800) 
      size += 2;
    else 
      size += 3;
    c = unicode[index++];
  }

  out = (unsigned char*)malloc(size + 1);
  if (out == NULL)
    return NULL;
  index = 0;

  c = unicode[index++];
  while (c)
  {
    if (c < 0x080) 
    {
      out[out_index++] = (unsigned char)c;
    }
    else if (c < 0x800) 
    {
      out[out_index++] = 0xc0 | (c >> 6);
      out[out_index++] = 0x80 | (c & 0x3f);
    }
    else
    {
      out[out_index++] = 0xe0 | (c >> 12);
      out[out_index++] = 0x80 | ((c >> 6) & 0x3f);
      out[out_index++] = 0x80 | (c & 0x3f);
    }
    c = unicode[index++];
  }
  out[out_index] = 0x00;

  return out;
}



WCHAR *make_unicode_string(const unsigned char *utf8)
{
  int size = 0, index = 0, out_index = 0;
  wchar_t *out;
  unsigned char c;

  if (!utf8) return NULL;

  /* first calculate the size of the target string */
  c = utf8[index++];
  while (c) 
  {
    if ((c & 0x80) == 0) 
    {
      index += 0;
    }
    else if ((c & 0xe0) == 0xe0) 
    {
      index += 2;
    }
    else
    {
      index += 1;
    }
    size += 1;
    c = utf8[index++];
  }

  out = (wchar_t*)malloc((size + 1) * sizeof(wchar_t));
  if (out == NULL)
    return NULL;
  index = 0;

  c = utf8[index++];
  while (c)
  {
    if((c & 0x80) == 0) 
    {
      out[out_index++] = c;
    } 
    else if((c & 0xe0) == 0xe0) 
    {
      out[out_index] = (c & 0x1F) << 12;
      c = utf8[index++];
      out[out_index] |= (c & 0x3F) << 6;
      c = utf8[index++];
      out[out_index++] |= (c & 0x3F);
    }
    else
    {
      out[out_index] = (c & 0x3F) << 6;
      c = utf8[index++];
      out[out_index++] |= (c & 0x3F);
    }
    c = utf8[index++];
  }
  out[out_index] = 0;

  return out;
}


// Returns 0 on error, 1 on success
static int utf8_decode(const unsigned char *from, char **to)
{
  int nResult = 0;

//  _ASSERTE(!(*to)); // You passed a non-zero pointer, make sure it doesnt point to unfreed memory

  // Validate the string
  if (!UTF8_IsValid(from))
    return 0;

  // Use the native conversion routines when available
  if (bHasCP_UTF8)
  {
    WCHAR *wszTemp = NULL;
    int inlen = (int)strlennull((char*)from);

    wszTemp = (WCHAR *)_alloca(sizeof(WCHAR) * (inlen + 1));

    // Convert the UTF-8 string to UCS
    if (MultiByteToWideChar(CP_UTF8, 0, (char*)from, -1, wszTemp, inlen + 1))
    {
      // Convert the UCS string to local ANSI codepage
      *to = (char*)malloc(inlen+1);
      if (WideCharToMultiByte(CP_ACP, 0, wszTemp, -1, *to, inlen+1, NULL, NULL))
      {
        nResult = 1;
      }
      else
      {
        SAFE_FREE((void**)to);
      }
    }
  }
  else
  {
    wchar_t *unicode;
    int chars;
    int err;

    unicode = make_unicode_string(from);
    if (unicode == NULL)
    {
//      fprintf(stderr, "Out of memory processing string from UTF8 to UNICODE16\n");
      return 0;
    }

    chars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, NULL, 0, NULL, NULL);

    if(chars == 0)
    {
//      fprintf(stderr, "Unicode translation error %d\n", GetLastError());
      SAFE_FREE((void**)&unicode);
      return 0;
    }

    *to = (char*)malloc((chars + 1)*sizeof(unsigned char));
    if(*to == NULL)
    {
//      fprintf(stderr, "Out of memory processing string to local charset\n");
      SAFE_FREE((void**)&unicode);
      return 0;
    }

    err = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, unicode, -1, *to, chars, NULL, NULL);
    if (err != chars)
    {
//      fprintf(stderr, "Unicode translation error %d\n", GetLastError());
      SAFE_FREE((void**)&unicode);
      SAFE_FREE((void**)to);
      return 0;
    }

    SAFE_FREE((void**)&unicode);

    nResult = 1;
  }

  return nResult;
}



TCHAR* ansi_to_tchar(const char* src, int codepage)
{
  if (g_UnicodeCore)
  {
    WCHAR *unicode;
    int wchars, err;

    wchars = MultiByteToWideChar(codepage, MB_PRECOMPOSED, src, (int)strlennull(src), NULL, 0);

    if (wchars == 0) return NULL; // Failure

    unicode = (WCHAR*)malloc((wchars + 1) * sizeof(WCHAR));
    unicode[wchars] = 0;

    err = MultiByteToWideChar(codepage, MB_PRECOMPOSED, src, (int)strlennull(src), unicode, wchars);
    if (err != wchars)
    {
      SAFE_FREE((void**)&unicode);
      return NULL; // Failure
    }

    return (TCHAR*)unicode;
  }
  else
    return strdupT((TCHAR*)src);
}

char* tchar_to_ansi(const TCHAR* src)
{
  if (g_UnicodeCore)
  {
    char *ansi;
    int chars;
    int err;

    chars = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (WCHAR*)src, -1, NULL, 0, NULL, NULL);

    if (chars == 0) return NULL; // Failure

    ansi = (char*)malloc((chars + 1)*sizeof(char));
    if (ansi == NULL) return NULL; // Failure

    err = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, (WCHAR*)src, -1, ansi, chars, NULL, NULL);
    if (err != chars)
    {
      SAFE_FREE((void**)&ansi);
      return NULL;
    }
    return ansi;
  }
  else
    return (char*)strdupT(src);
}

TCHAR* utf8_to_tchar(const unsigned char* utf)
{
  if (g_UnicodeCore)
    return (TCHAR*)make_unicode_string(utf);
  else
  {
    char* szAnsi = NULL;

    if (utf8_decode(utf, &szAnsi)) 
      return (TCHAR*)szAnsi;
    else
      return NULL; // Failure
  }
}