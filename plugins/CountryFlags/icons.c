/*
Miranda IM Country Flags Plugin
Copyright (C) 2006-1007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (Flags-License.txt); if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "flags.h"

extern HINSTANCE hInst;
extern int nCountriesCount;
extern struct CountryListEntry *countries;
static HANDLE hServiceLoadIcon,hServiceCreateMergedIcon;

/************************* Bitmap Access **************************/

static HANDLE *phIconHandles;

static int FASTCALL CountryNumberToBitmapIndex(int countryNumber)
{
	/* country number indices (same order as in flags.bmp) */
	const int BitmapIndexMap[232]={
		 0,    1,   7,  20,  27,  30,  31,  32,  33,  34,  36,  39,  40,  41,  43,  44,  45,  46,  47,  48,
		49,   51,  52,  53,  54,  55,  56,  57,  58,  60,  61,  62,  63,  64,  65,  66,  81,  82,  84,  86,
		90,   91,  92,  93,  94,  95,  98, 101, 102, 103, 104, 105, 106, 107, 178, 108, 109, 110, 111, 112,
		113, 116, 117, 118, 121, 122, 123, 212, 213, 216, 218, 220, 221, 222, 223, 224, 225, 226, 227, 228,
		229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 248, 249,
		250, 251, 252, 253, 254, 255, 256, 257, 258, 260, 261, 263, 264, 265, 266, 267, 268, 269, 290, 291,
		297, 298, 299, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 370, 371, 372, 373, 374, 375, 376,
		377, 378, 379, 380, 381, 382, 385, 386, 387, 389, 420, 421, 441, 442, 500, 501, 502, 503, 504, 505,
		506, 507, 508, 509, 590, 591, 592, 593, 595, 596, 597, 598, 599, 670, 671, 672, 673, 674, 675, 676,
		677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 705, 706, 708, 709,
		711, 850, 852, 853, 855, 856, 880, 886, 960, 961, 962, 963, 964, 965, 966, 967, 968, 971, 972, 973,
		974, 975, 976, 977, 994, 995,1141,2691,3811,4101,6101,6722
	};
	/* shared flags by multiple countries */
	switch(countryNumber) {
		case 262:               /* Reunion Island */
		case 594:               /* French Guiana */
		case 5901:              /* French Antilles */
			countryNumber=33;   /* France */
			break;
		case 120:               /* Barbuda */
			countryNumber=102;  /* Antigua and Barbuda */
			break;
		case 6702:              /* Tinian Island */
		case 6701:              /* Rota Island */
			countryNumber=670;  /* Saipan Island (Northern Mariana Islands) */
			break;
		case 115:               /* Saint Kitts */
		case 114:               /* Nevis */
			countryNumber=1141; /* Saint Kitts and Nevis */
			break;
		case 247:               /* Ascension Island */
			countryNumber=44;   /* United Kingdom */
			break;
		case 6721:              /* Australian Antarctic Territory */
			countryNumber=61;   /* Australia */
			break;
		case 5399:              /* Guantanamo Bay */
			countryNumber=1;    /* USA */
	}
	/* binary search in index array */
	{	int low=0,i,high;
		high=SIZEOF(BitmapIndexMap)-1;
		if(countryNumber<=BitmapIndexMap[high])
			while(low<=high) {
				i=low+((high-low)/2);
				/* never happens */
				if(i<0 || i>=SIZEOF(BitmapIndexMap)) DebugBreak();
				if(BitmapIndexMap[i]==countryNumber) return i;
				if(countryNumber>BitmapIndexMap[i]) low=i+1;      
				else high=i-1;
			}
	}
	/* Other,Unknown,Unspecified */
	return 0;
}

