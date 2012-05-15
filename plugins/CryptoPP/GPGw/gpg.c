#include "commonheaders.h"
#include <stdio.h>

char *txtgpgargslistpublickeys=
  "--homedir \"%home%\""
  " --with-colon" // felder durch : voneinander abgetrennt
  " --list-public-keys"; // oeffentliche schluessel auflisten
char *txtgpgargslistsecretkeys=
  "--homedir \"%home%\""
  " --with-colon"
  " --list-secret-keys"; // geheime schluessel auflisten
char *txtgpgargsimportpublickey=
  "--homedir \"%home%\""
  " --import \"%keyfile%\""; // schluessel importieren
char *txtgpgargsexportpublickey=
  "--homedir \"%home%\""
  " --batch"
  " --yes" // abfragen mit ja beantworten
  " --armor" // ausgabe als text
  " --comment \"\"" // kommentar unterdruecken ("Comment: For info see http://www.gnupg.org")
  " --no-version" // ausgabe der version unterdruecken ("Version: GnuPG v1.0.6 (MingW32)")
  " --export \"%userid%\""; // export des schluessels %userid%
char *txtgpgargsdetectuserid=
  "--homedir \"%home%\""
  " --batch" // interaktion verhindern (daraus folgt fehler)
  " --decrypt \"%cipherfile%\""; // datei %cipherfile% entschluesseln
char *txtgpgargsencrypt=
  "--homedir \"%home%\""
  " --batch"
  " --yes" // abfragen mit ja beantworten
  " --armor" // textausgabe
  " --comment \"\"" // kein kommentar hinzufuegen
  " --no-version" // keine versions informationen hinzufuegen
  " --recipient \"%userid%\"" // %userid% des empfaengers
  " --output \"%cipherfile%\"" // ausgabe in datei %cipherfile%
  " --encrypt \"%plainfile%\""; // eingabe kommt aus %plainfile%
char *txtgpgargsdecrypt=
  "--homedir \"%home%\""
  " --yes" // abfragen mit ja beantworten
  " --passphrase-fd 0" // passphrase von stdin
  " --output \"%plainfile%\"" // ausgabe in datei %plainfile%
  " --decrypt \"%cipherfile%\""; // eingabe kommt aus %cipherfile%

// oeffentliche zeichenketten
char gpgExecutable[argumentsize];
char gpgHomeDirectory[argumentsize];


// zeichenketten fuer den internen gebrauch
char *txtpub="pub";
char *txtsec="sec";
char *txtcrlf="\r\n";
char *txtcolon=":";
char *txtquotationmark="\"";
char *txtgpgcolon="gpg:";
char *txtplainfile="%plainfile%";
char *txtcipherfile="%cipherfile%";
char *txtuserid="%userid%";
char *txtkeyfile="%keyfile%";
char *txthome="%home%";
char *txtidseparator=", ";


void assembleCommandLine(char *aresult, const char *aexecutable, const char *aargs)
{
  strcpy(aresult, aexecutable);
  strcat(aresult, " ");
  strcat(aresult, aargs);
}


void detectKeys(char *aresult, char *aoutput, const char *alabel)
{
  char line[linesize];
  char part[linesize];
  char *linepos;
  char *partpos;
  long i;

  strcpy(aresult, "");
  linepos=aoutput;

  do
  {
    linepos=getNextPart(line, linepos, txtcrlf);
    if(linepos==NULL) break;

    partpos=line;
    partpos=getNextPart(part, partpos, txtcolon);

    if(strcmp(part, alabel)==0)
      for(i=1; i<=10; i++)
      {
        partpos=getNextPart(part, partpos, txtcolon);

        switch(i)
        {
          case 4:
            strcat(aresult, part);
            strcat(aresult, txtidseparator);
          break;
          case 9:
            strcat(aresult, part);
            strcat(aresult, txtcrlf);
          break;
        }
      }
  }
  while(linepos!=NULL);

  replace(aresult, "\\x3a", ":");
}


gpgResult gpgListPublicKeys(char *aresult)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "list public keys", "\n");
  output=(char *)malloc(1);
  strcpy(output, "");
  assembleCommandLine(commandline, gpgExecutable, txtgpgargslistpublickeys);
  replace(commandline, txthome, gpgHomeDirectory);
  pxresult=pxExecute(commandline, "", &output, &exitcode);

  if((pxresult!=pxSuccess)||(exitcode!=0))
  {
    free(output);
    return gpgExecuteFailed;
  }

  detectKeys(aresult, output, txtpub);

  free(output);
  return gpgSuccess;
}


gpgResult gpgListSecretKeys(char *aresult)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "list secret keys", "\n");
  output=(char *)malloc(1);
  strcpy(output, "");
  assembleCommandLine(commandline, gpgExecutable, txtgpgargslistsecretkeys);
  replace(commandline, txthome, gpgHomeDirectory);
  pxresult=pxExecute(commandline, "", &output, &exitcode);

  if((pxresult!=pxSuccess)||(exitcode!=0))
  {
    free(output);
    return gpgExecuteFailed;
  }

  detectKeys(aresult, output, txtsec);

  free(output);
  return gpgSuccess;
}


