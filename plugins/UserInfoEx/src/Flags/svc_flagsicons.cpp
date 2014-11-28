/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

part of this code based on:
Miranda IM Country Flags Plugin Copyright ©2006-2007 H. Herkenrath

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "..\commonheaders.h"

/************************* Bitmap Access **************************/

static int CountryNumberToBitmapIndex(int countryNumber)
{
	/* country number indices (same order as in flags.bmp) */
	const int BitmapIndexMap[239]={
		0,	1,	7,	20,	27,	30,	31,	32,	33,	34,
		36,	38,	39,	40,	41,	43,	44,	45,	46,	47,
		48,	49,	51,	52,	53,	54,	55,	56,	57,	58,
		60,	61,	62,	63,	64,	65,	66,	81,	82,	84,
		86,	90,	91,	92,	93,	94,	95,	98,	212,	213,
		216,	218,	220,	221,	222,	223,	224,	225,	226,	227,
		228,	229,	230,	231,	232,	233,	234,	235,	236,	237,
		238,	239,	240,	241,	242,	243,	244,	245,	246,	247,
		248,	249,	250,	251,	252,	253,	254,	255,	256,	257,
		258,	260,	261,	263,	264,	265,	266,	267,	268,	269,
		290,	291,	297,	298,	299,	350,	351,	352,	353,	354,
		355,	356,	357,	358,	359,	370,	371,	372,	373,	374,
		375,	376,	377,	378,	379,	380,	381,	382,	383,	385,
		386,	387,	389,	420,	421,	423,	441,	442,	500,	501,
		502,	503,	504,	505,	506,	507,	508,	509,	591,	592,
		593,	595,	597,	598,	599,	614,	670,	673,	674,	675,
		676,	677,	678,	679,	680,	681,	682,	683,	685,	686,
		688,	689,	690,	691,	692,	705,	850,	852,	853,	855,
		856,	880,	886,	960,	961,	962,	963,	964,	965,	966,
		967,	968,	971,	972,	973,	974,	975,	976,	977,	992,
		993,	994,	995,	996,	998,	1002,	1242,	1246,	1264,	1268,
		1284,	1340,	1345,	1441,	1473,	1649,	1664,	1670,	1671,	1684,
		1758,	1767,	1784,	1809,	1868,	1869,	1876,	1939,	2897,	3492,
		3883,	5995,	5999,	6720,	6723,	9999,	61891,	65535,	90392
	};
	/* shared flags by multiple countries */
	switch(countryNumber) {
		case 9999:              /* Other */
		case 65535:             /* 0xFFFF,"Unknown" */
			countryNumber=0;    /* Unspecified */
			break;
		case 262:               /* Reunion Island */
		case 594:               /* French Guiana */
		case 5901:              /* remove French Antilles */
			countryNumber=33;   /* France */
			break;
		case 120:               /* remove Barbuda */
			countryNumber=1268; /* Antigua and Barbuda */
			break;
		case 6702:              /* removed Tinian Island */
		case 6701:              /* removed Rota Island */
		case 670:               /* removed Saipan Island */
			countryNumber=1670;	/* Northern Mariana Islands, US Territory of*/
			break;
		case 115:               /* removed Saint Kitts */
		case 114:               /* removed Nevis */
			countryNumber=1869; /* Saint Kitts and Nevis */
			break;
		case 247:               /* Ascension Island */
			countryNumber=44;   /* United Kingdom */
			break;
		case 6720:              /* Australian Antarctic Territory */
			countryNumber=61;   /* Australia */
			break;
		case 5997:              /* Netherlands (Bonaire Island)*/
		case 59946:             /* Netherlands (Saba Island) */
		case 59938:             /* Netherlands (St. Eustatius Island) */
			countryNumber=599;  /* Netherlands Antilles (dissolved 2010) */
			//countryNumber=31;   /* Netherlands */
			break;
		case 5995:              /* St. Maarten (new country in 2010 (from Netherlands Antilles) new country in 2010 (from Netherlands Antilles)) */
			countryNumber=599;  /* Netherlands Antilles (dissolved 2010) */
			break;
		case 5999:              /* Curacao (new country in 2010 (from Netherlands Antilles) new country in 2010 (from Netherlands Antilles)) */
			countryNumber=599;  /* Netherlands Antilles (dissolved 2010) */
			break;
		case 5399:              /* missing Guantanamo Bay */
			countryNumber=1;    /* USA */
	}
	/* binary search in index array */
	if (countryNumber > 0) {
		int i,high/*,low=0*/;
		high=SIZEOF(BitmapIndexMap);
		/*old code need sortet BitmapIndexMap*/
		/*if (countryNumber<=BitmapIndexMap[high])
			while(low<=high) {
				i=low+((high-low)/2);
				// never happens
				if (i<0 || i>=SIZEOF(BitmapIndexMap)) DebugBreak();
				if (BitmapIndexMap[i]==countryNumber) return i;
				if (countryNumber>BitmapIndexMap[i]) low=i+1;      
				else high=i-1;
			}*/
		for ( i=0; i < high; i++ ) {
			if (BitmapIndexMap[i]==countryNumber) return i;
		}
	}
	/* Other,Unknown,Unspecified */
	return 0;
}

