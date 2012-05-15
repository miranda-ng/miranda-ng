#ifndef __PASSPHRASES_H__
#define __PASSPHRASES_H__

// funktions prototypen
void initPassphrases(void);
void releasePassphrases(void);
void addPassphrase(const char *akeyuserid, const char *apassphrase);
char *getPassphrase(const char *akeyuserid);

#endif // __PASSPHRASES_H__