gpgResult gpgImportPublicKey(const char *akey)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  char filename[fullfilenamesize];
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "import public key", "\n");
  if(! writeToFile(filename, akey)) return gpgWriteToFileFailed;

  output=(char *)malloc(1);
  strcpy(output, "");
  assembleCommandLine(commandline, gpgExecutable, txtgpgargsimportpublickey);
  replace(commandline, txtkeyfile, filename);
  replace(commandline, txthome, gpgHomeDirectory);
  pxresult=pxExecute(commandline, "", &output, &exitcode);
  remove(filename);
  free(output);

  if((pxresult!=pxSuccess)||(exitcode!=0))
    return gpgExecuteFailed;

  return gpgSuccess;
}


gpgResult gpgExportPublicKey(char *aresult, const char *auserid)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "export public key", "\n");
  output=(char *)malloc(1);
  strcpy(output, "");
  assembleCommandLine(commandline, gpgExecutable, txtgpgargsexportpublickey);
  replace(commandline, txtuserid, auserid);
  replace(commandline, txthome, gpgHomeDirectory);
  pxresult=pxExecute(commandline, "", &output, &exitcode);

  if((pxresult!=pxSuccess)||(exitcode!=0))
  {
    strcpy(aresult, "");
    free(output);
    return gpgExecuteFailed;
  }

  strcpy(aresult, output);

  free(output);
  return gpgSuccess;
}


gpgResult gpgDetectUserID(char *aresult, const char *aciphertext)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  char filename[fullfilenamesize];
  char line[linesize];
  char part[linesize];
  char *linepos;
  char *partpos;
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "detect user id", "\n");
  strcpy(aresult, "");

  if(! writeToFile(filename, aciphertext))
    return gpgWriteToFileFailed;

  output=(char *)malloc(1);
  strcpy(output, "");
  assembleCommandLine(commandline, gpgExecutable, txtgpgargsdetectuserid);
  replace(commandline, txtcipherfile, filename);
  replace(commandline, txthome, gpgHomeDirectory);
  pxresult=pxExecute(commandline, "", &output, &exitcode);
  remove(filename);

  if((pxresult!=pxSuccess)&&(pxresult!=pxSuccessExitCodeInvalid))
  {
    free(output);
    return gpgExecuteFailed;
  }

  linepos=output;

  do
  {
    linepos=getNextPart(line, linepos, txtcrlf);
    if(strncmp(line, txtgpgcolon, strlen(txtgpgcolon))!=0 && strstr(line, txtgpgcolon)==0)
    {
      partpos=line;
      partpos=getNextPart(part, partpos, txtquotationmark);
      getNextPart(part, partpos, txtquotationmark);
      linepos=NULL;
    }
  }
  while(linepos!=NULL);
  strcpy(aresult, part);

  free(output);
  return gpgSuccess;
}


gpgResult gpgEncrypt(char *aresult, const char *auserid, const char *aplaintext)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  char plainfile[fullfilenamesize];
  char cipherfile[fullfilenamesize];
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "encrypt", "\n");
  strcpy(aresult, "");

  if(! writeToFile(plainfile, aplaintext))
    return gpgWriteToFileFailed;

  output=(char *)malloc(1);
  strcpy(output, "");
  getTemporaryFileName(cipherfile);
  assembleCommandLine(commandline, gpgExecutable, txtgpgargsencrypt);
  replace(commandline, txtcipherfile, cipherfile);
  replace(commandline, txtplainfile, plainfile);
  replace(commandline, txtuserid, auserid);
  replace(commandline, txthome, gpgHomeDirectory);
  pxresult=pxExecute(commandline, "", &output, &exitcode);
  remove(plainfile);
  free(output);

  if((pxresult!=pxSuccess)||(exitcode!=0))
  {
	remove(cipherfile);
    return gpgExecuteFailed;
  }

  if(! readFromFile(aresult, cipherfile))
  {
	remove(cipherfile);
    return gpgReadFromFileFailed;
  }

  remove(cipherfile);
  return gpgSuccess;
}


gpgResult gpgDecrypt(char *aresult, const char *aciphertext, const char *apassphrase)
{
  pxResult pxresult;
  char commandline[commandlinesize];
  char plainfile[fullfilenamesize];
  char cipherfile[fullfilenamesize];
  char passphrase[linesize];
  DWORD exitcode;
  char *output;

  LogMessage(">>> ", "decrypt", "\n");
  strcpy(aresult, "");

  if(! writeToFile(cipherfile, aciphertext))
    return gpgWriteToFileFailed;

  output=(char *)malloc(1);
  strcpy(output, "");
  getTemporaryFileName(plainfile);
  assembleCommandLine(commandline, gpgExecutable, txtgpgargsdecrypt);
  replace(commandline, txtcipherfile, cipherfile);
  replace(commandline, txtplainfile, plainfile);
  replace(commandline, txthome, gpgHomeDirectory);
  strcpy(passphrase, apassphrase);
  strcat(passphrase, txtcrlf);
  pxresult=pxExecute(commandline, passphrase, &output, &exitcode);
  remove(cipherfile);
  free(output);

  if((pxresult!=pxSuccess)||(exitcode!=0))
  {
	remove(plainfile);
    return gpgExecuteFailed;
  }

  if(! readFromFile(aresult, plainfile))
  {
	remove(plainfile);
    return gpgReadFromFileFailed;
  }

  remove(plainfile);
  return gpgSuccess;
}