/************************* Utils **********************************/

static HANDLE *phIconHandles = NULL;

HICON LoadFlag(int countryNumber)
{
	/* create identifier */
	char *szCountry = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber,0);
	if (szCountry == NULL)
		szCountry = (char*)CallService(MS_UTILS_GETCOUNTRYBYNUMBER,countryNumber=0xFFFF,0);

	char szId[20];
	mir_snprintf(szId, SIZEOF(szId), (countryNumber == 0xFFFF) ? "%s_0x%X" : "%s_%i", "flags", countryNumber); /* buffer safe */
	return Skin_GetIcon(szId);
}

HANDLE LoadFlagHandle(int countryNumber)
{
	if (phIconHandles == NULL)
		return NULL;
		
	return phIconHandles[ CountryNumberToIndex(countryNumber) ];
}

int CountryNumberToIndex(int countryNumber)
{
	int nf = 0;
	for(int i=0; i < nCountriesCount; i++) {
		if (countries[i].id == countryNumber) return i;
		if (countries[i].id == 0xFFFF) nf=i;
	}
	return nf; /* Unknown */
}

FIBITMAP* ConvertTo(FIBITMAP* dib, UINT destBits, bool greyscale)
{
	FIBITMAP* dib_res = NULL;
	switch (destBits) {
		case 8:
			// convert to 8Bits
			if (greyscale) {
				dib_res = FIP->FI_ConvertTo8Bits(dib);
			}
			else {
				FIBITMAP* dib_tmp = FIP->FI_ConvertTo24Bits(dib);
				if (dib_tmp) {
					dib_res = FIP->FI_ColorQuantize(dib_tmp, FIQ_WUQUANT/*FIQ_NNQUANT*/);
					FIP->FI_Unload(dib_tmp);
				}
			} break;
		case 16:
			// convert to 16Bits
			dib_res = FIP->FI_ConvertTo16Bits555(dib);
			break;
		case 24:
			// convert to 24Bits
			dib_res = FIP->FI_ConvertTo24Bits(dib);
			break;
		case 32:
			// convert to 32Bits
			dib_res = FIP->FI_ConvertTo32Bits(dib);
			break;
		default:
			break;
	}
	FIP->FI_Unload(dib);
	return dib_res;
}

FIBITMAP* LoadResource(UINT ID, LPTSTR lpType)
{
	FIBITMAP *dib      = NULL;
	if (lpType) {
		HRSRC	hResInfo	= FindResource(ghInst,MAKEINTRESOURCE(ID),lpType);
		DWORD	ResSize		= SizeofResource(ghInst,hResInfo);
		HGLOBAL	hRes		= LoadResource(ghInst,hResInfo);
		BYTE*	buffer		= (BYTE*)LockResource(hRes);
		if (buffer)
		{
			// attach the binary data to a memory stream
			FIMEMORY *hmem = FIP->FI_OpenMemory(buffer, ResSize);
			// get the file type
			FREE_IMAGE_FORMAT fif = FIP->FI_GetFileTypeFromMemory(hmem, 0);
			// load an image from the memory stream
			dib = FIP->FI_LoadFromMemory(fif, hmem, 0);
			// always close the memory stream
			FIP->FI_CloseMemory(hmem);

			UnlockResource(buffer);
		}
		FreeResource(hRes);
	}
	else {
		HBITMAP hScrBM = 0;
		if (NULL == (hScrBM = (HBITMAP)LoadImage(ghInst,MAKEINTRESOURCE(ID), IMAGE_BITMAP, 0, 0,LR_SHARED)))
			return dib;
		dib = FIP->FI_CreateDIBFromHBITMAP(hScrBM);
		DeleteObject(hScrBM);
	}
	return dib;
}

