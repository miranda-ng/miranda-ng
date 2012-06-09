#include "common.h"
#pragma hdrstop

extern HINSTANCE hInst;
#define MAXLBUTS 32

LBUTOPT LBUTS[MAXLBUTS];
static int LButCnt = 0;

extern char *AS(char *str, const char *setting, char *addstr);

//wparam -id
INT_PTR LaunchService(WPARAM wParam, LPARAM lParam)
{
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	if ( lParam < 0 || lParam >= MAXLBUTS )
		return -1;

	if ( CreateProcess(NULL, LBUTS[lParam].lpath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	Sleep(20);
	CallService(MS_TTB_SETBUTTONSTATE, LBUTS[lParam].hframe, TTBST_RELEASED);
	return 0;
}

//wparam -id
//lparam  &LBTOPT	
INT_PTR GetLButton(WPARAM wParam, LPARAM lParam)
{
	if ( wParam < 0 || wParam >= MAXLBUTS )
		return -1;

	LBUTOPT *lbo = ( LBUTOPT* )lParam;
	if (lbo == NULL)
		return -1;

	if (LBUTS[wParam].hframe == 0)
		return -1;

	*lbo = LBUTS[wParam];
	return 0;
}

//wparam -id
//lparam  &LBTOPT	
INT_PTR ModifyLButton(WPARAM wParam, LPARAM lParam)
{
	if ( wParam < 0 || wParam >= LButCnt )
		return -1;

	LBUTOPT *lbo = ( LBUTOPT* )lParam;
	if (lbo == NULL)
		return -1;
	
	if (LBUTS[wParam].hframe == 0)
		return -1;

	DeleteLBut(wParam, 0);
	if (LBUTS[wParam].lpath != NULL){free(LBUTS[wParam].lpath);}
	if (LBUTS[wParam].name != NULL){free(LBUTS[wParam].name);}
	LBUTS[wParam].name = NULL;
	LBUTS[wParam].lpath = NULL;
	
	if (lbo->name != NULL)
		LBUTS[wParam].name = _strdup(lbo->name);
	if (lbo->lpath != NULL)
		LBUTS[wParam].lpath = _tcsdup(lbo->lpath);
	
	LBUTS[wParam].hframe = InsertLBut(wParam);
	SaveAllLButs();
	return 0;
}

INT_PTR InsertLBut(int id)
{
	HBITMAP DefLUp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LAUNCHDN));
	HBITMAP DefLDn = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_LAUNCHUP));
	
	TTBButton ttb = { 0 };
	ttb.cbSize = sizeof(ttb);
	ttb.hbBitmapDown = DefLDn;
	ttb.hbBitmapUp = DefLUp;
	ttb.dwFlags = TTBBF_VISIBLE|TTBBF_ISLBUTTON;
	ttb.pszServiceDown = TTB_LAUNCHSERVICE;
	ttb.lParamDown = id;
	ttb.name = LBUTS[id].name;
	LButCnt++;
	return TTBAddButton(( WPARAM )&ttb, 0);
}

INT_PTR DeleteLBut(WPARAM id, LPARAM lParam)
{
	if (LBUTS[id].hframe != 0) {
		TTBRemoveButton(LBUTS[id].hframe, 0);
		LBUTS[id].hframe = 0;
		if (LBUTS[id].name != NULL){free(LBUTS[id].name);}
		if (LBUTS[id].lpath != NULL){free(LBUTS[id].lpath);}
		LBUTS[id].name = NULL;
		LBUTS[id].lpath = NULL;

		LButCnt--;
	}
	return 0;
}

int LoadAllLButs()
{
	char buf[255];
	char buf1[10];
	char fixname[255];
	int id;
	
	//must be locked
	memset(buf, 0, SIZEOF(buf));	

	memset(LBUTS, 0, sizeof(LBUTS));
	for (int i = 0; i < MAXLBUTS; i++) {
		memset(buf1, 0, SIZEOF(buf1));
		itoa(i, buf1, 10);
		AS(fixname, "LBUT", buf1);
		id = DBGetContactSettingWord(0, TTB_OPTDIR, AS(buf, fixname, ""), -1);
		if (id != -1) {
			LBUTS[i].name = DBGetString(0, TTB_OPTDIR, AS(buf, fixname, "_name"));
			LBUTS[i].lpath = DBGetStringT(0, TTB_OPTDIR, AS(buf, fixname, "_lpath"));
			if (LBUTS[i].lpath == NULL)
				LBUTS[i].lpath = _tcsdup( _T("Execute Path"));
			LBUTS[i].hframe = 0;
			LBUTS[i].hframe = InsertLBut(i);
		}
	}
	return 0;
}

int SaveAllLButs()
{
	char buf[255];
	char buf1[10];
	char fixname[255];
	
	//must be locked
	memset(buf, 0, SIZEOF(buf));	
	
	for (int i = 0;i<MAXLBUTS;i++) {
		itoa(i, buf1, 10);
		if (LBUTS[i].hframe != 0) {
			AS(fixname, "LBUT", buf1);
			DBWriteContactSettingWord(0, TTB_OPTDIR, AS(buf, fixname, ""), i);
			DBWriteContactSettingString(0, TTB_OPTDIR, AS(buf, fixname, "_name"), LBUTS[i].name);
			DBWriteContactSettingTString(0, TTB_OPTDIR, AS(buf, fixname, "_lpath"), LBUTS[i].lpath);
		}
		else {
			AS(fixname, "LBUT", buf1);
			DBWriteContactSettingWord(0, TTB_OPTDIR, AS(buf, fixname, ""), -1);
			DBDeleteContactSetting(0, TTB_OPTDIR, AS(buf, fixname, ""));
			DBDeleteContactSetting(0, TTB_OPTDIR, AS(buf, fixname, "_lpath"));
			DBDeleteContactSetting(0, TTB_OPTDIR, AS(buf, fixname, "_name"));
		}
	}
	return 0;
}

//return 0 on success, -1 on error
INT_PTR InsertNewFreeLBut(WPARAM wParam, LPARAM lParam)
{
	char buf[256];
	if (LButCnt < MAXLBUTS) {
		for (int i = 0;i<MAXLBUTS;i++) {
			if (LBUTS[i].hframe == 0) {
				wsprintfA(buf, "%s %d", Translate("Default"), i);
				LBUTS[i].name = _strdup(buf);
				LBUTS[i].lpath = _tcsdup( _T("Execute Path"));
				LBUTS[i].hframe = InsertLBut(i);
				SaveAllLButs();
				return 0;
			}
		}
	}
	
	return -1;
}

int InitLBut()
{
	arServices.insert( CreateServiceFunction(TTB_LAUNCHSERVICE, LaunchService));
	arServices.insert( CreateServiceFunction(TTB_ADDLBUTTON, InsertNewFreeLBut));
	arServices.insert( CreateServiceFunction(TTB_REMOVELBUTTON, DeleteLBut));
	arServices.insert( CreateServiceFunction(TTB_MODIFYLBUTTON, ModifyLButton));
	arServices.insert( CreateServiceFunction(TTB_GETLBUTTON, GetLButton));

	LoadAllLButs();
	return 0;
}

int UnInitLBut()
{
	SaveAllLButs();	
	return 0;
}
