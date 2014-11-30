/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (c) 2008-2012 Boris Krasnovskiy
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "aim.h"

#pragma warning( disable: 4706 )

char* process_status_msg (const char *str, const char* sn)
{
	const char *src = str;
	size_t size = strlen(src) + 1;
	char* res = (char*)mir_alloc(size);
	char* dest = res;
	size_t len = strlen(sn);

	for (; *src; ++src)
	{
		if (src[0] == '\n' && (src == str || src[-1] != '\r'))
		{
			int off = dest - res;
			res = (char*)mir_realloc(res, ++size);
			dest = res + off;
			*(dest++) = '\r';
			*(dest++) = *src;
		}
		else if (src[0] == '%' && src[1] == 'n')
		{
			int off = dest - res;
			res = (char*)mir_realloc(res, size + len);
			dest = res + off;
			size += len;
			memcpy(dest, sn, len);
			dest += len;
			++src;
		}
		else if (src[0] == '%' && src[1] == 'd')
		{
			int off = dest - res;
			res = (char*)mir_realloc(res, size + 20);
			dest = res + off;
			size += 20;
			dest += GetDateFormatA(LOCALE_USER_DEFAULT, 0, NULL, NULL, dest, 20)-1;
			++src;
		}
		else if (src[0] == '%' && src[1] == 't')
		{
			int off = dest - res;
			res = (char*)mir_realloc(res, size + 20);
			dest = res + off;
			size += 20;
			dest += GetTimeFormatA(LOCALE_USER_DEFAULT, 0, NULL, NULL, dest, 20)-1;
			++src;
		}
		else
			*(dest++) = *src;
	}
	*dest = '\0';
	return res;
}


void  html_decode(char* str)
{
	char *p, *q;
//    char *rstr = (char*)mir_alloc(strlen(str)*2);

	if (str == NULL) return;

	for ( p=q=str; *p!='\0'; p++,q++ ) 
	{
		if ( *p == '&' ) 
		{
			if      ( !strnicmp( p, "&amp;", 5 ))  { *q = '&';  p += 4; }
			else if ( !strnicmp( p, "&apos;", 6 )) { *q = '\''; p += 5; }
			else if ( !strnicmp( p, "&gt;", 4 ))   { *q = '>';  p += 3; }
			else if ( !strnicmp( p, "&lt;", 4 ))   { *q = '<';  p += 3; }
			else if ( !strnicmp( p, "&quot;", 6 )) { *q = '"';  p += 5; }
			else if ( *(p+1) == '#' ) 
			{ 
				char* s = strchr(p, ';');
				if (s)
				{
					wchar_t t[2] = { (wchar_t)atoi(p+2), 0 };
					char *t1 = mir_utf8encodeW(t);
					if (t1 && *t1)
					{
						strcpy(q, t1);
						q += strlen(t1) - 1;
					}
					mir_free(t1);
					p = s;
				}
				else
					*q = *p; 
			}
			else { *q = *p;	}
		}
		else if ( *p == '<' )
		{
			if      ( !strnicmp( p, "<p>",  3 )) { strcpy(q, "\r\n\r\n"); q += 3; p += 2; }
			else if ( !strnicmp( p, "</p>", 4 )) { strcpy(q, "\r\n\r\n"); q += 3; p += 3; }
			else if ( !strnicmp( p, "<br>", 4 )) { strcpy(q, "\r\n"); ++q; p += 3; }
			else if ( !strnicmp( p, "<br />", 6 )) { strcpy(q, "\r\n"); ++q; p += 5; }
			else if ( !strnicmp( p, "<hr>", 4 )) { strcpy(q, "\r\n"); ++q; p += 3; }
			else if ( !strnicmp( p, "<hr />", 6 )) { strcpy(q, "\r\n"); ++q; p += 5; }
/*			
			else if ( !strnicmp( p, "<b>",  3 )) { strcpy(q, "[b]");  q += 2; p += 2; }
			else if ( !strnicmp( p, "</b>", 4 )) { strcpy(q, "[/b]"); q += 3; p += 3; }
			else if ( !strnicmp( p, "<u>",  3 )) { strcpy(q, "[u]");  q += 2; p += 2; }
			else if ( !strnicmp( p, "</u>", 4 )) { strcpy(q, "[/u]"); q += 3; p += 3; }
			else if ( !strnicmp( p, "<i>",  3 )) { strcpy(q, "[i]");  q += 2; p += 2; }
			else if ( !strnicmp( p, "</i>", 4 )) { strcpy(q, "[/i]"); q += 3; p += 3; }
			else if ( !strnicmp( p, "<s>",  3 )) { strcpy(q, "[s]");  q += 2; p += 2; }
			else if ( !strnicmp( p, "</s>", 4 )) { strcpy(q, "[/s]"); q += 3; p += 3; }
			else if ( !strnicmp( p, "<a href", 7 )) { 
				strcpy(q, "[url"); q += 4; p += 7;
				while (*p != '>' && *p) *(q++) = *(p++);
				*q = ']'; --p;
			}
			else if ( !strnicmp( p, "<font", 5 )) {
				char *l = strchr(p, '>');

				strcpy(q, "[url"); q += 4; p += 7;
				while (*p != '>' && *p) *(q++) = *(p++);
				*q = ']'; --p;
			}
*/
			else { 
				char *l = strchr(p, '>');
				if (l) { p = l; --q; } else *q = *p; 
			}
		}
		else 
			*q = *p;
	}
	*q = '\0';
}