/************************* Services *******************************/

static INT_PTR ServiceLoadFlagIcon(WPARAM wParam,LPARAM lParam)
{
	/* return handle */
	if ((BOOL)lParam) {
		if (phIconHandles==NULL) return NULL;
		return (INT_PTR)phIconHandles[CountryNumberToIndex((int)wParam)];
	}
	/* return icon */
	return (INT_PTR)LoadFlag(wParam);
}

static INT_PTR ServiceCreateMergedFlagIcon(WPARAM wParam,LPARAM lParam)
{
	//TODO: use freeimage to create merget icon and add RGB(A) support
	HICON hUpperIcon,hLowerIcon;
	ICONINFO icoi;
	BITMAP bm;
	HDC hdc;
	POINT aptTriangle[3];
	HICON hIcon=NULL;
	HRGN hrgn;
	HBITMAP hbmPrev;
	/* load both icons */
	if (NULL == (hLowerIcon=(HICON)ServiceLoadFlagIcon((WPARAM)lParam,0))) return NULL;
	hUpperIcon=(HICON)ServiceLoadFlagIcon(wParam,0);
	/* merge them */
	if (GetIconInfo(hLowerIcon,&icoi)) {
		if (hUpperIcon!=NULL && GetObject(icoi.hbmColor,sizeof(bm),&bm)) {
			hdc=CreateCompatibleDC(NULL);
			if (hdc!=NULL) {
				ZeroMemory(&aptTriangle,sizeof(aptTriangle));
				aptTriangle[1].y=bm.bmHeight-1;
				aptTriangle[2].x=bm.bmWidth-1;
				hrgn=CreatePolygonRgn(aptTriangle,SIZEOF(aptTriangle),WINDING);
				if (hrgn!=NULL) {
					SelectClipRgn(hdc,hrgn);
					hbmPrev=(HBITMAP)SelectObject(hdc,icoi.hbmColor);
					if (hbmPrev!=NULL) {  /* error on select? */
						if (DrawIconEx(hdc,0,0,hUpperIcon,bm.bmWidth,bm.bmHeight,0,NULL,DI_NOMIRROR|DI_IMAGE)) {
							if (SelectObject(hdc,icoi.hbmMask)!=NULL) /* error on select? */
								DrawIconEx(hdc,0,0,hUpperIcon,bm.bmWidth,bm.bmHeight,0,NULL,DI_NOMIRROR|DI_MASK);
						}
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
	return (INT_PTR)hIcon;
}

/************************* Misc ***********************************/

void InitIcons()
{
	/* all those flag icons storing in a large 24bit opaque bitmap to reduce file size */
	FIBITMAP *dib = LoadResource(IDB_FLAGSPNG, _T("PNG"));
	if (dib == NULL)
		return;

	UINT bit = ILC_COLOR32, bitDest = ILC_COLOR32;

	if (FIP->FI_GetBPP(dib) != bit)
	if (NULL == (dib = ConvertTo(dib, bit, 0)))
		return;

	//create new dib
	FIBITMAP *dib_ico = FIP->FI_Allocate(FIP->FI_GetWidth(dib), 16, bit, 0, 0, 0);
	if (dib_ico == NULL) {
		FIP->FI_Unload(dib);
		return;
	}

	//	res = FIP->FI_IsTransparent(dib_ico);
	if (bit < 32) {
		//disable transparency
		FIP->FI_SetTransparent(dib, FALSE);
		FIP->FI_SetTransparent(dib_ico, FALSE);
	}

	UINT h = FIP->FI_GetHeight(dib_ico);
	UINT w = FIP->FI_GetWidth(dib_ico);
	UINT t = ((h - FIP->FI_GetHeight(dib)) / 2) + 1;
	UINT b = t + FIP->FI_GetHeight(dib);
	HBITMAP hbmMask = 0;

	//copy dib to new dib_ico (centered)
	if (FIP->FI_Paste(dib_ico, dib, 0, t - 1, 255 + 1)) {
		FIP->FI_Unload(dib);	dib = NULL;
		switch (bitDest) {
		case 8:
		case 16:
		case 24:
			{//transparency by 1bit monocrome icon mask (for bit < 32)
				FIBITMAP *dib_mask;
				if (NULL == (dib_mask = FIP->FI_Allocate(w, h, 1, 0, 0, 0))) {
					FIP->FI_Unload(dib_ico);
					return;
				}
				for(unsigned y = 0; y < h; y++)
					for(unsigned x = 0; x < w; x++) {
						BYTE value = ((y<t || y >= b) ? 0 : 1);
						FIP->FI_SetPixelIndex(dib_mask, x, y, &value);
					}
				hbmMask = FIP->FI_CreateHBITMAPFromDIB(dib_mask);
				FIP->FI_Unload(dib_mask);

				//convert to target resolution
				if (!hbmMask || !(dib_ico = ConvertTo(dib_ico, bitDest, 0))) {
					FIP->FI_Unload(dib_ico);
					if (hbmMask) DeleteObject(hbmMask);
					return;
				}
			} break;
			case 32:
			{//transparency by alpha schannel
				//Calculate the number of bytes per pixel (3 for 24-bit or 4 for 32-bit)
				int bytespp = FIP->FI_GetLine(dib_ico) / w;
				//set alpha schannel
				for(unsigned y = 0; y < h; y++) {
					BYTE *bits = FIP->FI_GetScanLine(dib_ico, y);
					for(unsigned x = 0; x < w; x++) {
						bits[FI_RGBA_ALPHA] = (y<t || y>=b)? 0:255;
						// jump to next pixel
						bits += bytespp;
					}
				}
			} break;
			default:
				FIP->FI_Unload(dib_ico);
				return;
		}
	}
	else {
		FIP->FI_Unload(dib);
		FIP->FI_Unload(dib_ico);
		return;
	}

	HBITMAP hScrBM = FIP->FI_CreateHBITMAPFromDIB(dib_ico);
	FIP->FI_Unload(dib_ico);

	if (!hScrBM) {
		DeleteObject(hbmMask);
		return;
	}

	//create ImageList
	HIMAGELIST himl = ImageList_Create(16, 16, bitDest | ILC_MASK, 0, nCountriesCount);
	ImageList_Add(himl, hScrBM, hbmMask);
	DeleteObject(hScrBM);	hScrBM  = NULL;
	DeleteObject(hbmMask);	hbmMask = NULL;

	if (himl != NULL) {
		phIconHandles = (HANDLE*)mir_alloc(nCountriesCount*sizeof(HANDLE));
		if (phIconHandles != NULL) {
			char szId[20];
			SKINICONDESC sid = { sizeof(sid) };
			sid.ptszSection = LPGENT("Country flags");
			sid.pszName = szId;			// name to refer to icon when playing and in db
			sid.cx = GetSystemMetrics(SM_CXSMICON); 
			sid.cy = GetSystemMetrics(SM_CYSMICON);
			sid.flags = SIDF_SORTED | SIDF_TCHAR;

			for (int i=0; i < nCountriesCount; i++) {
				sid.ptszDescription = mir_a2t(LPGEN(countries[i].szName));
				/* create identifier */
				mir_snprintf(szId, SIZEOF(szId), (countries[i].id == 0xFFFF) ? "%s0x%X" : "%s%i", "flags_", countries[i].id); /* buffer safe */
				int index = CountryNumberToBitmapIndex(countries[i].id);
				/* create icon */
				sid.hDefaultIcon = ImageList_ExtractIcon(NULL, himl, index);
				index = CountryNumberToIndex(countries[i].id);

				phIconHandles[index] = Skin_AddIcon(&sid);
				if (sid.hDefaultIcon!=NULL) DestroyIcon(sid.hDefaultIcon);
				mir_free(sid.ptszDescription); sid.ptszDescription = NULL;
			}
		}
		ImageList_Destroy(himl);
	}

	/* create services */
	CreateServiceFunction(MS_FLAGS_LOADFLAGICON,ServiceLoadFlagIcon);
	CreateServiceFunction(MS_FLAGS_CREATEMERGEDFLAGICON,ServiceCreateMergedFlagIcon);
}

void UninitIcons()
{
	for(int i=0;i<nCountriesCount;++i) {
		/* create identifier */
		char szId[20];
		mir_snprintf(szId, SIZEOF(szId), (countries[i].id == 0xFFFF) ? "%s0x%X" : "%s%i", "flags_", countries[i].id); /* buffer safe */
		Skin_RemoveIcon(szId);
	}
	mir_free(phIconHandles);  /* does NULL check */
}
