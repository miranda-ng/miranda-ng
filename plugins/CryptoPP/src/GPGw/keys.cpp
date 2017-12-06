#include "../commonheaders.h"
#include "gpgw.h"

typedef char tkeyuserid[keyuseridsize];

tkeyuserid *keyuserids[2];
int keyuseridcount[2];


void initKeyUserIDs(const int atype)
{
  keyuseridcount[atype]=0;
  keyuserids[atype]=NULL;
}


void updateKeyUserIDs(const int atype)
{
  char *pos;
  gpgResult gpgresult;
  char buffer[largebuffersize];
  char keyuserid[keyuseridsize];

  releaseKeyUserIDs(atype);
  initKeyUserIDs(atype);

  memset(buffer, 0, sizeof(buffer));
  if(atype==publickeyuserid) gpgresult=gpgListPublicKeys(buffer);
  else gpgresult=gpgListSecretKeys(buffer);

  if(gpgresult!=gpgSuccess)
  {
//    if(atype==publickeyuserid) ErrorMessage(txterror, txtlistpublickeysfailed, txtverifyoptions);
//    else ErrorMessage(txterror, txtlistsecretkeysfailed, txtverifyoptions);
    return;
  }

  for(pos=buffer; pos!=NULL; )
  {
    pos=getNextPart(keyuserid, pos, txtcrlf);

    if(pos!=NULL)
    {
      keyuseridcount[atype]++;
      keyuserids[atype] = (tkeyuserid*)realloc(keyuserids[atype], sizeof(tkeyuserid)*keyuseridcount[atype]);
      strcpy(keyuserids[atype][keyuseridcount[atype]-1], keyuserid);
    }
  }
}


void releaseKeyUserIDs(const int atype)
{
	free(keyuserids[atype]);
}


char *getKeyUserID(const int atype, const int aindex)
{
	return keyuserids[atype][aindex];
}


int getKeyUserIDCount(const int atype)
{
	return keyuseridcount[atype];
}

