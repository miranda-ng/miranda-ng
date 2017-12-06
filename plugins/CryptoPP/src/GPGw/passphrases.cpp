#include "../commonheaders.h"
#include "gpgw.h"

// globale variablen
struct passphrase_t *passphrases;
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
    memset(passphrases[i].passphrase, 0, sizeof(passphrases[i].passphrase));

  free(passphrases);
}


void addPassphrase(const char *akeyuserid, const char *apassphrase)
{
  passphrasecount++;
  passphrases = (passphrase_t*)realloc(passphrases, sizeof(struct passphrase_t)*passphrasecount);

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

