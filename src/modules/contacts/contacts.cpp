/*
Miranda IM

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include "commonheaders.h"

#define NAMEORDERCOUNT 8
static TCHAR* nameOrderDescr[ NAMEORDERCOUNT ] =
{
	_T( "My custom name (not moveable)" ),
	_T( "Nick" ),
	_T( "FirstName" ),
	_T( "E-mail" ),
	_T( "LastName" ),
	_T( "Username" ),
	_T( "FirstName LastName" ),
	_T( "'(Unknown Contact)' (not moveable)" )
};

BYTE nameOrder[NAMEORDERCOUNT];

static int GetDatabaseString( CONTACTINFO *ci, const char* setting, DBVARIANT* dbv )
{
    if (strcmp(ci->szProto, "CList") && CallProtoService(ci->szProto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_INFOSETTINGSVC)
    {
        DBCONTACTGETSETTING cgs = { ci->szProto, setting, dbv };
        dbv->type = (ci->dwFlag & CNF_UNICODE) ? DBVT_WCHAR : DBVT_ASCIIZ;

        int res = CallProtoService(ci->szProto, PS_GETINFOSETTING, (WPARAM)ci->hContact, (LPARAM)&cgs);
        if (res != CALLSERVICE_NOTFOUND) return res;
    }

	if ( ci->dwFlag & CNF_UNICODE )
		return DBGetContactSettingWString(ci->hContact,ci->szProto,setting,dbv);

	return DBGetContactSettingString(ci->hContact,ci->szProto,setting,dbv);
}

static int ProcessDatabaseValueDefault(CONTACTINFO *ci, const char* setting)
{
	DBVARIANT dbv;
	if ( !GetDatabaseString( ci, setting, &dbv )) {
		switch (dbv.type) {
		case DBVT_ASCIIZ:
			if (!dbv.pszVal[0]) break;
		case DBVT_WCHAR:
			if (!dbv.pwszVal[0]) break;
			ci->type = CNFT_ASCIIZ;
			ci->pszVal = dbv.ptszVal;
			return 0;
		}
		DBFreeVariant( &dbv );
	}

	if ( DBGetContactSetting( ci->hContact, ci->szProto, setting, &dbv ))
		return 1;

	switch (dbv.type) {
	case DBVT_BYTE:
		ci->type = CNFT_BYTE;
		ci->bVal = dbv.bVal;
		return 0;
	case DBVT_WORD:
		ci->type = CNFT_WORD;
		ci->wVal = dbv.wVal;
		return 0;
	case DBVT_DWORD:
		ci->type = CNFT_DWORD;
		ci->dVal = dbv.dVal;
		return 0;
	}

	DBFreeVariant( &dbv );
	return 1;
}

static INT_PTR GetContactInfo(WPARAM, LPARAM lParam) {
	DBVARIANT dbv;
	CONTACTINFO *ci = (CONTACTINFO*)lParam;

	if (ci==NULL) return 1;
	if (ci->szProto==NULL) ci->szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT,(WPARAM)ci->hContact,0);
	if (ci->szProto==NULL) return 1;
	ci->type = 0;
	switch(ci->dwFlag & 0x7F) {
		case CNF_FIRSTNAME:  return ProcessDatabaseValueDefault( ci, "FirstName" );
		case CNF_LASTNAME:   return ProcessDatabaseValueDefault( ci, "LastName" );
		case CNF_NICK:       return ProcessDatabaseValueDefault( ci, "Nick" );
		case CNF_EMAIL:      return ProcessDatabaseValueDefault( ci, "e-mail" );
		case CNF_CITY:       return ProcessDatabaseValueDefault( ci, "City" );
		case CNF_STATE:      return ProcessDatabaseValueDefault( ci, "State" );
		case CNF_PHONE:      return ProcessDatabaseValueDefault( ci, "Phone" );
		case CNF_HOMEPAGE:   return ProcessDatabaseValueDefault( ci, "Homepage" );
		case CNF_ABOUT:      return ProcessDatabaseValueDefault( ci, "About" );
		case CNF_AGE:        return ProcessDatabaseValueDefault( ci, "Age" );
		case CNF_GENDER:     return ProcessDatabaseValueDefault( ci, "Gender" );
		case CNF_FAX:        return ProcessDatabaseValueDefault( ci, "Fax" );
		case CNF_CELLULAR:	return ProcessDatabaseValueDefault( ci, "Cellular" );
		case CNF_BIRTHDAY:	return ProcessDatabaseValueDefault( ci, "BirthDay" );
		case CNF_BIRTHMONTH:	return ProcessDatabaseValueDefault( ci, "BirthMonth" );
		case CNF_BIRTHYEAR:	return ProcessDatabaseValueDefault( ci, "BirthYear" );
		case CNF_STREET:		return ProcessDatabaseValueDefault( ci, "Street" );
		case CNF_ZIP:			return ProcessDatabaseValueDefault( ci, "ZIP" );
		case CNF_LANGUAGE1:	return ProcessDatabaseValueDefault( ci, "Language1" );
		case CNF_LANGUAGE2:	return ProcessDatabaseValueDefault( ci, "Language2" );
		case CNF_LANGUAGE3:	return ProcessDatabaseValueDefault( ci, "Language3" );
		case CNF_CONAME:		return ProcessDatabaseValueDefault( ci, "Company" );
		case CNF_CODEPT:     return ProcessDatabaseValueDefault( ci, "CompanyDepartment" );
		case CNF_COPOSITION: return ProcessDatabaseValueDefault( ci, "CompanyPosition" );
		case CNF_COSTREET:   return ProcessDatabaseValueDefault( ci, "CompanyStreet" );
		case CNF_COCITY:     return ProcessDatabaseValueDefault( ci, "CompanyCity" );
		case CNF_COSTATE:    return ProcessDatabaseValueDefault( ci, "CompanyState" );
		case CNF_COZIP:      return ProcessDatabaseValueDefault( ci, "CompanyZIP" );
		case CNF_COHOMEPAGE: return ProcessDatabaseValueDefault( ci, "CompanyHomepage" );

		case CNF_CUSTOMNICK:
		{
			char* saveProto = ci->szProto; ci->szProto = "CList";
			if ( ci->hContact != NULL && !ProcessDatabaseValueDefault( ci, "MyHandle" )) {
				ci->szProto = saveProto;
				return 0;
			}
			ci->szProto = saveProto;
			break;
		}
		case CNF_COUNTRY:
		case CNF_COCOUNTRY:
			if ( !GetDatabaseString( ci, (ci->dwFlag & 0x7F) == CNF_COUNTRY ? "CountryName" : "CompanyCountryName", &dbv ))
				return 0;

			if ( !DBGetContactSetting( ci->hContact, ci->szProto, (ci->dwFlag & 0x7F)==CNF_COUNTRY ? "Country" : "CompanyCountry", &dbv )) {
				if ( dbv.type == DBVT_WORD ) {
					int i,countryCount;
					struct CountryListEntry *countries;
					CallService(MS_UTILS_GETCOUNTRYLIST,(WPARAM)&countryCount,(LPARAM)&countries);
					for(i=0;i<countryCount;i++) {
						if(countries[i].id!=dbv.wVal) continue;

						if ( ci->dwFlag & CNF_UNICODE ) {
							int cbLen = MultiByteToWideChar( CP_ACP, 0, ( LPCSTR )countries[i].szName, -1, NULL, 0 );
							WCHAR* buf = ( WCHAR* )mir_alloc( sizeof( WCHAR )*(cbLen+1) );
							if ( buf != NULL )
								MultiByteToWideChar( CP_ACP, 0, ( LPCSTR )countries[i].szName, -1, buf, cbLen );
							ci->pszVal = ( TCHAR* )buf;
						}
						else ci->pszVal = ( TCHAR* )mir_strdup(countries[i].szName);

						ci->type = CNFT_ASCIIZ;
						DBFreeVariant(&dbv);
						return 0;
					}
				}
				else return ProcessDatabaseValueDefault( ci, (ci->dwFlag & 0x7F)==CNF_COUNTRY ? "Country" : "CompanyCountry" );
				DBFreeVariant(&dbv);
			}
			break;

		case CNF_FIRSTLAST:
			if( !GetDatabaseString( ci, "FirstName", &dbv )) {
				DBVARIANT dbv2;
				if(!GetDatabaseString(ci,"LastName",&dbv2)) {
					ci->type = CNFT_ASCIIZ;
					if ( ci->dwFlag & CNF_UNICODE ) {
						size_t len = wcslen(dbv.pwszVal) + wcslen(dbv2.pwszVal) + 2;
						WCHAR* buf = ( WCHAR* )mir_alloc( sizeof( WCHAR )*len );
						if ( buf != NULL )
							wcscat( wcscat( wcscpy( buf, dbv.pwszVal ), L" " ), dbv2.pwszVal );
						ci->pszVal = ( TCHAR* )buf;
					}
					else {
						size_t len = strlen(dbv.pszVal) + strlen(dbv2.pszVal) + 2;
						char* buf = ( char* )mir_alloc( len );
						if ( buf != NULL )
							strcat( strcat( strcpy( buf, dbv.pszVal ), " " ), dbv2.pszVal );
						ci->pszVal = ( TCHAR* )buf;
					}
					DBFreeVariant( &dbv );
					DBFreeVariant( &dbv2 );
					return 0;
				}
				DBFreeVariant( &dbv );
			}
			break;

		case CNF_UNIQUEID:
		{
			char *uid = (char*)CallProtoService(ci->szProto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
			if ((INT_PTR)uid!=CALLSERVICE_NOTFOUND&&uid)
				if (!ProcessDatabaseValueDefault(ci,uid))
					return 0;

			break;
		}
		case CNF_DISPLAYUID:
		{
			if (!ProcessDatabaseValueDefault(ci, "display_uid"))
				return 0;
			char *uid = (char*)CallProtoService(ci->szProto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
			if ((INT_PTR)uid!=CALLSERVICE_NOTFOUND&&uid)
				if (!ProcessDatabaseValueDefault(ci,uid))
					return 0;

			break;
		}
		case CNF_DISPLAYNC:
		case CNF_DISPLAY:
		{
			int i;
			for( i=0; i < NAMEORDERCOUNT; i++ ) {
				switch(nameOrder[i])  {
					case 0: // custom name
					{
						// make sure we aren't in CNF_DISPLAYNC mode
						// don't get custom name for NULL contact
						char* saveProto = ci->szProto; ci->szProto = "CList";
						if (ci->hContact!=NULL && (ci->dwFlag&0x7F)==CNF_DISPLAY && !ProcessDatabaseValueDefault(ci,"MyHandle")) {
							ci->szProto = saveProto;
							return 0;
						}
						ci->szProto = saveProto;
						break;
					}
					case 1:
						if ( !ProcessDatabaseValueDefault( ci, "Nick" )) // nick
							return 0;
						break;
					case 2:
						if ( !ProcessDatabaseValueDefault( ci, "FirstName" )) // First Name
							return 0;
						break;
					case 3:
						if ( !ProcessDatabaseValueDefault( ci, "e-mail" )) // E-mail
							return 0;
						break;
					case 4:
						if ( !ProcessDatabaseValueDefault( ci, "LastName" )) // Last Name
							return 0;
						break;
					case 5: // Unique id
					{
						// protocol must define a PFLAG_UNIQUEIDSETTING
						char *uid = (char*)CallProtoService(ci->szProto,PS_GETCAPS,PFLAG_UNIQUEIDSETTING,0);
						if ((INT_PTR)uid!=CALLSERVICE_NOTFOUND&&uid) {
							if (!GetDatabaseString(ci,uid,&dbv)) {
								if ( dbv.type == DBVT_BYTE || dbv.type == DBVT_WORD || dbv.type == DBVT_DWORD ) {
									long value = (dbv.type == DBVT_BYTE) ? dbv.bVal:(dbv.type==DBVT_WORD ? dbv.wVal : dbv.dVal);
									if ( ci->dwFlag & CNF_UNICODE ) {
										WCHAR buf[ 40 ];
										_ltow( value, buf, 10 );
										ci->pszVal = ( TCHAR* )mir_wstrdup( buf );
									}
									else {
										char buf[ 40 ];
										_ltoa( value, buf, 10 );
										ci->pszVal = ( TCHAR* )mir_strdup(buf);
									}
									ci->type = CNFT_ASCIIZ;
									return 0;
								}
								if (dbv.type == DBVT_ASCIIZ && !(ci->dwFlag & CNF_UNICODE)) {
									ci->type = CNFT_ASCIIZ;
									ci->pszVal = dbv.ptszVal;
									return 0;
								}
								if (dbv.type == DBVT_WCHAR && (ci->dwFlag & CNF_UNICODE)) {
								  ci->type = CNFT_ASCIIZ;
								  ci->pszVal = dbv.ptszVal;
								  return 0;
						}	}	}
						break;
					}
					case 6: // first + last name
						if(!GetDatabaseString(ci,"FirstName",&dbv)) {
							DBVARIANT dbv2;
							if(!GetDatabaseString(ci,"LastName",&dbv2)) {
								ci->type = CNFT_ASCIIZ;

								if ( ci->dwFlag & CNF_UNICODE ) {
									size_t len = wcslen(dbv.pwszVal) + wcslen(dbv2.pwszVal) + 2;
									WCHAR* buf = ( WCHAR* )mir_alloc( sizeof( WCHAR )*len );
									if ( buf != NULL )
										wcscat( wcscat( wcscpy( buf, dbv.pwszVal ), L" " ), dbv2.pwszVal );
									ci->pszVal = ( TCHAR* )buf;
								}
								else {
									size_t len = strlen(dbv.pszVal) + strlen(dbv2.pszVal) + 2;
									char* buf = ( char* )mir_alloc( len );
									if ( buf != NULL )
										strcat( strcat( strcpy( buf, dbv.pszVal ), " " ), dbv2.pszVal );
									ci->pszVal = ( TCHAR* )buf;
								}

								DBFreeVariant( &dbv );
								DBFreeVariant( &dbv2 );
								return 0;
							}
							DBFreeVariant( &dbv );
						}
						break;

					case 7:
						if ( ci->dwFlag & CNF_UNICODE )
							ci->pszVal = ( TCHAR* )mir_wstrdup( TranslateW( L"'(Unknown Contact)'" ));
						else
							ci->pszVal = ( TCHAR* )mir_strdup( Translate("'(Unknown Contact)'"));
						ci->type = CNFT_ASCIIZ;
						return 0;
			}	}	}
			break;

		case CNF_TIMEZONE: {
			HANDLE hTz = tmi.createByContact(ci->hContact, TZF_KNOWNONLY);
			if (hTz)
			{
				LPTIME_ZONE_INFORMATION tzi = tmi.getTzi(hTz);
				int offset = tzi->Bias + tzi->StandardBias;

				char str[80];
				mir_snprintf(str, SIZEOF(str), offset ? "UTC%+d:%02d" : "UTC", offset / -60, abs(offset % 60));
				ci->pszVal = ci->dwFlag & CNF_UNICODE ? (TCHAR*)mir_a2u(str) : (TCHAR*)mir_strdup(str);
				ci->type = CNFT_ASCIIZ;
				return 0;
			}
			break;
		}
		case CNF_MYNOTES: {
			char* saveProto = ci->szProto; ci->szProto = "UserInfo";
			if (!ProcessDatabaseValueDefault(ci,"MyNotes")) {
				ci->szProto = saveProto;
				return 0;
			}
			ci->szProto = saveProto;
			break;
	}	}

	return 1;
}

struct ContactOptionsData {
	int dragging;
	HTREEITEM hDragItem;
};

static INT_PTR CALLBACK ContactOpts(HWND hwndDlg, UINT msg, WPARAM, LPARAM lParam)
{	struct ContactOptionsData *dat;

	dat=(struct ContactOptionsData*)GetWindowLongPtr(hwndDlg,GWLP_USERDATA);
	switch (msg)
	{
		case WM_INITDIALOG:
		{	TranslateDialogDefault(hwndDlg);
			dat=(struct ContactOptionsData*)mir_alloc(sizeof(struct ContactOptionsData));
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)dat);
			dat->dragging=0;
			SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NAMEORDER),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NAMEORDER),GWL_STYLE)|TVS_NOHSCROLL);
			{	TVINSERTSTRUCT tvis;
				int i;
				tvis.hParent = NULL;
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT|TVIF_PARAM;
				for(i=0; i < SIZEOF(nameOrderDescr); i++ ) {
					tvis.item.lParam = nameOrder[i];
					tvis.item.pszText = TranslateTS( nameOrderDescr[ nameOrder[i]] );
					TreeView_InsertItem( GetDlgItem(hwndDlg,IDC_NAMEORDER), &tvis );
			}	}
			return TRUE;
		}
		case WM_NOTIFY:
			switch (((LPNMHDR)lParam)->idFrom) {
				case 0:
					if (((LPNMHDR)lParam)->code == PSN_APPLY)
					{	DBCONTACTWRITESETTING cws;
						TVITEM tvi;
						int i;
						cws.szModule = "Contact";
						cws.szSetting = "NameOrder";
						cws.value.type = DBVT_BLOB;
						cws.value.cpbVal = SIZEOF(nameOrderDescr);
						cws.value.pbVal = nameOrder;
						tvi.hItem = TreeView_GetRoot(GetDlgItem(hwndDlg,IDC_NAMEORDER));
						i=0;
						while( tvi.hItem != NULL ) {
							tvi.mask = TVIF_PARAM | TVIF_HANDLE;
							TreeView_GetItem( GetDlgItem(hwndDlg,IDC_NAMEORDER), &tvi );
							nameOrder[i++] = (BYTE)tvi.lParam;
							tvi.hItem = TreeView_GetNextSibling(GetDlgItem(hwndDlg,IDC_NAMEORDER),tvi.hItem);
						}
						CallService(MS_DB_CONTACT_WRITESETTING,(WPARAM)(HANDLE)NULL,(LPARAM)&cws);
						CallService(MS_CLIST_INVALIDATEDISPLAYNAME,(WPARAM)INVALID_HANDLE_VALUE,0);
					}
					break;
				case IDC_NAMEORDER:
					if (((LPNMHDR)lParam)->code == TVN_BEGINDRAGA) {
						LPNMTREEVIEWA notify = (LPNMTREEVIEWA)lParam;
						if ( notify->itemNew.lParam==0 || notify->itemNew.lParam == SIZEOF(nameOrderDescr)-1 )
							break;
						SetCapture(hwndDlg);
						dat->dragging=1;
						dat->hDragItem=((LPNMTREEVIEW)lParam)->itemNew.hItem;
						TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_NAMEORDER),dat->hDragItem);
					}
					break;
			}
			break;
		case WM_MOUSEMOVE:
			if(!dat->dragging) break;
			{	TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(lParam);
				hti.pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwndDlg,&hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg,IDC_NAMEORDER),&hti.pt);
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_NAMEORDER),&hti);
				if(hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
					hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_NAMEORDER))/2;
					TreeView_HitTest(GetDlgItem(hwndDlg,IDC_NAMEORDER),&hti);
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_NAMEORDER),hti.hItem,1);
				}
				else {
					if(hti.flags&TVHT_ABOVE) SendDlgItemMessage(hwndDlg,IDC_NAMEORDER,WM_VSCROLL,MAKEWPARAM(SB_LINEUP,0),0);
					if(hti.flags&TVHT_BELOW) SendDlgItemMessage(hwndDlg,IDC_NAMEORDER,WM_VSCROLL,MAKEWPARAM(SB_LINEDOWN,0),0);
					TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_NAMEORDER),NULL,0);
				}
			}
			break;
		case WM_LBUTTONUP:
			if(!dat->dragging) break;
			TreeView_SetInsertMark(GetDlgItem(hwndDlg,IDC_NAMEORDER),NULL,0);
			dat->dragging=0;
			ReleaseCapture();
			{	TVHITTESTINFO hti;
				TVITEM tvi;
				hti.pt.x=(short)LOWORD(lParam);
				hti.pt.y=(short)HIWORD(lParam);
				ClientToScreen(hwndDlg,&hti.pt);
				ScreenToClient(GetDlgItem(hwndDlg,IDC_NAMEORDER),&hti.pt);
				hti.pt.y-=TreeView_GetItemHeight(GetDlgItem(hwndDlg,IDC_NAMEORDER))/2;
				TreeView_HitTest(GetDlgItem(hwndDlg,IDC_NAMEORDER),&hti);
				if(dat->hDragItem==hti.hItem) break;
				tvi.mask=TVIF_HANDLE|TVIF_PARAM;
				tvi.hItem=hti.hItem;
				TreeView_GetItem(GetDlgItem(hwndDlg,IDC_NAMEORDER),&tvi);
				if(tvi.lParam == SIZEOF(nameOrderDescr)-1) break;
				if(hti.flags&(TVHT_ONITEM|TVHT_ONITEMRIGHT)) {
					TVINSERTSTRUCT tvis;
					TCHAR name[128];
					tvis.item.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_TEXT|TVIF_PARAM;
					tvis.item.stateMask=0xFFFFFFFF;
					tvis.item.pszText=name;
					tvis.item.cchTextMax=SIZEOF(name);
					tvis.item.hItem=dat->hDragItem;
					TreeView_GetItem(GetDlgItem(hwndDlg,IDC_NAMEORDER),&tvis.item);
					TreeView_DeleteItem(GetDlgItem(hwndDlg,IDC_NAMEORDER),dat->hDragItem);
					tvis.hParent=NULL;
					tvis.hInsertAfter=hti.hItem;
					TreeView_SelectItem(GetDlgItem(hwndDlg,IDC_NAMEORDER),TreeView_InsertItem(GetDlgItem(hwndDlg,IDC_NAMEORDER),&tvis));
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				}
			}
			break;
		case WM_DESTROY:
			mir_free(dat);
			break;
	}
	return FALSE;
}

static int ContactOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.cbSize = sizeof(odp);
	odp.position = -1000000000;
	odp.hInstance = hMirandaInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_CONTACT);
	odp.pszGroup = LPGEN("Customize");
	odp.pszTitle = LPGEN("Contacts");
	odp.pfnDlgProc = ContactOpts;
	odp.flags = ODPF_BOLDGROUPS;
	CallService( MS_OPT_ADDPAGE, wParam, ( LPARAM )&odp );
	return 0;
}

int LoadContactsModule(void) {
	{
		// Load the name order
		BYTE i;
		DBVARIANT dbv;

		for(i=0; i<NAMEORDERCOUNT; i++)
			nameOrder[i]=i;

		if(!DBGetContactSetting(NULL,"Contact","NameOrder",&dbv))
		{
			CopyMemory(nameOrder,dbv.pbVal,dbv.cpbVal);
			DBFreeVariant(&dbv);
		}
	}
	CreateServiceFunction(MS_CONTACT_GETCONTACTINFO,GetContactInfo);
	HookEvent(ME_OPT_INITIALISE,ContactOptInit);
	return 0;
}