char* html_encode(const char* str)
{
	char* s, *q;
	const char *p;
	int c;

	if (str == NULL) return NULL;

	for ( c=0, p=str; *p!='\0'; p++ ) 
	{
		switch ( *p ) 
		{
		case '&' : c += 5; break;
		case '\'': c += 6; break;
		case '>' : c += 4; break;
		case '<' : c += 4; break;
		case '"' : c += 6; break;
		case '\n': c += 4; break;
		default: c++; break;
		}
	}

	s = (char*)mir_alloc(c + 27); 
	strcpy(s, "<HTML><BODY>");
	for ( p=str,q=s+12; *p!='\0'; p++ ) 
	{
		switch ( *p ) 
		{
		case '&' : memcpy( q, "&amp;", 5 );  q += 5; break;
//		case '\'': memcpy( q, "&apos;", 6 ); q += 6; break;
		case '>' : memcpy( q, "&gt;", 4 );   q += 4; break;
		case '<' : memcpy( q, "&lt;", 4 );   q += 4; break;
		case '"' : memcpy( q, "&quot;", 6 ); q += 6; break;
		case '\r': break;
		case '\n': memcpy( q, "<BR>", 4 );   q += 4; break;
		default: *q = *p; ++q; break;
		}
	}
	strcpy(q, "</BODY></HTML>");

	return s;
}

