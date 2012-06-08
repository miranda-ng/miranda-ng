
#include "common.h"
#pragma hdrstop

extern HINSTANCE hInst;
#define MAXSEPS 32
int Separators[MAXSEPS];

static int SeparatorCnt=0;

int InsertSeparator(int id)
{
	TTBButton ttb;
	char buf[255];

	HBITMAP Separator=LoadBitmap(hInst,MAKEINTRESOURCE(IDB_SEP));
	//itoa(SeparatorCnt++,buf,10);
	
	wsprintf(buf,"%s %d",Translate("Separator"),id);
	memset(&ttb,0,sizeof(ttb));
			ttb.cbSize=sizeof(ttb);
			ttb.hbBitmapDown=Separator;
			ttb.hbBitmapUp=Separator;
			ttb.dwFlags=TTBBF_VISIBLE|TTBBF_ISSEPARATOR;
			ttb.pszServiceDown="";
			ttb.pszServiceUp="";
			ttb.lParamDown=id;
			ttb.name=buf;
			SeparatorCnt++;
 			return(TTBAddButton(&ttb,0));
};

INT_PTR DeleteSeparator(WPARAM id,LPARAM lParam)
{
	if ((id<0)||(id>=MAXSEPS))
	{
		MessageBoxA(0,"Wrong id","Error",0);
		return(0);
	};

	if (Separators[id]!=0)
	{
		TTBRemoveButton(Separators[id],0);
		Separators[id]=0;
		SeparatorCnt--;
	};
	SaveAllSeparators();
	return (1);
};
int LoadAllSeparators()
{
	char buf[255];
	char buf1[10];
	int id,i;
	
	//must be locked
	memset(buf,0,sizeof(buf));	

	memset(Separators,0,sizeof(Separators));
	for (i=0;i<MAXSEPS;i++)
	{
	memset(buf1,0,sizeof(buf1));
		_itoa(i,buf1,10);
		id=DBGetContactSettingWord(0,TTB_OPTDIR,AS(buf,"Sep",buf1),-1);
		if (id==i)
		{
		Separators[i]=InsertSeparator(i);
		};
	};
	return 0;
};
int SaveAllSeparators()
{
	char buf[255];
	char buf1[10];
	int i;
	for (i=0;i<MAXSEPS;i++)
	{
	memset(buf1,0,sizeof(buf1));
		_itoa(i,buf1,10);
		if (Separators[i]!=0)
		{
		DBWriteContactSettingWord(0,TTB_OPTDIR,AS(buf,"Sep",buf1),i);
		}
		else
		{
			DBWriteContactSettingWord(0,TTB_OPTDIR,AS(buf,"Sep",buf1),-1);
			DBDeleteContactSetting(0,TTB_OPTDIR,AS(buf,"Sep",buf1));
		};
	};
	return 0;
};
//return 0 on success,-1 on error
INT_PTR InsertNewFreeSeparator(WPARAM wParam,LPARAM lParam)
{
	if (SeparatorCnt<MAXSEPS)
	{
		int i;
		for (i=0;i<MAXSEPS;i++)
		{
			if (Separators[i]==0)
			{
				Separators[i]=InsertSeparator(i);
				SaveAllSeparators();
				return(0);
			};
		};
	};
	
	return(-1);
};
