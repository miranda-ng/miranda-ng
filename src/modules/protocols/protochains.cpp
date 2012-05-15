/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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
#include <m_protomod.h>

//Protocol chain is list of integers "0".."n", with network protocol named "p"
INT_PTR Proto_CallContactService(WPARAM wParam,LPARAM lParam)
//note that this is ChainSend() too, due to a quirk of function definitions
{
	CCSDATA *ccs=(CCSDATA*)lParam;
	int i;
	char str[10];
	DBVARIANT dbv;
	INT_PTR ret;
	PROTOACCOUNT* pa;

	if ( wParam == (WPARAM)(-1))
		return 1;

	for ( i = wParam;; i++ ) {
		_itoa( i, str, 10 );
		if ( DBGetContactSettingString( ccs->hContact, "_Filter", str, &dbv ))
			break;

		if (( ret = CallProtoService( dbv.pszVal, ccs->szProtoService, i+1, lParam )) != CALLSERVICE_NOTFOUND ) {
			//chain was started, exit
			mir_free( dbv.pszVal );
			return ret;
		}
		mir_free( dbv.pszVal );
	}
	if ( DBGetContactSettingString( ccs->hContact, "Protocol", "p", &dbv ))
		return 1;

	pa = Proto_GetAccount( dbv.pszVal );
	if ( pa == NULL || pa->ppro == NULL )
		ret = 1;
	else {
		if ( pa->bOldProto )
			ret = CallProtoServiceInt( ccs->hContact, dbv.pszVal, ccs->szProtoService, (WPARAM)(-1), ( LPARAM)ccs );
		else
			ret = CallProtoServiceInt( ccs->hContact, dbv.pszVal, ccs->szProtoService, ccs->wParam, ccs->lParam );
		if ( ret == CALLSERVICE_NOTFOUND )
			ret = 1;
	}

	mir_free(dbv.pszVal);
	return ret;
}

static INT_PTR CallRecvChain(WPARAM wParam,LPARAM lParam)
{
	CCSDATA *ccs=(CCSDATA*)lParam;
	int i;
	INT_PTR ret;
	char str[10];
	DBVARIANT dbv;
	PROTOACCOUNT* pa;

	if ( wParam == (WPARAM)(-1)) return 1;   //shouldn't happen - sanity check
	if ( wParam == 0 ) {	   //begin processing by finding end of chain
		for( ;;wParam++ ) {
			_itoa( wParam, str, 10 );
			if ( DBGetContactSettingString( ccs->hContact, "_Filter", str, &dbv ))
				break;
			mir_free(dbv.pszVal);
		}
	}
	else wParam--;

	for ( i = wParam-1; i >= 0; i-- ) {
		_itoa( i, str, 10 );
		if ( DBGetContactSettingString( ccs->hContact, "_Filter", str, &dbv ))  //never happens
			return 1;

		if (( ret = CallProtoService( dbv.pszVal, ccs->szProtoService, i+1, lParam )) != CALLSERVICE_NOTFOUND ) {
			//chain was started, exit
			mir_free( dbv.pszVal );
			return ret;
		}
		mir_free( dbv.pszVal );
	}

	//end of chain, call network protocol again
	if ( DBGetContactSettingString( ccs->hContact, "Protocol", "p", &dbv ))
		return 1;

	pa = Proto_GetAccount( dbv.pszVal );
	if ( pa == NULL || pa->ppro == NULL )
		ret = 1;
	else {
		if ( pa->bOldProto )
			ret = CallProtoServiceInt( ccs->hContact, dbv.pszVal, ccs->szProtoService, (WPARAM)(-1), ( LPARAM)ccs );
		else
			ret = CallProtoServiceInt( ccs->hContact, dbv.pszVal, ccs->szProtoService, ccs->wParam, ccs->lParam );
		if ( ret == CALLSERVICE_NOTFOUND )
			ret = 1;
	}

	mir_free( dbv.pszVal );
	return ret;
}

static INT_PTR Proto_ChainRecv(WPARAM wParam,LPARAM lParam)
{
	/* this will switch threads just like before */
	return CallServiceSync(MS_PROTO_CHAINRECV "ThreadSafe",wParam,lParam);
}

PROTOACCOUNT* __fastcall Proto_GetAccount(HANDLE hContact)
{
	DBVARIANT dbv;
	DBCONTACTGETSETTING dbcgs;
	char name[32];

	dbv.type = DBVT_ASCIIZ;
	dbv.pszVal = name;
	dbv.cchVal = SIZEOF(name);
	dbcgs.pValue = &dbv;
	dbcgs.szModule = "Protocol";
	dbcgs.szSetting = "p";
	if (CallService(MS_DB_CONTACT_GETSETTINGSTATIC, (WPARAM)hContact, (LPARAM)&dbcgs))
		return 0;

	return Proto_GetAccount((char* )dbv.pszVal);
}

static INT_PTR Proto_GetContactBaseProto(WPARAM wParam, LPARAM)
{
	PROTOACCOUNT* pa = Proto_GetAccount((HANDLE)wParam);
	return (INT_PTR)(Proto_IsAccountEnabled( pa ) ? pa->szModuleName : NULL);
}