// return value needs to be released using DestroyIcon()
// only operates on color icons, which isn't a problem here
static HICON FASTCALL ResizeIconCentered(HICON hIcon,int cx,int cy)
{
	HICON hResIcon=NULL;
	ICONINFO icoi;
	BITMAP bm;
	register HDC hdc;
	HBITMAP hbmPrev,hbm;
	POINT pt;
	hdc=CreateCompatibleDC(NULL);
	if(hdc!=NULL) {
		if(GetIconInfo(hIcon,&icoi)) {
			if(GetObject(icoi.hbmColor,sizeof(bm),&bm) && bm.bmWidth<=cx && bm.bmHeight<=cy) {
				pt.x=(cx-bm.bmWidth)/2;
				pt.y=(cy-bm.bmHeight)/2;
				hbmPrev=SelectObject(hdc,icoi.hbmColor);
				if(hbmPrev!=NULL) { /* error on select? */
					hbm=icoi.hbmColor;
					icoi.hbmColor=CreateCompatibleBitmap(hdc,cx,cy);
					if(icoi.hbmColor!=NULL)
						if(SelectObject(hdc,icoi.hbmColor)!=NULL) { /* error on select? */
							DeleteObject(hbm); /* delete prev color (XOR) */
							if(BitBlt(hdc,0,0,cx,cy,NULL,0,0,BLACKNESS)) /* transparency: AND=0, XOR=1 */
								if(DrawIconEx(hdc,pt.x,pt.y,hIcon,bm.bmWidth,bm.bmHeight,0,NULL,DI_IMAGE|DI_NOMIRROR)) {
									if(SelectObject(hdc,icoi.hbmMask)!=NULL) { /* error on select? */
										hbm=icoi.hbmMask;
										icoi.hbmMask=CreateBitmap(cx,cy,1,1,NULL); /* mono */
										if(icoi.hbmMask!=NULL)
											if(SelectObject(hdc,icoi.hbmMask)!=NULL) { /* error on select? */
												DeleteObject(hbm); /* delete prev mask (AND) */
												if(BitBlt(hdc,0,0,cx,cy,NULL,0,0,WHITENESS)) /* transparency: AND=0, XOR=1 */
													if(DrawIconEx(hdc,pt.x,pt.y,hIcon,0,0,0,NULL,DI_MASK|DI_NOMIRROR)) {
														SelectObject(hdc,hbmPrev);
														hResIcon=CreateIconIndirect(&icoi); /* bitmaps must not be selected */
													}
										}
									}
								}
						}
					SelectObject(hdc,hbmPrev);
				}
			}
			DeleteObject(icoi.hbmColor);
			DeleteObject(icoi.hbmMask);
		}
		DeleteDC(hdc);
	}
	return hResIcon;
}

/************************* Utils **********************************/

HICON FASTCALL LoadFlagIcon(int countryNumber)
{
	char szId[20],*szCountry;
	/* create identifier */
	szCountry=(char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber,0);
	if(szCountry==NULL) szCountry=(char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber=0xFFFF,0);
	wsprintfA(szId,(countryNumber==0xFFFF)?"%s0x%X":"%s%i","flags_",countryNumber); /* buffer safe */
	return (HICON)CallService(MS_SKIN2_GETICON,0,(LPARAM)szId);
}

int FASTCALL CountryNumberToIndex(int countryNumber)
{
	int i,nf=0;
	for(i=0;i<nCountriesCount;++i) {
		if(countries[i].id==countryNumber) return i;
		if(countries[i].id==0xFFFF) nf=i;
	}
	return nf; /* Unknown */

}

/************************* Services *******************************/

static INT_PTR ServiceLoadFlagIcon(WPARAM wParam,LPARAM lParam)
{
	/* return handle */
	if ((BOOL)lParam) {
		if(phIconHandles==NULL) return (int)(HANDLE)NULL;
		return (int)phIconHandles[CountryNumberToIndex((int)wParam)];
	}
	/* return icon */
	return (int)LoadFlagIcon(wParam);
}

