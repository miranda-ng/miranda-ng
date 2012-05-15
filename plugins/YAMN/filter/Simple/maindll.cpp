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

YAMN_FILTERIMPORTFCN FilterFunctions=	//we set for YAMN which is our filter function
{
	FilterMail,
	UnLoadFilter,				//No unloading
};

YAMN_FILTERREGISTRATION FilterRegistration=		//classical YAMN registration
{
	"Simple filter plugin for YAMN",
	__DATE__,
	"© porter+ majvan",
	"Classifies mails using the blacklist emails stored in file",
	"porterbox@hotmail.com",
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
	lstrcat(FilterPath,"\\simplefilterdeny.txt");

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
	if(FilterPath!=NULL)
		delete[] FilterPath;
	FilterPath=NULL;

	return 0;
}

extern "C" int __declspec(dllexport) UninstallFilter()
{
	if(FilterPath==NULL)
		MessageBox(NULL,"Cannot delete blacklist file when Simple Filter is not loaded. Please do it manually.","Simple Filter uninstalling",MB_OK|MB_ICONWARNING);
	else
		DeleteFile(FilterPath);
	return 0;
}


//And this is main filter function.
DWORD WINAPI FilterMail(HACCOUNT Account,DWORD AccountVer,HYAMNMAIL Mail,DWORD MailVer)
{
	FILE *fp;
	char EmailSpam[256];
	unsigned char spamLevel;
	struct CMimeItem *Browser;

	if(MailVer!=YAMN_MAILVERSION)	//we test if we work with the right YAMNMAIL
		return 0;
	if(Mail->MailData==NULL)		//MailData should be available
		return 0;
	fp=fopen(FilterPath,"rt");
	if(fp != NULL) {
		if(!(Mail->Flags & YAMN_MSG_VIRTUAL))
			for(Browser=Mail->MailData->TranslatedHeader;Browser!=NULL;Browser=Browser->Next) {	//we browse all header stored in Mail->TranslatedHeader
				if((!lstrcmp(Browser->name,"Return-Path")) || (!lstrcmp(Browser->name,"From"))) {		//and if we find 
					fseek(fp, 0L, SEEK_SET);
					while(!feof(fp)) {				
						if(fscanf(fp, "%255s", EmailSpam) != 0) {
							if(!feof(fp))
								if(fscanf(fp, "%d", &spamLevel)==0)
									spamLevel=2;
							if(spamLevel>4)
								spamLevel=2;
							if(strstr(Browser->value,EmailSpam)!=NULL) {
								if((Mail->Flags & (YAMN_MSG_SPAMMASK==0)) && (spamLevel==0))
									Mail->Flags&=~(YAMN_MSG_SOUND | YAMN_MSG_APP | YAMN_MSG_POPUP | YAMN_MSG_SYSTRAY | YAMN_MSG_BROWSER);
								else if((Mail->Flags & YAMN_MSG_SPAMMASK) < spamLevel)			//if some filter plugin set higher level of spam, we do nothing
									Mail->Flags=(Mail->Flags & ~YAMN_MSG_SPAMMASK)+spamLevel;	//else we set spam level 2 (clearing spam bits and then settting them to level 2
							}
						}
					}
				}
		}
		fclose(fp);
	}
	return 1;
}
