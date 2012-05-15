/*
Miranda Database Tool
Copyright (C) 2001-2005  Richard Hughes

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
#include "dbtool.h"

static DWORD ofsThisSettings,ofsDestPrevSettings;

int WorkSettingsChain(DWORD ofsContact,DBContact *dbc,int firstTime)
{
	DBContactSettings *dbcsNew,dbcsOld;
	DWORD ofsDestThis;
	int ret;

	if(firstTime) {
		ofsDestPrevSettings=0;
		ofsThisSettings=dbc->ofsFirstSettings;
		dbc->ofsFirstSettings=0;
	}
	if(ofsThisSettings==0)
		return ERROR_NO_MORE_ITEMS;
	if(!SignatureValid(ofsThisSettings,DBCONTACTSETTINGS_SIGNATURE)) {
		AddToStatus(STATUS_ERROR,TranslateT("Settings chain corrupted, further entries ignored"));
		return ERROR_NO_MORE_ITEMS;
	}
	if(PeekSegment(ofsThisSettings,&dbcsOld,sizeof(dbcsOld))!=ERROR_SUCCESS)
		return ERROR_NO_MORE_ITEMS;
	if(dbcsOld.cbBlob>256*1024 || dbcsOld.cbBlob==0) {
		AddToStatus(STATUS_ERROR,TranslateT("Infeasibly large settings blob: skipping"));
		ofsThisSettings=dbcsOld.ofsNext;
		return ERROR_SUCCESS;
	}
	dbcsNew=(DBContactSettings*)_alloca(offsetof(DBContactSettings,blob)+dbcsOld.cbBlob);
	if((ret=ReadSegment(ofsThisSettings,dbcsNew,offsetof(DBContactSettings,blob)+dbcsOld.cbBlob))!=ERROR_SUCCESS) {
		if(ret!=ERROR_HANDLE_EOF) {   //eof is OK because blank space at the end doesn't matter
			return ERROR_NO_MORE_ITEMS;
		}
	}
	if((dbcsNew->ofsModuleName=ConvertModuleNameOfs(dbcsOld.ofsModuleName))==0) {
		ofsThisSettings=dbcsOld.ofsNext;
		return ERROR_SUCCESS;
	}
	if(dbcsNew->blob[0]==0) {
		AddToStatus(STATUS_MESSAGE,TranslateT("Empty settings group at %08X: skipping"),ofsThisSettings);
		ofsThisSettings=dbcsOld.ofsNext;
		return ERROR_SUCCESS;
	}
	dbcsNew->ofsNext=0;
	//TODO? validate all settings in blob/compact if necessary
	if((ofsDestThis=WriteSegment(WSOFS_END,dbcsNew,offsetof(DBContactSettings,blob)+dbcsNew->cbBlob))==WS_ERROR) {
		return ERROR_HANDLE_DISK_FULL;
	}
	if(ofsDestPrevSettings) WriteSegment(ofsDestPrevSettings+offsetof(DBContactSettings,ofsNext),&ofsDestThis,sizeof(DWORD));
	else dbc->ofsFirstSettings=ofsDestThis;
	ofsDestPrevSettings=ofsDestThis;
	ofsThisSettings=dbcsOld.ofsNext;
	return ERROR_SUCCESS;
}