static INT_PTR ServiceCreateMergedFlagIcon(WPARAM wParam,LPARAM lParam)
{
	HICON hUpperIcon,hLowerIcon;
	ICONINFO icoi;
	BITMAP bm;
	HDC hdc;
	POINT aptTriangle[3];
	HICON hIcon=NULL;
	HRGN hrgn;
	HBITMAP hbmPrev;
	/* load both icons */
	hLowerIcon=(HICON)ServiceLoadFlagIcon((WPARAM)lParam,0);
	if(hLowerIcon==NULL) return (int)(HICON)NULL;
	hUpperIcon=(HICON)ServiceLoadFlagIcon(wParam,0);
	/* merge them */
	if(GetIconInfo(hLowerIcon,&icoi)) {
		if(hUpperIcon!=NULL && GetObject(icoi.hbmColor,sizeof(bm),&bm)) {
			hdc=CreateCompatibleDC(NULL);
			if(hdc!=NULL) {
				ZeroMemory(&aptTriangle,sizeof(aptTriangle));
				aptTriangle[1].y=bm.bmHeight-1;
				aptTriangle[2].x=bm.bmWidth-1;
				hrgn=CreatePolygonRgn(aptTriangle,SIZEOF(aptTriangle),WINDING);
				if(hrgn!=NULL) {
					SelectClipRgn(hdc,hrgn);
					DeleteObject(hrgn);
					hbmPrev=SelectObject(hdc,icoi.hbmColor);
					if(hbmPrev!=NULL) {  /* error on select? */
						if(DrawIconEx(hdc,0,0,hUpperIcon,bm.bmWidth,bm.bmHeight,0,NULL,DI_NOMIRROR|DI_IMAGE))
							if(SelectObject(hdc,icoi.hbmMask)!=NULL) /* error on select? */
								DrawIconEx(hdc,0,0,hUpperIcon,bm.bmWidth,bm.bmHeight,0,NULL,DI_NOMIRROR|DI_MASK);
						SelectObject(hdc,hbmPrev);
					}
					DeleteObject(hrgn);
				}
				DeleteDC(hdc);
			}
		}
		/* create icon */
		hIcon=CreateIconIndirect(&icoi);
		DeleteObject(icoi.hbmColor);
		DeleteObject(icoi.hbmMask);
	}
	return (int)hIcon;
}

/************************* Misc ***********************************/

void InitIcons(void)
{
	HIMAGELIST himl;
	SKINICONDESC sid;
	char szId[20];
	int i,index;
	HICON hIcon;

	WCHAR szName[64];
	LCID locale;
	locale=(LCID)CallService(MS_LANGPACK_GETLOCALE,0,0);


	/* register icons */
	ZeroMemory(&sid,sizeof(sid));
	sid.cbSize=sizeof(sid);
	sid.pszName=szId;
	sid.cx=GetSystemMetrics(SM_CXSMICON); 
	sid.cy=GetSystemMetrics(SM_CYSMICON);
	sid.flags=SIDF_SORTED|SIDF_TCHAR;
	sid.ptszSection=TranslateT("Country Flags");
	/* all those flag icons do not need any transparency mask (flags are always opaque),
	 * storing them in a large bitmap to reduce file size */
	himl=ImageList_LoadImage(hInst,MAKEINTRESOURCE(IDB_FLAGS),sid.cx,0,CLR_NONE,IMAGE_BITMAP,LR_CREATEDIBSECTION);
	if(himl!=NULL) {
		phIconHandles=(HANDLE*)mir_alloc(nCountriesCount*sizeof(HANDLE));
		if(phIconHandles!=NULL)
			for(i=0;i<nCountriesCount;++i) {

				MultiByteToWideChar(locale,0,countries[i].szName,-1,szName,SIZEOF(szName));
				szName[SIZEOF(szName)-1]=L'\0';
				sid.pwszDescription=TranslateW(szName);

				/* create identifier */
				wsprintfA(szId,(countries[i].id==0xFFFF)?"%s0x%X":"%s%i","flags_",countries[i].id); /* buffer safe */
				index=CountryNumberToBitmapIndex(countries[i].id);
				/* create icon */
				hIcon=ImageList_GetIcon(himl,index,ILD_NORMAL);
				sid.hDefaultIcon=(hIcon!=NULL)?ResizeIconCentered(hIcon,sid.cx,sid.cy):NULL;
				if(hIcon!=NULL) DestroyIcon(hIcon);
				index=CountryNumberToIndex(countries[i].id);
				phIconHandles[index] = Skin_AddIcon(&sid);
				if(sid.hDefaultIcon!=NULL) DestroyIcon(sid.hDefaultIcon);
			}
		ImageList_Destroy(himl);
	}
	/* create services */
	hServiceLoadIcon=CreateServiceFunction(MS_FLAGS_LOADFLAGICON,ServiceLoadFlagIcon);
	hServiceCreateMergedIcon=CreateServiceFunction(MS_FLAGS_CREATEMERGEDFLAGICON,ServiceCreateMergedFlagIcon);
}

void UninitIcons(void)
{
	DestroyServiceFunction(hServiceLoadIcon);
	DestroyServiceFunction(hServiceCreateMergedIcon);
}
