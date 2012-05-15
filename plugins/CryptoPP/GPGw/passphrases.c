#include "commonheaders.h"

// globale variablen
struct passphrase *passphrases;
int passphrasecount;


void initPassphrases(void)
{
  passphrasecount=0;
  passphrases=NULL;
}


void releasePassphrases(void)
{
  int i;

  for(i=0; i<passphrasecount; i++)
    ZeroMemory(passphrases[i].passphrase, sizeof(passphrases[i].passphrase));

  free(passphrases);
}


void addPassphrase(const char *akeyuserid, const char *apassphrase)
{
  passphrasecount++;
  passphrases=realloc(passphrases, sizeof(struct passphrase)*passphrasecount);

  strcpy(passphrases[passphrasecount-1].keyuserid, akeyuserid);
  strcpy(passphrases[passphrasecount-1].passphrase, apassphrase);
}


char *getPassphrase(const char *akeyuserid)
{
  int i;
  char *result;

  result=NULL;
  for(i=0; i<passphrasecount; i++)
  {
    if(strcmp(passphrases[i].keyuserid, akeyuserid)==0)
      result=passphrases[i].passphrase;
  }

  return result;
}