char* html_to_bbcodes(char *src)
{
	char *ptr;
	char *ptrl;
	char *rptr;
	char* dest = mir_strdup(src);
	while ((ptr = strstr(dest, "<B>")) != NULL || (ptr = strstr(dest, "<b>")) != NULL)
	{
		*ptr = '[';
		*(ptr+1) = 'b';
		*(ptr+2) = ']';
		if ((ptr = strstr(dest, "</B>")) != NULL || (ptr = strstr(dest, "</b>")) != NULL)
		{
			*ptr = '[';
			*(ptr+2) = 'b';
			*(ptr+3) = ']';
		}
		else
		{
			dest=(char*)mir_realloc(dest,strlen(dest)+6);
			memcpy(&dest[strlen(dest)],"[/b]",5);
		}
	}
	while ((ptr = strstr(dest, "<I>")) != NULL || (ptr = strstr(dest, "<i>")) != NULL)
	{
		*ptr =  '[';
		*(ptr+1) = 'i';
		*(ptr+2) = ']';
		if ((ptr = strstr(dest, "</I>")) != NULL || (ptr = strstr(dest, "</i>")) != NULL)
		{
			*ptr = '[';
			*(ptr+2) = 'i';
			*(ptr+3) = ']';
		}
		else
		{
			dest=(char*)mir_realloc(dest,strlen(dest)+6);
			memcpy(&dest[strlen(dest)],"[/i]",5);
		}
	}
	while ((ptr = strstr(dest, "<U>")) != NULL || (ptr = strstr(dest, "<u>")) != NULL)
	{
		*ptr = '[';
		*(ptr+1) = 'u';
		*(ptr+2) = ']';
		if ((ptr = strstr(dest, "</U>")) != NULL || (ptr = strstr(dest, "</u>")) != NULL)
		{
			*ptr = '[';
			*(ptr+2) = 'u';
			*(ptr+3) = ']';
		}
		else
		{
			dest=(char*)mir_realloc(dest,strlen(dest)+6);
			memcpy(&dest[strlen(dest)],"[/u]",5);
		}
	}
	while ((ptr = strstr(dest, "<S>")) != NULL || (ptr = strstr(dest, "<s>")) != NULL)
	{
		*ptr = '[';
		*(ptr+1) = 's';
		*(ptr+2) = ']';
		if ((ptr = strstr(dest, "</S>")) != NULL || (ptr = strstr(dest, "</s>")) != NULL)
		{
			*ptr = '[';
			*(ptr+2) = 's';
			*(ptr+3) = ']';
		}
		else
		{
			dest=(char*)mir_realloc(dest,strlen(dest)+6);
			memcpy(&dest[strlen(dest)],"[/s]",5);
		}
	}
	rptr = dest;
	while (ptr = strstr(rptr,"<A HREF"))
	{
		char* begin=ptr;
		ptrl = ptr + 4;
		memcpy(ptrl,"[url=",5);
		memmove(ptr, ptrl, strlen(ptrl) + 1);
		if ((ptr = strstr(ptrl,">")))
		{	
			ptr-=1;
			memmove(ptr, ptr+1, strlen(ptr+1) + 1);
			*(ptr)=']';
			ptrl-=1;
			char* s1 = strstr(ptrl,"</A");
			char* s2 = strstr(rptr,"<A HREF");
			if (s1&&s1<s2||s1&&!s2)
			{
				ptr=s1;
				ptr=strip_tag_within(begin,ptr);
				memmove(ptr+2, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/url]",6);
			}
			else if (s2&&s2<s1||s2&&!s1)
			{
				ptr=s2;
				ptr=strip_tag_within(begin,ptr);
				int addr=ptr-rptr;
				dest=(char*)mir_realloc(dest,strlen(dest)+8);
				rptr=dest;
				ptr=rptr+addr;
				memmove(ptr+6, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/url]",6);
			}
			else
			{
				strip_tag_within(begin,&dest[strlen(dest)]);
				//int addr=ptr-rptr;
				dest=(char*)mir_realloc(dest,strlen(dest)+8);
				rptr=dest;
				ptr=dest;
				memcpy(&ptr[strlen(ptr)],"[/url]",7);
			}
		}
		else
			rptr++;
	}
	rptr = dest;
	while (ptr = strstr(rptr,"<a href"))
	{
		char* begin=ptr;
		ptrl = ptr + 4;
		memcpy(ptrl,"[url=",5);
		memmove(ptr, ptrl, strlen(ptrl) + 1);
		if ((ptr = strstr(ptrl,">")))
		{
			ptr-=1;
			memmove(ptr, ptr+1, strlen(ptr+1) + 1);
			*(ptr)=']';
			ptrl-=1;
			char* s1 = strstr(ptrl,"</a");
			char* s2 = strstr(ptrl,"<a href");
			if (s1&&s1<s2||s1&&!s2)
			{
				ptr=s1;
				ptr=strip_tag_within(begin,ptr);
				memmove(ptr+2, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/url]",6);
			}
			else if (s2&&s2<s1||s2&&!s1)
			{
				ptr=s2;
				ptr=strip_tag_within(begin,ptr);
				int addr=ptr-rptr;
				dest=(char*)mir_realloc(dest,strlen(dest)+8);
				rptr=dest;
				ptr=rptr+addr;
				memmove(ptr+6, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/url]",6);
			}
			else
			{
				strip_tag_within(begin,&dest[strlen(dest)]);
				//int addr=ptr-rptr;
				dest=(char*)mir_realloc(dest,strlen(dest)+8);
				rptr=dest;
				ptr=dest;
				memcpy(&ptr[strlen(ptr)],"[/url]",7);
			}
		}
		else
			rptr++;
	}
	rptr = dest;
	while (ptr = strstr(rptr, "<FONT COLOR=\""))
	{
		int addr=ptr-rptr;
		dest=(char*)mir_realloc(dest,strlen(dest)+8);
		rptr=dest;
		ptr=rptr+addr;
		ptrl = ptr + 6;
		memcpy(ptrl,"[color=",7);
		memmove(ptr, ptrl, strlen(ptrl) + 1);
		if ((ptr = strstr(ptrl, ">")))
		{
			memmove(ptrl+7,ptr,strlen(ptr)+1);
			*(ptrl+7)=']';
			ptr=ptrl+7;
			char* s1 = strstr(ptr,"</FONT");
			char* s2 = strstr(ptr,"<FONT COLOR=\"");	
			if (s1&&s1<s2||s1&&!s2)
			{
				ptr=s1;
				memmove(ptr+1, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/color]",8);
			}
			else if (s2&&s2<s1||s2&&!s1)
			{
				ptr=s2;
				memmove(ptr+8, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/color]",8);
			}
			else
			{
				ptr=dest;
				memcpy(&ptr[strlen(ptr)],"[/color]",9);
			}
		}
		else
			rptr++;
	}
	rptr = dest;
	while (ptr = strstr(rptr, "<font color=\""))
	{
		int addr=ptr-rptr;
		dest=(char*)mir_realloc(dest,strlen(dest)+8);
		rptr=dest;
		ptr=rptr+addr;
		ptrl = ptr + 6;
		memcpy(ptrl,"[color=",7);
		memmove(ptr, ptrl, strlen(ptrl) + 1);
		if ((ptr = strstr(ptrl, ">")))
		{
			memmove(ptrl+7,ptr,strlen(ptr)+1);
			*(ptrl+7)=']';
			ptr=ptrl+7;
			char* s1 = strstr(ptr,"</font");
			char* s2 = strstr(ptr,"<font color=\"");	
			if (s1&&s1<s2||s1&&!s2)
			{
				ptr=s1;
				memmove(ptr+1, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/color]",8);
			}
			else if (s2&&s2<s1||s2&&!s1)
			{
				ptr=s2;
				memmove(ptr+8, ptr, strlen(ptr) + 1);
				memcpy(ptr,"[/color]",8);
			}
			else
			{
				ptr=dest;
				memcpy(&ptr[strlen(ptr)],"[/color]",9);
			}
		}
		else
			rptr++;
	}
	rptr = dest;
	while ((ptr = strstr(rptr, "<FONT COLOR=")) || (ptr = strstr(rptr, "<font color=")))
	{
		int addr=ptr-rptr;
		dest=(char*)mir_realloc(dest,strlen(dest)+8);
		rptr=dest;
		ptr=rptr+addr;
		ptrl = ptr + 5;
		memcpy(ptrl,"[color=",7);
		memmove(ptr, ptrl, strlen(ptrl) + 1);
		if ((ptr = strstr(ptrl, ">")))
		{
			*(ptr)=']';
			if ((ptrl = strstr(ptr, "</FONT")) || (ptrl = strstr(ptr, "</font")))
			{
				memmove(ptrl+1, ptrl, strlen(ptrl) + 1);
				memcpy(ptrl,"[/color]",8);
			}
			else
			{
				memcpy(&dest[strlen(dest)],"[/color]",9);
			}
		}
		else
			rptr++;
	}
/*
	rptr = dest;
	while ((ptr = strstr(rptr, "aim:")) || (ptr = strstr(rptr, "AIM:")))
	{
		int addr=ptr-rptr;
		int len=0;
		for (len
		dest=(char*)mir_realloc(dest,strlen(dest)+8);
		rptr=dest;
		ptr=rptr+addr;
		ptrl = ptr + 5;
		memcpy(ptrl,"[url=",7);
		memmove(ptr, ptrl, strlen(ptrl) + 1);
	}
*/
	return dest;
}

