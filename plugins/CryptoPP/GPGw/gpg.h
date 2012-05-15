#ifndef __GPG_H__
#define __GPG_H__

typedef enum
{
  gpgSuccess,
  gpgUnknownError,
  gpgWriteToFileFailed,
  gpgReadFromFileFailed,
  gpgExecuteFailed
}
gpgResult;

// konstanten fuer die initialisierung der parameter
extern char *txtgpgargslistpublickeys;
extern char *txtgpgargslistsecretkeys;
extern char *txtgpgargsimportpublickey;
extern char *txtgpgargsexportpublickey;
extern char *txtgpgargsdetectuserid;
extern char *txtgpgargsencrypt;
extern char *txtgpgargsdecrypt;

// sonstige konstanten
extern char *txtcrlf;
extern char *txtidseparator;

// zeichenketten
extern char gpgExecutable[];
extern char gpgHomeDirectory[];

// funktionsprototypen
gpgResult gpgListPublicKeys(char *aresult);
gpgResult gpgListSecretKeys(char *aresult);
gpgResult gpgImportPublicKey(const char *akey);
gpgResult gpgExportPublicKey(char *aresult, const char *auserid);
gpgResult gpgDetectUserID(char *aresult, const char *aciphertext);
gpgResult gpgEncrypt(char *aresult, const char *auserid, const char *aplaintext);
gpgResult gpgDecrypt(char *aresult, const char *aciphertext, const char *apassphrase);

#endif // __GPG_H__
