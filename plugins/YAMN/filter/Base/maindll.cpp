//---------------------------------------------------------------------------
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <newpluginapi.h>
#include <m_system.h>
#include <m_langpack.h>
#include <m_options.h>
#include "../../m_filterplugin.h"
#if !defined(_WIN64)
	#include "aggressiveoptimize.h"
#endif

typedef INT_PTR(* MIRANDASERVICE)(WPARAM,LPARAM);

DWORD WINAPI FilterMail(HACCOUNT Account,DWORD AccountVer,HYAMNMAIL Mail,DWORD MailVer);//Function marks mail as spam when it is spam...
DWORD WINAPI UnLoadFilter(LPVOID);

int LoadRules();	//Load rules from file
int findsubstr(char *original,char *pattern); //finds if original contains substring

YAMN_FILTERIMPORTFCN FilterFunctions=	//we set for YAMN which is our filter function
{
	FilterMail,
	UnLoadFilter,
};

struct cFilterTable
{
	char account[256];
	char name[256];
	char value[256];
	unsigned char sl;
} *ft=NULL;
int fts=0;

YAMN_FILTERREGISTRATION FilterRegistration=		//classical YAMN registration
{
#ifdef DEBUG_FILTER
	"Base filter plugin for YAMN (debug)",
#else
	"Base filter plugin for YAMN",
#endif
	__DATE__,
	"© majvan",
	"Classifies mails using the rules stored in file",
	"om3tn@psg.sk",
	"http://www.majvan.host.sk/Projekty/YAMN?fm=soft",
};

char *FilterPath=NULL;

struct YAMNExportedFcn
{
	YAMN_SETFILTERPLUGINFCNIMPORTFCN	SetFilterPluginFcnImportFcn;
	MIRANDASERVICE RegisterFilterPlugin;
} YAMNFcn,*pYAMNFcn;			//exported functions from YAMN we will use

HYAMNFILTERPLUGIN POPFilePlugin;	//handle of this plugin for YAMN
HINSTANCE hInst;		//handle of this DLL for Windows

#ifdef DEBUG_FILTER
extern void InitDebug();
extern void UnInitDebug();
extern void DebugLog(HANDLE File,const char *fmt,...);
extern HANDLE FilterFile;
#endif

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fwdreason, LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return true;
}

extern "C" int __declspec(dllexport) LoadFilter(MIRANDASERVICE GetYAMNFcnPtr)
{
	FilterPath=new char[MAX_PATH];
	char *delim;
	pYAMNFcn=&YAMNFcn;

	GetModuleFileName(GetModuleHandle(NULL),FilterPath,MAX_PATH);
	if(NULL!=(delim=strrchr(FilterPath,'\\')))
		*delim=0;
	lstrcat(FilterPath,"\\basefilterdeny.txt");
#ifdef DEBUG_FILTER
	InitDebug();
#endif

	if(!LoadRules())
		return 0;

	pYAMNFcn->RegisterFilterPlugin=(MIRANDASERVICE)GetYAMNFcnPtr((WPARAM)MS_YAMN_REGISTERFILTERPLUGIN,(LPARAM)0);
	pYAMNFcn->SetFilterPluginFcnImportFcn=(YAMN_SETFILTERPLUGINFCNIMPORTFCN)GetYAMNFcnPtr((WPARAM)YAMN_SETFILTERPLUGINFCNIMPORTID,(LPARAM)0);
//Register our filter plugin to YAMN
	if(NULL==(POPFilePlugin=(HYAMNFILTERPLUGIN)pYAMNFcn->RegisterFilterPlugin((WPARAM)&FilterRegistration,(LPARAM)YAMN_FILTERREGISTRATIONVERSION)))
		return 0;
//And add our imported functions for YAMN
	if(!pYAMNFcn->SetFilterPluginFcnImportFcn(POPFilePlugin,0xb0000000,&FilterFunctions,YAMN_FILTERIMPORTFCNVERSION))
		return 0;
	return 1;		//Load luccess
}

DWORD WINAPI UnLoadFilter(LPVOID)
{
#ifdef DEBUG_FILTER
	UnInitDebug();
#endif
	if(FilterPath!=NULL)
		delete[] FilterPath;
	FilterPath=NULL;

	return 0;
}

extern "C" int __declspec(dllexport) UninstallFilter()
{
	if(FilterPath==NULL)
		MessageBox(NULL,"Cannot delete blacklist file when Base Filter is not loaded. Please do it manually.","Base Filter uninstalling",MB_OK|MB_ICONWARNING);
	else
		DeleteFile(FilterPath);
	return 0;
}


