#include "commonheaders.h"

LPCSTR szModuleName = MODULENAME;
LPCSTR szVersionStr = MODULENAME" DLL ("__VERSION_STRING")";
HINSTANCE g_hInst;

char temporarydirectory[fullfilenamesize];
char logfile[fullfilenamesize];
/*
char *txtbeginpgppublickeyblock="-----BEGIN PGP PUBLIC KEY BLOCK-----";
char *txtendpgppublickeyblock="-----END PGP PUBLIC KEY BLOCK-----";
*/
char *txtbeginpgpmessage="-----BEGIN PGP MESSAGE-----";
char *txtendpgpmessage="-----END PGP MESSAGE-----";


void __cdecl ErrorMessage(const char *alevel, const char *atext, const char *ahint)
{
  char buffer[errormessagesize];

  strcpy(buffer, atext);
  strcat(buffer, " ");
  strcat(buffer, ahint);
  MessageBox(NULL, buffer, alevel, MB_OK);
}


void __cdecl LogMessage(const char *astart, const char *atext, const char *aend)
{
  FILE *log;

  if(logfile[0]=='\0') return;

  log=fopen(logfile, "a");
  if(log!=NULL)
  {
	fputs(astart, log);
	fputs(atext, log);
	fputs(aend, log);
    fclose(log);
  }
}
