#include "commonheaders.h"

// escape sequence type
struct escapesequence
{
  char *sequence;
  char *code;
};

// supported escape sequences
struct escapesequence escsequence[]=
{
  {"\\n", "\n"},
  {"\\r", "\r"},
  {"\\t", "\t"}
};

// number of supported escape sequences
int escsequencecount=sizeof(escsequence)/sizeof(struct escapesequence);


void replace(char *atext, const char *apattern, const char *areplacement)
{
  char *pos, *last;
  long textsize;
  long patternsize;
  long replacementsize;

  pos=atext;
  patternsize=strlen(apattern);
  replacementsize=strlen(areplacement);

  do
  {
    // textstelle suchen
    last=pos;
    pos=strstr(pos, apattern);
    // etwas gefunden?
    if(pos==NULL) break;

    // laenge des textes ermitteln
    textsize=strlen(last);
    // platz schaffen
    memmove(pos+replacementsize, pos+patternsize, textsize-((pos-last)+patternsize)+1);
    // ersetzen
    strncpy(pos, areplacement, replacementsize);
    pos+=replacementsize;
  }
  while(pos!=NULL);
}


char *getNextPart(char *aresult, char *atext, const char *aseparator)
{
  char *pos;

  strcpy(aresult, "");
  pos=strstr(atext, aseparator);

  if(pos!=NULL)
  {
    strncat(aresult, atext, pos-atext);
    pos+=strlen(aseparator);
  }
  else strcpy(aresult, atext);

  return pos;
}


void getLastPart(char *aresult, char *atext)
{
	strcpy(aresult, atext);
}


void appendText(char **abuffer, const char *atext, int atextsize)
{
  int size;

  if(*abuffer==NULL) size=0;
  else size=strlen(*abuffer);
  size++; // abschliessende 0
  if(atextsize==0) atextsize=strlen(atext);
  size+=atextsize;

  *abuffer=(char *)realloc(*abuffer, size);
  strncat(*abuffer, atext, atextsize);
}


BOOL existsFile(const char *afilename)
{
  int attr;

  if(strlen(afilename)==0) return FALSE;

  attr = GetFileAttributes(afilename);

  return ( (attr!=-1) && ((attr&FILE_ATTRIBUTE_DIRECTORY)==0));
}


BOOL existsPath(const char *apathname)
{
  int attr;

  if(strlen(apathname)==0) return FALSE;

  attr = GetFileAttributes(apathname);

  return ( (attr!=-1) && (attr&FILE_ATTRIBUTE_DIRECTORY) );
}


BOOL writeToFile(char *afilename, const char *atext)
{
  FILE *handle;
  size_t written;
  size_t length;

  getTemporaryFileName(afilename);

  handle=fopen(afilename, "wb");

  if(handle==NULL)
  {
    LogMessage("--- ", "write to file", "\n");
	LogMessage("filename:\n", afilename, "\n");
    return FALSE;
  }

  length=strlen(atext);
  written=fwrite(atext, sizeof(char), length, handle);

  if(written!=length) return FALSE;

  fclose(handle);
  return TRUE;
}


BOOL readFromFile(char *aresult, const char *afilename)
{
  FILE *handle;
  size_t filesize;
  size_t read;

  handle=fopen(afilename, "rb");
  if(handle==NULL) 
  {
    LogMessage("--- ", "read from file", "\n");
	LogMessage("filename:\n", afilename, "\n");
    return FALSE;
  }

  fseek(handle, 0, SEEK_END);
  filesize=ftell(handle);
  fseek(handle, 0, SEEK_SET);

  read=fread(aresult, sizeof(char), filesize, handle);
  if(read!=filesize) return FALSE;

  fclose(handle);
  return TRUE;
}


void getTemporaryFileName(char *aresult)
{
  GetTempFileName(temporarydirectory,"gpg",0,aresult);
}


void quoteEscapeSequences(char *atext)
{
  int i;

  for(i=0; i<escsequencecount; i++)
    replace(atext, escsequence[i].code, escsequence[i].sequence);
}


void unquoteEscapeSequences(char *atext)
{
  int i;

  for(i=0; i<escsequencecount; i++)
    replace(atext, escsequence[i].sequence, escsequence[i].code);
}