char* bbcodes_to_html(const char *src)
{
	char *ptr;
	char *rptr;
	char* dest = mir_strdup(src);
	while ((ptr = strstr(dest, "[b]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+1) = 'b';
		*(ptr+2) = '>';
	}
	while ((ptr = strstr(dest, "[/b]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+2) = 'b';
		*(ptr+3) = '>';
	}
	while ((ptr = strstr(dest, "[i]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+1) = 'i';
		*(ptr+2) = '>';
	}
	while ((ptr = strstr(dest, "[/i]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+2) = 'i';
		*(ptr+3) = '>';
	}
	while ((ptr = strstr(dest, "[u]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+1) = 'u';
		*(ptr+2) = '>';
	}
	while ((ptr = strstr(dest, "[/u]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+2) = 'u';
		*(ptr+3) = '>';
	}
	while ((ptr = strstr(dest, "[s]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+1) = 's';
		*(ptr+2) = '>';
	}
	while ((ptr = strstr(dest, "[/s]")) != NULL) 
	{
		*ptr = '<';
		*(ptr+2) = 's';
		*(ptr+3) = '>';
	}
	rptr = dest;
	while ((ptr = strstr(rptr, "[color=")))
	{
		int addr=ptr-rptr;
		dest=(char*)mir_realloc(dest,strlen(dest)+8);
		rptr=dest;
		ptr=rptr+addr;
		memmove(ptr+5, ptr, strlen(ptr) + 1);
		memcpy(ptr,"<font ",6);
		if ((ptr = strstr(ptr,"]")))
		{
			*(ptr)='>';
			if ((ptr = strstr(ptr,"[/color]")))
			{
				memcpy(ptr,"</font>",7);
				memmove(ptr+7,ptr+8,strlen(ptr+8)+1);
			}
		}
		else
			rptr++;
	}
	while ((ptr = strstr(rptr, "[url=")))
	{
		int addr=ptr-rptr;
		dest=(char*)mir_realloc(dest,strlen(dest)+8);
		rptr=dest;
		ptr=rptr+addr;
		memmove(ptr+3, ptr, strlen(ptr)+1);
		memcpy(ptr,"<a href",7);
		if ((ptr = strstr(ptr, "]")))
		{
			*(ptr)='>';
			if ((ptr = strstr(ptr, "[/url]")))
			{
				memcpy(ptr,"</a>",4);
				memmove(ptr+4,ptr+6,strlen(ptr+6)+1);
			}
		}
		else
			rptr++;
	}
	return dest;
}

void strip_tag(char* begin, char* end)
{
	memmove(begin,end+1,strlen(end+1)+1);
}

//strip a tag within a string
char* strip_tag_within(char* begin, char* end)
{
	while(char* sub_begin=strchr(begin,'<'))
	{	
		if (sub_begin<end)//less than the original ending
		{
			char* sub_end=strchr(begin,'>');
			strip_tag(sub_begin,sub_end);
			end=end-(sub_end-sub_begin)-1;
		}
		else
			break;
	}
	return end;
}

char* rtf_to_html(HWND hwndDlg,int DlgItem)
{
	char* buf=(char*)mir_alloc(4024);
	size_t pos=0;
	int start=0;
	int end=1;
	BOOL Bold=false;
	BOOL Italic=false;
	BOOL Underline=false;
	char Face[32]="";
	COLORREF Color;
	COLORREF BackColor;
	int Size=0;
	GETTEXTLENGTHEX tl;
	tl.flags=GTL_DEFAULT;
	tl.codepage=CP_ACP;

	int oldstart = 0, oldend = 0;
	SendDlgItemMessage(hwndDlg, DlgItem, EM_GETSEL, (WPARAM)&oldstart, (LPARAM)&oldend);

	int length=SendDlgItemMessage(hwndDlg, DlgItem, EM_GETTEXTLENGTHEX,(WPARAM)&tl,0);
	while(start<length)
	{
		SendDlgItemMessage(hwndDlg, DlgItem, EM_SETSEL, start, end);
		CHARFORMAT2A cfOld;
		cfOld.cbSize = sizeof(cfOld);
		cfOld.dwMask = CFM_BOLD|CFM_ITALIC|CFM_UNDERLINE|CFM_SIZE|CFM_COLOR|CFM_BACKCOLOR|CFM_FACE;
		SendDlgItemMessageA(hwndDlg, DlgItem, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfOld);
		BOOL isBold = (cfOld.dwEffects & CFE_BOLD) && (cfOld.dwMask & CFM_BOLD);
		BOOL isItalic = (cfOld.dwEffects & CFE_ITALIC) && (cfOld.dwMask & CFM_ITALIC);
		BOOL isUnderline = (cfOld.dwEffects & CFE_UNDERLINE) && (cfOld.dwMask & CFM_UNDERLINE);
		COLORREF isColor=cfOld.crTextColor;
		COLORREF isBackColor=cfOld.crBackColor;
		int isSize;
		if (cfOld.yHeight==38*20)
			isSize=7;
		else if (cfOld.yHeight==24*20)
			isSize=6;
		else if (cfOld.yHeight==18*20)
			isSize=5;
		else if (cfOld.yHeight==14*20)
			isSize=4;
		else if (cfOld.yHeight==12*20)
			isSize=3;
		else if (cfOld.yHeight==10*20)
			isSize=2;
		else if (cfOld.yHeight==8*20)
			isSize=1;
		else
			isSize=3;
		TCHAR text[3] =_T("");
		SendDlgItemMessage(hwndDlg, DlgItem, EM_GETSELTEXT, 0, (LPARAM)&text);
		if (Bold!=isBold)
		{
			Bold=isBold;
			if (isBold)
			{
				strcpy(&buf[pos],"<b>");
				pos+=3;
			}
			else
			{
				if (start!=0)
				{
					strcpy(&buf[pos],"</b>");
					pos+=4;	
				}
			}
		}
		if (Italic!=isItalic)
		{
			Italic=isItalic;
			if (isItalic)
			{
				strcpy(&buf[pos],"<i>");
				pos+=3;
			}
			else
			{
				if (start!=0)
				{
					strcpy(&buf[pos],"</i>");
					pos+=4;	
				}
			}
		}
		if (Underline!=isUnderline)
		{
			Underline=isUnderline;
			if (isUnderline)
			{
				strcpy(&buf[pos],"<u>");
				pos+=3;
			}
			else
			{
				if (start!=0)
				{
					strcpy(&buf[pos],"</u>");
					pos+=4;	
				}
			}
		}
		if ( Size != isSize || Color != isColor || BackColor != isBackColor || mir_strcmp( Face, cfOld.szFaceName ))
		{
			Size=isSize;
			Color=isColor;
			BackColor=isBackColor;
			strcpy(Face,cfOld.szFaceName);
			if (start!=0)
			{
				strcpy(&buf[pos],"</font>");
				pos+=7;
			}
			strcpy(&buf[pos],"<font");
			pos+=5;
			strcpy(&buf[pos],"	face=\"");
			pos+=7;
			strcpy(&buf[pos],Face);
			pos+=strlen(Face);
			strcpy(&buf[pos],"\"");
			pos++;
			if (!(cfOld.dwEffects & CFE_AUTOBACKCOLOR))
			{
				strcpy(&buf[pos]," back=#");
				pos+=6;
				char chBackColor[7];
				_itoa((_htonl(BackColor)>>8),chBackColor,16);
				size_t len=strlen(chBackColor);
				if (len<6)
				{
					memmove(chBackColor+(6-len),chBackColor,len+1);
					for(int i=0;i<6;i++)
						chBackColor[i]='0';
				}
				strcpy(&buf[pos],chBackColor);
				pos+=6;
			}
			if (!(cfOld.dwEffects & CFE_AUTOCOLOR))
			{
				strcpy(&buf[pos]," color=#");
				pos+=8;
				char chColor[7];
				_itoa((_htonl(Color)>>8),chColor,16);
				size_t len=strlen(chColor);
				if (len<6)
				{
					memmove(chColor+(6-len),chColor,len+1);
					for(int i=0;i<6;i++)
						chColor[i]='0';
				}
				strcpy(&buf[pos],chColor);
				pos+=6;
			}
			strcpy(&buf[pos]," size=");
			pos+=6;
			char chSize[2];
			_itoa(Size,chSize,10);
			strcpy(&buf[pos],chSize);
			pos++;

			strcpy(&buf[pos],">");
			pos++;
		}
		if (text[0]=='\r')
		{
			strcpy(&buf[pos],"<br>");
			pos+=4;
		}
		else
		{
			char* txt = mir_utf8encodeT(text);
			strcpy(&buf[pos], txt);
			pos += strlen(txt);
			mir_free(txt);
		}
		start++;
		end++;
	}
	if (Bold)
	{
		strcpy(&buf[pos],"</b>");
		pos+=4;	
	}
	if (Italic)
	{
		strcpy(&buf[pos],"</i>");
		pos+=4;	
	}
	if (Underline)
	{
		strcpy(&buf[pos],"</u>");
		pos+=4;	
	}
	strcpy(&buf[pos],"</font>");
	pos+=7;

	SendDlgItemMessage(hwndDlg, DlgItem, EM_SETSEL, oldstart, oldend);

	return buf;
}

void wcs_htons(wchar_t * ch)
{
	if (ch == NULL) return;
	for(size_t i=0;i<wcslen(ch);i++)
		ch[i]=_htons(ch[i]);
}

char* bytes_to_string(char* bytes, int num_bytes)
{
	if (num_bytes == 0) return NULL;

	char* string = (char*)mir_alloc(num_bytes*2+1);
	for(int i=0;i<num_bytes;i++)
	{
		char store[2];
		unsigned char bit=(bytes[i]&0xF0)>>4;
		_itoa(bit,store,16);
		memcpy(&string[i*2],store,1);
		bit=(bytes[i]&0x0F);
		_itoa(bit,store,16);
		memcpy(&string[i*2+1],store,1);
	}
	string[num_bytes*2]='\0';
	return string;
}

void string_to_bytes(char* string, char* bytes)
{
	char sbyte[3];
	sbyte[2]='\0';
	size_t length=strlen(string);
	for (size_t i=0; i<length; i+=2)
	{
		sbyte[0]=string[i];
		sbyte[1]=string[i+1];
		bytes[i/2]=(char)strtol(sbyte,NULL,16);
	}
}

bool is_utf(const char* msg)
{
	bool res = false;
	if (msg)
	{
		for (unsigned i=0; !res; ++i)
		{
			char c = msg[i];
			if (c == 0) break;
			res = (c & 0x80) != 0;
		}
	}
	return res;
}

char* get_fname(char* path)
{
	char* pszFile = strrchr(path, '\\');
	if (pszFile) pszFile++; else pszFile = path;

	return pszFile;
}

TCHAR* get_dir(TCHAR* path)
{
	TCHAR *cpath = mir_tstrdup(path);

	TCHAR* swd = _tcsrchr(cpath, '\\'); 
	if (swd) swd[1] = 0; else cpath[0] = 0;

	return cpath;
}

aimString::aimString(char* str)
{
	if (str == NULL)
	{
		szString = NULL;
		size = 0;
		unicode = false;
	}
	else
	{
		unicode = is_utf(str);
		if (unicode)
		{
			wszString = mir_utf8decodeW(str);
			wcs_htons(wszString);
			size = wcslen(wszString) * sizeof(wchar_t);
		}
		else
		{
			szString = mir_utf8decodeA(str);
			size = strlen(szString);
		}
	}
}

#pragma warning( default: 4706 )