static INT_PTR Proto_GetContactBaseAccount(WPARAM wParam, LPARAM)
{
	PROTOACCOUNT* pa = Proto_GetAccount((HANDLE)wParam);
    return (INT_PTR)(pa ? pa->szModuleName : NULL);
}

static INT_PTR Proto_IsProtoOnContact(WPARAM wParam,LPARAM lParam)
{
	int i;
	char str[10];
	DBVARIANT dbv;

    if (!lParam) return 0;

	if(!DBGetContactSettingString((HANDLE)wParam,"Protocol","p",&dbv)) {
		if(!_stricmp((char*)lParam,dbv.pszVal)) {
			mir_free(dbv.pszVal);
			return -1;
		}
		mir_free(dbv.pszVal);
	}
	for(i=0;;i++) {
		_itoa(i,str,10);
		if(DBGetContactSettingString((HANDLE)wParam,"_Filter",str,&dbv)) break;
		if(!strcmp((char*)lParam,dbv.pszVal)) {
			mir_free(dbv.pszVal);
			return i+1;
		}
		mir_free(dbv.pszVal);
	}
	return 0;
}

static INT_PTR Proto_AddToContact(WPARAM wParam,LPARAM lParam)
{
	PROTOCOLDESCRIPTOR *pd,*pdCompare;

	pd = Proto_IsProtocolLoaded(( char* )lParam );
	if ( pd == NULL ) {
		PROTOACCOUNT* pa = Proto_GetAccount(( char* )lParam );
		if ( pa ) {
			DBWriteContactSettingString((HANDLE)wParam,"Protocol","p",(char*)lParam);
			return 0;
		}
		return 1;
	}

	if ( pd->type == PROTOTYPE_PROTOCOL ) {
		DBWriteContactSettingString((HANDLE)wParam,"Protocol","p",(char*)lParam);
		return 0;
	}
	if(Proto_IsProtoOnContact(wParam,lParam)) return 1;
	{ /* v:0.3.3 + PROTO FILTERS ARE NOW KEPT IN THEIR OWN DB MODULE! */
		int i;
		char str[10],*lastProto;
		DBVARIANT dbv;

		for(i=0;;i++) {
			_itoa(i,str,10);
			if(DBGetContactSettingString((HANDLE)wParam,"_Filter",str,&dbv)) break;
			pdCompare = Proto_IsProtocolLoaded(( char* )dbv.pszVal );
			mir_free(dbv.pszVal);
			if(pdCompare==NULL) continue;
			if(pd->type > pdCompare->type) break;
		}
		//put the new module at position i
		lastProto=mir_strdup((char*)lParam);
		for(;;i++) {
			_itoa(i,str,10);
			if(DBGetContactSettingString((HANDLE)wParam,"_Filter",str,&dbv)) {
				DBWriteContactSettingString((HANDLE)wParam,"_Filter",str,lastProto);
				mir_free(lastProto);
				break;
			}
			DBWriteContactSettingString((HANDLE)wParam,"_Filter",str,lastProto);
			mir_free(lastProto);
			lastProto=dbv.pszVal;
		}
	}
	return 0;
}

static INT_PTR Proto_RemoveFromContact(WPARAM wParam,LPARAM lParam)
{
	int i;
	DBVARIANT dbv;
	char str[10];

	i = Proto_IsProtoOnContact(wParam,lParam);
	if(!i) return 1;
	if(i==-1)
		DBDeleteContactSetting((HANDLE)wParam,"Protocol","p");
	else {
		for(i--;;i++) {			//we have to decrease i, as Proto_IsOnContact returns +1 more number than read from database
			_itoa(i+1,str,10);
			if(0!=DBGetContactSettingString((HANDLE)wParam,"_Filter",str,&dbv)) {
				_itoa(i,str,10);
				DBDeleteContactSetting((HANDLE)wParam,"_Filter",str);
				break;
			}
			_itoa(i,str,10);
			DBWriteContactSettingString((HANDLE)wParam,"_Filter",str,dbv.pszVal);
			mir_free(dbv.pszVal);
		}
	}
	return 0;
}

int LoadProtoChains(void)
{
	CreateServiceFunction(MS_PROTO_CALLCONTACTSERVICE,Proto_CallContactService);
	CreateServiceFunction(MS_PROTO_CHAINSEND,Proto_CallContactService);
	CreateServiceFunction(MS_PROTO_CHAINRECV,Proto_ChainRecv);
	CreateServiceFunction(MS_PROTO_CHAINRECV "ThreadSafe",CallRecvChain);
	CreateServiceFunction(MS_PROTO_GETCONTACTBASEPROTO,Proto_GetContactBaseProto);
	CreateServiceFunction(MS_PROTO_GETCONTACTBASEACCOUNT,Proto_GetContactBaseAccount);
	CreateServiceFunction(MS_PROTO_ISPROTOONCONTACT,Proto_IsProtoOnContact);
	CreateServiceFunction(MS_PROTO_ADDTOCONTACT,Proto_AddToContact);
	CreateServiceFunction(MS_PROTO_REMOVEFROMCONTACT,Proto_RemoveFromContact);
	return 0;
}