//And this is main filter function.
DWORD WINAPI FilterMail(HACCOUNT Account,DWORD AccountVer,HYAMNMAIL Mail,DWORD MailVer)
{
	struct CMimeItem *Browser;

	if(MailVer!=YAMN_MAILVERSION)	//we test if we work with the right YAMNMAIL
		return 0;
	if(Mail->MailData==NULL)		//MailData should be available
		return 0;

#ifdef DEBUG_FILTER
	DebugLog(FilterFile,"<New mail>\n");
#endif
	if(!(Mail->Flags & YAMN_MSG_VIRTUAL))
		for(Browser=Mail->MailData->TranslatedHeader;Browser!=NULL;Browser=Browser->Next)	//we browse all header stored in Mail->TranslatedHeader
		{
#ifdef DEBUG_FILTER
			DebugLog(FilterFile,"<Testing header item %s: %s>\n",Browser->name,Browser->value);
#endif
			for(int i=0;i<fts;i++)
				if(!lstrcmpi(Browser->name,ft[i].name))
				{
#ifdef DEBUG_FILTER
					DebugLog(FilterFile,"\t\t<Found appropriate selector %s>\n",Browser->name);
#endif
					if(findsubstr(Browser->value,ft[i].value))		//and if we find
					{
						if((ft[i].sl==0) && ((Mail->Flags & YAMN_MSG_SPAMMASK)==0))
						{
							Mail->Flags&=~(YAMN_MSG_POPUP | YAMN_MSG_SYSTRAY | YAMN_MSG_BROWSER | YAMN_MSG_SOUND | YAMN_MSG_APP | YAMN_MSG_NEVENT);
#ifdef DEBUG_FILTER
							DebugLog(FilterFile,"\t\tSetting individual flags not to notify mail, but does not consider as spam.");
#endif
						}
						else if((Mail->Flags & YAMN_MSG_SPAMMASK) < ft[i].sl)			//if some filter plugin set higher level of spam, we do nothing
						{
							Mail->Flags=(Mail->Flags & ~YAMN_MSG_SPAMMASK)+ft[i].sl;	//else we set spam level 2 (clearing spam bits and then settting them to level 2
#ifdef DEBUG_FILTER
							DebugLog(FilterFile,"\t\tMail marked to be spam #%d\n",Mail->Flags & YAMN_MSG_SPAMMASK);
#endif
						}
					}
#ifdef DEBUG_FILTER
					DebugLog(FilterFile,"\t\t</Found appropriate selector>\n");
#endif
				}
#ifdef DEBUG_FILTER
			DebugLog(FilterFile,"</Testing header>\n");
#endif
		}
#ifdef DEBUG_FILTER
	DebugLog(FilterFile,"</New mail>\n\n");
#endif
	return 1;
}

int LoadRules()
{
	char *account=NULL;
	char name[256];
	char value[256];
	char BadCompiler[512+5];
	unsigned char sl;
	FILE *fp;

#ifdef DEBUG_FILTER
	DebugLog(FilterFile,"<Loading rules from file %s>\n",FilterPath);
#endif

	fp=fopen(FilterPath,"rt");
	if(fp==NULL)
		return 0;

	while(!feof(fp))
	{
		if(fscanf(fp,"%255s",name) && !feof(fp) && (name[0]!=0))
		{
			if(fscanf(fp,"%255s",value) && !feof(fp) && (value[0]!=0))
			{
				if(fscanf(fp,"%d",&sl))
				{
					fts++;
					ft=(struct cFilterTable *)realloc((void *)ft,sizeof(cFilterTable)*fts);
					lstrcpy(ft[fts-1].name,name);
					lstrcpy(ft[fts-1].value,value);
					ft[fts-1].sl=sl;

					sprintf(BadCompiler,"%s %s %d",name,value,sl);
#ifdef DEBUG_FILTER
					DebugLog(FilterFile,"\t<Rule><selector>%s</selector><value>%s</value><spamlevel>%d</spamlevel>\n",name,value,sl);
#endif
				}
			}
		}
	}

	fclose(fp);
#ifdef DEBUG_FILTER
	DebugLog(FilterFile,"</Loading rules>\n");
#endif
	return 1;
}

int findsubstr(char *original,char *pattern)
{
	int ol=lstrlen(original);
	int pl=lstrlen(pattern);

	for(int i=0;(i+pl)<=ol;i++)
		if(!_strnicmp(original+i,pattern,pl))
			return 1;
	return 0;
}